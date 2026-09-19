/*
 * =========================================================================================
 * 📡 ชุดโปรแกรมทดลองที่ 1: การทดสอบและประเมินประสิทธิภาพเซนเซอร์ HLK-LD2410C (24 GHz mmWave)
 * =========================================================================================
 * โครงการ: ระบบตรวจจับการล้มอัจฉริยะ (Smart Fall Detection System)
 * วัตถุประสงค์การทดลอง:
 *  1. วัดระยะเวลาการตอบสนอง (Detection Latency) เมื่อมีบุคคลก้าวเข้าสู่พื้นที่ตรวจจับ
 *  2. วัดระยะเวลาหน่วงก่อนคืนสถานะห้องว่าง (Exit Timeout & Hold-time Delay)
 *  3. ทดสอบความไวในการตรวจจับการเคลื่อนไหวขนาดเล็ก (Micro-motion / การหายใจขณะนั่งนิ่ง)
 *  4. บันทึกสถิติเพื่อนำข้อมูลไปลงตารางผลการทดลองในบทที่ 4 (Chapter 4: Experimental Results)
 * 
 * การต่อสายฮาร์ดแวร์ (ESP32-S3):
 *  - ขา OUT ของ HLK-LD2410C -> GPIO 16 (Digital Input)
 *  - ไฟ RGB บนบอร์ด ESP32-S3 -> GPIO 48
 *  - บัซเซอร์/ไซเรน (Buzzer) -> GPIO 15 (Active LOW Relay / Buzzer)
 *  - ขา VCC (5V), GND ร่วมกัน
 * =========================================================================================
 */

#include <Arduino.h>

// ================= HARDWARE PIN DEFINITIONS =================
#define LD2410_OUT_PIN     16   // ขา OUT ของ HLK-LD2410C
#define BUZZER_PIN         15   // ขาควบคุม Buzzer / Relay (Active LOW)
#define RGB_LED_PIN        48   // ไฟสถานะ RGB บนบอร์ด ESP32-S3
#define RGB_BRIGHTNESS     0.25 // ความสว่างไฟ RGB (0.0 - 1.0)

// ================= EXPERIMENT PARAMETERS =================
bool active_level_high = true;            // true = สัญญาณ HIGH (3.3V) คือมีคนอยู่ (Occupied)
unsigned long presence_timeout = 5000;    // เวลาหน่วง (ms) ก่อนยืนยันว่าห้องว่าง (ปรับเปลี่ยนได้ทาง Serial)
bool enable_buzzer_beep = true;           // ส่งเสียง Beep สั้นๆ เมื่อสถานะเปลี่ยนเพื่อช่วยนับการทดลอง

// ================= STATISTICAL METRICS =================
struct ExperimentStats {
  uint32_t total_trials = 0;              // จำนวนครั้งที่เดินเข้า-ออก
  uint32_t false_trigger_count = 0;       // จำนวนครั้งที่ตรวจจับติดๆ ดับๆ ชั่วคราว (< 1 วินาที)
  unsigned long last_enter_timestamp = 0; // บันทึกเวลาที่คนก้าวเข้า
  unsigned long last_exit_timestamp = 0;  // บันทึกเวลาที่คนเดินออก
  unsigned long total_occupied_time_ms = 0; // เวลารวมทั้งหมดที่มีคนอยู่
  unsigned long min_occupied_time_ms = 99999999;
  unsigned long max_occupied_time_ms = 0;
};

ExperimentStats stats;

// สถานะปัจจุบัน: "EMPTY" หรือ "OCCUPIED"
enum RoomState { ROOM_EMPTY, ROOM_OCCUPIED };
RoomState current_state = ROOM_EMPTY;

unsigned long last_signal_high_time = 0; // เวลาล่าสุดที่ขา OUT เป็น HIGH
unsigned long last_debug_print_time = 0;
bool is_counting_down = false;

// ฟังก์ชันควบคุมไฟ RGB LED
void setRGB(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(RGB_LED_PIN, (uint8_t)(r * RGB_BRIGHTNESS), (uint8_t)(g * RGB_BRIGHTNESS), (uint8_t)(b * RGB_BRIGHTNESS));
}

// ฟังก์ชันส่งเสียง Beep สั้นๆ ยืนยันการเปลี่ยนสถานะ
void beepNotification(int duration_ms, int count = 1) {
  if (!enable_buzzer_beep) return;
  for (int i = 0; i < count; i++) {
    digitalWrite(BUZZER_PIN, LOW); // เปิด (Active LOW)
    delay(duration_ms);
    digitalWrite(BUZZER_PIN, HIGH); // ปิด
    if (count > 1) delay(50);
  }
}

// พิมพ์เมนูคำสั่ง Serial
void printHelpMenu() {
  Serial.println(F("\n======================================================="));
  Serial.println(F("🎮 เมนูควบคุมการทดลอง (ส่งคำสั่งผ่าน Serial Monitor):"));
  Serial.println(F("  [s] -> แสดงตารางสถิติผลการทดลองปัจจุบัน (Summary Table)"));
  Serial.println(F("  [r] -> รีเซ็ตสถิติการทดลองทั้งหมด (Reset Stats)"));
  Serial.println(F("  [1] -> ตั้งค่า Presence Timeout เป็น 3 วินาที (3,000 ms)"));
  Serial.println(F("  [2] -> ตั้งค่า Presence Timeout เป็น 5 วินาที (5,000 ms) [ค่ามาตรฐาน]"));
  Serial.println(F("  [3] -> ตั้งค่า Presence Timeout เป็น 10 วินาที (10,000 ms)"));
  Serial.println(F("  [b] -> สลับเปิด/ปิด เสียง Buzzer เตือนสั้นๆ"));
  Serial.println(F("  [p] -> สลับขั้วสัญญาณ Active Level (HIGH / LOW)"));
  Serial.println(F("  [?] -> แสดงเมนูช่วยเหลือนี้อีกครั้ง"));
  Serial.println(F("=======================================================\n"));
}

// แสดงตารางสรุปผลการทดลอง
void printSummaryTable() {
  Serial.println(F("\n📊 ================== EXPERIMENT SUMMARY REPORT =================="));
  Serial.printf("  1. จำนวนรอบการทดลองทั้งหมด (Total Trials)    : %u รอบ\n", stats.total_trials);
  Serial.printf("  2. สัญญาณกระพริบรบกวนหลอก (False Chatter)   : %u ครั้ง\n", stats.false_trigger_count);
  Serial.printf("  3. ค่า Presence Timeout ที่ตั้งไว้          : %lu ms (%.1f วินาที)\n", presence_timeout, (float)presence_timeout / 1000.0);
  
  if (stats.total_trials > 0) {
    float avg_duration = (float)stats.total_occupied_time_ms / (float)stats.total_trials / 1000.0;
    Serial.printf("  4. เวลามีคนอยู่ในพื้นที่เฉลี่ย (Avg Duration) : %.2f วินาที\n", avg_duration);
    Serial.printf("  5. เวลาอยู่ในพื้นที่สั้นสุด (Min Duration)    : %.2f วินาที\n", (float)stats.min_occupied_time_ms / 1000.0);
    Serial.printf("  6. เวลาอยู่ในพื้นที่ยาวนานสุด (Max Duration)  : %.2f วินาที\n", (float)stats.max_occupied_time_ms / 1000.0);
  } else {
    Serial.println(F("  (ยังไม่มีรอบการทดลองที่เสร็จสิ้นสมบูรณ์)"));
  }
  
  Serial.printf("  7. ขั้วสัญญาณที่ใช้งาน (Active Polarity)   : %s\n", active_level_high ? "HIGH (3.3V)" : "LOW (0V)");
  Serial.printf("  8. สถานะปัจจุบันของห้อง (Current State)    : %s\n", current_state == ROOM_OCCUPIED ? "🟢 OCCUPIED (มีคน)" : "⚪ EMPTY (ห้องว่าง)");
  Serial.println(F("==================================================================\n"));
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  pinMode(LD2410_OUT_PIN, INPUT_PULLDOWN);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH); // ปิดรีเลย์/บัซเซอร์เริ่มต้น

  setRGB(0, 150, 0); // เริ่มต้นเป็นสีเขียว (ห้องว่าง)

  Serial.println(F("\n==================================================================="));
  Serial.println(F("   🚀 HLK-LD2410C (24 GHz mmWave) STANDALONE EXPERIMENT SKETCH     "));
  Serial.println(F("==================================================================="));
  Serial.printf("📍 Monitoring OUT Pin: GPIO %d (Active %s)\n", LD2410_OUT_PIN, active_level_high ? "HIGH" : "LOW");
  Serial.printf("⏱️ Default Presence Timeout: %lu ms\n", presence_timeout);
  
  printHelpMenu();
  Serial.println(F("🟢 ระบบพร้อมเริ่มบันทึกการทดลอง... เดินเข้าสู่พื้นที่เรดาร์ได้เลย!"));
}

void loop() {
  unsigned long now = millis();

  // อ่านค่าจากขา OUT ของเรดาร์ LD2410C
  int pin_val = digitalRead(LD2410_OUT_PIN);
  bool raw_presence = (active_level_high ? (pin_val == HIGH) : (pin_val == LOW));

  // -------------------------------------------------------------
  // กรณีตรวจพบว่ามีคนอยู่ในห้อง (Presence Detected)
  // -------------------------------------------------------------
  if (raw_presence) {
    last_signal_high_time = now;
    is_counting_down = false;

    if (current_state == ROOM_EMPTY) {
      current_state = ROOM_OCCUPIED;
      stats.last_enter_timestamp = now;
      stats.total_trials++;

      setRGB(0, 80, 255); // ไฟสีฟ้าคราม: ตรวจพบคน
      beepNotification(80, 1); // Beep 1 ครั้ง

      Serial.println(F("-------------------------------------------------------------------"));
      Serial.printf("🚶 [TRIAL #%u ENTER] ตรวจพบคนก้าวเข้าสู่พื้นที่! เวลา: %.3f s\n", 
                    stats.total_trials, (float)now / 1000.0);
      Serial.println(F("   👉 ไฟ RGB เปลี่ยนเป็นสีฟ้า (OCCUPIED) | กำลังจับเวลาระยะเวลาการอยู่ในห้อง..."));
      Serial.println(F("-------------------------------------------------------------------"));
    }
  } 
  // -------------------------------------------------------------
  // กรณีไม่พบสัญญาณการเคลื่อนไหว (No Presence Detected)
  // -------------------------------------------------------------
  else {
    if (current_state == ROOM_OCCUPIED) {
      unsigned long elapsed_since_last_presence = now - last_signal_high_time;

      // แสดงการนับถอยหลัง (ไฟสีเหลือง)
      if (elapsed_since_last_presence < presence_timeout) {
        if (!is_counting_down) {
          is_counting_down = true;
          setRGB(150, 150, 0); // ไฟสีเหลือง: กำลังหน่วงเวลาก่อนตัดเป็นห้องว่าง
        }
      } 
      // หมดเวลาหน่วง -> ยืนยันว่าคนเดินออกจากห้องน้ำแล้ว (Exit Confirmed)
      else {
        current_state = ROOM_EMPTY;
        stats.last_exit_timestamp = now;
        is_counting_down = false;

        unsigned long occupied_duration_ms = stats.last_exit_timestamp - stats.last_enter_timestamp - presence_timeout;
        if (occupied_duration_ms < 500) {
          stats.false_trigger_count++;
        }

        stats.total_occupied_time_ms += occupied_duration_ms;
        if (occupied_duration_ms < stats.min_occupied_time_ms) stats.min_occupied_time_ms = occupied_duration_ms;
        if (occupied_duration_ms > stats.max_occupied_time_ms) stats.max_occupied_time_ms = occupied_duration_ms;

        setRGB(0, 150, 0); // ไฟสีเขียว: ห้องว่าง
        beepNotification(60, 2); // Beep 2 ครั้ง

        Serial.println(F("-------------------------------------------------------------------"));
        Serial.printf("🚪 [TRIAL #%u EXIT] ยืนยันคนออกจากห้องน้ำแล้ว! เวลา: %.3f s\n", 
                      stats.total_trials, (float)now / 1000.0);
        Serial.printf("   ⏱️ เวลาอยู่ในพื้นที่สุทธิ (Net Occupied Time) : %.2f วินาที (%lu ms)\n", 
                      (float)occupied_duration_ms / 1000.0, occupied_duration_ms);
        Serial.printf("   ⏳ เวลาหน่วงก่อนคืนสถานะ (Hold Timeout)        : %.2f วินาที (%lu ms)\n", 
                      (float)presence_timeout / 1000.0, presence_timeout);
        Serial.println(F("   👉 ไฟ RGB เปลี่ยนกลับเป็นสีเขียว (EMPTY) พร้อมทดสอบรอบถัดไป"));
        Serial.println(F("-------------------------------------------------------------------\n"));
      }
    }
  }

  // ปริ้นต์ Log ติดตามทุกๆ 3 วินาที เพื่อดูสถานะสัญญาณดิบ (Raw Pin Voltage)
  if (now - last_debug_print_time >= 3000) {
    last_debug_print_time = now;
    Serial.printf("📡 [HEARTBEAT] Pin %d = %s | State: %s | Active Polarity: %s\n",
                  LD2410_OUT_PIN,
                  pin_val == HIGH ? "HIGH (3.3V)" : "LOW (0V)",
                  current_state == ROOM_OCCUPIED ? "OCCUPIED (มีคน)" : "EMPTY (ว่าง)",
                  active_level_high ? "HIGH" : "LOW");
  }

  // -------------------------------------------------------------
  // จัดการคำสั่ง Serial Input จากผู้ใช้งาน
  // -------------------------------------------------------------
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    while (Serial.available() > 0) Serial.read(); // ล้าง buffer

    switch (cmd) {
      case 's':
      case 'S':
        printSummaryTable();
        break;

      case 'r':
      case 'R':
        stats = ExperimentStats();
        Serial.println(F("🔄 [RESET] รีเซ็ตสถิติการทดลองเป็น 0 เรียบร้อย!"));
        break;

      case '1':
        presence_timeout = 3000;
        Serial.println(F("⚙️ [CONFIG] ตั้งค่า Presence Timeout = 3,000 ms (3 วินาที)"));
        break;

      case '2':
        presence_timeout = 5000;
        Serial.println(F("⚙️ [CONFIG] ตั้งค่า Presence Timeout = 5,000 ms (5 วินาที - มาตรฐาน)"));
        break;

      case '3':
        presence_timeout = 10000;
        Serial.println(F("⚙️ [CONFIG] ตั้งค่า Presence Timeout = 10,000 ms (10 วินาที)"));
        break;

      case 'b':
      case 'B':
        enable_buzzer_beep = !enable_buzzer_beep;
        Serial.printf("🔔 [CONFIG] เสียง Buzzer Beep: %s\n", enable_buzzer_beep ? "ENABLED (เปิด)" : "DISABLED (ปิด)");
        break;

      case 'p':
      case 'P':
        active_level_high = !active_level_high;
        Serial.printf("⚡ [CONFIG] สลับ Active Level เป็น: %s\n", active_level_high ? "Active HIGH (3.3V = มีคน)" : "Active LOW (0V = มีคน)");
        break;

      case '?':
      case 'h':
      case 'H':
        printHelpMenu();
        break;

      default:
        break;
    }
  }

  delay(20);
}
