#pragma once

#include "SystemConfig.h"

// ================= HLK-LD6002C (60 GHz FALL DETECTION RADAR) =================
#if USE_LD6002C

inline HardwareSerial RadarSerial(1);

inline bool          fall_alerted_this_session = false;
inline unsigned long fall_start_time           = 0;
inline bool          is_falling_candidate      = false;
inline unsigned long fall_recovery_start_time  = 0;
inline bool          is_recovering_candidate   = false;

inline void resetRadarFallFlags() {
  fall_alerted_this_session = false;
  is_falling_candidate      = false;
  is_recovering_candidate   = false;
  fall_start_time           = 0;
  fall_recovery_start_time  = 0;
}

inline void setRadarFallAlerted(bool alerted) {
  fall_alerted_this_session = alerted;
  is_falling_candidate      = false;
}

// คำนวณ Checksum (~XOR) ตามโปรโตคอล HLK-LD6002C
inline uint8_t calculateChecksum(const uint8_t* data, size_t len) {
  uint8_t ret = 0;
  for (size_t i = 0; i < len; ++i) ret ^= data[i];
  return (uint8_t)(~ret);
}

inline void sendConfigToRadar(uint16_t type, float value) {
  uint8_t frame[13];
  frame[0] = 0x01; // SOF
  frame[1] = 0x00; // ID High
  frame[2] = 0x00; // ID Low
  frame[3] = 0x00; // LEN High
  frame[4] = 0x04; // LEN Low
  frame[5] = (type >> 8) & 0xFF; // TYPE High
  frame[6] = type & 0xFF;        // TYPE Low (0x04 ติดตั้งสูง, 0x08 เกณฑ์ล้ม)
  frame[7] = calculateChecksum(frame, 7); // HEAD_CKSUM
  
  uint8_t *valPtr = (uint8_t *)&value;
  frame[8]  = valPtr[0];
  frame[9]  = valPtr[1];
  frame[10] = valPtr[2];
  frame[11] = valPtr[3];
  frame[12] = calculateChecksum(&frame[8], 4); // DATA_CKSUM
  
  RadarSerial.write(frame, 13);
  
  Serial.printf("📡 [RADAR CONFIG] ส่งคำสั่ง 0x%04X ค่า: %.2f\n", type, value);
}

inline void sendRoomSizeToRadar(float width, float length) {
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
  frame[7] = calculateChecksum(frame, 7); // HEAD_CKSUM
  
  memcpy(&frame[8],  &rect_XL, 4);
  memcpy(&frame[12], &rect_XR, 4);
  memcpy(&frame[16], &rect_ZF, 4);
  memcpy(&frame[20], &rect_ZB, 4);
  frame[24] = calculateChecksum(&frame[8], 16); // DATA_CKSUM
  
  RadarSerial.write(frame, 25);
  
  Serial.printf("📡 [RADAR CONFIG] ส่งขอบเขตห้อง (0x0E0C) กว้าง +/-%.1fm, ยาว +/-%.1fm\n", width, length);
}

inline void setupRadarLD6002C() {
  pinMode(RADAR_RX_PIN, INPUT_PULLUP); // ป้องกันสัญญาณขยะเมื่อสายลอย
  RadarSerial.setRxBufferSize(1024);
  RadarSerial.begin(115200, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);
  RadarSerial.setTimeout(50);
  Serial.println("📡 [SYSTEM] HLK-LD6002C Radar UART initialized at 115200 baud.");
  Serial.println("⏳ [SYSTEM] Waiting 2.0 seconds for radar module boot-up...");
  delay(2000);
  
  // ปล่อยให้เรดาร์รันด้วยค่ามาตรฐานโรงงาน ไม่ยิงคำสั่ง config ซ้ำซ้อนตอนเปิดเครื่อง
  // หากมีค่าที่ดึงมาจากหน้าเว็บ Dashboard ฟังก์ชัน fetchDeviceSettings() จะเป็นผู้ส่งคำสั่งไปปรับปรุงอย่างปลอดภัย
  Serial.println("🟢 [SYSTEM] HLK-LD6002C ready with calibrated factory settings.");
}

inline void readRadar() {
  String target_event = current_event;
  String display_status = "";
  bool got_valid_packet = false;

  while (RadarSerial.available() > 0) {
    // 1. ตรวจหาหัวเฟรม 0x01
    if (RadarSerial.peek() != 0x01) {
      RadarSerial.read();
      continue;
    }

    if (RadarSerial.available() < 8) break;

    // อ่าน Header 8 ไบต์ [SOF, idH, idL, lenH, lenL, typeH, typeL, headCk]
    uint8_t header[8];
    RadarSerial.readBytes(header, 8);

    uint16_t dataLen  = (header[3] << 8) | header[4];
    uint16_t msgType  = (header[5] << 8) | header[6];
    uint8_t  rxHeadCk = header[7];

    if (calculateChecksum(header, 7) != rxHeadCk) {
      continue; // Checksum ส่วน Header ไม่ตรง ข้ามไป
    }

    // อ่าน Payload และตรวจสอบ Checksum ส่วน Payload
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
        continue; // Checksum ส่วน Payload ไม่ตรง ข้ามไป
      }

      got_valid_packet = true;

      // ================= 2. วิเคราะห์ข้อมูล =================
      // ก. เฟรมตรวจจับการล้ม (Type 0x0E02)
      if (msgType == 0x0E02) {
        uint8_t is_fall = payload[0]; // 0x01 = ล้ม, 0x00 = ปกติ

        if (is_fall == 0x01) {
          // ตรรกะระบบหลัก (Sensor Fusion): ประมวลผลการล้มเฉพาะเมื่อมีคนอยู่ในห้องน้ำ (current_event != "exit")
          // หากห้องว่าง (exit) จะตัดทิ้งเพื่อป้องกัน False Alarm จากสิ่งของตกหล่น
          if (current_event != "exit") {
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
            target_event = "exit";
          }
        } else {
          // สภาวะปกติ (is_fall == 0)
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

        static uint8_t last_printed_is_fall = 0xFF;
        static unsigned long last_radar_log_time = 0;
        unsigned long now = millis();
        if (is_fall != last_printed_is_fall || now - last_radar_log_time >= 3000) {
          last_printed_is_fall = is_fall;
          last_radar_log_time = now;
          Serial.print("📡 [RAW SENSOR] Direct output from radar -> is_fall: ");
          Serial.println(is_fall == 0x01 ? "1 (🚨 FALL / ล้ม)" : "0 (🟢 NORMAL / ปกติ)");
        }
      }
      // ข. เฟรมตอบรับคำสั่ง Config (ACK: 0x0E04, 0x0E08, 0x0E0C)
      else if (msgType == 0x0E04 || msgType == 0x0E08 || msgType == 0x0E0C) {
        Serial.printf("📥 [RADAR ACK] ตอบรับคำสั่ง Config: 0x%04X\n", msgType);
      }
      // ค. เฟรมตรวจพบคนอยู่ในห้อง (0x0F09)
      else if (msgType == 0x0F09) {
        // payload[0] == 0x01 คือมีคนอยู่ในห้อง
      }
      // ง. เฟรมความสูงเป้าหมาย (0x0E0E)
      else if (msgType == 0x0E0E && dataLen == 4) {
        float target_height;
        memcpy(&target_height, payload, 4);
      }
    }
  }

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

#else
inline void resetRadarFallFlags() {}
inline void setRadarFallAlerted(bool alerted) {}
inline void sendConfigToRadar(uint16_t type, float value) {}
inline void sendRoomSizeToRadar(float width, float length) {}
inline void setupRadarLD6002C() {}
inline void readRadar() {}
#endif // USE_LD6002C


// ================= HLK-LD2410C (PRESENCE DETECTION RADAR) =================
#if USE_LD2410C

inline void setupRadarLD2410C() {
  pinMode(LD2410_OUT_PIN, INPUT_PULLDOWN);
  Serial.println("📡 [SYSTEM] HLK-LD2410C GPIO OUT pin initialized (Active HIGH).");
}

inline void readLD2410() {
  static unsigned long last_presence_time  = 0;
  static unsigned long last_ld2410_log_time = 0;

  int presence = digitalRead(LD2410_OUT_PIN);
  bool is_occupied = (presence == HIGH);
  unsigned long now = millis();
  
  if (now - last_ld2410_log_time >= 3000) {
    last_ld2410_log_time = now;
    Serial.print("📡 [LD2410 DEBUG] Human presence state: ");
    Serial.println(is_occupied ? "OCCUPIED (มีคนอยู่)" : "EMPTY (ห้องว่าง)");
  }

  if (is_occupied) {
    last_presence_time = millis();
    
    if (current_event == "exit") {
      current_event = "enter";
      sendEvent("event", "enter", "กำลังใช้งาน (ตรวจพบคนเข้าห้องน้ำ)");
      resetRadarFallFlags();
    }
  } else {
    // 1. กรณีคนเดินออกจากห้องน้ำตามปกติ
    if (current_event == "enter" && (millis() - last_presence_time > presence_timeout)) {
      current_event = "exit";
      sendEvent("event", "exit", "ว่าง (ไม่มีคนอยู่)");
      resetRadarFallFlags();
    }
    // 2. Failsafe: หากอยู่ในสถานะคนล้ม แต่เซนเซอร์ยืนยันว่าไม่มีคนอยู่ในห้องต่อเนื่องเกิน 20 วินาที
    //    (ป้องกันเหตุการณ์คนเดินออกไปแล้ว หรือการตรวจจับล้มผิดพลาดค้างตลอดกาล)
    else if (current_event == "fall" && (millis() - last_presence_time > fall_empty_room_timeout)) {
      Serial.println("\nℹ️ [LD2410] Room empty for >20s during fall alert. Auto-recovering to exit.");
      current_event = "exit";
      sendEvent("event", "exit", "ว่าง (รีเซ็ตอัตโนมัติ/ห้องว่าง)");
      resetRadarFallFlags();
    }
  }
}

#else
inline void setupRadarLD2410C() {}
inline void readLD2410() {}
#endif // USE_LD2410C
