/*
 * =========================================================================================
 * 📡 ชุดโปรแกรมทดลองและบันทึกสถิติไร้สาย: เรดาร์ตรวจจับการล้ม HLK-LD6002C (60 GHz)
 * =========================================================================================
 * โครงการ: ระบบตรวจจับการล้มอัจฉริยะ (Smart Fall Detection System)
 * 🌟 ออกแบบพิเศษสำหรับเซนเซอร์ที่ติดตั้งบนเพดาน (Ceiling Mount) ที่ไม่สามารถต่อสาย USB ได้!
 * 
 * 📱 ระบบควบคุม, ตั้งค่าเซนเซอร์ & ตรวจสอบการเชื่อมต่อแบบไร้สาย (Wireless Web Dashboard):
 *  1. ปล่อย Wi-Fi Hotspot ในตัว: SSID "LD6002C-EXPERIMENT" (รหัสผ่าน 12345678)
 *     หรือเชื่อมต่อ Wi-Fi ที่บ้าน/มือถืออัตโนมัติ
 *  2. เปิดเว็บบนมือถือ/แท็บเล็ต/โน้ตบุ๊กได้ทันทีผ่าน Browser: http://192.168.4.1
 *  3. 📡 แสดงสถานะการเชื่อมต่อเซนเซอร์สด (Live Sensor Connection Watchdog):
 *     - แสดงผลทันทีว่าบอร์ด ESP32-S3 ยังคุยกับเรดาร์ LD6002C อยู่หรือไม่ (ONLINE / OFFLINE)
 *     - นับความเร็วการรับข้อมูลแบบเรียลไทม์ (Packets Per Second / Hz)
 *     - 🔀 ปุ่มสลับขา RX/TX (18/17 ⇄ 17/18) ผ่านหน้าจอมือถือได้ทันทีหากต่อสายไขว้สลับกัน
 *  4. ⚙️ ปรับตั้งค่าเซนเซอร์เรดาร์ไร้สายจากมือถือ:
 *     - ความสูงติดตั้งเซนเซอร์ (Mount Height: 2.0 - 3.5 m)
 *     - เกณฑ์ความสูงตัดสินการล้ม (Fall Threshold: 0.3 - 1.0 m)
 *     - ขอบเขตพื้นที่ห้อง (Room Width & Length Boundary)
 *     - เวลาหน่วงยืนยันการล้ม (Fall Delay) / เวลายืนยันการลุก (Recovery Delay)
 *     - บันทึกลง Flash Memory (NVS) เปิดเครื่องใหม่ค่าไม่หาย!
 *  5. มีปุ่มกดเลือกท่าทดสอบ (ล้มไปข้างหน้า, หงายหลัง, ก้มเก็บของ, นั่งชักโครก ฯลฯ) บนหน้าจอสัมผัส
 *  6. คำนวณ Confusion Matrix (TP, TN, FP, FN), Accuracy, Sensitivity, Specificity แบบเรียลไทม์
 *  7. ปุ่ม "ดาวน์โหลด CSV" ลงมือถือ/คอมได้ทันทีเพื่อเปิดใน Excel
 *  8. บันทึกและวิเคราะห์สัญญาณสด (Live Console Log) แสดงผลบนเว็บแบบเรียลไทม์
 *  9. รองรับ ArduinoOTA อัปเดตเฟิร์มแวร์แบบไร้สายผ่าน Wi-Fi ได้โดยไม่ต้องปีนขึ้นไปถอดบอร์ด
 * 
 * ขาเชื่อมต่อ (ESP32-S3):
 *  - ขา RX ของ ESP32-S3 -> GPIO 18 (ต่อกับ TX0 ของเรดาร์)
 *  - ขา TX ของ ESP32-S3 -> GPIO 17 (ต่อกับ RX0 ของเรดาร์)
 *  *(สามารถกดปุ่มสลับเป็น RX=17, TX=18 บนหน้าจอมือถือได้แบบไร้สาย)*
 * =========================================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <Preferences.h>

// ================= HARDWARE PIN DEFINITIONS =================
int radar_rx_pin           = 18;  // ขา RX ของ ESP32-S3 (ต่อกับ TX0 ของเรดาร์)
int radar_tx_pin           = 17;  // ขา TX ของ ESP32-S3 (ต่อกับ RX0 ของเรดาร์)
#define RADAR_DIGITAL_PIN  3    // ขาอ่านสัญญาณขาดิจิตอลตรง (ต่อกับ TX2 ของ LD6002C)
#define BUZZER_PIN         15   // ขาควบคุม Buzzer / Siren Relay (Active LOW)
#define RGB_LED_PIN        48   // ไฟสถานะ RGB บนบอร์ด ESP32-S3
#define RGB_BRIGHTNESS     0.25 // ความสว่างไฟ RGB (0.0 - 1.0)
#define RADAR_BAUD         115200 // ความเร็วบอดเรตมาตรฐานของเรดาร์

// ================= WI-FI CONFIGURATION =================
const char* station_ssid     = "PINKU 8541";       // WiFi บ้าน/Hotspot มือถือ
const char* station_password = "468Bt9@2";
const char* ap_ssid          = "LD6002C-EXPERIMENT"; // Wi-Fi ตรงจากบอร์ดบนเพดาน
const char* ap_password      = "12345678";

// พอร์ต Serial สำหรับเชื่อมต่อกับเซนเซอร์เรดาร์ HLK-LD6002C
HardwareSerial RadarSerial(1);
WebServer server(80);
Preferences preferences;

// ================= EXPERIMENT PARAMETERS =================
float mount_height = 2.20;               // ความสูงติดตั้งเซนเซอร์ (2.20 เมตร)
float fall_threshold = 0.50;             // เกณฑ์ความสูงการล้ม (0.50 เมตร)
float room_width = 1.50;                 // ความกว้างห้อง (1.50 เมตร)
float room_length = 1.50;                // ความยาวห้อง (1.50 เมตร)

// ตัวแปรการตรวจวัดระดับความสูงและขาดิจิตอล
float raw_radar_distance = 0.0f; // ระยะห่างจากเพดาน (เมตร) เช่น ยืน=0.64, นั่ง=1.14, นอน=1.98
float person_height      = 0.0f; // ความสูงจริงของคนจากพื้น = mount_height - raw_radar_distance
int   digital_fall_state = LOW;  // สถานะขาดิจิตอล TX2 (GPIO 3)

unsigned long fall_confirmation_delay = 2000; // เวลาหน่วงยืนยันล้มจริง (2 วินาที)
unsigned long fall_recovery_delay     = 2500; // เวลาหน่วงยืนยันการลุกขึ้นยืน (2.5 วินาที)
unsigned long adl_trial_timeout       = 10000; // เวลารอทดสอบท่าปกติ (10 วินาที) ก่อนตัดสินเป็น TN อัตโนมัติ

bool enable_buzzer = true;              // เปิด/ปิดเสียงไซเรน Buzzer
bool enable_raw_hex_dump = false;       // แสดงแพ็กเก็ตดิบทุกไบต์
String last_radar_ack_msg = "ยังไม่มีการส่งคำสั่ง";

// ================= CHECKSUM (~XOR) =================
uint8_t calculateChecksum(const uint8_t* data, size_t len) {
  uint8_t ret = 0;
  for (size_t i = 0; i < len; ++i) ret ^= data[i];
  return (uint8_t)(~ret);
}

// ================= LIVE WEB CONSOLE LOG SYSTEM =================
struct WebLogEntry {
  uint32_t id;
  String text;
};
const int MAX_WEB_LOGS = 80;
WebLogEntry web_logs[MAX_WEB_LOGS];
int web_log_head = 0;
int web_log_count = 0;
uint32_t next_log_id = 1;

void addWebLog(const String& msg) {
  Serial.println(msg);
  unsigned long ms = millis();
  unsigned long s = ms / 1000;
  unsigned long m = s / 60;
  char timeStr[16];
  snprintf(timeStr, sizeof(timeStr), "[%02lu:%02lu.%03lu] ", m % 60, s % 60, ms % 1000);
  
  web_logs[web_log_head].id = next_log_id++;
  web_logs[web_log_head].text = String(timeStr) + msg;
  web_log_head = (web_log_head + 1) % MAX_WEB_LOGS;
  if (web_log_count < MAX_WEB_LOGS) web_log_count++;
}

// ================= SENSOR CONNECTION WATCHDOG =================
unsigned long last_radar_packet_time = 0; // เวลาล่าสุดที่ได้รับแพ็กเก็ตถูกต้องจากเรดาร์
uint32_t total_radar_packets = 0;        // จำนวนแพ็กเก็ตที่ได้รับทั้งหมด
uint16_t packets_per_sec = 0;            // อัตราการส่งข้อมูลของเรดาร์ (packets/second)
uint16_t packet_counter_current_sec = 0;
unsigned long last_fps_calc_time = 0;
uint16_t last_ack_type = 0;

// Forward declaration
void applyAllConfigsToRadar();

// ================= NVS FLASH STORAGE =================
void loadSettingsFromNVS() {
  preferences.begin("ld6002c-exp", true);
  radar_rx_pin            = preferences.getInt("rx_pin", 18);
  radar_tx_pin            = preferences.getInt("tx_pin", 17);
  mount_height            = preferences.getFloat("m_height", 2.20f);
  fall_threshold          = preferences.getFloat("f_thresh", 0.50f);
  room_width              = preferences.getFloat("r_width", 1.50f);
  room_length             = preferences.getFloat("r_length", 1.50f);
  fall_confirmation_delay = preferences.getULong("f_delay", 2000UL);
  fall_recovery_delay     = preferences.getULong("r_delay", 2500UL);
  adl_trial_timeout       = preferences.getULong("adl_time", 10000UL);
  enable_buzzer           = preferences.getBool("buzzer", true);
  preferences.end();

  Serial.println(F("💾 [NVS] โหลดการตั้งค่าล่าสุดจาก Flash Memory:"));
  Serial.printf("   👉 ขา UART เรดาร์: RX=GPIO %d | TX=GPIO %d\n", radar_rx_pin, radar_tx_pin);
  Serial.printf("   👉 ความสูงติดตั้ง: %.2f m | เกณฑ์ล้ม: %.2f m | ขนาดห้อง: %.1f x %.1f m\n", 
                mount_height, fall_threshold, room_width, room_length);
  Serial.printf("   👉 หน่วงล้ม: %lu ms | หน่วงลุก: %lu ms | หน่วง ADL: %lu ms\n",
                fall_confirmation_delay, fall_recovery_delay, adl_trial_timeout);
}

void saveSettingsToNVS() {
  preferences.begin("ld6002c-exp", false);
  preferences.putInt("rx_pin", radar_rx_pin);
  preferences.putInt("tx_pin", radar_tx_pin);
  preferences.putFloat("m_height", mount_height);
  preferences.putFloat("f_thresh", fall_threshold);
  preferences.putFloat("r_width", room_width);
  preferences.putFloat("r_length", room_length);
  preferences.putULong("f_delay", fall_confirmation_delay);
  preferences.putULong("r_delay", fall_recovery_delay);
  preferences.putULong("adl_time", adl_trial_timeout);
  preferences.putBool("buzzer", enable_buzzer);
  preferences.end();

  Serial.println(F("💾 [NVS] บันทึกการตั้งค่าลง Flash Memory เรียบร้อย!"));
}

// ================= DATA STRUCTURES FOR STATISTICS =================
struct TrialRecord {
  uint16_t trial_no;
  char scenario_name[36];
  bool ground_truth_fall;   // true = ล้มจริง, false = กิจกรรมปกติ
  bool detected_fall;       // true = เรดาร์ตัดสินว่าล้ม, false = เรดาร์มองว่าปกติ
  uint32_t detection_latency_ms; // เวลาตั้งแต่ตัวแตะพื้น (Impact) จนถึงเรดาร์ตรวจพบ (ms)
  uint32_t confirm_time_ms; // เวลาที่ใช้ในการยืนยันล้ม (ms)
  uint32_t recovery_time_ms;// เวลาที่ใช้ในการลุกขึ้น (ms)
  char result_code[4];      // "TP", "TN", "FP", "FN"
};

const int MAX_TRIALS = 100;
TrialRecord trial_history[MAX_TRIALS];
uint16_t total_recorded_trials = 0;

// สถานะการทดสอบปัจจุบัน
enum TestMode { MODE_FREE_RUN, MODE_SCENARIO_TRIAL };
TestMode current_mode = MODE_FREE_RUN;

struct ActiveTrial {
  bool in_progress = false;
  char name[36];
  bool ground_truth_fall;
  unsigned long start_time = 0;
  unsigned long impact_time = 0; // เวลาที่กดปุ่มว่าล้มลงพื้นแล้ว (Impact Moment)
  bool fall_ever_triggered = false;
  uint32_t detection_latency_ms = 0;
  uint32_t measured_confirm_ms = 0;
  uint32_t measured_recovery_ms = 0;
} active_trial;

// สถานะการล้มของเรดาร์
enum FallState { STATE_NORMAL, STATE_FALL_CANDIDATE, STATE_FALL_CONFIRMED, STATE_RECOVERING };
FallState current_fall_state = STATE_NORMAL;

unsigned long fall_candidate_start_time = 0;
unsigned long recovery_start_time = 0;
uint8_t last_raw_is_fall = 0x00;
uint32_t last_confirmed_latency = 0;
uint32_t last_recovery_latency = 0;

// ================= HARDWARE CONTROL FUNCTIONS =================
void setRGB(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(RGB_LED_PIN, (uint8_t)(r * RGB_BRIGHTNESS), (uint8_t)(g * RGB_BRIGHTNESS), (uint8_t)(b * RGB_BRIGHTNESS));
}

void setBuzzer(bool active) {
  if (!enable_buzzer) {
    digitalWrite(BUZZER_PIN, HIGH);
    return;
  }
  digitalWrite(BUZZER_PIN, active ? LOW : HIGH); // Active LOW
}

// ================= HLK-LD6002C PROTOCOL TRANSMITTER =================
void sendConfigToRadar(uint16_t type, float value) {
  uint8_t frame[13];
  frame[0] = 0x01; // SOF
  frame[1] = 0x00; frame[2] = 0x00;
  frame[3] = 0x00; frame[4] = 0x04;
  frame[5] = (type >> 8) & 0xFF;
  frame[6] = type & 0xFF;
  frame[7] = calculateChecksum(frame, 7);
  
  uint8_t *valPtr = (uint8_t *)&value;
  frame[8] = valPtr[0]; frame[9] = valPtr[1]; frame[10] = valPtr[2]; frame[11] = valPtr[3];
  frame[12] = calculateChecksum(&frame[8], 4);
  
  RadarSerial.write(frame, 13);
  Serial.printf("📡 [CONFIG TX] ส่งคำสั่ง 0x%04X ค่า: %.2f (TX Pin %d)\n", type, value, radar_tx_pin);
}

void sendConfigToRadarInt(uint16_t type, uint32_t value) {
  uint8_t frame[13];
  frame[0] = 0x01; // SOF
  frame[1] = 0x00; frame[2] = 0x00;
  frame[3] = 0x00; frame[4] = 0x04;
  frame[5] = (type >> 8) & 0xFF;
  frame[6] = type & 0xFF;
  frame[7] = calculateChecksum(frame, 7);
  
  frame[8]  = value & 0xFF;
  frame[9]  = (value >> 8) & 0xFF;
  frame[10] = (value >> 16) & 0xFF;
  frame[11] = (value >> 24) & 0xFF;
  frame[12] = calculateChecksum(&frame[8], 4);
  
  RadarSerial.write(frame, 13);
  Serial.printf("📡 [CONFIG TX INT] ส่งคำสั่ง 0x%04X ค่า: %lu (TX Pin %d)\n", type, value, radar_tx_pin);
}

void sendRoomSizeToRadar(float width, float length) {
  float rect_XL = -width; float rect_XR = width;
  float rect_ZF = length; float rect_ZB = -length;
  
  uint8_t frame[25];
  frame[0] = 0x01;
  frame[1] = 0x00; frame[2] = 0x00;
  frame[3] = 0x00; frame[4] = 0x10;
  frame[5] = 0x0E; frame[6] = 0x0C;
  frame[7] = calculateChecksum(frame, 7);
  
  memcpy(&frame[8],  &rect_XL, 4);
  memcpy(&frame[12], &rect_XR, 4);
  memcpy(&frame[16], &rect_ZF, 4);
  memcpy(&frame[20], &rect_ZB, 4);
  frame[24] = calculateChecksum(&frame[8], 16);
  
  RadarSerial.write(frame, 25);
  Serial.printf("📡 [CONFIG TX] กำหนดขนาดห้อง (0x0E0C): กว้าง +/-%.1fm, ยาว +/-%.1fm (TX Pin %d)\n", width, length, radar_tx_pin);
}

void applyAllConfigsToRadar() {
  uint32_t height_cm = (uint32_t)(mount_height * 100.0f);
  uint32_t threshold_cm = (uint32_t)(fall_threshold * 100.0f);

  sendConfigToRadarInt(0x0E04, height_cm);     // ตั้งค่าความสูงติดตั้ง (cm)
  delay(100);
  sendConfigToRadar(0x0E04, mount_height);     // ตั้งค่าความสูงติดตั้ง (m)
  delay(100);

  sendConfigToRadarInt(0x0E08, threshold_cm);  // ตั้งค่าเกณฑ์ตรวจล้ม (cm)
  delay(100);
  sendConfigToRadar(0x0E08, fall_threshold);   // ตั้งค่าเกณฑ์ตรวจล้ม (m)
  delay(100);

  sendRoomSizeToRadar(room_width, room_length);
  delay(100);
  last_radar_ack_msg = "ส่งคำสั่งแล้ว กำลังรอเรดาร์ตอบรับ...";
  addWebLog("📤 [CONFIG] ส่งพารามิเตอร์เข้าเรดาร์ (สูง " + String(mount_height, 1) + "m, เกณฑ์ " + String(fall_threshold, 2) + "m, ขอบเขต " + String(room_width, 1) + "x" + String(room_length, 1) + "m)");
}

// ================= TRIAL & METRIC RECORDING =================
void finishActiveTrial(bool detected_as_fall) {
  if (!active_trial.in_progress) return;

  active_trial.in_progress = false;
  current_mode = MODE_FREE_RUN;

  if (total_recorded_trials < MAX_TRIALS) {
    TrialRecord &rec = trial_history[total_recorded_trials];
    rec.trial_no = total_recorded_trials + 1;
    strncpy(rec.scenario_name, active_trial.name, sizeof(rec.scenario_name) - 1);
    rec.ground_truth_fall = active_trial.ground_truth_fall;
    rec.detected_fall = detected_as_fall;
    rec.detection_latency_ms = active_trial.detection_latency_ms;
    rec.confirm_time_ms = active_trial.measured_confirm_ms;
    rec.recovery_time_ms = active_trial.measured_recovery_ms;

    if (rec.ground_truth_fall && rec.detected_fall) {
      strcpy(rec.result_code, "TP");
    } else if (!rec.ground_truth_fall && !rec.detected_fall) {
      strcpy(rec.result_code, "TN");
    } else if (!rec.ground_truth_fall && rec.detected_fall) {
      strcpy(rec.result_code, "FP");
    } else {
      strcpy(rec.result_code, "FN");
    }

    total_recorded_trials++;

    String latStr = (rec.detection_latency_ms > 0) ? (" | ตรวจจับใน: " + String(rec.detection_latency_ms / 1000.0f, 2) + "s") : "";
    addWebLog("🏁 [TRIAL #" + String(rec.trial_no) + "] " + String(rec.scenario_name) + " -> ผลลัพธ์: " + String(rec.result_code) + " (Truth=" + (rec.ground_truth_fall ? "FALL" : "NORMAL") + ", Radar=" + (rec.detected_fall ? "FALL" : "NORMAL") + ")" + latStr);

    Serial.println(F("\n======================================================================="));
    Serial.printf("🏁 [บันทึกผลรอบที่ #%u] ท่าทดสอบ: %s\n", rec.trial_no, rec.scenario_name);
    Serial.printf("   👉 ความจริง (Ground Truth)  : %s\n", rec.ground_truth_fall ? "FALL (ล้ม)" : "NORMAL (ปกติ)");
    Serial.printf("   👉 เรดาร์ตรวจพบ (Detection)  : %s\n", rec.detected_fall ? "FALL (ตรวจพบล้ม)" : "NORMAL (ไม่พบล้ม)");
    if (rec.detection_latency_ms > 0) {
      Serial.printf("   ⏱️ เวลาตอบสนอง (Latency)    : %.2f วินาที (นับจากตัวแตะพื้น)\n", rec.detection_latency_ms / 1000.0f);
    }
    Serial.printf("   📊 ผลการประเมิน (Outcome)    : %s\n", rec.result_code);
    Serial.println(F("=======================================================================\n"));
  }
}

void startScenarioTrial(const char* name, bool is_fall) {
  if (active_trial.in_progress) {
    finishActiveTrial(active_trial.fall_ever_triggered);
  }

  active_trial.in_progress = true;
  strncpy(active_trial.name, name, sizeof(active_trial.name) - 1);
  active_trial.ground_truth_fall = is_fall;
  active_trial.start_time = millis();
  active_trial.impact_time = 0;
  active_trial.detection_latency_ms = 0;
  active_trial.fall_ever_triggered = false;
  active_trial.measured_confirm_ms = 0;
  active_trial.measured_recovery_ms = 0;
  current_mode = MODE_SCENARIO_TRIAL;

  addWebLog("🎬 [TRIAL #" + String(total_recorded_trials + 1) + "] เริ่มทดสอบ: " + String(active_trial.name) + " (" + (is_fall ? "FALL" : "ADL") + ")");

  Serial.println(F("\n-----------------------------------------------------------------------"));
  Serial.printf("🎬 [เริ่มการทดลองรอบที่ #%u] ท่า: %s (%s)\n", 
                total_recorded_trials + 1, active_trial.name, is_fall ? "FALL" : "ADL");
  Serial.println(F("-----------------------------------------------------------------------"));
}

// ================= WEB DASHBOARD HTML / UI =================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="th">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>HLK-LD6002C Wireless Lab & Config</title>
  <style>
    :root {
      --bg: #0f172a;
      --card: #1e293b;
      --text: #f8fafc;
      --muted: #94a3b8;
      --border: #334155;
      --primary: #38bdf8;
      --success: #22c55e;
      --warning: #f59e0b;
      --danger: #ef4444;
      --indigo: #818cf8;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    body { background: var(--bg); color: var(--text); padding: 12px; line-height: 1.4; }
    .container { max-width: 680px; margin: 0 auto; display: flex; flex-direction: column; gap: 12px; }
    
    .header { text-align: center; padding: 6px 0 10px 0; border-bottom: 1px solid var(--border); }
    .header h1 { font-size: 1.25rem; color: var(--primary); }
    .header p { font-size: 0.8rem; color: var(--muted); }
    
    .card { background: var(--card); border: 1px solid var(--border); border-radius: 12px; padding: 14px; }
    .card-title { font-size: 0.95rem; font-weight: 600; margin-bottom: 10px; display: flex; justify-content: space-between; align-items: center; }
    
    /* Sensor Connection Card */
    .conn-card-online { border-left: 5px solid var(--success) !important; }
    .conn-card-offline { border-left: 5px solid var(--danger) !important; background: #27141a !important; }
    .dot-pulse { width: 11px; height: 11px; border-radius: 50%; display: inline-block; transition: 0.3s; }
    .dot-online { background: var(--success); box-shadow: 0 0 8px var(--success); }
    .dot-offline { background: var(--danger); box-shadow: 0 0 8px var(--danger); animation: pulse 1s infinite; }

    /* Live Status Badge */
    .status-box { padding: 16px; border-radius: 10px; text-align: center; transition: all 0.3s; }
    .status-normal { background: #064e3b; border: 2px solid var(--success); }
    .status-candidate { background: #78350f; border: 2px solid var(--warning); }
    .status-fall { background: #7f1d1d; border: 2px solid var(--danger); animation: pulse 1s infinite; }
    .status-recovering { background: #713f12; border: 2px solid #eab308; }
    @keyframes pulse { 0%, 100% { opacity: 1; } 50% { opacity: 0.75; } }
    .status-text { font-size: 1.45rem; font-weight: bold; margin-bottom: 4px; }
    .status-meta { font-size: 0.8rem; color: #cbd5e1; }

    /* Button Grids */
    .btn-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 8px; margin-top: 6px; }
    .btn { padding: 11px 8px; border: none; border-radius: 8px; font-size: 0.88rem; font-weight: 600; cursor: pointer; transition: 0.15s; text-align: center; display: inline-flex; align-items: center; justify-content: center; }
    .btn:active { transform: scale(0.97); }
    .btn-fall { background: #991b1b; color: white; border: 1px solid #dc2626; }
    .btn-fall:hover { background: #b91c1c; }
    .btn-adl { background: #1e3a8a; color: white; border: 1px solid #2563eb; }
    .btn-adl:hover { background: #1d4ed8; }
    .btn-action { background: #334155; color: white; border: 1px solid #475569; }
    .btn-action:hover { background: #475569; }
    .btn-danger { background: var(--danger); color: white; }
    .btn-success { background: var(--success); color: white; }
    .btn-primary { background: #0284c7; color: white; }
    .btn-warning { background: #d97706; color: white; }
    
    /* Metrics Grid */
    .metrics-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: 6px; text-align: center; }
    .metric-box { background: #0f172a; padding: 7px; border-radius: 6px; border: 1px solid var(--border); }
    .metric-val { font-size: 1.1rem; font-weight: bold; color: var(--primary); }
    .metric-lbl { font-size: 0.68rem; color: var(--muted); }

    /* Table */
    .table-container { max-height: 220px; overflow-y: auto; margin-top: 8px; }
    table { width: 100%; border-collapse: collapse; font-size: 0.78rem; }
    th, td { padding: 6px 8px; text-align: left; border-bottom: 1px solid var(--border); }
    th { background: #0f172a; color: var(--muted); position: sticky; top: 0; }
    .badge { padding: 2px 6px; border-radius: 4px; font-weight: bold; font-size: 0.72rem; }
    .badge-TP { background: #064e3b; color: #34d399; }
    .badge-TN { background: #1e3a8a; color: #60a5fa; }
    .badge-FP { background: #78350f; color: #fbbf24; }
    .badge-FN { background: #7f1d1d; color: #f87171; }

    /* Form Inputs for Sensor Settings */
    .form-group { margin-bottom: 12px; }
    .form-label { display: flex; justify-content: space-between; font-size: 0.82rem; margin-bottom: 4px; color: #e2e8f0; font-weight: 500; }
    .form-tip { font-size: 0.72rem; color: var(--muted); font-weight: normal; }
    .input-row { display: flex; gap: 8px; align-items: center; }
    .form-input { flex: 1; background: #0f172a; border: 1px solid var(--border); color: #fff; padding: 8px 10px; border-radius: 6px; font-size: 0.9rem; }
    .form-input:focus { outline: none; border-color: var(--primary); }
    
    .preset-pills { display: flex; gap: 4px; margin-top: 4px; flex-wrap: wrap; }
    .pill { background: #0f172a; border: 1px solid var(--border); color: var(--primary); padding: 2px 8px; border-radius: 12px; font-size: 0.72rem; cursor: pointer; }
    .pill:hover { background: #1e293b; border-color: var(--primary); }
    
    .toast { display: none; padding: 8px 12px; background: #064e3b; color: #34d399; border: 1px solid var(--success); border-radius: 6px; font-size: 0.82rem; margin-top: 8px; text-align: center; }

    /* Live Terminal Console */
    .terminal-box { background: #070b12; border: 1px solid #1e293b; border-radius: 8px; padding: 10px; height: 260px; overflow-y: auto; font-family: "SFMono-Regular", Consolas, "Liberation Mono", Menlo, monospace; font-size: 0.76rem; line-height: 1.5; color: #cbd5e1; white-space: pre-wrap; word-break: break-all; }
    .log-line { margin-bottom: 2px; padding: 1px 4px; border-radius: 3px; }
    .log-fall { color: #fca5a5; font-weight: bold; background: rgba(239, 68, 68, 0.2); border-left: 3px solid #ef4444; }
    .log-candidate { color: #fde047; font-weight: bold; background: rgba(245, 158, 11, 0.15); border-left: 3px solid #f59e0b; }
    .log-recovery { color: #86efac; font-weight: bold; background: rgba(34, 197, 94, 0.15); border-left: 3px solid #22c55e; }
    .log-hex { color: #7dd3fc; font-size: 0.72rem; opacity: 0.9; }
    .log-ack { color: #d8b4fe; }
    .log-sys { color: #94a3b8; }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>📡 HLK-LD6002C Wireless Lab & Config</h1>
      <p>ระบบทดสอบเรดาร์เพดาน, บันทึกสถิติ & ตั้งค่าพารามิเตอร์ไร้สาย</p>
    </div>

    <!-- 📡 Sensor Connection Health Card -->
    <div class="card conn-card-online" id="sensorConnCard">
      <div style="display:flex; justify-content:space-between; align-items:center; flex-wrap:wrap; gap:6px;">
        <div style="display:flex; align-items:center; gap:8px;">
          <span id="sensorConnDot" class="dot-pulse dot-online"></span>
          <strong id="sensorConnTitle" style="font-size:0.92rem; color:var(--success);">กำลังตรวจสอบการเชื่อมต่อเรดาร์...</strong>
        </div>
        <div style="display:flex; gap:6px;">
          <button onclick="pingSensor()" class="btn btn-action" style="padding:4px 9px; font-size:0.75rem;">🔍 Ping</button>
          <button onclick="swapUartPins()" class="btn btn-warning" style="padding:4px 9px; font-size:0.75rem;">🔀 สลับขา RX/TX</button>
        </div>
      </div>
      <div id="sensorConnMeta" style="font-size:0.78rem; color:var(--muted); margin-top:6px;">
        ความเร็วข้อมูล: กำลังตรวจจับ... | ขาสัญญาณ: RX=GPIO 18, TX=GPIO 17
      </div>
    </div>

    <!-- Live Fall Status Card -->
    <div class="card">
      <div id="statusBox" class="status-box status-normal">
        <div id="statusText" class="status-text">🟢 ท่ายืนปกติ (NORMAL)</div>
        <div id="statusMeta" class="status-meta">กำลังโหลดสถานะ...</div>
      </div>
      <div id="trialProgress" style="display:none; margin-top:10px; font-size:0.85rem; color:var(--warning); text-align:center;">
        <div style="font-size:0.95rem; margin-bottom:6px;">
          ⏳ กำลังทดสอบ: <strong id="activeTrialName"></strong> (<span id="activeTimer">0</span>s)
        </div>
        
        <!-- ปุ่มกดมาร์กจังหวะตัวแตะพื้น (เฉพาะรอบทดสอบท่าล้ม) -->
        <div id="impactArea" style="display:none; margin:10px 0;">
          <button id="impactBtn" onclick="markImpact()" class="btn btn-danger" style="width:100%; padding:12px; font-size:1.05rem; font-weight:bold; box-shadow:0 0 14px rgba(239,68,68,0.7); border:2px solid #fff; animation:pulse 1.2s infinite; cursor:pointer;">
            💥 ล้มลงพื้นแล้ว! (กดเมื่อตัวแตะพื้น)
          </button>
          <div id="impactStatus" style="display:none; padding:8px; background:#450a0a; border-radius:8px; border:1px solid var(--danger); color:#fca5a5; font-weight:bold; font-size:0.92rem;">
            ⏱️ บันทึกเวลาล้มแล้ว! รอเรดาร์ประมวลผล... (<span id="impactElapsed">0.0</span>s)
          </div>
        </div>

        <div style="margin-top:6px;">
          <button onclick="endTrial()" class="btn btn-action" style="padding:4px 12px; font-size:0.8rem;">⏹️ บันทึกจบรอบ / ไม่ตรวจพบ</button>
        </div>
      </div>
    </div>

    <!-- 📏 Live Real Person Height & Sensor Verification Card -->
    <div class="card" style="border-left: 5px solid var(--primary);">
      <div class="card-title">
        <span>📏 ระดับความสูงเป้าหมายสด (Target Height - แสดงเพื่อดีบัค)</span>
        <span id="heightBadge" class="badge" style="background:#0369a1; color:#e0f2fe;">กำลังวัดค่า...</span>
      </div>
      <div style="display:flex; justify-content:space-between; align-items:baseline; flex-wrap:wrap; gap:8px;">
        <div style="font-size:2rem; font-weight:bold; color:var(--primary);" id="personHeightVal">-- ม.</div>
        <div style="font-size:0.82rem; color:var(--muted);" id="radarDepthVal">ระยะจากเพดาน: -- ม.</div>
      </div>
      <div style="font-size:0.85rem; margin-top:4px; font-weight:600;" id="postureDesc">กำลังรอข้อมูล...</div>

      <!-- Visual Height Bar -->
      <div style="margin-top:10px;">
        <div style="background:#070b12; height:16px; border-radius:8px; overflow:hidden; position:relative; border:1px solid var(--border);">
          <div id="heightGaugeBar" style="height:100%; width:0%; background:linear-gradient(90deg, var(--danger) 0%, var(--warning) 30%, var(--success) 70%); transition:width 0.3s; border-radius:8px;"></div>
          <div id="heightThreshMark" style="position:absolute; top:0; bottom:0; width:2px; background:#fff; left:22%;"></div>
        </div>
        <div style="display:flex; justify-content:space-between; font-size:0.72rem; color:var(--muted); margin-top:3px;">
          <span>0.0m (พื้น)</span>
          <span style="color:var(--warning);">▲ เกณฑ์ล้ม (<span id="lblThresh">0.50</span>m)</span>
          <span>เพดาน (<span id="lblMount">2.20</span>m)</span>
        </div>
      </div>

      <!-- Multi-Channel Indicator Row -->
      <div style="display:grid; grid-template-columns:1fr 1fr; gap:6px; margin-top:10px;">
        <div style="background:#0f172a; padding:6px 8px; border-radius:6px; border:1px solid var(--border); font-size:0.75rem;">
          <span style="color:var(--muted);">📡 UART (0x0E02): </span>
          <b id="indUart" style="color:var(--success);">0 (ปกติ)</b>
        </div>
        <div style="background:#0f172a; padding:6px 8px; border-radius:6px; border:1px solid var(--border); font-size:0.75rem;">
          <span style="color:var(--muted);">⚡ ขาดิจิตอล TX2 (GPIO 3): </span>
          <b id="indDigital" style="color:var(--success);">LOW (ปกติ)</b>
        </div>
      </div>
    </div>

    <!-- ⚙️ Radar Sensor Settings Card -->
    <div class="card">
      <div class="card-title">
        <span>⚙️ 1. ปรับตั้งค่าเซนเซอร์เรดาร์ (Sensor Calibration)</span>
        <span id="ackBadge" style="font-size:0.75rem; color:var(--indigo);"></span>
      </div>

      <div class="form-group">
        <div class="form-label">
          <span>ความสูงติดตั้งเรดาร์จากพื้น (Mount Height)</span>
          <span class="form-tip">วัดจากพื้นถึงตัวเรดาร์</span>
        </div>
        <div class="input-row">
          <input type="number" step="0.1" min="1.5" max="3.5" id="cfg_height" class="form-input" placeholder="2.0">
          <span style="color:var(--muted); font-size:0.85rem;">เมตร (m)</span>
        </div>
        <div class="preset-pills">
          <span class="pill" onclick="setVal('cfg_height', 2.0)">2.0m</span>
          <span class="pill" onclick="setVal('cfg_height', 2.2)">2.2m</span>
          <span class="pill" onclick="setVal('cfg_height', 2.4)">2.4m</span>
          <span class="pill" onclick="setVal('cfg_height', 2.6)">2.6m</span>
          <span class="pill" onclick="setVal('cfg_height', 2.8)">2.8m</span>
          <span class="pill" onclick="setVal('cfg_height', 3.0)">3.0m</span>
        </div>
      </div>

      <div class="form-group">
        <div class="form-label">
          <span>เกณฑ์ความสูงตรวจจับการล้ม (Fall Threshold)</span>
          <span class="form-tip">ต่ำกว่านี้ถือว่าล้ม (ค่ามาตรฐาน 0.50m)</span>
        </div>
        <div class="input-row">
          <input type="number" step="0.05" min="0.3" max="1.2" id="cfg_threshold" class="form-input" placeholder="0.5">
          <span style="color:var(--muted); font-size:0.85rem;">เมตร (m)</span>
        </div>
        <div class="preset-pills">
          <span class="pill" onclick="setVal('cfg_threshold', 0.4)">0.40m</span>
          <span class="pill" onclick="setVal('cfg_threshold', 0.5)">0.50m (มาตรฐาน)</span>
          <span class="pill" onclick="setVal('cfg_threshold', 0.6)">0.60m</span>
          <span class="pill" onclick="setVal('cfg_threshold', 0.7)">0.70m</span>
        </div>
      </div>

      <div class="form-group">
        <div class="form-label">
          <span>ขอบเขตห้อง (Room Dimensions: Width x Length)</span>
          <span class="form-tip">จำกัดไม่ให้คลื่นทะลุผนัง</span>
        </div>
        <div class="input-row">
          <input type="number" step="0.5" min="1.0" max="8.0" id="cfg_width" class="form-input" placeholder="กว้าง (m)">
          <span style="color:var(--muted);">x</span>
          <input type="number" step="0.5" min="1.0" max="8.0" id="cfg_length" class="form-input" placeholder="ยาว (m)">
          <span style="color:var(--muted); font-size:0.85rem;">เมตร</span>
        </div>
        <div class="preset-pills">
          <span class="pill" onclick="setVal('cfg_width', 2.5); setVal('cfg_length', 2.5)">2.5 x 2.5m</span>
          <span class="pill" onclick="setVal('cfg_width', 3.0); setVal('cfg_length', 3.0)">3.0 x 3.0m</span>
          <span class="pill" onclick="setVal('cfg_width', 4.0); setVal('cfg_length', 4.0)">4.0 x 4.0m</span>
        </div>
      </div>

      <div class="form-group">
        <div class="form-label">
          <span>เวลาหน่วงยืนยันการล้ม (Fall Delay)</span>
          <span class="form-tip">ต้องล้มค้างนานเกินเวลานี้เพื่อยืนยัน</span>
        </div>
        <div class="input-row">
          <input type="number" step="100" min="500" max="5000" id="cfg_confirm_delay" class="form-input" placeholder="2000">
          <span style="color:var(--muted); font-size:0.85rem;">ms</span>
        </div>
        <div class="preset-pills">
          <span class="pill" onclick="setVal('cfg_confirm_delay', 1500)">1,500ms (1.5s)</span>
          <span class="pill" onclick="setVal('cfg_confirm_delay', 2000)">2,000ms (2.0s มาตรฐาน)</span>
          <span class="pill" onclick="setVal('cfg_confirm_delay', 2500)">2,500ms (2.5s)</span>
          <span class="pill" onclick="setVal('cfg_confirm_delay', 3000)">3,000ms (3.0s)</span>
        </div>
      </div>

      <div class="form-group">
        <div class="form-label">
          <span>เวลาหน่วงยืนยันการลุกขึ้นยืน (Recovery Delay)</span>
          <span class="form-tip">ยืนตัวตรงต่อเนื่องเพื่อคืนสถานะ</span>
        </div>
        <div class="input-row">
          <input type="number" step="100" min="500" max="5000" id="cfg_recovery_delay" class="form-input" placeholder="2500">
          <span style="color:var(--muted); font-size:0.85rem;">ms</span>
        </div>
        <div class="preset-pills">
          <span class="pill" onclick="setVal('cfg_recovery_delay', 2000)">2,000ms (2.0s)</span>
          <span class="pill" onclick="setVal('cfg_recovery_delay', 2500)">2,500ms (2.5s มาตรฐาน)</span>
          <span class="pill" onclick="setVal('cfg_recovery_delay', 3000)">3,000ms (3.0s)</span>
        </div>
      </div>

      <div style="display:flex; gap:8px; margin-top:14px;">
        <button onclick="saveSensorConfig()" class="btn btn-primary" style="flex:2;">💾 บันทึกและส่งเข้าชิปเรดาร์ทันที</button>
        <button onclick="toggleBuzzer()" id="buzzerBtn" class="btn btn-action" style="flex:1;">🔔 ไซเรน</button>
      </div>
      <div id="saveToast" class="toast">✅ ส่งคำสั่งสอบเทียบเข้าเรดาร์ & บันทึกลง Flash เรียบร้อย!</div>
    </div>

    <!-- Scenario Buttons -->
    <div class="card">
      <div class="card-title">
        <span>🚨 2. เลือกท่าล้มจำลอง (Fall Scenarios)</span>
      </div>
      <div class="btn-grid">
        <button onclick="startTrial('Forward Fall (ล้มไปข้างหน้า)', true)" class="btn btn-fall">🚶‍♂️➡️ ล้มไปข้างหน้า</button>
        <button onclick="startTrial('Backward Fall (ล้มหงายหลัง)', true)" class="btn btn-fall">🚶‍♂️⬅️ ล้มหงายหลัง</button>
        <button onclick="startTrial('Lateral Fall (ล้มตะแคงข้าง)', true)" class="btn btn-fall">🚶‍♂️↘️ ล้มตะแคงข้าง</button>
        <button onclick="startTrial('Slow Collapse (ล้มทรุดตัว)', true)" class="btn btn-fall">🚶‍♂️⬇️ ล้มทรุดตัวลง</button>
      </div>

      <div class="card-title" style="margin-top:14px;">
        <span>🚶 3. เลือกท่ากิจกรรมทั่วไป (ADL Scenarios)</span>
      </div>
      <div class="btn-grid">
        <button onclick="startTrial('Bending Down (ก้มเก็บของ)', false)" class="btn btn-adl">📦 ก้มเก็บของ</button>
        <button onclick="startTrial('Crouching (นั่งยองๆ)', false)" class="btn btn-adl">👟 นั่งยองๆ / ผูกเชือก</button>
        <button onclick="startTrial('Sitting (นั่งชักโครก)', false)" class="btn btn-adl">🚽 นั่งชักโครก/เก้าอี้</button>
        <button onclick="startTrial('Walking (เดินไปมา)', false)" class="btn btn-adl">🚶 เดินไปมาในห้อง</button>
      </div>
    </div>

    <!-- Metrics & Confusion Matrix -->
    <div class="card">
      <div class="card-title">
        <span>📊 4. สถิติทางวิชาการ (Evaluation Metrics)</span>
        <span id="trialCount" style="color:var(--primary);">0 รอบ</span>
      </div>
      <div class="metrics-grid">
        <div class="metric-box"><div id="mTP" class="metric-val">0</div><div class="metric-lbl">TP (ล้มถูก)</div></div>
        <div class="metric-box"><div id="mTN" class="metric-val">0</div><div class="metric-lbl">TN (ปกติถูก)</div></div>
        <div class="metric-box"><div id="mFP" class="metric-val">0</div><div class="metric-lbl">FP (เตือนลวง)</div></div>
        <div class="metric-box"><div id="mFN" class="metric-val">0</div><div class="metric-lbl">FN (หลุด)</div></div>
      </div>
      <div class="metrics-grid" style="margin-top:6px;">
        <div class="metric-box"><div id="mAcc" class="metric-val">0%</div><div class="metric-lbl">Accuracy</div></div>
        <div class="metric-box"><div id="mSens" class="metric-val">0%</div><div class="metric-lbl">Sensitivity</div></div>
        <div class="metric-box"><div id="mSpec" class="metric-val">0%</div><div class="metric-lbl">Specificity</div></div>
        <div class="metric-box"><div id="mF1" class="metric-val">0%</div><div class="metric-lbl">F1-Score</div></div>
      </div>
    </div>

    <!-- Trials Table & Download -->
    <div class="card">
      <div class="card-title">
        <span>📋 5. ตารางประวัติการทดลอง (Recorded Trials)</span>
        <div style="display:flex; gap:6px;">
          <a href="/api/csv" class="btn btn-success" style="padding:4px 10px; font-size:0.8rem; text-decoration:none;">📥 ดาวน์โหลด CSV</a>
          <button onclick="resetStats()" class="btn btn-danger" style="padding:4px 8px; font-size:0.8rem;">🔄 ล้าง</button>
        </div>
      </div>
      <div class="table-container">
        <table>
          <thead>
            <tr>
              <th>รอบ</th>
              <th>ท่าทาง</th>
              <th>ความจริง</th>
              <th>เรดาร์</th>
              <th>ตอบสนอง</th>
              <th>หน่วง</th>
              <th>ผล</th>
            </tr>
          </thead>
          <tbody id="trialTableBody">
            <tr><td colspan="7" style="text-align:center; color:var(--muted);">ยังไม่มีข้อมูลการทดลอง</td></tr>
          </tbody>
        </table>
      </div>
    </div>

    <!-- Live Console & Radar Logs -->
    <div class="card" style="margin-bottom:20px;">
      <div class="card-title">
        <div style="display:flex; align-items:center; gap:6px;">
          <span>🖥️ 6. บันทึกและวิเคราะห์สัญญาณสด (Live Radar & System Logs)</span>
          <span id="logCountBadge" style="background:#0f172a; border:1px solid var(--border); padding:1px 6px; border-radius:10px; font-size:0.72rem; color:var(--primary);">0 logs</span>
        </div>
        <div style="display:flex; gap:8px; align-items:center;">
          <label style="font-size:0.75rem; color:var(--muted); cursor:pointer; display:flex; align-items:center; gap:4px;">
            <input type="checkbox" id="chkRawHex" onchange="toggleRawHex()"> Raw HEX
          </label>
          <label style="font-size:0.75rem; color:var(--muted); cursor:pointer; display:flex; align-items:center; gap:4px;">
            <input type="checkbox" id="chkAutoScroll" checked> Auto-Scroll
          </label>
          <button onclick="clearWebLogs()" class="btn btn-action" style="padding:3px 8px; font-size:0.75rem;">🗑️ ล้าง</button>
        </div>
      </div>
      <div id="logTerminal" class="terminal-box">
        <div class="log-line log-sys">กำลังเชื่อมต่อกับระบบ Live Radar Console...</div>
      </div>
    </div>

    <!-- 🚀 Wireless Web OTA Firmware Update Card -->
    <div class="card" style="border: 1px solid #a855f7; margin-bottom:20px;">
      <div class="card-title" style="color:#c084fc;">
        <span>🚀 7. อัปโหลดโค้ดออนไลน์ไร้สาย (Web Browser OTA Update)</span>
        <span class="badge" style="background:rgba(168,85,247,0.2); color:#c084fc;">ไม่ต้องเสียบสาย USB</span>
      </div>
      <p style="font-size:0.8rem; color:var(--muted); margin-bottom:8px;">
        คอมไพล์ใน Arduino IDE (เมนู <b>Sketch ➔ Export Compiled Binary</b> หรือกด <b>Ctrl+Alt+S</b>) แล้วนำไฟล์ <code>.bin</code> มาเลือกและกดอัปโหลดตรงนี้ได้ทันที:
      </p>
      <form id="otaForm" style="display:flex; gap:8px; flex-wrap:wrap; align-items:center;">
        <input type="file" id="otaFileInput" accept=".bin" class="form-input" style="flex:1;">
        <button type="submit" class="btn" style="background:#9333ea; color:#fff; width:auto; padding:8px 16px;">⚡ อัปโหลดไร้สาย</button>
      </form>
      <div id="otaProgBox" style="display:none; margin-top:8px;">
        <div style="background:#070b12; height:10px; border-radius:5px; overflow:hidden; border:1px solid var(--border);">
          <div id="otaProgBar" style="height:100%; width:0%; background:#a855f7; transition:width 0.2s;"></div>
        </div>
        <div id="otaProgTxt" style="font-size:0.75rem; color:var(--muted); margin-top:3px;">กำลังส่งไฟล์เฟิร์มแวร์...</div>
      </div>
    </div>
  </div>

  <script>
    let activeTrialTimer = null;
    let trialStartTime = 0;
    let configLoaded = false;

    // Web OTA AJAX Upload
    window.addEventListener('DOMContentLoaded', () => {
      const otaF = document.getElementById('otaForm');
      if (otaF) {
        otaF.addEventListener('submit', function(e) {
          e.preventDefault();
          const fi = document.getElementById('otaFileInput');
          if (!fi.files.length) { alert('กรุณาเลือกไฟล์ .bin ก่อนครับ'); return; }
          const fd = new FormData();
          fd.append('update', fi.files[0]);
          const xhr = new XMLHttpRequest();
          document.getElementById('otaProgBox').style.display = 'block';
          const bar = document.getElementById('otaProgBar');
          const txt = document.getElementById('otaProgTxt');
          xhr.upload.addEventListener('progress', function(p) {
            if (p.lengthComputable) {
              const pct = Math.round((p.loaded / p.total) * 100);
              bar.style.width = pct + '%';
              txt.innerText = 'กำลังอัปโหลดไร้สาย: ' + pct + '%';
            }
          });
          xhr.onload = function() {
            if (xhr.status === 200) {
              bar.style.background = 'var(--success)';
              txt.innerText = '🎉 อัปโหลดสำเร็จ 100%! บอร์ด ESP32-S3 กำลังรีบูตใน 5 วินาที...';
              setTimeout(() => location.reload(), 6000);
            } else {
              bar.style.background = 'var(--danger)';
              txt.innerText = '❌ ล้มเหลว: ' + xhr.responseText;
            }
          };
          xhr.open('POST', '/update');
          xhr.send(fd);
        });
      }
    });

    function setVal(id, val) {
      document.getElementById(id).value = val;
    }

    function pingSensor() {
      fetch('/api/ping', { method: 'POST' })
        .then(() => {
          document.getElementById('sensorConnMeta').innerText = '🔍 กำลังส่งสัญญาณ Ping ไปยังเรดาร์...';
          setTimeout(updateStatus, 300);
        });
    }

    function swapUartPins() {
      if (confirm('ต้องการสลับขา RX/TX ของ ESP32-S3 (18/17 <-> 17/18) หรือไม่?')) {
        fetch('/api/config/swap-pins', { method: 'POST' })
          .then(r => r.json())
          .then(res => {
            alert('✅ สลับขาเรียบร้อย: RX = GPIO ' + res.rx + ', TX = GPIO ' + res.tx);
            updateStatus();
          });
      }
    }

    function saveSensorConfig() {
      const h = parseFloat(document.getElementById('cfg_height').value);
      const th = parseFloat(document.getElementById('cfg_threshold').value);
      const w = parseFloat(document.getElementById('cfg_width').value);
      const l = parseFloat(document.getElementById('cfg_length').value);
      const cd = parseInt(document.getElementById('cfg_confirm_delay').value);
      const rd = parseInt(document.getElementById('cfg_recovery_delay').value);

      const url = `/api/config/save?height=${h}&threshold=${th}&width=${w}&length=${l}&confirm_delay=${cd}&recovery_delay=${rd}`;
      fetch(url, { method: 'POST' })
        .then(r => r.json())
        .then(res => {
          const t = document.getElementById('saveToast');
          t.style.display = 'block';
          setTimeout(() => { t.style.display = 'none'; }, 3000);
          updateStatus();
        })
        .catch(err => alert('เกิดข้อผิดพลาดในการส่งคำสั่ง: ' + err));
    }

    function startTrial(name, isFall) {
      fetch('/api/trial/start?name=' + encodeURIComponent(name) + '&fall=' + (isFall ? '1' : '0'), { method: 'POST' })
        .then(() => {
          trialStartTime = Date.now();
          updateStatus();
        });
    }

    function endTrial() {
      fetch('/api/trial/end', { method: 'POST' }).then(() => updateStatus());
    }

    function markImpact() {
      fetch('/api/trial/impact', { method: 'POST' }).then(() => updateStatus());
    }

    function resetStats() {
      if (confirm('ต้องการล้างข้อมูลสถิติการทดลองทั้งหมดหรือไม่?')) {
        fetch('/api/reset', { method: 'POST' }).then(() => updateStatus());
      }
    }

    function toggleBuzzer() {
      fetch('/api/config/buzzer', { method: 'POST' }).then(() => updateStatus());
    }

    function updateStatus() {
      fetch('/api/status')
        .then(r => r.json())
        .then(data => {
          // 📡 Update Sensor Connection Card
          const connCard = document.getElementById('sensorConnCard');
          const connDot = document.getElementById('sensorConnDot');
          const connTitle = document.getElementById('sensorConnTitle');
          const connMeta = document.getElementById('sensorConnMeta');

          if (data.sensor_connected) {
            connCard.className = 'card conn-card-online';
            connDot.className = 'dot-pulse dot-online';
            connTitle.innerText = '🟢 เรดาร์ HLK-LD6002C เชื่อมต่อปกติ (ONLINE)';
            connTitle.style.color = 'var(--success)';
            connMeta.innerHTML = `📡 อัตราข้อมูล: <strong>${data.packet_rate} pkt/s</strong> | รับแล้ว: <strong>${data.total_packets.toLocaleString()}</strong> แพ็กเก็ต | ขา ESP32: <strong>RX=${data.rx_pin}, TX=${data.tx_pin}</strong>`;
          } else {
            connCard.className = 'card conn-card-offline';
            connDot.className = 'dot-pulse dot-offline';
            connTitle.innerText = '🔴 ขาดการเชื่อมต่อกับเรดาร์ (OFFLINE / NO DATA)';
            connTitle.style.color = 'var(--danger)';
            connMeta.innerHTML = `⚠️ ไม่ได้รับแพ็กเก็ตเกิน <strong>${data.last_packet_sec > 0 ? data.last_packet_sec + 's' : 'ระยะเวลาที่กำหนด'}</strong>! ตรวจเช็กไฟ 5V หรือกด [🔀 สลับขา RX/TX] (ปัจจุบัน RX=${data.rx_pin}, TX=${data.tx_pin})`;
          }

          // Update Fall Status Card
          const box = document.getElementById('statusBox');
          const txt = document.getElementById('statusText');
          const meta = document.getElementById('statusMeta');
          
          box.className = 'status-box status-' + data.state_class;
          txt.innerText = data.state_title;
          meta.innerText = 'Raw Byte: 0x0' + data.raw_byte + ' | สูง: ' + data.mount_height + 'm | เกณฑ์: ' + data.fall_threshold + 'm | หน่วง: ' + data.confirm_delay + 'ms';

          document.getElementById('ackBadge').innerText = data.last_ack;

          // Fill inputs only once
          if (!configLoaded && data.mount_height) {
            configLoaded = true;
            document.getElementById('cfg_height').value = data.mount_height;
            document.getElementById('cfg_threshold').value = data.fall_threshold;
            document.getElementById('cfg_width').value = data.room_width;
            document.getElementById('cfg_length').value = data.room_length;
            document.getElementById('cfg_confirm_delay').value = data.confirm_delay;
            document.getElementById('cfg_recovery_delay').value = data.recovery_delay;
          }

          // Active trial bar
          const pBar = document.getElementById('trialProgress');
          if (data.in_trial) {
            pBar.style.display = 'block';
            document.getElementById('activeTrialName').innerText = data.trial_name;
            const sec = Math.floor((Date.now() - trialStartTime) / 1000);
            document.getElementById('activeTimer').innerText = sec;

            const impactArea = document.getElementById('impactArea');
            const impactBtn = document.getElementById('impactBtn');
            const impactStatus = document.getElementById('impactStatus');
            const impactElapsed = document.getElementById('impactElapsed');

            if (data.trial_is_fall) {
              impactArea.style.display = 'block';
              if (data.impact_marked) {
                impactBtn.style.display = 'none';
                impactStatus.style.display = 'block';
                impactElapsed.innerText = data.impact_elapsed_sec;
              } else {
                impactBtn.style.display = 'block';
                impactStatus.style.display = 'none';
              }
            } else {
              impactArea.style.display = 'none';
            }
          } else {
            pBar.style.display = 'none';
          }

          // Metrics
          document.getElementById('trialCount').innerText = data.total_trials + ' รอบ';
          document.getElementById('mTP').innerText = data.tp;
          document.getElementById('mTN').innerText = data.tn;
          document.getElementById('mFP').innerText = data.fp;
          document.getElementById('mFN').innerText = data.fn;
          document.getElementById('mAcc').innerText = data.accuracy.toFixed(1) + '%';
          document.getElementById('mSens').innerText = data.sensitivity.toFixed(1) + '%';
          document.getElementById('mSpec').innerText = data.specificity.toFixed(1) + '%';
          document.getElementById('mF1').innerText = data.f1.toFixed(1) + '%';

          document.getElementById('buzzerBtn').innerText = data.buzzer_on ? '🔔 ไซเรน (เปิด)' : '🔕 ไซเรน (ปิด)';

          // 📏 อัปเดตข้อมูลระดับความสูงคนจริงและสถานะช่องทาง
          const pH = data.person_height || 0;
          const rawD = data.raw_distance || 0;
          const pEl = document.getElementById('personHeightVal');
          const dEl = document.getElementById('radarDepthVal');
          const descEl = document.getElementById('postureDesc');
          const hBadge = document.getElementById('heightBadge');
          const gBar = document.getElementById('heightGaugeBar');

          if (rawD > 0.1) {
            pEl.innerText = pH.toFixed(2) + ' ม.';
            dEl.innerText = 'ระยะจากเพดาน: ' + rawD.toFixed(2) + ' ม.';
            const maxH = data.mount_height || 2.2;
            const pct = Math.min(Math.max((pH / maxH) * 100, 0), 100);
            gBar.style.width = pct + '%';

            if (pH <= data.fall_threshold && pH > 0.05) {
              pEl.style.color = 'var(--danger)';
              descEl.innerHTML = '<span style="color:var(--danger)">🔴 ระดับนอนราบ (~' + pH.toFixed(2) + 'm) [แสดงค่าดีบัค]</span>';
              hBadge.innerText = 'ระดับนอนราบ';
              hBadge.style.background = '#991b1b';
            } else if (pH <= 1.20 && pH > 0.45) {
              pEl.style.color = 'var(--warning)';
              descEl.innerHTML = '<span style="color:var(--warning)">🟠 ระดับนั่ง (~' + pH.toFixed(2) + 'm)</span>';
              hBadge.innerText = 'ระดับนั่ง';
              hBadge.style.background = '#9a3412';
            } else {
              pEl.style.color = 'var(--success)';
              descEl.innerHTML = '<span style="color:var(--success)">🟢 ระดับยืน (~' + pH.toFixed(2) + 'm)</span>';
              hBadge.innerText = 'ระดับยืน';
              hBadge.style.background = '#065f46';
            }
          } else {
            pEl.innerText = '-- ม.';
            dEl.innerText = 'ระยะจากเพดาน: -- ม.';
            descEl.innerText = 'กำลังรอข้อมูลความสูงจากเรดาร์...';
            gBar.style.width = '0%';
          }

          if (data.mount_height > 0) {
            const threshPct = (data.fall_threshold / data.mount_height) * 100;
            document.getElementById('heightThreshMark').style.left = threshPct + '%';
            document.getElementById('lblThresh').innerText = data.fall_threshold.toFixed(2);
            document.getElementById('lblMount').innerText = data.mount_height.toFixed(2);
          }

          // สรุปสถานะช่องทางย่อย
          const indUart = document.getElementById('indUart');
          if (data.raw_byte === 1) {
            indUart.innerText = '1 (ล้ม!)';
            indUart.style.color = 'var(--danger)';
          } else {
            indUart.innerText = '0 (ปกติ)';
            indUart.style.color = 'var(--success)';
          }

          const indDig = document.getElementById('indDigital');
          if (data.digital_pin === 1) {
            indDig.innerText = 'HIGH (ล้ม!)';
            indDig.style.color = 'var(--danger)';
          } else {
            indDig.innerText = 'LOW (ปกติ)';
            indDig.style.color = 'var(--success)';
          }

          // Table
          if (data.trials && data.trials.length > 0) {
            let rows = '';
            for (let i = data.trials.length - 1; i >= 0; i--) {
              const t = data.trials[i];
              rows += `<tr>
                <td>#${t.no}</td>
                <td>${t.name}</td>
                <td>${t.truth}</td>
                <td>${t.radar}</td>
                <td style="color:${t.detect_lat > 0 ? 'var(--primary)' : 'var(--muted)'}; font-weight:600;">${t.detect_lat > 0 ? (t.detect_lat / 1000).toFixed(2) + 's' : '-'}</td>
                <td>${t.lat > 0 ? t.lat + 'ms' : '-'}</td>
                <td><span class="badge badge-${t.res}">${t.res}</span></td>
              </tr>`;
            }
            document.getElementById('trialTableBody').innerHTML = rows;
          } else {
            document.getElementById('trialTableBody').innerHTML = '<tr><td colspan="7" style="text-align:center; color:var(--muted);">ยังไม่มีข้อมูลการทดลอง</td></tr>';
          }
        })
        .catch(err => console.log(err));
    }

    setInterval(updateStatus, 400);
    updateStatus();

    let lastLogId = 0;
    function fetchWebLogs() {
      fetch('/api/logs?since=' + lastLogId)
        .then(r => r.json())
        .then(data => {
          if (data.raw_hex !== undefined) {
            document.getElementById('chkRawHex').checked = data.raw_hex;
          }
          if (data.logs && data.logs.length > 0) {
            const term = document.getElementById('logTerminal');
            if (lastLogId === 0) {
              term.innerHTML = '';
            }
            data.logs.forEach(item => {
              const line = document.createElement('div');
              line.className = 'log-line';
              const text = item.msg;
              if (text.includes('ALERT') || text.includes('คนล้มจริง') || text.includes('FALL CONFIRMED') || text.includes('OFFLINE')) {
                line.className += ' log-fall';
              } else if (text.includes('CANDIDATE') || text.includes('แนวโน้มการล้ม') || text.includes('Recovering') || text.includes('ลุกขึ้น')) {
                line.className += ' log-candidate';
              } else if (text.includes('Recovery Confirmed') || text.includes('ปกติ') || text.includes('ONLINE') || text.includes('สำเร็จ')) {
                line.className += ' log-recovery';
              } else if (text.includes('[HEX')) {
                line.className += ' log-hex';
              } else if (text.includes('[ACK]') || text.includes('[CONFIG') || text.includes('[PING]')) {
                line.className += ' log-ack';
              } else {
                line.className += ' log-sys';
              }
              line.textContent = text;
              term.appendChild(line);
              if (item.id > lastLogId) lastLogId = item.id;
            });
            document.getElementById('logCountBadge').innerText = lastLogId + ' logs';
            if (document.getElementById('chkAutoScroll').checked) {
              term.scrollTop = term.scrollHeight;
            }
          }
        })
        .catch(err => console.log(err));
    }

    function toggleRawHex() {
      fetch('/api/config/rawhex', { method: 'POST' })
        .then(r => r.json())
        .then(res => {
          document.getElementById('chkRawHex').checked = res.raw_hex;
        });
    }

    function clearWebLogs() {
      fetch('/api/logs/clear', { method: 'POST' })
        .then(() => {
          lastLogId = 0;
          document.getElementById('logTerminal').innerHTML = '<div class="log-line log-sys">🧹 ล้างหน้าต่าง Log เรียบร้อย</div>';
          document.getElementById('logCountBadge').innerText = '0 logs';
        });
    }

    setInterval(fetchWebLogs, 500);
    fetchWebLogs();

    // 🚀 จัดการอัปโหลดไฟล์ OTA ผ่าน AJAX Form
    const otaForm = document.getElementById('otaForm');
    if (otaForm) {
      otaForm.addEventListener('submit', function(e) {
        e.preventDefault();
        const fileInput = document.getElementById('otaFileInput');
        if (!fileInput.files.length) {
          alert('กรุณาเลือกไฟล์ .bin ก่อนครับ');
          return;
        }
        const file = fileInput.files[0];
        const formData = new FormData();
        formData.append('update', file);

        const xhr = new XMLHttpRequest();
        document.getElementById('otaProgBox').style.display = 'block';
        const bar = document.getElementById('otaProgBar');
        const txt = document.getElementById('otaProgTxt');

        xhr.upload.addEventListener('progress', function(p) {
          if (p.lengthComputable) {
            const percent = Math.round((p.loaded / p.total) * 100);
            bar.style.width = percent + '%';
            txt.textContent = 'กำลังอัปโหลดไร้สาย: ' + percent + '%';
          }
        });

        xhr.onload = function() {
          if (xhr.status === 200) {
            bar.style.background = '#10b981';
            txt.textContent = '🎉 อัปโหลดสำเร็จ 100%! บอร์ด ESP32-S3 กำลังรีบูตใน 5 วินาที...';
            setTimeout(() => location.reload(), 6000);
          } else {
            bar.style.background = '#ef4444';
            txt.textContent = '❌ อัปโหลดล้มเหลว: ' + xhr.responseText;
          }
        };

        xhr.onerror = function() {
          bar.style.background = '#ef4444';
          txt.textContent = '❌ เกิดข้อผิดพลาดในการเชื่อมต่อเครือข่าย';
        };

        xhr.open('POST', '/update');
        xhr.send(formData);
      });
    }
  </script>
</body>
</html>
)rawliteral";

// ================= HTTP REST API HANDLERS =================
void handleRoot() {
  server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void handleStatusApi() {
  unsigned long now = millis();
  uint16_t tp = 0, tn = 0, fp = 0, fn = 0;
  for (int i = 0; i < total_recorded_trials; i++) {
    if (strcmp(trial_history[i].result_code, "TP") == 0) tp++;
    else if (strcmp(trial_history[i].result_code, "TN") == 0) tn++;
    else if (strcmp(trial_history[i].result_code, "FP") == 0) fp++;
    else if (strcmp(trial_history[i].result_code, "FN") == 0) fn++;
  }

  uint16_t n = total_recorded_trials;
  float accuracy = (n > 0) ? (float)(tp + tn) / n * 100.0 : 0.0;
  float sensitivity = ((tp + fn) > 0) ? (float)tp / (tp + fn) * 100.0 : 0.0;
  float specificity = ((tn + fp) > 0) ? (float)tn / (tn + fp) * 100.0 : 0.0;
  float precision = ((tp + fp) > 0) ? (float)tp / (tp + fp) * 100.0 : 0.0;
  float f1 = ((precision + sensitivity) > 0) ? 2.0 * (precision * sensitivity) / (precision + sensitivity) : 0.0;

  bool is_sensor_connected = (last_radar_packet_time > 0 && (now - last_radar_packet_time < 3000));
  float last_packet_sec = (last_radar_packet_time > 0) ? (float)(now - last_radar_packet_time) / 1000.0 : -1.0;

  String json = "{";
  json += "\"sensor_connected\":" + String(is_sensor_connected ? "true" : "false") + ",";
  json += "\"packet_rate\":" + String(packets_per_sec) + ",";
  json += "\"total_packets\":" + String(total_radar_packets) + ",";
  json += "\"last_packet_sec\":" + String(last_packet_sec, 1) + ",";
  json += "\"rx_pin\":" + String(radar_rx_pin) + ",";
  json += "\"tx_pin\":" + String(radar_tx_pin) + ",";

  json += "\"raw_byte\":" + String(last_raw_is_fall == 0x01 ? 1 : 0) + ",";
  json += "\"digital_pin\":" + String(digital_fall_state) + ",";
  json += "\"person_height\":" + String(person_height, 2) + ",";
  json += "\"raw_distance\":" + String(raw_radar_distance, 2) + ",";
  json += "\"buzzer_on\":" + String(enable_buzzer ? "true" : "false") + ",";
  json += "\"mount_height\":" + String(mount_height, 2) + ",";
  json += "\"fall_threshold\":" + String(fall_threshold, 2) + ",";
  json += "\"room_width\":" + String(room_width, 1) + ",";
  json += "\"room_length\":" + String(room_length, 1) + ",";
  json += "\"confirm_delay\":" + String(fall_confirmation_delay) + ",";
  json += "\"recovery_delay\":" + String(fall_recovery_delay) + ",";
  json += "\"last_ack\":\"" + last_radar_ack_msg + "\",";

  switch (current_fall_state) {
    case STATE_NORMAL:
      json += "\"state_class\":\"normal\",\"state_title\":\"🟢 ท่ายืนปกติ (NORMAL)\",";
      break;
    case STATE_FALL_CANDIDATE:
      json += "\"state_class\":\"candidate\",\"state_title\":\"🟠 กำลังตรวจสอบการล้ม (CANDIDATE)\",";
      break;
    case STATE_FALL_CONFIRMED:
      json += "\"state_class\":\"fall\",\"state_title\":\"🚨 ยืนยันคนล้มจริง! (FALL CONFIRMED)\",";
      break;
    case STATE_RECOVERING:
      json += "\"state_class\":\"recovering\",\"state_title\":\"🟡 กำลังตรวจสอบการลุกขึ้น (RECOVERING)\",";
      break;
  }

  json += "\"in_trial\":" + String(active_trial.in_progress ? "true" : "false") + ",";
  json += "\"trial_name\":\"" + String(active_trial.name) + "\",";
  json += "\"trial_is_fall\":" + String(active_trial.ground_truth_fall ? "true" : "false") + ",";
  json += "\"impact_marked\":" + String(active_trial.impact_time > 0 ? "true" : "false") + ",";
  json += "\"impact_elapsed_sec\":\"" + String((active_trial.in_progress && active_trial.impact_time > 0) ? String((millis() - active_trial.impact_time) / 1000.0f, 1) : "0.0") + "\",";
  json += "\"total_trials\":" + String(n) + ",";
  json += "\"tp\":" + String(tp) + ",";
  json += "\"tn\":" + String(tn) + ",";
  json += "\"fp\":" + String(fp) + ",";
  json += "\"fn\":" + String(fn) + ",";
  json += "\"accuracy\":" + String(accuracy, 1) + ",";
  json += "\"sensitivity\":" + String(sensitivity, 1) + ",";
  json += "\"specificity\":" + String(specificity, 1) + ",";
  json += "\"f1\":" + String(f1, 1) + ",";

  json += "\"trials\":[";
  for (int i = 0; i < total_recorded_trials; i++) {
    const TrialRecord &r = trial_history[i];
    json += "{\"no\":" + String(r.trial_no) + ",";
    json += "\"name\":\"" + String(r.scenario_name) + "\",";
    json += "\"truth\":\"" + String(r.ground_truth_fall ? "FALL" : "NORMAL") + "\",";
    json += "\"radar\":\"" + String(r.detected_fall ? "FALL" : "NORMAL") + "\",";
    json += "\"detect_lat\":" + String(r.detection_latency_ms) + ",";
    json += "\"lat\":" + String(r.confirm_time_ms) + ",";
    json += "\"res\":\"" + String(r.result_code) + "\"}";
    if (i < total_recorded_trials - 1) json += ",";
  }
  json += "]}";

  server.send(200, "application/json", json);
}

void handleConfigSave() {
  bool changed = false;

  if (server.hasArg("height")) {
    mount_height = server.arg("height").toFloat();
    changed = true;
  }
  if (server.hasArg("threshold")) {
    fall_threshold = server.arg("threshold").toFloat();
    changed = true;
  }
  if (server.hasArg("width")) {
    room_width = server.arg("width").toFloat();
    changed = true;
  }
  if (server.hasArg("length")) {
    room_length = server.arg("length").toFloat();
    changed = true;
  }
  if (server.hasArg("confirm_delay")) {
    fall_confirmation_delay = server.arg("confirm_delay").toInt();
    changed = true;
  }
  if (server.hasArg("recovery_delay")) {
    fall_recovery_delay = server.arg("recovery_delay").toInt();
    changed = true;
  }

  if (changed) {
    applyAllConfigsToRadar();
    saveSettingsToNVS();
    addWebLog("💾 [CONFIG] ปรับพารามิเตอร์: สูง=" + String(mount_height, 1) + "m, เกณฑ์=" + String(fall_threshold, 2) + "m, หน่วงล้ม=" + String(fall_confirmation_delay) + "ms");
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"บันทึกและส่งคำสั่งเข้าเรดาร์เรียบร้อย\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"ไม่มีพารามิเตอร์ที่ต้องการปรับปรุง\"}");
  }
}

void handleSwapPins() {
  int temp = radar_rx_pin;
  radar_rx_pin = radar_tx_pin;
  radar_tx_pin = temp;
  
  pinMode(radar_rx_pin, INPUT_PULLUP);
  RadarSerial.end();
  delay(50);
  RadarSerial.setRxBufferSize(1024);
  RadarSerial.begin(RADAR_BAUD, SERIAL_8N1, radar_rx_pin, radar_tx_pin);
  RadarSerial.setTimeout(50);
  
  preferences.begin("ld6002c-exp", false);
  preferences.putInt("rx_pin", radar_rx_pin);
  preferences.putInt("tx_pin", radar_tx_pin);
  preferences.end();
  
  addWebLog("🔀 [UART SWAP] สลับขาสำเร็จ -> RX=GPIO " + String(radar_rx_pin) + ", TX=GPIO " + String(radar_tx_pin));
  server.send(200, "application/json", "{\"status\":\"ok\",\"rx\":" + String(radar_rx_pin) + ",\"tx\":" + String(radar_tx_pin) + "}");
}

void handlePingApi() {
  sendConfigToRadar(0x0E04, mount_height);
  last_radar_ack_msg = "ส่งคำสั่ง Ping สอบถามเรดาร์แล้ว...";
  addWebLog("🔍 [PING] ส่งคำสั่งสอบถามสถานะไปยังเรดาร์...");
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleTrialStart() {
  if (server.hasArg("name")) {
    String name = server.arg("name");
    bool isFall = (server.arg("fall") == "1");
    startScenarioTrial(name.c_str(), isFall);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing name");
  }
}

void handleTrialEnd() {
  if (active_trial.in_progress) {
    finishActiveTrial(active_trial.fall_ever_triggered);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(200, "text/plain", "No active trial");
  }
}

void handleTrialImpact() {
  if (active_trial.in_progress) {
    active_trial.impact_time = millis();
    addWebLog("💥 [IMPACT MARKED] บันทึกจังหวะตัวแตะพื้นแล้ว! เริ่มจับเวลาตรวจจับเรดาร์...");
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "No active trial");
  }
}

void handleCsvExport() {
  String csv = "Trial_No,Scenario_Name,Ground_Truth,Radar_Output,Detection_Latency_ms,Confirm_Time_ms,Recovery_Time_ms,Result\n";
  for (int i = 0; i < total_recorded_trials; i++) {
    const TrialRecord &r = trial_history[i];
    csv += String(r.trial_no) + ",\"" + String(r.scenario_name) + "\",";
    csv += String(r.ground_truth_fall ? "FALL" : "NORMAL") + ",";
    csv += String(r.detected_fall ? "FALL" : "NORMAL") + ",";
    csv += String(r.detection_latency_ms) + ",";
    csv += String(r.confirm_time_ms) + "," + String(r.recovery_time_ms) + ",";
    csv += String(r.result_code) + "\n";
  }
  server.sendHeader("Content-Disposition", "attachment; filename=ld6002c_experiment_trials.csv");
  server.send(200, "text/csv; charset=utf-8", csv);
}

void handleResetApi() {
  total_recorded_trials = 0;
  active_trial.in_progress = false;
  current_mode = MODE_FREE_RUN;
  current_fall_state = STATE_NORMAL;
  setBuzzer(false);
  setRGB(0, 150, 0);
  addWebLog("🔄 [RESET] ล้างข้อมูลสถิติและการทดลองทั้งหมดเรียบร้อย");
  server.send(200, "text/plain", "OK");
}

void handleConfigBuzzer() {
  enable_buzzer = !enable_buzzer;
  if (!enable_buzzer) setBuzzer(false);
  preferences.begin("ld6002c-exp", false);
  preferences.putBool("buzzer", enable_buzzer);
  preferences.end();
  addWebLog(enable_buzzer ? "🔔 [BUZZER] เปิดเสียงแจ้งเตือนไซเรน (ON)" : "🔕 [BUZZER] ปิดเสียงแจ้งเตือนไซเรน (OFF)");
  server.send(200, "text/plain", enable_buzzer ? "ON" : "OFF");
}

void handleLogsApi() {
  uint32_t since = 0;
  if (server.hasArg("since")) {
    since = (uint32_t)server.arg("since").toInt();
  }

  String json = "{";
  json += "\"raw_hex\":" + String(enable_raw_hex_dump ? "true" : "false") + ",";
  json += "\"logs\":[";

  int startIndex = (web_log_head - web_log_count + MAX_WEB_LOGS) % MAX_WEB_LOGS;
  bool first = true;

  for (int i = 0; i < web_log_count; i++) {
    int idx = (startIndex + i) % MAX_WEB_LOGS;
    if (web_logs[idx].id > since) {
      if (!first) json += ",";
      first = false;
      
      String escaped = "";
      for (size_t c = 0; c < web_logs[idx].text.length(); c++) {
        char ch = web_logs[idx].text[c];
        if (ch == '\"') escaped += "\\\"";
        else if (ch == '\\') escaped += "\\\\";
        else if (ch == '\r') continue;
        else if (ch == '\n') escaped += " ";
        else escaped += ch;
      }

      json += "{\"id\":" + String(web_logs[idx].id) + ",\"msg\":\"" + escaped + "\"}";
    }
  }

  json += "]}";
  server.send(200, "application/json", json);
}

void handleRawHexToggle() {
  enable_raw_hex_dump = !enable_raw_hex_dump;
  addWebLog(enable_raw_hex_dump ? "🔍 [CONSOLE] เปิดการแสดงผล Raw HEX Packets" : "🔍 [CONSOLE] ปิดการแสดงผล Raw HEX Packets");
  server.send(200, "application/json", "{\"status\":\"ok\",\"raw_hex\":" + String(enable_raw_hex_dump ? "true" : "false") + "}");
}

void handleLogsClear() {
  web_log_count = 0;
  web_log_head = 0;
  addWebLog("🧹 ล้างหน้าต่าง Log เรียบร้อย");
  server.send(200, "text/plain", "OK");
}

// ================= ARDUINO SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH); // ปิดไซเรนเริ่มต้น

  setRGB(0, 150, 0);

  Serial.println(F("\n==================================================================="));
  Serial.println(F("   🚀 HLK-LD6002C (60 GHz) WIRELESS LAB & CONFIG DASHBOARD         "));
  Serial.println(F("==================================================================="));
  
  // 1. โหลดการตั้งค่าพารามิเตอร์ล่าสุดจาก NVS Flash
  loadSettingsFromNVS();

  // 2. เริ่มต้น Wi-Fi แบบคู่ (เชื่อมต่อ Hotspot + ปล่อย Access Point ตรงจากเพดาน)
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, ap_password);
  Serial.print(F("📡 [WIFI AP] ปล่อย Wi-Fi ชื่อ: ")); Serial.println(ap_ssid);
  Serial.print(F("👉 [WIFI AP] เชื่อมต่อแล้วเปิดเว็บบนมือถือ: http://")); Serial.println(WiFi.softAPIP());

  WiFi.begin(station_ssid, station_password);
  Serial.print(F("📶 [WIFI STA] กำลังเชื่อมต่อเครือข่าย: ")); Serial.println(station_ssid);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 15) {
    delay(200);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("✅ [WIFI STA] เชื่อมต่อสำเร็จ! เข้าเว็บผ่าน IP เครือข่าย: http://"));
    Serial.println(WiFi.localIP());
  }

  // 3. เริ่มต้น mDNS (เข้าผ่าน http://esp32-radar.local ได้)
  if (MDNS.begin("esp32-radar")) {
    Serial.println(F("🌐 [mDNS] ใช้งานผ่านลิงก์: http://esp32-radar.local"));
  }

  // 4. เริ่มต้น ArduinoOTA (อัปเดตโค้ดไร้สายได้เลย)
  ArduinoOTA.setHostname("esp32s3-ld6002c-exp");
  ArduinoOTA.begin();

  pinMode(RADAR_DIGITAL_PIN, INPUT); // ขา TX2 ของเรดาร์ (GPIO 3)

  // 5. ติดตั้งเส้นทาง Web Server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatusApi);
  server.on("/api/config/save", HTTP_POST, handleConfigSave);
  server.on("/api/config/swap-pins", HTTP_POST, handleSwapPins);
  server.on("/api/ping", HTTP_POST, handlePingApi);
  server.on("/api/trial/start", HTTP_POST, handleTrialStart);
  server.on("/api/trial/end", HTTP_POST, handleTrialEnd);
  server.on("/api/trial/impact", HTTP_POST, handleTrialImpact);
  server.on("/api/csv", HTTP_GET, handleCsvExport);
  server.on("/api/reset", HTTP_POST, handleResetApi);
  server.on("/api/config/buzzer", HTTP_POST, handleConfigBuzzer);
  server.on("/api/logs", HTTP_GET, handleLogsApi);
  server.on("/api/config/rawhex", HTTP_POST, handleRawHexToggle);
  server.on("/api/logs/clear", HTTP_POST, handleLogsClear);

  // Web Browser OTA Handler
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    delay(500);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update File: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("Update Success: %u bytes\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });

  server.begin();
  Serial.println(F("🌐 [WEB SERVER] พร้อมให้บริการเรียบร้อย!"));

  addWebLog("🚀 บอร์ดพร้อมทำงาน (ESP32-S3 Wireless Lab)");
  addWebLog("📡 Wi-Fi AP: " + String(ap_ssid) + " -> http://" + WiFi.softAPIP().toString());
  if (WiFi.status() == WL_CONNECTED) {
    addWebLog("📶 Wi-Fi STA: " + String(station_ssid) + " -> http://" + WiFi.localIP().toString());
  }
  addWebLog("🔌 เรดาร์ UART เริ่มต้นที่ RX=" + String(radar_rx_pin) + ", TX=" + String(radar_tx_pin) + " | Digital=GPIO " + String(RADAR_DIGITAL_PIN));

  // 6. เริ่มต้นเรดาร์ HLK-LD6002C
  pinMode(radar_rx_pin, INPUT_PULLUP);
  RadarSerial.setRxBufferSize(1024);
  RadarSerial.begin(RADAR_BAUD, SERIAL_8N1, radar_rx_pin, radar_tx_pin);
  RadarSerial.setTimeout(50);

  // ส่งค่าคอนฟิกเริ่มต้น (สูง 2.2m, ขนาด 1.5x1.5m)
  delay(1500);
  applyAllConfigsToRadar();
  addWebLog("🟢 ระบบพร้อมเริ่มทดลองและมอนิเตอร์บนเว็บได้ทันที!");
  Serial.println(F("🟢 ระบบพร้อมเริ่มทดลองและปรับแต่งค่าบนมือถือได้ทันที!"));
}

// ================= FALL DETECTION EVALUATION (UART 0x0E02 & DIGITAL TX2) =================
void updateFallEvaluation(unsigned long now) {
  // ตรวจพบล้มจากสัญญาณเรดาร์โดยตรง:
  // 1. เรดาร์ส่งเฟรมตรวจพบล้มผ่าน UART (last_raw_is_fall == 0x01)
  // หรือ 2. ขาดิจิตอลตรง TX2 (GPIO 3) ส่ง HIGH (3.3V)
  // (ระดับความสูงเป้าหมาย 0x0E0E แสดงผลและบันทึกเพื่อการดีบัคเท่านั้น ไม่นำมาตัดสินการล้ม)
  bool fall_detected = (last_raw_is_fall == 0x01) || (digital_fall_state == HIGH);

  if (fall_detected) {
    recovery_start_time = 0;

    if (current_fall_state == STATE_NORMAL) {
      current_fall_state = STATE_FALL_CANDIDATE;
      fall_candidate_start_time = now;
      setRGB(255, 120, 0); // สีส้ม

      // คำนวณ Detection Latency ถ้าผู้ทดลองกดปุ่มมาร์กเวลาล้มลงพื้นไว้
      if (active_trial.in_progress && active_trial.impact_time > 0 && active_trial.detection_latency_ms == 0) {
        active_trial.detection_latency_ms = (now >= active_trial.impact_time) ? (now - active_trial.impact_time) : 0;
        addWebLog("⏱️ [DETECTION LATENCY] เรดาร์เริ่มตรวจพบล้มใน " + String(active_trial.detection_latency_ms / 1000.0f, 2) + " วินาที (หลังตัวแตะพื้น)");
      }

      addWebLog("⚠️ [ตรวจพบการล้ม] UART=" + String(last_raw_is_fall) + " | TX2=" + String(digital_fall_state == HIGH ? "HIGH" : "LOW") + " | รอหน่วงยืนยัน " + String(fall_confirmation_delay) + "ms (สูง: " + String(person_height, 2) + "m)");
    } 
    else if (current_fall_state == STATE_FALL_CANDIDATE) {
      if (now - fall_candidate_start_time >= fall_confirmation_delay) {
        current_fall_state = STATE_FALL_CONFIRMED;
        setRGB(255, 0, 0); // สีแดง
        setBuzzer(true);

        last_confirmed_latency = now - fall_candidate_start_time;

        if (active_trial.in_progress && active_trial.impact_time > 0 && active_trial.detection_latency_ms == 0) {
          active_trial.detection_latency_ms = (now >= active_trial.impact_time) ? (now - active_trial.impact_time) : 0;
        }

        String latInfo = (active_trial.detection_latency_ms > 0) ? (" | ความเร็วตรวจจับ: " + String(active_trial.detection_latency_ms / 1000.0f, 2) + "s") : "";
        addWebLog("🚨🚨🚨 [ALERT CONFIRMED] ยืนยันคนล้มจริง! (UART=" + String(last_raw_is_fall) + ", TX2=" + String(digital_fall_state == HIGH ? "HIGH" : "LOW") + ")" + latInfo + " | หน่วงเวลา: " + String(last_confirmed_latency) + "ms | สูง: " + String(person_height, 2) + "m");

        if (active_trial.in_progress) {
          active_trial.fall_ever_triggered = true;
          active_trial.measured_confirm_ms = last_confirmed_latency;

          if (active_trial.ground_truth_fall) {
            finishActiveTrial(true);
          }
        }
      }
    }
  } 
  else {
    // สภาวะปกติ: เมื่อสัญญาณเรดาร์ทั้ง UART เป็น 0 และขาดิจิตอลกลับเป็น LOW
    bool is_normal = (last_raw_is_fall == 0x00 && digital_fall_state == LOW);

    if (is_normal) {
      if (current_fall_state == STATE_FALL_CANDIDATE) {
        current_fall_state = STATE_NORMAL;
        setRGB(0, 150, 0); // คืนสีเขียว
        addWebLog("🟢 [RADAR] ยกเลิกการเตือน (สัญญาณเรดาร์กลับสู่ปกติ UART=0, TX2=LOW)");
      }
      else if (current_fall_state == STATE_FALL_CONFIRMED) {
        current_fall_state = STATE_RECOVERING;
        recovery_start_time = now;
        setRGB(255, 255, 0); // สีเหลือง
        addWebLog("🟡 [RADAR] สัญญาณล้มหายไป (Recovering) | รอหน่วงยืนยัน " + String(fall_recovery_delay) + "ms");
      }
      else if (current_fall_state == STATE_RECOVERING) {
        if (now - recovery_start_time >= fall_recovery_delay) {
          current_fall_state = STATE_NORMAL;
          setRGB(0, 150, 0); // คืนสีเขียว
          setBuzzer(false);

          last_recovery_latency = now - recovery_start_time;
          addWebLog("🟢 [RADAR] ผู้ใช้งานลุกขึ้นยืนสำเร็จ (Recovery Confirmed) | ใช้เวลา: " + String(last_recovery_latency) + "ms");

          if (active_trial.in_progress && active_trial.ground_truth_fall) {
            active_trial.measured_recovery_ms = last_recovery_latency;
          }
        }
      }
      else if (current_fall_state == STATE_NORMAL) {
        static unsigned long lastNormalLog = 0;
        if (now - lastNormalLog >= 5000) {
          lastNormalLog = now;
          addWebLog("🟢 [STATUS] สภาวะปกติ (UART=0, TX2=LOW | สูง: " + String(person_height, 2) + "m)");
        }
      }
    }
  }
}

// ================= ARDUINO LOOP =================
void loop() {
  server.handleClient();
  ArduinoOTA.handle();

  unsigned long now = millis();

  // คำนวณความเร็วแพ็กเก็ต (Packets per second) ทุกๆ 1 วินาที
  if (now - last_fps_calc_time >= 1000) {
    last_fps_calc_time = now;
    packets_per_sec = packet_counter_current_sec;
    packet_counter_current_sec = 0;
  }

  // เฝ้าระวังการเชื่อมต่อเรดาร์ (Watchdog Status)
  static bool was_sensor_connected = false;
  bool is_sensor_connected = (last_radar_packet_time > 0 && (now - last_radar_packet_time < 3000));
  if (is_sensor_connected && !was_sensor_connected) {
    was_sensor_connected = true;
    addWebLog("🟢 [WATCHDOG] เรดาร์เชื่อมต่อสำเร็จ! กำลังรับข้อมูล (RX=" + String(radar_rx_pin) + ", TX=" + String(radar_tx_pin) + ")");
  } else if (!is_sensor_connected && was_sensor_connected && now > 5000) {
    was_sensor_connected = false;
    addWebLog("🔴 [WATCHDOG] ขาดการเชื่อมต่อกับเรดาร์เกิน 3 วินาที (OFFLINE)");
  }

  // 1. อ่านสัญญาณขาดิจิตอลตรงจากเรดาร์ (ขา TX2 / GPIO 3)
  static int prev_dig_state = -1;
  digital_fall_state = digitalRead(RADAR_DIGITAL_PIN);
  if (digital_fall_state != prev_dig_state) {
    prev_dig_state = digital_fall_state;
    if (digital_fall_state == HIGH) {
      addWebLog("⚡ [TX2 DIGITAL] เรดาร์ส่งสัญญาณ HIGH (3.3V) -> ตรวจพบล้ม!");
    } else {
      addWebLog("⚡ [TX2 DIGITAL] เรดาร์กลับสู่ LOW (0V) -> ปกติ");
    }
  }

  // ตรวจสอบ ADL Timeout
  if (active_trial.in_progress && !active_trial.ground_truth_fall) {
    if (now - active_trial.start_time >= adl_trial_timeout) {
      finishActiveTrial(active_trial.fall_ever_triggered);
    }
  }

  // อ่านค่าจากเซนเซอร์เรดาร์ HLK-LD6002C
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

      // === บันทึกสถิติแพ็กเก็ต (Watchdog Heartbeat) ===
      last_radar_packet_time = now;
      total_radar_packets++;
      packet_counter_current_sec++;

      // แสดงผล Raw HEX Dump หากเปิดใช้งาน
      if (enable_raw_hex_dump) {
        static unsigned long last_hex_dump = 0;
        bool isImportant = (msgType == 0x0E02 || msgType == 0x0E04 || msgType == 0x0E08 || msgType == 0x0E0C);
        if (isImportant || (now - last_hex_dump >= 500)) {
          last_hex_dump = now;
          String hexStr = "📦 [HEX 0x" + String(msgType, HEX) + "] 01 ";
          for (int h = 1; h < 8; h++) {
            if (header[h] < 0x10) hexStr += "0";
            hexStr += String(header[h], HEX) + " ";
          }
          int printLen = min((int)dataLen, 8);
          for (int d = 0; d < printLen; d++) {
            if (payload[d] < 0x10) hexStr += "0";
            hexStr += String(payload[d], HEX) + " ";
          }
          if (dataLen > 8) hexStr += "..";
          hexStr.toUpperCase();
          addWebLog(hexStr);
        }
      }

      // ================= 2. วิเคราะห์ข้อมูล =================
      // ก. เฟรมตรวจจับการล้ม (Type 0x0E02)
      if (msgType == 0x0E02) {
        static uint8_t prev_raw_is_fall = 0xFF;
        last_raw_is_fall = payload[0]; // 0x01 = ล้ม, 0x00 = ปกติ
        if (last_raw_is_fall != prev_raw_is_fall) {
          prev_raw_is_fall = last_raw_is_fall;
          addWebLog("📡 [UART 0x0E02] สัญญาณจากเรดาร์: " + String(last_raw_is_fall == 0x01 ? "1 (DETECT FALL 🚨)" : "0 (NORMAL 🟢)"));
        }
      }
      // ข. เฟรมตอบรับการตั้งค่า (ACK: 0x0E04, 0x0E08, 0x0E0C)
      else if (msgType == 0x0E04 || msgType == 0x0E08 || msgType == 0x0E0C) {
        last_ack_type = msgType;
        last_radar_ack_msg = "ตอบรับคำสั่ง: 0x" + String(msgType, HEX) + " (" + String(now / 1000) + "s)";
        addWebLog("📥 [RADAR ACK] ตอบรับคำสั่ง Config: 0x" + String(msgType, HEX));
        Serial.printf("📥 [RADAR ACK] ตอบรับคำสั่ง Config: 0x%04X\n", msgType);
      }
      // ค. เฟรมตรวจพบคนอยู่ในห้อง (0x0F09)
      else if (msgType == 0x0F09) {
        static unsigned long lastOccupiedLog = 0;
        static uint8_t prev_occupied = 0xFF;
        uint8_t occupied = payload[0];
        if (occupied != prev_occupied || (now - lastOccupiedLog >= 10000)) {
          prev_occupied = occupied;
          lastOccupiedLog = now;
          addWebLog(occupied == 0x01 ? "👤 [PRESENCE] ตรวจพบคนอยู่ในห้อง" : "🚪 [PRESENCE] ห้องว่าง (ไม่มีคน)");
        }
      }
      // ง. เฟรมความสูงเป้าหมาย (0x0E0E)
      else if (msgType == 0x0E0E && dataLen == 4) {
        float dist;
        memcpy(&dist, payload, 4);
        if (dist >= 0.1f && dist <= 5.0f) {
          raw_radar_distance = dist;
          // คำนวณความสูงจริงของคนจากพื้น: ความสูงติดตั้ง (2.2m) - ระยะจากเพดาน (เช่น ยืน 0.64m -> คนสูง 1.56m)
          person_height = max(0.0f, mount_height - dist);

          static unsigned long lastHeightLog = 0;
          static float lastLoggedH = -1.0f;
          if ((now - lastHeightLog >= 2500) || (abs(person_height - lastLoggedH) >= 0.25f)) {
            lastHeightLog = now;
            lastLoggedH = person_height;
            String posture = "ยืนปกติ";
            if (person_height <= fall_threshold && person_height > 0.05f) posture = "นอนราบ (ล้ม!)";
            else if (person_height <= 1.20f && person_height > 0.45f) posture = "นั่ง";
            addWebLog("📏 [TARGET] ความสูงคน: " + String(person_height, 2) + "m | ระยะเพดาน: " + String(dist, 2) + "m (" + posture + ")");
          }
        }
      }
    }
  }

  // ประเมินผลสถานะการล้ม (Multi-Source Sensor Fusion) ทุกรอบ loop อย่างต่อเนื่อง
  updateFallEvaluation(now);

  // ไฟกะพริบสีแดงขณะยืนยันคนล้ม (หรือกระพริบสีฟ้าช้าๆ หากเรดาร์หลุดการติดต่อ)
  if (current_fall_state == STATE_FALL_CONFIRMED) {
    static unsigned long last_flash = 0;
    static bool flash = false;
    if (now - last_flash >= 250) {
      last_flash = now;
      flash = !flash;
      if (flash) setRGB(255, 0, 0);
      else setRGB(0, 0, 0);
    }
  } else if (!is_sensor_connected && now > 5000) {
    // แจ้งเตือนผ่านไฟ RGB บนบอร์ด: ถ้าหลุดการติดต่อกับเรดาร์ ให้กระพริบสีน้ำเงินเตือน
    static unsigned long last_offline_flash = 0;
    static bool offline_flash = false;
    if (now - last_offline_flash >= 600) {
      last_offline_flash = now;
      offline_flash = !offline_flash;
      if (offline_flash) setRGB(0, 0, 180);
      else setRGB(0, 0, 0);
    }
  }
}
