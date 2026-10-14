#include <Arduino.h>

// ================= ขาเชื่อมต่อ (PINS) =================
#define RADAR_RX_PIN 18     // ต่อกับ TX ของ LD6002C
#define RADAR_TX_PIN 17     // ต่อกับ RX ของ LD6002C
#define LD2410_OUT_PIN 16   // ต่อกับขา OUT ของ LD2410C (สมมติเป็นขา 19 ปรับเปลี่ยนได้)
#define LED_PIN 48          // ขา RGB LED บนบอร์ด ESP32-S3 (WS2812)

HardwareSerial RadarSerial(1);

// ================= ตัวแปรสถานะ (STATE VARIABLES) =================
bool is_occupied = false;
bool is_fall = false;
unsigned long last_presence_time = 0;
unsigned long last_ld2410_log_time = 0;
const unsigned long fall_empty_room_timeout = 10000; // Failsafe: 10 วินาที ถ้ายืนยันว่าไม่มีคนให้ยกเลิกการล้ม

// ฟังก์ชันคำนวณ Checksum ของ LD6002C
uint8_t calculateChecksum(const uint8_t* data, size_t len) {
  uint8_t ret = 0;
  for (size_t i = 0; i < len; ++i) ret ^= data[i];
  return (uint8_t)(~ret);
}

// ฟังก์ชันสำหรับปรับระยะ "เกณฑ์การล้ม" ของ LD6002C
void setFallThreshold(float threshold_m) {
  uint8_t cmd[13];
  cmd[0] = 0x01; cmd[1] = 0x00; cmd[2] = 0x00; 
  cmd[3] = 0x00; cmd[4] = 0x04; cmd[5] = 0x0E; cmd[6] = 0x08; 
  cmd[7] = calculateChecksum(cmd, 7); 
  memcpy(&cmd[8], &threshold_m, 4);   
  cmd[12] = calculateChecksum(&cmd[8], 4); 
  
  RadarSerial.write(cmd, 13);
  Serial.printf("⚙️ [LD6002C] ส่งคำสั่งปรับระยะเกณฑ์การล้มเป็น %.2f เมตร\n", threshold_m);
}

void setup() {
  Serial.begin(115200);
  neopixelWrite(LED_PIN, 0, 0, 0); // ปิดไฟ
  
  // ตั้งค่า LD2410C
  pinMode(LD2410_OUT_PIN, INPUT_PULLDOWN);
  
  // ตั้งค่า LD6002C
  RadarSerial.setRxBufferSize(1024);
  RadarSerial.begin(115200, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);
  
  delay(2000); // รอเรดาร์บูต
  
  Serial.println("\n=================================");
  Serial.println(" 📡 Sensor Fusion: LD6002C + LD2410C ");
  Serial.println("=================================\n");
  
  setFallThreshold(1.5); // ตั้งเกณฑ์การล้ม (สำหรับทดสอบแนวนอน)
  Serial.println("🟢 ระบบพร้อมใช้งาน... กำลังเฝ้าระวัง!");
}

void loop() {
  unsigned long now = millis();

  // ==========================================
  // 1. อ่านค่าจาก LD2410C (ตรวจจับคนอยู่)
  // ==========================================
  int presence = digitalRead(LD2410_OUT_PIN);
  bool current_occupied = (presence == HIGH);
  
  // โชว์สถานะทุกๆ 3 วินาที
  if (now - last_ld2410_log_time >= 3000) {
    last_ld2410_log_time = now;
    Serial.print("📡 [LD2410C] สถานะ: ");
    Serial.println(current_occupied ? "👤 OCCUPIED (มีคนอยู่)" : "👻 EMPTY (ห้องว่าง)");
  }

  if (current_occupied) {
    last_presence_time = now;
    if (!is_occupied) {
      is_occupied = true;
      if (!is_fall) neopixelWrite(LED_PIN, 0, 20, 0); // เข้ามาใหม่ = ไฟเขียว
    }
  } else {
    if (is_occupied && (now - last_presence_time > 2000)) {
      is_occupied = false; // ห้องว่าง
      if (!is_fall) neopixelWrite(LED_PIN, 0, 0, 20); // ห้องว่าง = ไฟน้ำเงิน
    }
    
    // Failsafe: หากอยู่ในสถานะคนล้ม แต่ LD2410C ยืนยันว่าไม่มีคนอยู่เกิน 10 วินาที ให้รีเซ็ตการล้ม
    if (is_fall && (now - last_presence_time > fall_empty_room_timeout)) {
      Serial.println("\nℹ️ [FAILSAFE] ไม่มีคนอยู่เกิน 10 วินาที... ยกเลิกสถานะการล้มอัตโนมัติ!");
      is_fall = false;
      neopixelWrite(LED_PIN, 0, 0, 20); // กลับเป็นไฟน้ำเงิน
    }
  }

  // ==========================================
  // 2. อ่านค่าจาก LD6002C (ตรวจจับการล้มผ่าน UART)
  // ==========================================
  if (RadarSerial.available() > 0) {
    if (RadarSerial.peek() != 0x01) { RadarSerial.read(); return; }
    if (RadarSerial.available() < 8) return;
    
    uint8_t header[8];
    RadarSerial.readBytes(header, 8);
    if (calculateChecksum(header, 7) != header[7]) return; 
    
    uint16_t dataLen = (header[3] << 8) | header[4];
    uint16_t msgType = (header[5] << 8) | header[6];
    
    if (dataLen > 256) {
      uint32_t flush_timer = millis();
      while (RadarSerial.available() && millis() - flush_timer < 100) RadarSerial.read(); 
      return;
    }
    
    uint32_t timeout_timer = millis();
    while (RadarSerial.available() < (dataLen + 1)) {
      if (millis() - timeout_timer > 100) return; 
    }
    
    uint8_t payload[256]; 
    if (dataLen > 0) RadarSerial.readBytes(payload, dataLen);
    uint8_t dataCksum = RadarSerial.read(); 
    
    if (dataLen > 0 && calculateChecksum(payload, dataLen) != dataCksum) return; 
    
    // ตรวจจับแพ็กเกจ "การล้ม" (0x0E02)
    if (msgType == 0x0E02 && dataLen == 1) {
      uint8_t fall_state = payload[0]; 
      
      // อัปเดตสถานะการล้ม (ถ้าไม่มีการเคลียร์จาก Failsafe ซะก่อน)
      if (fall_state == 1 && !is_fall) {
        is_fall = true;
        neopixelWrite(LED_PIN, 255, 0, 0); // 🚨 คนล้ม = ไฟแดงสว่าง
        Serial.println("--------------------------------------------------");
        Serial.println("🚨 [LD6002C] ตรวจพบการล้ม! (ระยะ < 1.5m)");
      } 
      else if (fall_state == 0 && is_fall) {
        is_fall = false;
        // ลุกขึ้นแล้ว กลับไปโชว์ไฟตามสถานะ LD2410C
        if (is_occupied) neopixelWrite(LED_PIN, 0, 20, 0);
        else neopixelWrite(LED_PIN, 0, 0, 20);
        
        Serial.println("--------------------------------------------------");
        Serial.println("🟢 [LD6002C] สถานะปกติ (ลุกขึ้นแล้ว / ออกนอกระยะ)");
      }
    }
  }
}