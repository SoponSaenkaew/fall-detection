#include <Arduino.h>

// ขาเชื่อมต่อ
#define RADAR_RX_PIN 18  // ต่อกับขา TX ของ LD6002C
#define RADAR_TX_PIN 17  // ต่อกับขา RX ของ LD6002C
#define RADAR_BAUD    115200 // ความเร็วที่ถูกต้อง 100%

HardwareSerial RadarSerial(1);

// ตัวแปรตรวจจับการล้ม
bool is_falling_candidate    = false;
bool fall_alerted            = false;
unsigned long fall_start_time = 0;
const unsigned long FALL_CONFIRM_MS = 2000; // ล้มค้างเกิน 2 วินาทีถึงยืนยัน

// คำนวณ Checksum (~XOR)
uint8_t calculateChecksum(const uint8_t* data, size_t len) {
    uint8_t ret = 0;
    for (size_t i = 0; i < len; ++i) ret ^= data[i];
    return (uint8_t)(~ret);
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n===============================================================");
    Serial.println("   🟢 HLK-LD6002C FALL DETECTION SYSTEM (115200 BAUD)          ");
    Serial.println("===============================================================");

    pinMode(RADAR_RX_PIN, INPUT_PULLUP);
    RadarSerial.begin(RADAR_BAUD, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);

    Serial.println("✅ เชื่อมต่อเรดาร์ที่ 115200 baud สำเร็จ");
    Serial.println("👀 เริ่มต้นตรวจจับสถานะการล้ม...\n");
}

void loop() {
    static unsigned long lastLogTime = 0;

    while (RadarSerial.available() > 0) {
        // 1. หาหัวเฟรม 0x01
        if (RadarSerial.peek() != 0x01) {
            RadarSerial.read();
            continue;
        }

        if (RadarSerial.available() < 8) break;

        // อ่าน Header 8 ไบต์
        uint8_t header[8];
        RadarSerial.readBytes(header, 8);

        uint16_t dataLen  = (header[3] << 8) | header[4];
        uint16_t msgType  = (header[5] << 8) | header[6];
        uint8_t  rxHeadCk = header[7];

        if (calculateChecksum(header, 7) != rxHeadCk) {
            continue; // Checksum หัวเฟรมไม่ตรง ข้ามไป
        }

        // อ่าน Payload
        if (dataLen > 0 && dataLen <= 64) {
            unsigned long waitStart = millis();
            while (RadarSerial.available() < (dataLen + 1) && (millis() - waitStart < 20)) {
                delayMicroseconds(50);
            }
            if (RadarSerial.available() < (dataLen + 1)) break;

            uint8_t payload[64];
            RadarSerial.readBytes(payload, dataLen);
            uint8_t rxDataCk = RadarSerial.read();

            if (calculateChecksum(payload, dataLen) != rxDataCk) {
                continue;
            }

            // ================= 2. วิเคราะห์ข้อมูล =================
            // ก. เฟรมตรวจจับการล้ม (Type 0x0E02)
            if (msgType == 0x0E02) {
                uint8_t is_fall = payload[0]; // 0x01 = ล้ม, 0x00 = ปกติ

                if (is_fall == 0x01) {
                    if (!fall_alerted) {
                        if (!is_falling_candidate) {
                            is_falling_candidate = true;
                            fall_start_time = millis();
                            Serial.println("⚠️ [RADAR] ตรวจพบแนวโน้มการล้ม! กำลังจับเวลา 2 วินาทีเพื่อยืนยัน...");
                        } else if (millis() - fall_start_time >= FALL_CONFIRM_MS) {
                            fall_alerted = true;
                            is_falling_candidate = false;
                            Serial.println("\n🚨🚨🚨 [ALERT] ตรวจพบคนล้มจริง! (Fall Confirmed) 🚨🚨🚨\n");
                            // >>> ใส่โค้ดสั่งบัซเซอร์ / ส่ง LINE / MQTT ตรงนี้ <<<
                        }
                    }
                } else {
                    // กลับสู่สภาวะปกติ
                    if (is_falling_candidate) {
                        is_falling_candidate = false;
                        Serial.println("ℹ️ [RADAR] ยกเลิกการเตือน (คนก้มตัวชั่วคราวแล้วลุกขึ้น)");
                    }
                    if (fall_alerted) {
                        fall_alerted = false;
                        Serial.println("🟢 [RADAR] ผู้ใช้งานลุกขึ้นยืนแล้ว (ระบบกลับสู่สภาวะปกติ)");
                    }

                    // แสดงสถานะปกติทุก 2 วินาที ให้รู้ว่าระบบยังตรวจจับอยู่ตลอด
                    if (millis() - lastLogTime >= 2000) {
                        lastLogTime = millis();
                        Serial.println("🟢 [STATUS] สภาวะปกติ: ยืน / นั่ง (is_fall = 0)");
                    }
                }
            }
            // ข. เฟรมตรวจพบคนอยู่ในห้อง (Type 0x0F09)
            else if (msgType == 0x0F09) {
                // payload[0] == 0x01 คือมีคนอยู่ในห้อง
            }
            // ค. เฟรมความสูงเป้าหมาย (Type 0x0E0E)
            else if (msgType == 0x0E0E && dataLen == 4) {
                float target_height;
                memcpy(&target_height, payload, 4);
                // Serial.printf("📏 ความสูงเป้าหมาย: %.2f ม.\n", target_height);
            }
        }
    }
}