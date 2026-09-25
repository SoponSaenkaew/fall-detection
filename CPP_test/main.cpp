#include "Arduino.h"

// ==============================================================================
// 📡 HLK-LD6002C RADAR DRIVER (เขียนในรูปแบบเดียวกับบนบอร์ด ESP32-S3 100%)
// สามารถคัดลอกโค้ดส่วนนี้ไปวางใน RadarSensors.h บนบอร์ดจริงได้ทันที!
// ==============================================================================

// 1. ฮาร์ดแวร์ UART พอร์ต 1
HardwareSerial RadarSerial(1);

// 2. พารามิเตอร์ระบบและตัวแปรสถานะ (แบบเดียวกับใน SystemConfig.h)
const int RADAR_RX_PIN = 18;
const int RADAR_TX_PIN = 17;

float fall_threshold = 0.5;   // เกณฑ์ความสูงตรวจจับการล้ม (เมตร)
float mount_height   = 2.0;   // ความสูงติดตั้งเซนเซอร์ (เมตร)
float room_width     = 4.0;   // ความกว้างห้อง (เมตร)
float room_length    = 4.0;   // ความยาวห้อง (เมตร)

String current_event = "exit"; // สถานะปัจจุบัน ("exit", "enter", "fall")

const unsigned long fall_confirmation_delay = 2000; // หน่วงเวลายืนยันล้มจริง 2 วิ
const unsigned long fall_recovery_delay     = 2500; // หน่วงเวลายืนยันลุกยืน 2.5 วิ

bool          fall_alerted_this_session = false;
unsigned long fall_start_time           = 0;
bool          is_falling_candidate      = false;
unsigned long fall_recovery_start_time  = 0;
bool          is_recovering_candidate   = false;

// 3. จำลองฟังก์ชันส่ง Event (บนบอร์ดจริงจะส่ง MQTT/HTTP)
void sendEvent(const char* type, const char* name, const char* display_status) {
    Serial.print("🚀 [EVENT DISPATCH] Type: ");
    Serial.print(type);
    Serial.print(" | Event: ");
    Serial.print(name);
    Serial.print(" | Status: ");
    Serial.println(display_status);
}

void resetRadarFallFlags() {
    fall_alerted_this_session = false;
    is_falling_candidate      = false;
    is_recovering_candidate   = false;
    fall_start_time           = 0;
    fall_recovery_start_time  = 0;
}

void setRadarFallAlerted(bool alerted) {
    fall_alerted_this_session = alerted;
    is_falling_candidate      = false;
}

// 4. ฟังก์ชันส่งคำสั่งคอนฟิกค่าเดี่ยวลงเรดาร์ (เช่น ความสูง หรือ เกณฑ์ล้ม)
void sendConfigToRadar(uint16_t type, float value) {
    uint8_t frame[13];
    frame[0] = 0x01; // SOF
    frame[1] = 0x00; // ID High
    frame[2] = 0x00; // ID Low
    frame[3] = 0x00; // LEN High
    frame[4] = 0x04; // LEN Low
    frame[5] = (type >> 8) & 0xFF; // TYPE High
    frame[6] = type & 0xFF;        // TYPE Low (0x04 ติดตั้งสูง, 0x08 เกณฑ์ล้ม)
  
    uint8_t headXor = 0;
    for (int i = 0; i < 7; i++) {
        headXor ^= frame[i];
    }
    frame[7] = ~headXor; // HEAD_CKSUM
  
    uint8_t *valPtr = (uint8_t *)&value;
    frame[8]  = valPtr[0];
    frame[9]  = valPtr[1];
    frame[10] = valPtr[2];
    frame[11] = valPtr[3];
  
    uint8_t dataXor = 0;
    for (int i = 8; i < 12; i++) {
        dataXor ^= frame[i];
    }
    frame[12] = ~dataXor; // DATA_CKSUM
  
    RadarSerial.write(frame, 13);
  
    Serial.print("📡 [RADAR CONFIG] Sent Command 0x");
    Serial.print(type, HEX);
    Serial.print(" with value: ");
    Serial.println(value);
}

// 5. ฟังก์ชันส่งขนาดห้องลงเรดาร์
void sendRoomSizeToRadar(float width, float length) {
    float rect_XL = -width;        // ขอบเขตซ้าย
    float rect_XR = width;         // ขอบเขตขวา
    float rect_ZF = length;        // ขอบเขตหน้า
    float rect_ZB = -length;       // ขอบเขตหลัง
  
    uint8_t frame[25];
    frame[0] = 0x01; // SOF
    frame[1] = 0x00; // ID High
    frame[2] = 0x00; // ID Low
    frame[3] = 0x00; // LEN High
    frame[4] = 0x10; // LEN Low (16 ไบต์)
    frame[5] = 0x0E; // TYPE High
    frame[6] = 0x0C; // TYPE Low (0x0E0C)
  
    uint8_t headXor = 0;
    for (int i = 0; i < 7; i++) {
        headXor ^= frame[i];
    }
    frame[7] = ~headXor; // HEAD_CKSUM
  
    memcpy(&frame[8],  &rect_XL, 4);
    memcpy(&frame[12], &rect_XR, 4);
    memcpy(&frame[16], &rect_ZF, 4);
    memcpy(&frame[20], &rect_ZB, 4);
  
    uint8_t dataXor = 0;
    for (int i = 8; i < 24; i++) {
        dataXor ^= frame[i];
    }
    frame[24] = ~dataXor; // DATA_CKSUM
  
    RadarSerial.write(frame, 25);
  
    Serial.print("📡 [RADAR CONFIG] Sent Room Dimensions (0x0E0C) - Width: ");
    Serial.print(width);
    Serial.print(" m, Length: ");
    Serial.print(length);
    Serial.println(" m");
}

// 6. ฟังก์ชันเริ่มต้นเรดาร์
void setupRadarLD6002C() {
    RadarSerial.begin(115200, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);
    Serial.println("📡 [SYSTEM] HLK-LD6002C Radar UART initialized at 115200 baud.");
    Serial.println("⏳ [SYSTEM] Waiting for radar module boot-up...");
    delay(500); // บนคอมหน่วง 500ms เพื่อความรวดเร็วในการทดสอบ
  
    sendConfigToRadar(0x0E04, mount_height);
    delay(50);
    sendConfigToRadar(0x0E08, fall_threshold);
    delay(50);
    sendRoomSizeToRadar(room_width, room_length);
    delay(50);
}

// 7. ฟังก์ชันอ่านและถอดรหัสข้อมูล UART จากเรดาร์ (จุดที่คุณทดลองเขียน/ปรับปรุง)
void readRadar() {
    String target_event = current_event;
    String display_status = "";
    bool got_valid_packet = false;

    while (RadarSerial.available() > 0) {
        uint8_t topByte = RadarSerial.peek();
    
        // มองหาไบต์เริ่มเฟรม 0x01
        if (topByte != 0x01) {
            RadarSerial.read(); // ข้ามไบต์ขยะ
            continue;
        }
    
        // ตรวจสอบว่ามีข้อมูลอย่างน้อย 8 ไบต์ (SOF + Header 7 ไบต์) หรือไม่
        if (RadarSerial.available() < 8) {
            break;
        }

        RadarSerial.read(); // ดึงไบต์ 0x01 ออกจากบัฟเฟอร์
    
        uint8_t header[7];
        RadarSerial.readBytes(header, 7);
      
        uint8_t xorSum = 0x01;
        for (int i = 0; i < 6; i++) {
            xorSum ^= header[i];
        }
        uint8_t calculated_head_cksum = ~xorSum;
    
        // ตรวจสอบ Header Checksum
        bool isHeadChecksumValid = (calculated_head_cksum == header[6]) || 
                                   (xorSum == header[6]) || 
                                   (header[0] == 0x0E || header[4] == 0x0E);
                                   
        if (isHeadChecksumValid) {
            uint16_t dataLen = (header[2] << 8) | header[3];
            uint16_t totalPayloadBytes = dataLen + 1; // รวม DATA_CKSUM 1 ไบต์
      
            if (totalPayloadBytes > 0 && totalPayloadBytes <= 256) {
                uint8_t dataBuf[256];
                RadarSerial.readBytes(dataBuf, totalPayloadBytes);
        
                uint16_t typeA = (header[0] << 8) | header[1];
                uint16_t typeB = (header[4] << 8) | header[5];
        
                bool isFallFrame = (typeA == 0x0E02 || typeA == 0x020E || typeB == 0x0E02 || typeB == 0x020E);
                bool isConfigResponse = (typeA == 0x0E04 || typeA == 0x0E08 || typeB == 0x0E04 || typeB == 0x0E08 || typeA == 0x0E0C || typeB == 0x0E0C);

                if (isFallFrame) {
                    uint8_t is_fall = dataBuf[0];    
                    uint8_t data_cksum = dataBuf[dataLen]; 
          
                    if (dataLen == 1 || data_cksum == (uint8_t)~is_fall) {
                        got_valid_packet = true;
            
                        if (is_fall == 0x01) {
                            is_recovering_candidate = false;
            
                            if (!fall_alerted_this_session && current_event != "fall") {
                                if (!is_falling_candidate) {
                                    is_falling_candidate = true;
                                    fall_start_time = millis();
                                    Serial.println("⚠️ [RADAR] Detect potential fall! Start timer for confirmation...");
                                    target_event = current_event;
                                } else {
                                    if (millis() - fall_start_time >= fall_confirmation_delay) {
                                        target_event = "fall";
                                        display_status = "ตรวจพบคนล้ม (เรดาร์)";
                                        fall_alerted_this_session = true;
                                        is_falling_candidate = false;
                                        Serial.println("🚨 [RADAR] Fall confirmed! Sending alert...");
                                    } else {
                                        target_event = current_event;
                                    }
                                }
                            } else {
                                target_event = "fall";
                            }
                        } else {
                            if (is_falling_candidate) {
                                is_falling_candidate = false;
                                Serial.println("ℹ️ [RADAR] Fall canceled (transient noise or crouch detected).");
                            }

                            if (current_event == "fall") {
                                if (!is_recovering_candidate) {
                                    is_recovering_candidate = true;
                                    fall_recovery_start_time = millis();
                                    target_event = "fall";
                                } else {
                                    if (millis() - fall_recovery_start_time >= fall_recovery_delay) {
                                        target_event = "enter";
                                        display_status = "กำลังใช้งาน (เรดาร์)";
                                        fall_alerted_this_session = false;
                                        is_recovering_candidate = false;
                                        Serial.println("🟢 [RADAR] Target recovered/stood up! Switched back to enter.");
                                    } else {
                                        target_event = "fall";
                                    }
                                }
                            } else if (current_event == "enter") {
                                target_event = "enter";
                                display_status = "กำลังใช้งาน (เรดาร์)";
                            } else {
                                target_event = "exit";
                                display_status = "ว่าง";
                            }
                        }

                        Serial.print("📡 [RAW SENSOR] Direct output from radar -> is_fall: ");
                        Serial.println(is_fall == 0x01 ? "1 (🚨 FALL / ล้ม)" : "0 (🟢 NORMAL / ปกติ)");
                    }
                } 
                else if (isConfigResponse) {
                    Serial.print("📡 [RADAR RESPONSE] Received frame TYPE: 0x");
                    Serial.print(typeA, HEX);
                    Serial.print(" / 0x");
                    Serial.print(typeB, HEX);
                    Serial.println(" | Config OK");
                }
            }
        }
    }

    // จัดการ Event เมื่อสถานะเปลี่ยน
    if (got_valid_packet && target_event != current_event) {
        if (target_event == "fall") {
            Serial.println("\n🚨 [RADAR DETECTED] !!! PERSON FELL !!!");
            current_event = "fall";
            sendEvent("event", "fall", display_status.c_str());
        } 
        else if (target_event == "enter" && current_event == "fall") {
            Serial.println("\n🟢 [RADAR DETECTED] Person is no longer in fall state.");
            current_event = "enter";
            sendEvent("event", "enter", display_status.c_str());
        }
    }
}

// ---------------- Standard Arduino Entrypoints ----------------
void setup() {
    Serial.println("=============================================");
    Serial.println("Starting ESP32-S3 Radar Simulation Node...");
    Serial.println("=============================================");
    setupRadarLD6002C();
}

void loop() {
    readRadar();
}

// ==============================================================================
// 🧪 HARNESS RUNNER (สำหรับรันบนคอมพิวเตอร์และป้อนข้อมูลทดสอบเข้า loop)
// ==============================================================================
int main() {
    setup();

    // กำหนดสถานะเริ่มต้นให้มีคนเข้าห้องน้ำแล้ว (enter) เพื่อให้พร้อมทดสอบตรวจจับการล้ม
    current_event = "enter";
    Serial.println("\n[SIMULATOR] จำลองสถานะปัจจุบัน: คนเข้าห้องน้ำแล้ว (current_event = \"enter\")");

    while (true) {
        Serial.println("\n---------------------------------------------");
        Serial.println("เลือกชุดข้อมูลจำลองส่งเข้า UART (เหมือนเรดาร์ส่งมา):");
        Serial.println("1. ส่งเฟรมล้ม (0x0E02, is_fall = 1)");
        Serial.println("2. ส่งเฟรมปกติ/ยืน (0x0E02, is_fall = 0)");
        Serial.println("3. ส่งสัญญาณรบกวน (Noise ขยะก่อนแล้วตามด้วยเฟรมล้ม)");
        Serial.println("4. ส่งเฟรมล้มค้างเกิน 2 วินาที (ทดสอบยืนยันล้มจริง)");
        Serial.println("5. ส่งเฟรมลุกยืนค้างเกิน 2.5 วินาที (ทดสอบ Recovery)");
        Serial.println("6. ออกจากโปรแกรม");
        Serial.print("เลือกข้อ (1-6): ");

        int choice = 0;
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) break;
            std::cin.clear();
            std::string discard;
            std::cin >> discard;
            continue;
        }

        if (choice == 6) break;

        switch (choice) {
            case 1: {
                // แพ็กเก็ตตรวจพบคนล้มตาม Datasheet: 01 00 00 00 01 0E 02 F3 01 FE
                uint8_t fall_pkt[] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x0E, 0x02, 0xF3, 0x01, 0xFE};
                Serial.println("\n📥 [INJECT] ป้อนเฟรมล้มเข้า RadarSerial...");
                RadarSerial.injectPacket(fall_pkt, sizeof(fall_pkt));
                loop();
                break;
            }
            case 2: {
                // แพ็กเก็ตปกติ: 01 00 00 00 01 0E 02 F3 00 FF
                uint8_t norm_pkt[] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x0E, 0x02, 0xF3, 0x00, 0xFF};
                Serial.println("\n📥 [INJECT] ป้อนเฟรมปกติเข้า RadarSerial...");
                RadarSerial.injectPacket(norm_pkt, sizeof(norm_pkt));
                loop();
                break;
            }
            case 3: {
                // มีไบต์ขยะ AA 55 FF ก่อนแล้วค่อยเจอ 01 ...
                uint8_t noise_pkt[] = {0xAA, 0x55, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x01, 0x0E, 0x02, 0xF3, 0x01, 0xFE};
                Serial.println("\n📥 [INJECT] ป้อนสัญญาณรบกวน + เฟรมล้ม...");
                RadarSerial.injectPacket(noise_pkt, sizeof(noise_pkt));
                loop();
                break;
            }
            case 4: {
                uint8_t fall_pkt[] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x0E, 0x02, 0xF3, 0x01, 0xFE};
                Serial.println("\n📥 [INJECT] จำลองการล้มค้าง 2.2 วินาที...");
                RadarSerial.injectPacket(fall_pkt, sizeof(fall_pkt));
                loop();
                Serial.println("⏳ รอเวลาผ่านไป 2200 ms...");
                delay(2200);
                RadarSerial.injectPacket(fall_pkt, sizeof(fall_pkt));
                loop();
                break;
            }
            case 5: {
                uint8_t norm_pkt[] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x0E, 0x02, 0xF3, 0x00, 0xFF};
                Serial.println("\n📥 [INJECT] จำลองคนลุกขึ้นยืนต่อเนื่อง 2.6 วินาที...");
                RadarSerial.injectPacket(norm_pkt, sizeof(norm_pkt));
                loop();
                Serial.println("⏳ รอเวลาผ่านไป 2600 ms...");
                delay(2600);
                RadarSerial.injectPacket(norm_pkt, sizeof(norm_pkt));
                loop();
                break;
            }
            default:
                Serial.println("กรุณาเลือก 1-6");
                break;
        }
    }

    Serial.println("จบการทำงาน");
    return 0;
}