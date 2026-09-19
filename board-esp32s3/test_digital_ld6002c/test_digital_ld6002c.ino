/*
 * =========================================================================================
 * 🧪 HLK-LD6002C DIGITAL FALL PIN TESTER (ทดสอบสัญญาณขาดิจิตอล TX2 ของเรดาร์)
 * =========================================================================================
 * 🌟 วัตถุประสงค์:
 *    - ทดสอบอ่านสัญญาณดิจิตอลจากขา TX2 ของเรดาร์ HLK-LD6002C โดยตรง
 *    - ขา TX2 เป็น Hardware Fall Detection Pin:
 *        🟢 สภาวะปกติ (ไม่ล้ม) -> Logic LOW  (0V)
 *        🔴 ตรวจพบคนล้ม      -> Logic HIGH (3.3V)
 *    - ทดสอบขาสัญญาณ Output ของ ESP32-S3:
 *        🔊 Buzzer (GPIO 15) ทำงานแบบ Active LOW
 *        💡 RGB LED (GPIO 48) บนบอร์ด
 *
 * การต่อสาย (ESP32-S3):
 *  - ESP32-S3 GPIO 3        -> ขา TX2 ของโมดูล HLK-LD6002C
 *  - ESP32-S3 GND           -> ขา GND ของเรดาร์ (Common GND สำคัญมาก!)
 *  - ESP32-S3 GPIO 15       -> ขา Buzzer (Active LOW)
 *  - ESP32-S3 GPIO 48       -> ไฟ RGB NeoPixel (ออนบอร์ด)
 *  - Serial Monitor: 115,200 baud
 * =========================================================================================
 */

#include <Arduino.h>

// ================= กำหนดขาใช้งาน =================
#define RADAR_DIGITAL_PIN  3    // ขา ESP32 ที่ต่อเข้ากับขา TX2 ของเรดาร์ LD6002C (ใช้ GPIO 3)
#define BUZZER_PIN         15   // ขา Buzzer (Active LOW: LOW = ดัง, HIGH = ดับ)
#define RGB_LED_PIN        48   // ไฟ RGB NeoPixel บนบอร์ด ESP32-S3
#define RGB_BRIGHTNESS     0.25 // ความสว่างไฟ RGB (25%)

// ฟังก์ชันควบคุมไฟ RGB บนบอร์ด
void setRGB(uint8_t r, uint8_t g, uint8_t b) {
    neopixelWrite(RGB_LED_PIN, 
                  (uint8_t)(r * RGB_BRIGHTNESS), 
                  (uint8_t)(g * RGB_BRIGHTNESS), 
                  (uint8_t)(b * RGB_BRIGHTNESS));
}

// ฟังก์ชันควบคุม Buzzer (Active LOW)
void setBuzzer(bool active) {
    digitalWrite(BUZZER_PIN, active ? LOW : HIGH);
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println(F("\n==============================================================="));
    Serial.println(F("  🧪 HLK-LD6002C DIGITAL FALL PIN TESTER (ทดสอบขาดิจิตอล TX2)   "));
    Serial.println(F("==============================================================="));

    // 1. ตั้งค่าขาอ่านสัญญาณดิจิตอลจากเรดาร์
    pinMode(RADAR_DIGITAL_PIN, INPUT);

    // 2. ตั้งค่าขา Buzzer
    pinMode(BUZZER_PIN, OUTPUT);
    setBuzzer(false); // ปิดเสียง Buzzer เริ่มต้น
    setRGB(0, 150, 0); // สีเขียว = พร้อมทดสอบ

    // 3. ทดสอบเสียง Buzzer สั้นๆ เพื่อยืนยันว่าขา Output และ Buzzer ทำงานได้
    Serial.println(F("🔊 ทดสอบส่งสัญญาณขับ Buzzer สั้นๆ (Self-Test 0.2s)..."));
    setBuzzer(true);
    delay(200);
    setBuzzer(false);
    Serial.println(F("✅ ขับ Buzzer สำเร็จ (ขา GPIO 15 ปกติ)"));

    Serial.println(F("---------------------------------------------------------------"));
    Serial.printf("👀 เริ่มต้นอ่านค่าจากขา GPIO 3 (ต่อกับ TX2 ของเรดาร์)...\n");
    Serial.println(F("   🟢 ปกติ (ไม่ล้ม) : LOW  (0V)   -> ไฟเขียว / เสียงเงียบ"));
    Serial.println(F("   🔴 ตรวจพบล้ม     : HIGH (3.3V) -> ไฟแดงกะพริบ / ไซเรนดัง"));
    Serial.println(F("===============================================================\n"));
}

void loop() {
    unsigned long now = millis();
    static int last_state = -1;
    static unsigned long last_periodic_print = 0;
    static unsigned long last_flash = 0;
    static bool flash_state = false;

    // อ่านค่าดิจิตอลจากขาเรดาร์ (0 หรือ 1)
    int current_state = digitalRead(RADAR_DIGITAL_PIN);

    // ตรวจจับเมื่อสถานะมีการเปลี่ยนแปลง (Edge Detection)
    if (current_state != last_state) {
        last_state = current_state;

        if (current_state == HIGH) {
            // ขาเรดาร์ส่งลอจิก HIGH (3.3V) -> เรดาร์รายงานว่าคนล้ม!
            setRGB(255, 0, 0); // สีแดง
            setBuzzer(true);   // เปิดไซเรน
            Serial.println(F("\n🚨🚨🚨 [DIGITAL PIN TRIGGERED] 🚨🚨🚨"));
            Serial.printf("👉 ขา GPIO 3 ได้รับลอจิก HIGH (3.3V) จากขา TX2 ของเรดาร์!\n");
            Serial.println(F("👉 สถานะ: **ตรวจพบการล้ม (Fall Detected)**"));
            Serial.println(F("---------------------------------------------------------------"));
        } else {
            // ขาเรดาร์ส่งลอจิก LOW (0V) -> สภาวะปกติ
            setRGB(0, 150, 0);  // คืนไฟเขียว
            setBuzzer(false);   // ปิดไซเรน
            Serial.println(F("\n🟢 [DIGITAL PIN NORMAL]"));
            Serial.printf("👉 ขา GPIO 3 กลับสู่ลอจิก LOW (0V)\n");
            Serial.println(F("👉 สถานะ: ปกติ (ไม่ล้ม หรือผู้ใช้งานลุกขึ้นแล้ว)"));
            Serial.println(F("---------------------------------------------------------------"));
        }
    }

    // หากอยู่ในสถานะล้ม ให้ไฟแดงกะพริบเพื่อเตือนภัย
    if (current_state == HIGH) {
        if (now - last_flash >= 200) {
            last_flash = now;
            flash_state = !flash_state;
            if (flash_state) {
                setRGB(255, 0, 0); // แดง
            } else {
                setRGB(0, 0, 0);   // ดับ
            }
        }
    }

    // แสดงสถานะตรวจเช็กทุกๆ 1 วินาที ให้เห็นว่าลูปยังทำงานปกติ
    if (now - last_periodic_print >= 1000) {
        last_periodic_print = now;
        Serial.printf("[T=%lu s] 📡 สถานะขา GPIO 3 (TX2): %s (Logic %d)\n",
                      now / 1000,
                      (current_state == HIGH) ? "🔴 HIGH (ตรวจพบล้ม)" : "🟢 LOW (ปกติ)",
                      current_state);
    }
}
