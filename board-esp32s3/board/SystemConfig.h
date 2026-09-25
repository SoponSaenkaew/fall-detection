#pragma once

#include <Arduino.h>
#include <Preferences.h>

// ================= USER CONFIGURATION =================
inline const char* ssid          = "PINKU 8541";
inline const char* password      = "468Bt9@2";
inline const char* device_id     = "LD6002C_TEST_01";

// วิธีส่งข้อมูล: true = MQTT (แนะนำ), false = HTTP POST
#define USE_MQTT true

// การตั้งค่า HTTPS POST (พอร์ต 443)
inline const char* backend_url   = "https://fall-detection.sopon-project.me/api/v1/events"; 

// การตั้งค่า MQTT Broker (ใช้ Public IP ตรงเพื่อเลี่ยงปัญหา Hotspot DNS)
inline const char* mqtt_broker   = "40.81.24.21";
inline const int   mqtt_port     = 1883;
inline const char* mqtt_topic    = "sensor/events";

// ================= HARDWARE & SENSOR PINS =================
// 1. เรดาร์ตรวจจับการล้ม HLK-LD6002C (60 GHz)
#define USE_LD6002C true
#define RADAR_RX_PIN 18       // ขา RX ของ ESP32 (ต่อกับ Pin 7 TX0 ของเรดาร์) 
#define RADAR_TX_PIN 17       // ขา TX ของ ESP32 (ต่อกับ Pin 8 RX0 ของเรดาร์) 

// 2. เรดาร์ตรวจคนเข้า-ออกห้องน้ำ HLK-LD2410C (ผ่านขา OUT)
#define USE_LD2410C true
#define LD2410_OUT_PIN 16
#define LD2410_ACTIVE_LEVEL HIGH  // สัญญาณขา OUT ตามเดิม: 3.3V (HIGH) = มีคนอยู่, 0V (LOW) = ห้องว่าง
inline const unsigned long presence_timeout        = 5000;  // เวลาหน่วงกรณีไม่พบการเคลื่อนไหว (มิลลิวินาที) ก่อนปรับเป็นสถานะห้องว่าง (exit)
inline const unsigned long fall_empty_room_timeout = 20000; // เวลาหน่วง (ms) ป้องกันสถานะล้มค้าง หากไม่มีคนในห้องจริงเกิน 20 วิ

// 3. เอาต์พุตควบคุม ไซเรน / รีเลย์ / บัซเซอร์
#define BUZZER_PIN 15

// 4. ปุ่มกดทดสอบจำลองสถานะ (Simulation Buttons)
inline const int BUTTON_ENTER_PIN = 4;   // จำลองคนเข้าห้อง (enter)
inline const int BUTTON_FALL_PIN  = 5;   // จำลองคนล้ม (fall)
inline const int BUTTON_EXIT_PIN  = 1;   // จำลองคนออกห้อง (exit) - ขา 1 ตามโค้ดเดิม

// 5. ไฟแสดงผล RGB LED บนบอร์ด ESP32-S3 DevKitC-1
#define RGB_LED_PIN 48
#define RGB_BRIGHTNESS 0.15

// 6. พารามิเตอร์หน่วงเวลายืนยันการล้ม / การลุกยืน
inline const unsigned long fall_confirmation_delay = 0; // ล้มค้าง 2 วินาทีเพื่อยืนยันล้มจริง
inline const unsigned long fall_recovery_delay     = 2500; // ยืนตรง 2.5 วินาทีเพื่อยืนยันลุกขึ้น

// 7. พารามิเตอร์ตรวจจับการเข้าห้องน้ำนานเกินกำหนด (ค่าดีฟอลต์ 30 นาที)
inline float         stay_timeout_minutes    = 30.0;
inline unsigned long prolonged_stay_timeout  = 30 * 60 * 1000UL;
inline unsigned long enter_start_time        = 0;
inline bool          prolonged_stay_alerted  = false;

// ================= RUNTIME STATE & STORAGE =================
inline String current_event   = "exit";
inline float  fall_threshold  = 0.5;   // เกณฑ์ความสูงตรวจจับการล้ม
inline float  mount_height    = 2.0;   // ความสูงติดตั้งเซนเซอร์ (เมตร)
inline float  room_width      = 4.0;   // ความกว้างห้องน้ำ (เมตร)
inline float  room_length     = 4.0;   // ความยาวห้องน้ำ (เมตร)

// ================= CROSS-MODULE FORWARD DECLARATIONS =================
void setRGBColor(uint8_t r, uint8_t g, uint8_t b);
void sendEvent(const char* type, const char* name, const char* display_status);
bool isWiFiConnected();
bool isMqttConnected();
void resetRadarFallFlags();
void setRadarFallAlerted(bool alerted);
void sendConfigToRadar(uint16_t type, float value);
void sendRoomSizeToRadar(float width, float length);

// ================= NVS FLASH MEMORY FUNCTIONS =================
inline void loadSavedConfig() {
  Preferences preferences;
  preferences.begin("fall-detector", true);
  fall_threshold       = preferences.getFloat("threshold", fall_threshold);
  mount_height         = preferences.getFloat("height", mount_height);
  room_width           = preferences.getFloat("width", room_width);
  room_length          = preferences.getFloat("length", room_length);
  stay_timeout_minutes = preferences.getFloat("stay_timeout", stay_timeout_minutes);
  if (stay_timeout_minutes <= 0.0) stay_timeout_minutes = 30.0;
  prolonged_stay_timeout = (unsigned long)(stay_timeout_minutes * 60.0 * 1000.0);
  preferences.end();

  Serial.println("💾 [SYSTEM] Loaded saved configurations from Flash memory:");
  Serial.print("   👉 fall_threshold       : "); Serial.println(fall_threshold);
  Serial.print("   👉 mount_height          : "); Serial.print(mount_height); Serial.println(" m");
  Serial.print("   👉 room_width            : "); Serial.print(room_width); Serial.println(" m");
  Serial.print("   👉 room_length           : "); Serial.print(room_length); Serial.println(" m");
  Serial.print("   👉 stay_timeout_minutes  : "); Serial.print(stay_timeout_minutes); Serial.println(" min\n");
}

inline void saveConfigToNVS(float threshold, float height, float width, float length, float stay_timeout = -1.0) {
  fall_threshold = threshold;
  mount_height   = height;
  room_width     = width;
  room_length    = length;
  if (stay_timeout > 0.0) {
    stay_timeout_minutes   = stay_timeout;
    prolonged_stay_timeout = (unsigned long)(stay_timeout_minutes * 60.0 * 1000.0);
  }

  Preferences preferences;
  preferences.begin("fall-detector", false);
  preferences.putFloat("threshold", fall_threshold);
  preferences.putFloat("height", mount_height);
  preferences.putFloat("width", room_width);
  preferences.putFloat("length", room_length);
  preferences.putFloat("stay_timeout", stay_timeout_minutes);
  preferences.end();

  Serial.println("💾 [SYSTEM] Saved updated configurations to Flash memory!");
}
