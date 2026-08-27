#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h> // ต้องติดตั้ง Library "PubSubClient" โดย Nick O'Leary ใน Arduino IDE ก่อนอัปโหลดนะคะ
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiClientSecure.h> // รองรับการส่งข้อมูลแบบ HTTPS (SSL) ไปยังเซิร์ฟเวอร์ Azure
#include <WebServer.h>
#include <ElegantOTA.h>
#include <Preferences.h>

Preferences preferences;

// ================= WEB LOG SERVER CONFIGURATION =================
#include <WebSocketsServer.h>
WebServer webServer(80);
WebSocketsServer webSocket(81); // WebSocket ทำงานที่พอร์ต 81

class WebLogger : public Print {
private:
  String lineBuffer = "";
  String recentLogs = "";  // เก็บล็อกสไลด์ล่าสุดใน RAM
  String startupLogs = ""; // เก็บล็อกช่วงเริ่มต้นระบบ (setup) แบบถาวร
  bool isStartupDone = false;
public:
  void begin(unsigned long baud) {
    Serial0.begin(baud);
  }
  void begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin) {
    Serial0.begin(baud, config, rxPin, txPin);
  }
  
  void markStartupDone() {
    isStartupDone = true;
  }
  
  void sanitizeUTF8(String &s) {
    // ตัดไบต์ส่วนขยายของ UTF-8 (0x80 - 0xBF) ที่ขาดออกจากไบต์นำหน้าที่หัวสตริงทิ้ง เพื่อไม่ให้การตัดสตริงทำให้ UTF-8 เสียหาย
    while (s.length() > 0 && ((uint8_t)s[0] & 0xC0) == 0x80) {
      s = s.substring(1);
    }
  }

  String getStartupLogs() {
    String logs = startupLogs;
    sanitizeUTF8(logs);
    return logs;
  }
  
  String getRecentLogs() {
    String logs = recentLogs;
    sanitizeUTF8(logs);
    return logs;
  }
  
  String getTimestamp() {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 0)) { // ตั้งค่า Timeout = 0ms เพื่อไม่ให้บอร์ดเกิดการบล็อคค้าง
      char timeString[15];
      strftime(timeString, sizeof(timeString), "[%H:%M:%S] ", &timeinfo);
      return String(timeString);
    } else {
      // หากเวลายังไม่พร้อมซิงก์ คืนค่า Uptime สั้นๆ แทน (เช่น [+3.40s])
      char uptimeString[15];
      snprintf(uptimeString, sizeof(uptimeString), "[+%.2fs] ", millis() / 1000.0);
      return String(uptimeString);
    }
  }
  
  size_t write(uint8_t c) override {
    Serial0.write(c);
    
    // เติม Timestamp ในบัฟเฟอร์เมื่อเริ่มแถวใหม่
    if (lineBuffer.length() == 0) {
      lineBuffer += getTimestamp();
    }
    
    lineBuffer += (char)c;
    
    if (c == '\n' || lineBuffer.length() >= 1024) {
      sanitizeUTF8(lineBuffer);
      webSocket.broadcastTXT(lineBuffer);
      
      // บันทึกสะสมล็อกแบบเป็นบรรทัด ช่วยถนอม RAM และลดปัญหาการจองพื้นที่หน่วยความจำซ้ำๆ (Fragmentation)
      recentLogs += lineBuffer;
      if (recentLogs.length() > 2000) {
        recentLogs = recentLogs.substring(recentLogs.length() - 1200);
        sanitizeUTF8(recentLogs);
      }
      
      if (!isStartupDone) {
        startupLogs += lineBuffer;
        if (startupLogs.length() > 4000) {
          startupLogs = startupLogs.substring(startupLogs.length() - 3200);
          sanitizeUTF8(startupLogs);
        }
      }
      
      lineBuffer = "";
    }
    return 1;
  }
  
  size_t write(const uint8_t *buffer, size_t size) override {
    for (size_t i = 0; i < size; i++) {
      write(buffer[i]);
    }
    return size;
  }
};
WebLogger WebLog;
#define Serial WebLog

// ================= USER CONFIGURATION =================
const char* ssid = "PINKU 8541";
const char* password = "468Bt9@2";

// ไอดีอุปกรณ์ (ต้องตรงกับรหัสที่ตั้งไว้ในระบบ/ฐานข้อมูลหลังบ้าน เช่น LD6002C_TEST_01)
const char* device_id = "LD6002C_TEST_01";

// วิธีส่งข้อมูล: เลือกใช้งาน true = MQTT, false = HTTP POST
// 💡 แนะนำ: ตั้งค่าเป็น false เพื่อใช้ HTTP POST ก่อน เนื่องจาก Azure VM มักปิดพอร์ต MQTT (1883) ไว้
#define USE_MQTT true

// --- การตั้งค่าสำหรับวิธีที่ 1: HTTPS POST (พอร์ต 443) ---
const char* backend_url = "https://fall-detection.sopon-project.me/api/v1/events"; 

// --- การตั้งค่าสำหรับวิธีที่ 2: MQTT Broker ---
const char* mqtt_broker = "fall-detection.sopon-project.me"; 
const int mqtt_port = 1883;
const char* mqtt_topic = "sensor/events";

// --- การตั้งค่าสำหรับเซนเซอร์ HLK-LD6002C (mmWave Radar - ตรวจจับการล้ม) ---
#define USE_LD6002C true      // เปิดใช้งานการอ่านค่าจากเซนเซอร์ HLK-LD6002C
// #define USE_LD6002C false      // เปิดใช้งานการอ่านค่าจากเซนเซอร์ HLK-LD6002C
#define RADAR_RX_PIN 18       // ขา RX ของ ESP32 (ต่อกับ TX0 ของเรดาร์)
#define RADAR_TX_PIN 17       // ขา TX ของ ESP32 (ต่อกับ RX0 ของเรดาร์)

// --- การตั้งค่าสำหรับเซนเซอร์ HLK-LD2410C (mmWave Radar - ตรวจจับคนเข้า-ออก) ---
#define USE_LD2410C true      // เปิดใช้งานการตรวจจับคนเข้า-ออกด้วยเซนเซอร์ HLK-LD2410C (ผ่านขา OUT)
#define LD2410_OUT_PIN 16     // ขา OUT ของ HLK-LD2410C (ต่อกับ GPIO 16 ของ ESP32-S3)
const unsigned long presence_timeout = 5000; // เวลาหน่วงกรณีไม่พบการเคลื่อนไหว (มิลลิวินาที) ก่อนปรับเป็นสถานะห้องว่าง (exit) (ปรับเป็น 10 วินาทีเพื่อความเสถียรในการทดสอบ)

// ================= HARDWARE CONFIGURATION =================
#define BUZZER_PIN 15                     // ขาพินควบคุม Buzzer (ต่อเข้าขา IN ของ Relay/MOSFET)

// กำหนดขา Pin สำหรับต่อปุ่มกดแบบจำลองสถานะ (ต่อขากลางลง GND และอีกขาเข้ากับ Pin บอร์ด)
const int BUTTON_ENTER_PIN = 4;   // ปุ่มกดเพื่อส่งสถานะ: "กำลังใช้งาน" (enter)
const int BUTTON_FALL_PIN  = 5;   // ปุ่มกดเพื่อส่งสถานะ: "คนล้ม!!! ⚠️" (fall)
const int BUTTON_EXIT_PIN  = 1;  // ปุ่มกดเพื่อส่งสถานะ: "ว่าง/ออกห้อง" (exit)

// ขาสำหรับไฟ RGB LED บนบอร์ด ESP32-S3 DevKitC-1
#define RGB_LED_PIN 48
#define RGB_BRIGHTNESS 0.15 // ปรับความสว่างของไฟ RGB (0.00 ถึง 1.00 โดย 0.15 คือสว่าง 15%)

// ================= STATE VARIABLES =================
String current_event = "exit"; // เริ่มต้นด้วยสถานะว่าง: "exit"
unsigned long last_flash_time = 0;
bool flash_state = false;

// ตัวแปรควบคุมการเชื่อมต่อ MQTT แบบไม่บล็อกการทำงาน (Non-blocking)
unsigned long last_mqtt_reconnect_attempt = 0; 

// ตัวแปรควบคุมเซนเซอร์ HLK-LD2410C
unsigned long last_presence_time = 0; // จับเวลาล่าสุดที่ตรวจพบคนในห้อง
bool fall_alerted_this_session = false; // ตัวแปรป้องกันการแจ้งเตือนล้มซ้ำในรอบการใช้งานเดียวกัน (ล้มแล้วเตือนรอบเดียวจนกว่าคนจะออกจากห้อง)

// ตัวแปรและค่าพารามิเตอร์ระบบยืนยันการล้มด้วยเวลา (Fall Time Confirmation)
const unsigned long fall_confirmation_delay = 2000; // ระยะเวลาที่ต้องตรวจพบล้มค้างเพื่อยืนยันล้มจริง (2 วินาที) ป้องกัน Noise หลอก
unsigned long fall_start_time = 0;                  // จับเวลาวินาทีที่เริ่มมีแนวโน้มการล้ม
bool is_falling_candidate = false;                  // บอกว่าตรวจพบสัญญาณล้มชั่วคราวและกำลังอยู่ในช่วงนับถอยหลังยืนยัน

const unsigned long fall_recovery_delay = 2500;     // ระยะเวลาที่ต้องยืนตัวตรงเพื่อยืนยันว่าลุกขึ้นแล้ว (2.5 วินาที)
unsigned long fall_recovery_start_time = 0;
bool is_recovering_candidate = false;

// ตัวแปรสำหรับการดึงข้อมูลการตั้งค่าจากเว็บ
float fall_threshold = 0.5;   // เกณฑ์การตรวจจับการล้ม
float mount_height = 2.0;     // ความสูงติดตั้งเซนเซอร์ (เมตร)
float room_width = 4.0;       // ความกว้างห้องน้ำ (เมตร)
float room_length = 4.0;      // ความยาวห้องน้ำ (เมตร)

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// พอร์ต Serial สำหรับเชื่อมต่อกับเซนเซอร์เรดาร์ HLK-LD6002C
HardwareSerial RadarSerial(1);
unsigned long last_radar_log_time = 0; // ตัวแปรสำหรับจับเวลาการปริ้นล็อกตรวจเช็กเรดาร์
unsigned long last_height_log_time = 0; // ตัวแปรสำหรับสกัดเวลาการปริ้นล็อกข้อมูลความสูง

// ================= DIAGNOSTIC HELPERS =================
String getWiFiStatusString(wl_status_t status) {
  switch (status) {
    case WL_NO_SHIELD:        return "WL_NO_SHIELD (ไม่มีโมดูล WiFi ติดตั้งอยู่)";
    case WL_IDLE_STATUS:      return "WL_IDLE_STATUS (WiFi อยู่ในสถานะเตรียมพร้อม)";
    case WL_NO_SSID_AVAIL:    return "WL_NO_SSID_AVAIL (ไม่พบชื่อ WiFi (SSID) ที่ระบุ / สัญญาณไม่ถึง)";
    case WL_SCAN_COMPLETED:   return "WL_SCAN_COMPLETED (สแกนหาเครือข่ายเสร็จสิ้น)";
    case WL_CONNECTED:        return "WL_CONNECTED (เชื่อมต่อสำเร็จเรียบร้อย!)";
    case WL_CONNECT_FAILED:   return "WL_CONNECT_FAILED (รหัสผ่าน WiFi ผิดพลาด / เชื่อมต่อไม่สำเร็จ)";
    case WL_CONNECTION_LOST:  return "WL_CONNECTION_LOST (หลุดการเชื่อมต่อระหว่างการทำงาน)";
    case WL_DISCONNECTED:     return "WL_DISCONNECTED (ไม่ได้เชื่อมต่อ / กำลังอยู่ในโหมดค้นหา)";
    default:                  return "UNKNOWN WIFI STATUS (ไม่ทราบรหัสสถานะ)";
  }
}

String getMQTTErrorString(int rc) {
  switch (rc) {
    case -4: return "-4 MQTT_CONNECTION_TIMEOUT (เซิร์ฟเวอร์ไม่ตอบสนองในเวลาที่กำหนด/พอร์ตอาจถูกปิดกั้น)";
    case -3: return "-3 MQTT_CONNECTION_LOST (การเชื่อมต่อกับ Broker หลุดระหว่างการสื่อสาร)";
    case -2: return "-2 MQTT_CONNECT_FAILED (ไม่สามารถเปิดช่องเชื่อมต่อเน็ตเวิร์กไปยัง Broker ได้)";
    case -1: return "-1 MQTT_DISCONNECTED (บอร์ดตัดการเชื่อมต่อจาก Broker)";
    case 1:  return "1 MQTT_CONNECT_BAD_PROTOCOL (เวอร์ชันของโปรโตคอล MQTT ไม่ตรงกัน)";
    case 2:  return "2 MQTT_CONNECT_BAD_CLIENT_ID (Broker ปฏิเสธ Client ID นี้เนื่องจากรูปแบบไม่ถูกต้อง)";
    case 3:  return "3 MQTT_CONNECT_UNAVAILABLE (เซิร์ฟเวอร์ Broker ปิดปรับปรุงหรือไม่พร้อมให้บริการ)";
    case 4:  return "4 MQTT_CONNECT_BAD_CREDENTIALS (ชื่อผู้ใช้ (Username) หรือรหัสผ่านของ MQTT ผิดพลาด)";
    case 5:  return "5 MQTT_CONNECT_UNAUTHORIZED (บอร์ดไม่มีสิทธิ์เชื่อมต่อกับ Broker)";
    default: return "UNKNOWN MQTT ERROR CODE (ไม่พบรหัสข้อผิดพลาดนี้)";
  }
}

String getHTTPErrorString(int code) {
  if (code < 0) {
    switch (code) {
      case -1:  return "-1 CONNECTION_REFUSED (เซิร์ฟเวอร์ปฏิเสธเชื่อมต่อ: หลังบ้านอาจปิดอยู่ หรือ IP/พอร์ตผิด)";
      case -2:  return "-2 SEND_HEADER_FAILED (ส่งส่วนหัว HTTP Headers ไปยังหลังบ้านไม่สำเร็จ)";
      case -3:  return "-3 SEND_PAYLOAD_FAILED (ส่งข้อมูล Payload JSON ไปยังหลังบ้านล้มเหลว)";
      case -4:  return "-4 NOT_CONNECTED (บอร์ดไม่ได้ต่อเน็ต/หลุด WiFi ระหว่างส่งข้อมูล)";
      case -5:  return "-5 CONNECTION_LOST (สัญญาณขาดหายไประหว่างกำลังส่งข้อมูล)";
      case -6:  return "-6 NO_HTTP_SERVER (ไม่พบเครื่องเซิร์ฟเวอร์ HTTP ตามลิงก์ URL ที่ตั้งไว้)";
      case -11: return "-11 OUT_OF_RAM (ESP32 หน่วยความจำแรมชั่วคราวเต็ม)";
      default:  return "HTTP CLIENT INTERNAL ERROR (เกิดข้อผิดพลาดภายใน HTTP Client)";
    }
  } else {
    switch (code) {
      case 200: return "200 OK (ส่งข้อมูลสำเร็จ และหลังบ้านรับบันทึกเรียบร้อย!)";
      case 201: return "201 Created (ส่งข้อมูลสำเร็จและสร้างเหตุการณ์บันทึกใหม่แล้ว)";
      case 400: return "400 Bad Request (ข้อมูลที่ส่งไปมีรูปแบบ JSON ไม่ถูกต้อง)";
      case 401: return "401 Unauthorized (สิทธิ์การเข้าถึง API ไม่ถูกต้อง)";
      case 404: return "404 Not Found (ไม่พบปลายทาง: ลิงก์ URL ของ API /api/v1/events ผิดพลาด)";
      case 500: return "500 Internal Server Error (ระบบหลังบ้านเกิดข้อผิดพลาด: **น่าจะเพราะยังไม่ได้ลงทะเบียน ID 'LD6002C_TEST_01' ในฐานข้อมูล**)";
      case 502: return "502 Bad Gateway (Nginx Proxy เปิดอยู่ แต่เซิร์ฟเวอร์ Go Backend ด้านในยังไม่ได้รัน!)";
      case 504: return "504 Gateway Timeout (Proxy หมดเวลารอการตอบรับจากระบบหลังบ้าน)";
      default:  return "HTTP Response Code " + String(code);
    }
  }
}

// ================= CUSTOM LIGHTWEIGHT JSON PARSER =================
// ฟังก์ชันช่วยดึงค่าตัวเลขทศนิยม (Float) ออกจากข้อความ JSON โดยตรงโดยไม่ต้องใช้ Library
float getJsonFloatValue(String json, String key) {
  int keyIndex = json.indexOf("\"" + key + "\":");
  if (keyIndex == -1) return -1.0; 
  
  int valueStartIndex = keyIndex + key.length() + 3; // เลื่อนตำแหน่งข้ามคำค้นหาและเครื่องหมาย ":"
  
  // หาจุดสิ้นสุดของค่าตัวเลข (หยุดเมื่อเจอ "," หรือ "}")
  int commaIndex = json.indexOf(",", valueStartIndex);
  int braceIndex = json.indexOf("}", valueStartIndex);
  int endIndex = -1;
  
  if (commaIndex != -1 && braceIndex != -1) {
    endIndex = min(commaIndex, braceIndex);
  } else if (commaIndex != -1) {
    endIndex = commaIndex;
  } else {
    endIndex = braceIndex;
  }
  
  if (endIndex == -1) return -1.0;
  
  String valStr = json.substring(valueStartIndex, endIndex);
  valStr.trim();
  
  // แสดงผลดีบักการดึงข้อมูล JSON
  Serial.print("🔍 [JSON PARSE] Key: ");
  Serial.print(key);
  Serial.print(" | Raw String: '");
  Serial.print(valStr);
  Serial.print("' | Float Value: ");
  Serial.println(valStr.toFloat());
  
  return valStr.toFloat();
}

// ================= HLK-LD6002C CONFIG SENDER =================
#if USE_LD6002C
// ฟังก์ชันสำหรับส่งคำสั่งไปตั้งค่าตัวชิปเรดาร์ HLK-LD6002C ผ่าน Serial1 (ตามโปรโตคอลการสื่อสาร)
void sendConfigToRadar(uint16_t type, float value, bool printToWeb = true) {
  uint8_t frame[13];
  frame[0] = 0x01; // SOF
  frame[1] = 0x00; // ID High
  frame[2] = 0x00; // ID Low
  frame[3] = 0x00; // LEN High
  frame[4] = 0x04; // LEN Low
  frame[5] = (type >> 8) & 0xFF; // TYPE High (0x0E)
  frame[6] = type & 0xFF;        // TYPE Low (0x04 สำหรับความสูงติดตั้ง, 0x08 สำหรับเกณฑ์ตรวจล้ม)
  
  // คำนวณหา HEAD_CKSUM ของส่วนหัว
  uint8_t headXor = 0;
  for (int i = 0; i < 7; i++) {
    headXor ^= frame[i];
  }
  frame[7] = ~headXor; // HEAD_CKSUM
  
  // ใส่ข้อมูลค่าพารามิเตอร์ (4 ไบต์ แบบ Float Little Endian)
  uint8_t *valPtr = (uint8_t *)&value;
  frame[8] = valPtr[0];
  frame[9] = valPtr[1];
  frame[10] = valPtr[2];
  frame[11] = valPtr[3];
  
  // คำนวณหา DATA_CKSUM ของข้อมูล
  uint8_t dataXor = 0;
  for (int i = 8; i < 12; i++) {
    dataXor ^= frame[i];
  }
  frame[12] = ~dataXor; // DATA_CKSUM
  
  // ส่งข้อมูลเฟรมออกทางพอร์ต Serial ที่ต่อกับเรดาร์
  RadarSerial.write(frame, 13);
  
  if (printToWeb) {
    Serial.print("📡 [RADAR CONFIG] Sent Command 0x");
    Serial.print(type, HEX);
    Serial.print(" with value: ");
    Serial.println(value);
  } else {
    Serial0.print("📡 [RADAR CONFIG] Sent Command 0x");
    Serial0.print(type, HEX);
    Serial0.print(" with value: ");
    Serial0.println(value);
  }
}

// ฟังก์ชันส่งค่าพารามิเตอร์แบบจำนวนเต็ม 32 บิตไปยังเรดาร์ HLK-LD6002C (สำหรับคำสั่ง 0x0E01)
void sendConfigToRadarInt(uint16_t type, uint32_t value, bool printToWeb = true) {
  uint8_t frame[13];
  frame[0] = 0x01; // SOF
  frame[1] = 0x00; // ID High
  frame[2] = 0x00; // ID Low
  frame[3] = 0x00; // LEN High
  frame[4] = 0x04; // LEN Low
  frame[5] = (type >> 8) & 0xFF; // TYPE High
  frame[6] = type & 0xFF;        // TYPE Low
  
  uint8_t headXor = 0;
  for (int i = 0; i < 7; i++) {
    headXor ^= frame[i];
  }
  frame[7] = ~headXor; // HEAD_CKSUM
  
  frame[8] = value & 0xFF;
  frame[9] = (value >> 8) & 0xFF;
  frame[10] = (value >> 16) & 0xFF;
  frame[11] = (value >> 24) & 0xFF;
  
  uint8_t dataXor = 0;
  for (int i = 8; i < 12; i++) {
    dataXor ^= frame[i];
  }
  frame[12] = ~dataXor; // DATA_CKSUM
  
  RadarSerial.write(frame, 13);
  
  if (printToWeb) {
    Serial.print("📡 [RADAR CONFIG] Sent Command 0x");
    Serial.print(type, HEX);
    Serial.print(" with value (int): ");
    Serial.println(value);
  } else {
    Serial0.print("📡 [RADAR CONFIG] Sent Command 0x");
    Serial0.print(type, HEX);
    Serial0.print(" with value (int): ");
    Serial0.println(value);
  }
}

// ฟังก์ชันสำหรับส่งคำสั่งตั้งค่าพื้นที่แจ้งเตือนการล้ม (Alarm Area) 0x0E0C (ขนาดห้องน้ำ กว้าง x ยาว)
void sendRoomSizeToRadar(float width, float length, bool printToWeb = true) {
  float rect_XL = -width;        // ขอบเขตซ้าย (แกน X ทิศทางซ้าย)
  float rect_XR = width;         // ขอบเขตขวา (แกน X ทิศทางขวา)
  float rect_ZF = length;        // ขอบเขตหน้า (แกน Z ทิศทางข้างหน้า)
  float rect_ZB = -length;       // ขอบเขตหลัง (แกน Z ทิศทางข้างหลัง)
  
  uint8_t frame[25];
  frame[0] = 0x01; // SOF
  frame[1] = 0x00; // ID High
  frame[2] = 0x00; // ID Low
  frame[3] = 0x00; // LEN High
  frame[4] = 0x10; // LEN Low (ความยาวข้อมูล DATA = 16 ไบต์)
  frame[5] = 0x0E; // TYPE High
  frame[6] = 0x0C; // TYPE Low (0x0E0C)
  
  // คำนวณหา HEAD_CKSUM ของส่วนหัว (ไบต์ 0 ถึง 6)
  uint8_t headXor = 0;
  for (int i = 0; i < 7; i++) {
    headXor ^= frame[i];
  }
  frame[7] = ~headXor; // HEAD_CKSUM
  
  // ใส่ข้อมูลพารามิเตอร์ 4 ตัวแบบ Float (ไบต์ละ 4 ตัว รวมเป็น 16 ไบต์)
  memcpy(&frame[8], &rect_XL, 4);
  memcpy(&frame[12], &rect_XR, 4);
  memcpy(&frame[16], &rect_ZF, 4);
  memcpy(&frame[20], &rect_ZB, 4);
  
  // คำนวณหา DATA_CKSUM ของข้อมูล (ไบต์ 8 ถึง 23)
  uint8_t dataXor = 0;
  for (int i = 8; i < 24; i++) {
    dataXor ^= frame[i];
  }
  frame[24] = ~dataXor; // DATA_CKSUM
  
  // ส่งเฟรมข้อมูลไปที่ชิปเรดาร์
  RadarSerial.write(frame, 25);
  
  if (printToWeb) {
    Serial.print("📡 [RADAR CONFIG] Sent Room Dimensions (0x0E0C) - Width: ");
    Serial.print(width);
    Serial.print(" m, Length: ");
    Serial.print(length);
    Serial.println(" m");
  } else {
    Serial0.print("📡 [RADAR CONFIG] Sent Room Dimensions (0x0E0C) - Width: ");
    Serial0.print(width);
    Serial0.print(" m, Length: ");
    Serial0.print(length);
    Serial0.println(" m");
  }
}
#endif

// ================= WEB CONFIGURATION SERVICE =================
// ดึงข้อมูลการตั้งค่าขนาดห้องน้ำและระยะเซนเซอร์ล่าสุดจากหลังบ้าน
bool fetchDeviceSettings() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ [CONFIG] WiFi disconnected. Cannot fetch settings.");
    return false;
  }

  WiFiClientSecure clientSecure;
  clientSecure.setInsecure(); // ข้ามการเช็ก SSL เพื่อให้รันบน Azure HTTPS ได้สะดวก
  
  HTTPClient http;
  String config_url = "https://fall-detection.sopon-project.me/api/v1/config/" + String(device_id);
  
  Serial.println("\n📡 [CONFIG] Fetching latest parameters from web...");
  Serial.print("🔗 [CONFIG] Endpoint: ");
  Serial.println(config_url);
  
  http.begin(clientSecure, config_url);
  int httpResponseCode = http.GET();
  
  if (httpResponseCode == 200) {
    String payload = http.getString();
    Serial.print("📥 [CONFIG] Server payload: ");
    Serial.println(payload);
    
    // ดึงค่าพารามิเตอร์แต่ละตัวด้วย Custom Parser
    float temp_threshold = getJsonFloatValue(payload, "fall_threshold");
    float temp_height    = getJsonFloatValue(payload, "mount_height");
    float temp_width     = getJsonFloatValue(payload, "room_width");
    float temp_length    = getJsonFloatValue(payload, "room_length");
    
    // บันทึกค่าลงตัวแปรหากดึงข้อมูลมาได้สำเร็จ
    if (temp_threshold >= 0) {
      fall_threshold = temp_threshold;
#if USE_LD6002C
      sendConfigToRadarInt(0x0E08, (uint32_t)(fall_threshold * 100.0f));
      delay(50);
      sendConfigToRadar(0x0E08, fall_threshold); // ส่งค่าเกณฑ์ความสูงตรวจจับการล้มไปที่ชิปเรดาร์
      delay(100);
#endif
    }
    if (temp_height >= 0) {
      mount_height = temp_height;
#if USE_LD6002C
      sendConfigToRadarInt(0x0E04, (uint32_t)(mount_height * 100.0f));
      delay(50);
      sendConfigToRadar(0x0E04, mount_height); // ส่งค่าความสูงติดตั้งไปที่ชิปเรดาร์
      delay(100);
#endif
    }
    bool sizeChanged = false;
    if (temp_width >= 0) {
      room_width = temp_width;
      sizeChanged = true;
    }
    if (temp_length >= 0) {
      room_length = temp_length;
      sizeChanged = true;
    }
#if USE_LD6002C
    if (sizeChanged) {
      sendRoomSizeToRadar(room_width, room_length);
      delay(100);
    }
#endif
    
    // บันทึกเก็บลง Preferences (Flash Memory) แบบถาวรเพื่อนำมาโหลดเป็นค่าเริ่มต้นในภายหลัง
    preferences.begin("fall-detector", false);
    preferences.putFloat("threshold", fall_threshold);
    preferences.putFloat("height", mount_height);
    preferences.putFloat("width", room_width);
    preferences.putFloat("length", room_length);
    preferences.end();
    
    Serial.println("✅ [CONFIG] Local parameters updated successfully!");
    Serial.print("   👉 fall_threshold : "); Serial.println(fall_threshold);
    Serial.print("   👉 mount_height    : "); Serial.print(mount_height); Serial.println(" m");
    Serial.print("   👉 room_width      : "); Serial.print(room_width); Serial.println(" m");
    Serial.print("   👉 room_length     : "); Serial.print(room_length); Serial.println(" m\n");
    
    http.end();
    return true;
  } else {
    Serial.print("❌ [CONFIG] Fetch Failed. HTTP response: ");
    Serial.println(getHTTPErrorString(httpResponseCode));
    http.end();
    return false;
  }
}

// WebSocket Event Handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_CONNECTED) {
    // 1. ส่งประวัติล็อกช่วงเริ่มระบบที่ล็อคไว้ถาวรออกไปก่อน
    String bootLogs = WebLog.getStartupLogs();
    webSocket.sendTXT(num, bootLogs);
    
    // 2. ส่งล็อกเหตุการณ์สไลด์ล่าสุดตามไป
    String liveLogs = WebLog.getRecentLogs();
    webSocket.sendTXT(num, liveLogs);
  }
}

// ================= SETUP & LOOP =================
void setup() {
  Serial.begin(115200);
  delay(2000); // หน่วงเวลา 2 วินาทีให้ชิป USB Serial เชื่อมต่อเสร็จก่อนปริ้นข้อความเริ่มต้น

  // โหลดค่าพารามิเตอร์ล่าสุดที่เซฟเก็บไว้ใน Flash Memory (Preferences)
  preferences.begin("fall-detector", true); // เปิดโหมด Read-only
  fall_threshold = preferences.getFloat("threshold", fall_threshold);
  mount_height = preferences.getFloat("height", mount_height);
  room_width = preferences.getFloat("width", room_width);
  room_length = preferences.getFloat("length", room_length);
  preferences.end();
  
  Serial.println("💾 [SYSTEM] Loaded saved configurations from Flash memory:");
  Serial.print("   👉 fall_threshold : "); Serial.println(fall_threshold);
  Serial.print("   👉 mount_height    : "); Serial.print(mount_height); Serial.println(" m");
  Serial.print("   👉 room_width      : "); Serial.print(room_width); Serial.println(" m");
  Serial.print("   👉 room_length     : "); Serial.print(room_length); Serial.println(" m\n");
  Serial.println("\n=============================================");
  Serial.println("Starting ESP32-S3 Config Sync Mode...");
  Serial.print("Target Device ID: ");
  Serial.println(device_id);
  Serial.print("Communication mode: ");
  Serial.println(USE_MQTT ? "MQTT (QoS 0)" : "HTTP POST (HTTPS Secure)");
  Serial.println("=============================================\n");

  // ตั้งค่าปุ่มกดแบบ INPUT_PULLUP (ปุ่มกดทำงานเมื่อต่อลง GND)
  pinMode(BUTTON_ENTER_PIN, INPUT_PULLUP);
  pinMode(BUTTON_FALL_PIN, INPUT_PULLUP);
  pinMode(BUTTON_EXIT_PIN, INPUT_PULLUP);

  // ตั้งค่าพินควบคุม Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH); // สั่ง HIGH เพื่อปิดรีเลย์แบบ Low Trigger เริ่มต้นป้องกันเสียงดังขณะเปิดเครื่อง

  connectWiFi();

  // ซิงค์เวลาจากอินเทอร์เน็ตผ่าน NTP Server และตั้งค่าโซนเวลาประเทศไทย (UTC+7)
  configTzTime("ICT-7", "pool.ntp.org", "time.nist.gov");

  // --- ตั้งค่าสำหรับ ArduinoOTA (ให้พร้อมใช้งานทันทีหลังต่อ WiFi) ---
  ArduinoOTA.setHostname("esp32s3-fall");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) type = "sketch";
    else type = "filesystem";
    Serial.println("Start updating " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
  Serial.println("📡 [SYSTEM] Ready for OTA updates!");

  // เริ่มต้นใช้งาน Serial สำหรับเชื่อมต่อกับเซนเซอร์เรดาร์ HLK-LD6002C
#if USE_LD6002C
  RadarSerial.begin(115200, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);
  Serial.println("📡 [SYSTEM] HLK-LD6002C Radar UART initialized at 115200 baud.");
  Serial.println("⏳ [SYSTEM] Waiting 2.5 seconds for radar module boot-up...");
  delay(2500); // หน่วงเวลาให้เซนเซอร์เรดาร์บูตระบบเสร็จสมบูรณ์ก่อนรับส่งคำสั่งตั้งค่า
  
  // ส่งค่าติดตั้งเริ่มต้นไปยังเรดาร์หลังเริ่มระบบ (ส่งทั้ง Integer cm และ Float m)
  uint32_t height_cm = (uint32_t)(mount_height * 100.0f);
  uint32_t threshold_cm = (uint32_t)(fall_threshold * 100.0f);

  sendConfigToRadarInt(0x0E04, height_cm, false);     // ตั้งค่าความสูงติดตั้ง (220 cm)
  delay(100);
  sendConfigToRadar(0x0E04, mount_height, false);     // ตั้งค่าความสูงติดตั้ง (2.20 m)
  delay(100);

  sendConfigToRadarInt(0x0E08, threshold_cm, false);  // ตั้งค่าเกณฑ์ตรวจล้ม (75 cm)
  delay(100);
  sendConfigToRadar(0x0E08, fall_threshold, false);  // ตั้งค่าเกณฑ์ตรวจล้ม (0.75 m)
  delay(100);

  sendRoomSizeToRadar(room_width, room_length, false); // ตั้งค่าพื้นที่ตรวจจับล้ม (Alarm Area: 3x3m)
  delay(100);
  // sendConfigToRadarInt(0x0E01, 1, false); // คอมเมนต์ออกชั่วคราวเนื่องจากเรดาร์บางล็อตอาจจะไม่รองรับคำสั่งนี้และแฮงก์
#endif

  // ตั้งค่าสำหรับเซนเซอร์ HLK-LD2410C
#if USE_LD2410C
  pinMode(LD2410_OUT_PIN, INPUT_PULLDOWN);
  Serial.println("📡 [SYSTEM] HLK-LD2410C GPIO OUT pin initialized.");
#endif

  // ดึงค่าการตั้งค่าจาก Server ครั้งแรกทันทีหลังจากเชื่อมต่อ WiFi สำเร็จ
  fetchDeviceSettings();

  if (USE_MQTT) {
    mqttClient.setServer(mqtt_broker, mqtt_port);
  }

  // --- ตั้งค่าหน้าเว็บหลัก (/) สำหรับแสดงสถานะแบบสวยงาม ---
  webServer.on("/", []() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
    html += "<title>Smart Fall Detection Dashboard</title>";
    html += "<style>";
    html += "body { background: #0b0b10; color: #e2e2e9; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; margin: 0; padding: 20px; display: flex; flex-direction: column; align-items: center; justify-content: center; min-height: 95vh; }";
    html += ".container { background: rgba(22, 22, 34, 0.6); backdrop-filter: blur(10px); border: 1px solid rgba(255, 255, 255, 0.05); border-radius: 20px; padding: 30px; width: 90%; max-width: 500px; box-shadow: 0 10px 30px rgba(0,0,0,0.5); text-align: center; }";
    html += "h1 { margin: 0 0 5px 0; font-size: 24px; font-weight: 600; letter-spacing: 0.5px; }";
    html += ".device-info { font-size: 13px; color: #8888aa; margin-bottom: 25px; }";
    html += ".status-ring-container { position: relative; width: 200px; height: 200px; margin: 0 auto 30px auto; display: flex; align-items: center; justify-content: center; }";
    html += ".status-ring { position: absolute; width: 100%; height: 100%; border-radius: 50%; border: 8px solid #1a1a26; box-sizing: border-box; transition: all 0.5s ease; }";
    html += ".status-ring.empty { border-color: rgba(0, 255, 136, 0.1); box-shadow: 0 0 20px rgba(0, 255, 136, 0.2), inset 0 0 20px rgba(0, 255, 136, 0.1); }";
    html += ".status-ring.occupied { border-color: rgba(0, 150, 255, 0.1); box-shadow: 0 0 30px rgba(0, 150, 255, 0.3), inset 0 0 30px rgba(0, 150, 255, 0.15); animation: pulse 2s infinite; }";
    html += ".status-ring.fall { border-color: rgba(255, 50, 50, 0.2); box-shadow: 0 0 40px rgba(255, 50, 50, 0.6), inset 0 0 40px rgba(255, 50, 50, 0.3); animation: flash 1s infinite; }";
    html += "@keyframes pulse { 0% { transform: scale(1); opacity: 0.9; } 50% { transform: scale(1.03); opacity: 1; } 100% { transform: scale(1); opacity: 0.9; } }";
    html += "@keyframes flash { 0% { transform: scale(1); border-color: rgba(255,50,50,0.8); } 50% { transform: scale(1.05); border-color: rgba(255,50,50,0.2); } 100% { transform: scale(1); border-color: rgba(255,50,50,0.8); } }";
    html += ".status-text-big { font-size: 26px; font-weight: 700; transition: color 0.5s ease; }";
    html += ".status-text-big.empty { color: #00ff88; }";
    html += ".status-text-big.occupied { color: #00a2ff; }";
    html += ".status-text-big.fall { color: #ff3333; }";
    html += ".stats-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; margin-top: 20px; }";
    html += ".stat-card { background: #151522; border: 1px solid #232335; padding: 15px; border-radius: 12px; text-align: left; }";
    html += ".stat-label { font-size: 11px; color: #8888aa; text-transform: uppercase; letter-spacing: 0.5px; margin-bottom: 5px; }";
    html += ".stat-value { font-size: 15px; font-weight: bold; color: #ffffff; }";
    html += ".status-dot { display: inline-block; width: 8px; height: 8px; border-radius: 50%; margin-right: 5px; }";
    html += ".status-dot.green { background: #00ff88; box-shadow: 0 0 8px #00ff88; }";
    html += ".status-dot.red { background: #ff3333; box-shadow: 0 0 8px #ff3333; }";
    html += ".footer-link { margin-top: 25px; font-size: 12px; }";
    html += ".footer-link a { color: #00ff88; text-decoration: none; border-bottom: 1px dashed #00ff88; }";
    html += "</style></head><body>";
    html += "  <div class='container'>";
    html += "    <h1>🏠 Bathroom Safety Dashboard</h1>";
    html += "    <div class='device-info'>Device ID: <strong id='device_id'>" + String(device_id) + "</strong> | Uptime: <span id='uptime'>0</span> s</div>";
    html += "    <div class='status-ring-container'>";
    html += "      <div id='ring' class='status-ring empty'></div>";
    html += "      <div id='status_big' class='status-text-big empty'>ห้องว่าง</div>";
    html += "    </div>";
    html += "    <div class='stats-grid'>";
    html += "      <div class='stat-card'>";
    html += "        <div class='stat-label'>เซนเซอร์เรดาร์ (ล้ม)</div>";
    html += "        <div id='radar_val' class='stat-value'>NORMAL (ปกติ)</div>";
    html += "      </div>";
    html += "      <div class='stat-card'>";
    html += "        <div class='stat-label'>ระดับความสูงเป้าหมาย</div>";
    html += "        <div id='height_val' class='stat-value'>0.00 m</div>";
    html += "      </div>";
    html += "      <div class='stat-card'>";
    html += "        <div class='stat-label'>การเชื่อมต่อ MQTT</div>";
    html += "        <div id='mqtt_val' class='stat-value'><span class='status-dot green'></span>Connected</div>";
    html += "      </div>";
    html += "      <div class='stat-card'>";
    html += "        <div class='stat-label'>การส่งข้อมูลหลังบ้าน</div>";
    html += "        <div id='post_val' class='stat-value'>พร้อมใช้งาน</div>";
    html += "      </div>";
    html += "    </div>";
    html += "    <div class='stat-card' style='margin-top: 20px; display: flex; flex-direction: column; text-align: left;'>";
    html += "      <div class='stat-label'>⚙️ System Setup & Config Log</div>";
    html += "      <pre id='startupLog' style='margin: 8px 0 0 0; padding: 10px; background: #0c0c14; border: 1px solid #232335; border-radius: 8px; color: #00d2ff; font-family: monospace; font-size: 11px; height: 120px; overflow-y: auto; white-space: pre-wrap; word-break: break-all; box-sizing: border-box;'>Awaiting system setup configuration...</pre>";
    html += "    </div>";
    html += "    <div class='footer-link'>";
    html += "      <a href='/debug'>💻 สลับไปที่หน้าระบบการดีบักเชิงลึก (Debug Console)</a>";
    html += "    </div>";
    html += "  </div>";
    html += "  <script>";
    html += "    var ring = document.getElementById('ring');";
    html += "    var statusBig = document.getElementById('status_big');";
    html += "    var radarVal = document.getElementById('radar_val');";
    html += "    var heightVal = document.getElementById('height_val');";
    html += "    var mqttVal = document.getElementById('mqtt_val');";
    html += "    var postVal = document.getElementById('post_val');";
    html += "    var startupLog = document.getElementById('startupLog');";
    html += "    var roomPresence = 'empty';";
    html += "    var radarFall = false;";
    html += "    var ws;";
    html += "    var firstMsg = true;";
    html += "    function connectWS() {";
    html += "      ws = new WebSocket('ws://' + window.location.hostname + ':81/');";
    html += "      ws.onopen = function() {";
    html += "        firstMsg = true;";
    html += "      };";
    html += "      ws.onclose = function() {";
    html += "        setTimeout(connectWS, 1500);";
    html += "      };";
    html += "      ws.onmessage = function(evt) {";
    html += "        if (firstMsg) {";
    html += "          firstMsg = false;";
    html += "          startupLog.innerHTML = evt.data;";
    html += "          startupLog.scrollTop = startupLog.scrollHeight;";
    html += "          if (evt.data.includes('presence state: OCCUPIED')) roomPresence = 'occupied';";
    html += "          if (evt.data.includes('presence state: EMPTY')) roomPresence = 'empty';";
    html += "          if (evt.data.includes('is_fall: 1') || evt.data.includes('PERSON FELL')) radarFall = true;";
    html += "          if (evt.data.includes('[RADAR HEIGHT]')) {";
    html += "            var m = evt.data.match(/Height:\\\\s*([\\\\d.]+)/g);";
    html += "            if (m && m.length > 0) {";
    html += "              var lastM = m[m.length - 1].match(/Height:\\\\s*([\\\\d.]+)/);";
    html += "              if (lastM) heightVal.innerText = lastM[1] + ' m';";
    html += "            }";
    html += "          }";
    html += "          updateUIState();";
    html += "          return;";
    html += "        }";
    html += "        var lines = evt.data.split('\\n');";
    html += "        for (var i = 0; i < lines.length; i++) {";
    html += "          var line = lines[i];";
    html += "          if (line.trim() === '') continue;";
    html += "          if (line.includes('[CONFIG]') || line.includes('[JSON PARSE]') || line.includes('[RADAR CONFIG]') || line.includes('[WIFI]') || line.includes('[SYSTEM]') || line.includes('[MQTT] Connecting') || line.includes('[MQTT] Connection') || line.includes('CONNECTED!') || line.includes('FAILED. Error code') || line.includes('DIAGNOSTIC')) {";
    html += "            if (startupLog.innerHTML.includes('Awaiting')) startupLog.innerHTML = '';";
    html += "            startupLog.innerHTML += line + '\\n';";
    html += "            startupLog.scrollTop = startupLog.scrollHeight;";
    html += "          }";
    html += "          if (line.includes('CONNECTED!')) {";
    html += "            mqttVal.innerHTML = \"<span class='status-dot green'></span>Connected\";";
    html += "          } else if (line.includes('FAILED. Error code')) {";
    html += "            mqttVal.innerHTML = \"<span class='status-dot red'></span>Disconnected\";";
    html += "          }";
    html += "          if (line.includes('[RADAR HEIGHT]') || line.includes('HEIGHT')) {";
    html += "            var matchHeight = line.match(/Height:\\\\s*([\\\\d.]+)/) || line.match(/([\\\\d.]+)\\\\s*m/i);";
    html += "            if (matchHeight) {";
    html += "              var hEl = document.getElementById('height_val');";
    html += "              if (hEl) hEl.innerText = matchHeight[1] + ' m';";
    html += "            }";
    html += "          }";
    html += "          if (line.includes('Publish Succeeded') || line.includes('HTTP POST Succeeded')) {";
    html += "            postVal.innerText = 'ส่งข้อมูลสำเร็จ';";
    html += "            postVal.style.color = '#00ff88';";
    html += "          } else if (line.includes('Publish FAILED') || line.includes('HTTP POST FAILED')) {";
    html += "            postVal.innerText = 'ส่งข้อมูลล้มเหลว';";
    html += "            postVal.style.color = '#ff3333';";
    html += "          }";
    html += "          if (line.includes('presence state: EMPTY')) {";
    html += "            roomPresence = 'empty';";
    html += "            updateUIState();";
    html += "          } else if (line.includes('presence state: OCCUPIED')) {";
    html += "            roomPresence = 'occupied';";
    html += "            updateUIState();";
    html += "          }";
    html += "          if (line.includes('is_fall: 1') || line.includes('PERSON FELL') || line.includes('Fall confirmed!')) {";
    html += "            radarFall = true;";
    html += "            updateUIState();";
    html += "          } else if (line.includes('is_fall: 0') || line.includes('NORMAL (🟢 ปกติ)') || line.includes('Fall canceled')) {";
    html += "            radarFall = false;";
    html += "            updateUIState();";
    html += "          }";
    html += "        }";
    html += "      };";
    html += "    }";
    html += "    function updateUIState() {";
    html += "      if (radarFall) {";
    html += "        updateUI('fall');";
    html += "      } else if (roomPresence === 'occupied') {";
    html += "        updateUI('occupied');";
    html += "      } else {";
    html += "        updateUI('empty');";
    html += "      }";
    html += "    }";
    html += "    function updateUI(state) {";
    html += "      ring.className = 'status-ring ' + state;";
    html += "      statusBig.className = 'status-text-big ' + state;";
    html += "      if (state === 'empty') {";
    html += "        statusBig.innerText = 'ห้องว่าง';";
    html += "        radarVal.innerText = 'NORMAL (ปกติ)';";
    html += "        radarVal.style.color = '#e2e2e9';";
    html += "      } else if (state === 'occupied') {";
    html += "        statusBig.innerText = 'กำลังใช้งาน';";
    html += "        radarVal.innerText = 'NORMAL (ปกติ)';";
    html += "        radarVal.style.color = '#00a2ff';";
    html += "      } else if (state === 'fall') {";
    html += "        statusBig.innerText = 'ตรวจพบคนล้ม! ⚠️';";
    html += "        radarVal.innerText = '🚨 FALL (ล้ม!!!)';";
    html += "        radarVal.style.color = '#ff3333';";
    html += "      }";
    html += "    }";
    html += "    connectWS();";
    html += "    setInterval(function() { document.getElementById('uptime').innerText = Math.floor(performance.now()/1000); }, 1000);";
    html += "  </script></body></html>";
    webServer.send(200, "text/html", html);
  });

  // --- ตั้งค่าหน้าเว็บสำหรับดีบักโดยเฉพาะ (/debug) ---
  webServer.on("/debug", []() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
    html += "<title>ESP32-S3 Dual Log Console</title>";
    html += "<style>";
    html += "body { background: #0f0f15; color: #00ff88; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; padding: 20px; display: flex; flex-direction: column; height: 98vh; box-sizing: border-box; }";
    html += "h1 { color: #ffffff; margin: 0 0 10px 0; font-size: 22px; text-shadow: 0 0 10px rgba(255,255,255,0.1); }";
    html += ".status-bar { background: #161622; padding: 10px 15px; border-radius: 8px; font-size: 13px; margin-bottom: 15px; border: 1px solid #232335; display: flex; justify-content: space-between; }";
    html += ".console-container { display: flex; flex-direction: row; gap: 20px; flex-grow: 1; min-height: 0; }";
    html += ".console-panel { display: flex; flex-direction: column; width: 50%; min-height: 0; }";
    html += ".panel-title { font-size: 12px; color: #8888aa; margin: 0 0 5px 5px; text-transform: uppercase; font-weight: bold; letter-spacing: 1px; }";
    html += "pre { margin: 0; padding: 15px; border-radius: 8px; font-family: 'Consolas', 'Courier New', monospace; font-size: 13px; overflow-y: auto; box-sizing: border-box; flex-grow: 1; }";
    html += "#radarLog { background: #14141d; border: 1px solid #1a3344; color: #00d2ff; }";
    html += "#presenceLog { background: #0d0d12; border: 1px solid #1a3322; color: #33ff99; }";
    html += "@media (max-width: 768px) {";
    html += "  .console-container { flex-direction: column; }";
    html += "  .console-panel { width: 100%; height: 40vh; }";
    html += "}";
    html += "</style></head><body>";
    html += "<h1>📡 ESP32-S3 Dual Log Console</h1>";
    html += "<div class='status-bar'>";
    html += "  <div>Device ID: <strong>" + String(device_id) + "</strong> | IP: <strong>" + WiFi.localIP().toString() + "</strong> | 📊 Target Height: <strong id='debug_height_val' style='color:#00d2ff;'>0.00 m</strong></div>";
    html += "  <div>Uptime: <span id='uptime'>0</span> s | Connection: <span id='ws_status' style='color:red;font-weight:bold;'>Disconnected</span> | <a href='/' style='color:#00ff88;text-decoration:none;'>🏠 แดชบอร์ดหลัก</a></div>";
    html += "</div>";
    html += "<div class='console-container'>";
    html += "  <div class='console-panel'>";
    html += "    <div class='panel-title'>🛰️ HLK-LD6002C Radar Log (Fall Detection)</div>";
    html += "    <pre id='radarLog'>Awaiting HLK-LD6002C radar activity...</pre>";
    html += "  </div>";
    html += "  <div class='console-panel'>";
    html += "    <div class='panel-title'>🚶‍♂️ HLK-LD2410C Radar Log (Presence Detection)</div>";
    html += "    <pre id='presenceLog'>Awaiting HLK-LD2410C presence activity...</pre>";
    html += "  </div>";
    html += "</div>";
    html += "<script>";
    html += "var radarLog = document.getElementById('radarLog');";
    html += "var presenceLog = document.getElementById('presenceLog');";
    html += "var wsStatus = document.getElementById('ws_status');";
    html += "var ws;";
    html += "function connectWS() {";
    html += "  ws = new WebSocket('ws://' + window.location.hostname + ':81/');";
    html += "  ws.onopen = function() { ";
    html += "    wsStatus.innerText = 'Connected'; ";
    html += "    wsStatus.style.color = '#00ff88'; ";
    html += "    radarLog.innerHTML = '--- Connected to radar stream ---\\n'; ";
    html += "    presenceLog.innerHTML = '--- Connected to presence stream ---\\n'; ";
    html += "  };";
    html += "  ws.onclose = function() { ";
    html += "    wsStatus.innerText = 'Disconnected'; ";
    html += "    wsStatus.style.color = 'red'; ";
    html += "    setTimeout(connectWS, 1500); ";
    html += "  };";
    html += "  ws.onmessage = function(evt) {";
    html += "    var lines = evt.data.split('\\n');";
    html += "    for (var i = 0; i < lines.length; i++) {";
    html += "      var line = lines[i];";
    html += "      if (line.trim() === '') continue;";
    html += "      if (line.includes('[RADAR HEIGHT]')) {";
    html += "        var matchH = line.match(/Height:\\\\s*([\\\\d.]+)/);";
    html += "        if (matchH) {";
    html += "          var hElem = document.getElementById('debug_height_val');";
    html += "          if (hElem) hElem.innerText = matchH[1] + ' m';";
    html += "        }";
    html += "      }";
    html += "      if (line.includes('[RADAR') || line.includes('is_fall:') || line.includes('[RAW SENSOR]') || line.includes('FALL') || line.includes('HEIGHT') || line.includes('radar')) {";
    html += "        if (radarLog.innerHTML.includes('Awaiting') || radarLog.innerHTML.includes('Connected')) radarLog.innerHTML = '';";
    html += "        radarLog.innerHTML += line + '\\n';";
    html += "        if(radarLog.innerHTML.length > 25000) { radarLog.innerHTML = radarLog.innerHTML.substring(radarLog.innerHTML.length - 15000); }";
    html += "        radarLog.scrollTop = radarLog.scrollHeight;";
    html += "      }";
    html += "      else if (line.includes('[LD2410') || line.includes('presence state:') || line.includes('Human presence') || line.includes('EMPTY') || line.includes('OCCUPIED') || line.includes('presence')) {";
    html += "        if (presenceLog.innerHTML.includes('Awaiting') || presenceLog.innerHTML.includes('Connected')) presenceLog.innerHTML = '';";
    html += "        presenceLog.innerHTML += line + '\\n';";
    html += "        if(presenceLog.innerHTML.length > 25000) { presenceLog.innerHTML = presenceLog.innerHTML.substring(presenceLog.innerHTML.length - 15000); }";
    html += "        presenceLog.scrollTop = presenceLog.scrollHeight;";
    html += "      }";
    html += "    }";
    html += "  };";
    html += "}";
    html += "connectWS();";
    html += "setInterval(function() { document.getElementById('uptime').innerText = Math.floor(performance.now()/1000); }, 1000);";
    html += "</script></body></html>";
    webServer.send(200, "text/html", html);
  });
  
  webServer.begin();
  webSocket.begin(); // เริ่มต้นใช้งาน WebSocket Server บนพอร์ต 81
  webSocket.onEvent(webSocketEvent); // ผูกฟังก์ชันเพื่อจัดการการเชื่อมต่อ
  ElegantOTA.begin(&webServer);
  Serial0.println("📡 [SYSTEM] Web Log Server started on port 80");
  Serial0.println("📡 [SYSTEM] WebSocket Log Server started on port 81");
  WebLog.markStartupDone(); // สิ้นสุดกระบวนการเริ่มต้นระบบ หยุดบันทึกเข้าล็อกถาวร
}

void loop() {
  // รักษาการทำงานของระบบไร้สาย OTA
  ArduinoOTA.handle();

  // จัดการการทำงานของ Web Server เพื่อให้บริการหน้าล็อก
  webServer.handleClient();
  ElegantOTA.loop();
  
  // จัดการการส่งข้อมูลแบบเรียลไทม์ผ่าน WebSocket
  webSocket.loop();



  // อ่านและถอดรหัสค่าจากเซนเซอร์เรดาร์ HLK-LD6002C
#if USE_LD6002C
  readRadar();
#endif

  // อ่านและจัดการค่าจากเซนเซอร์ตรวจจับคน HLK-LD2410C
#if USE_LD2410C
  readLD2410();
#endif

  // ตรวจสอบสถานะการเชื่อมต่อ WiFi เสมอ
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  // หากเลือกใช้ MQTT ให้ตรวจสอบสถานะการเชื่อมต่อ
  if (USE_MQTT) {
    if (!mqttClient.connected()) {
      connectMQTTNonBlocking();
    } else {
      mqttClient.loop();
    }
  }

  // --- อ่านสถานะปุ่มกด (Active Low: ปุ่มถูกกดเมื่อสถานะเป็น LOW) ---
  if (digitalRead(BUTTON_ENTER_PIN) == LOW) {
    current_event = "enter";
    fall_alerted_this_session = false;
    is_falling_candidate = false;
    is_recovering_candidate = false;
    sendEvent("event", "enter", "กำลังใช้งาน");
    delay(500); // ป้องกันปุ่มเบิ้ล (Debounce)
  }
  else if (digitalRead(BUTTON_FALL_PIN) == LOW) {
    current_event = "fall";
    fall_alerted_this_session = true;
    is_falling_candidate = false;
    sendEvent("event", "fall", "ตรวจพบคนล้ม");
    delay(500);
  }
  else if (digitalRead(BUTTON_EXIT_PIN) == LOW) {
    current_event = "exit";
    fall_alerted_this_session = false;
    is_falling_candidate = false;
    is_recovering_candidate = false;
    fall_start_time = 0;
    sendEvent("event", "exit", "ว่าง");
    delay(500);
  }

  // อัปเดตสีไฟ RGB LED ตามสถานะปัจจุบันของระบบและบอร์ด
  updateRGBStatus();

  delay(50);
}

// ฟังก์ชันระบุสีไฟ RGB พร้อมปรับลดความสว่างตามสัดส่วน
void setRGBColor(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(RGB_LED_PIN, r * RGB_BRIGHTNESS, g * RGB_BRIGHTNESS, b * RGB_BRIGHTNESS);
}

// ฟังก์ชันอัปเดตไฟแสดงสถานะการเชื่อมต่อและเซนเซอร์
void updateRGBStatus() {
  if (WiFi.status() != WL_CONNECTED) {
    // 1. WiFi หลุด -> ไฟสีน้ำเงินกะพริบช้า ๆ
    unsigned long current_time = millis();
    if (current_time - last_flash_time >= 500) {
      last_flash_time = current_time;
      flash_state = !flash_state;
      if (flash_state) setRGBColor(0, 0, 100);
      else setRGBColor(0, 0, 0);
    }
    return;
  }

  if (USE_MQTT && !mqttClient.connected()) {
    // 2. WiFi ติด แต่ต่อ MQTT ไม่ผ่าน -> ไฟสีเหลืองค้าง (แดง + เขียว)
    setRGBColor(100, 100, 0);
    return;
  }

  // 3. แสดงสีตามสถานะเซนเซอร์/การใช้งานห้องน้ำ และควบคุม Buzzer
  if (current_event == "enter") {
    // มีคนเข้าใช้งานห้องน้ำ -> ไฟสีฟ้าครามค้าง (Cyan/Blue)
    setRGBColor(0, 100, 200);
    digitalWrite(BUZZER_PIN, HIGH); // ปิดรีเลย์แบบ Low Trigger (ไม่มีเสียงเตือน)
  } 
  else if (current_event == "fall") {
    // ตรวจพบเหตุฉุกเฉิน คนล้ม!!! -> ไฟสีแดงกะพริบถี่ ๆ แจ้งเตือนอันตราย และเปิดเสียงเตือนเป็นจังหวะตามไฟกะพริบ
    unsigned long current_time = millis();
    if (current_time - last_flash_time >= 200) {
      last_flash_time = current_time;
      flash_state = !flash_state;
      if (flash_state) {
        setRGBColor(255, 0, 0);
        digitalWrite(BUZZER_PIN, LOW); // สั่ง LOW เพื่อเปิดรีเลย์แบบ Low Trigger (เสียงดัง)
      } else {
        setRGBColor(0, 0, 0);
        digitalWrite(BUZZER_PIN, HIGH);  // สั่ง HIGH เพื่อปิดรีเลย์แบบ Low Trigger (เสียงดับ)
      }
    }
  } 
  else {
    // ห้องว่าง ปลอดภัยปกติ -> ไฟสีเขียวค้าง
    setRGBColor(0, 150, 0);
    digitalWrite(BUZZER_PIN, HIGH); // ปิดรีเลย์แบบ Low Trigger (ไม่มีเสียงเตือน)
  }
}

// ฟังก์ชันเชื่อมต่อ WiFi
void connectWiFi() {
  Serial.println();
  Serial.print("📶 [WIFI] Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    setRGBColor(0, 0, 100);
    delay(250);
    setRGBColor(0, 0, 0);
    delay(250);
    Serial.print(".");
    attempts++;
  }
  
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✅ [WIFI] WiFi Connected Successfully!");
    Serial.print("👉 [WIFI] IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.print("❌ [WIFI] Connection Failed! Reason: ");
    Serial.println(getWiFiStatusString(WiFi.status()));
  }
}

// ฟังก์ชันเชื่อมต่อ MQTT Broker แบบไม่บล็อกการทำงาน (Non-blocking)
void connectMQTTNonBlocking() {
  unsigned long now = millis();
  if (now - last_mqtt_reconnect_attempt > 5000) {
    last_mqtt_reconnect_attempt = now;
    setRGBColor(100, 100, 0);
    Serial.print("📡 [MQTT] Connecting to Broker at ");
    Serial.print(mqtt_broker);
    Serial.print(":");
    Serial.print(mqtt_port);
    Serial.print("... ");
    
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("✅ CONNECTED!");
      setRGBColor(0, 150, 0);
    } else {
      Serial.print("❌ FAILED. Error code: ");
      Serial.println(getMQTTErrorString(mqttClient.state()));
    }
  }
}

// ฟังก์ชันส่งข้อมูลเหตุการณ์ไปยังหลังบ้าน
void sendEvent(const char* type, const char* name, const char* display_status) {
  Serial.println("\n---------------------------------------------");
  Serial.print("📣 [EVENT] Triggered Action: ");
  Serial.println(display_status);

  String jsonPayload = "{\"device_id\":\"" + String(device_id) + "\",\"type\":\"" + String(type) + "\",\"name\":\"" + String(name) + "\",\"value\":\"" + String(display_status) + "\"}";

  if (USE_MQTT) {
    if (mqttClient.connected()) {
      Serial.print("📤 [MQTT] Publishing to [");
      Serial.print(mqtt_topic);
      Serial.println("]... ");
      Serial.print("📝 [MQTT] Payload: ");
      Serial.println(jsonPayload);
      
      if (mqttClient.publish(mqtt_topic, jsonPayload.c_str())) {
        Serial.println("✅ [MQTT] Publish Succeeded!");
      } else {
        Serial.println("❌ [MQTT] Publish FAILED!");
      }
    } else {
      Serial.print("❌ [MQTT] Connection is offline. Cannot publish event. Error: ");
      Serial.println(getMQTTErrorString(mqttClient.state()));
    }
  } 
  else {
    WiFiClientSecure clientSecure;
    clientSecure.setInsecure(); // ละเว้นการตรวจสอบใบรับรอง SSL
    
    HTTPClient http;
    Serial.print("📤 [HTTP] Sending POST (HTTPS) to: ");
    Serial.println(backend_url);
    Serial.print("📝 [HTTP] Payload: ");
    Serial.println(jsonPayload);
    
    http.begin(clientSecure, backend_url);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST(jsonPayload);
    
    Serial.print("📥 [HTTP] Status response: ");
    Serial.println(getHTTPErrorString(httpResponseCode));
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("📥 [HTTP] Server Response: ");
      Serial.println(response);
    }
    http.end();
  }
  Serial.println("---------------------------------------------\n");
}

#if USE_LD6002C
// ฟังก์ชันสำหรับกรองค่าความสูง (Height Signal Filter - Fast Responsive EMA + Range Validation)
void processFilteredHeight(float input_val, String &target_event, String &display_status) {
  static float filtered_h = 0.0;
  static float last_pushed_h = -1.0;
  static unsigned long last_push_time = 0;
  unsigned long now = millis();

  // แปลงระยะห่างจากเพดาน (Distance from Sensor) เป็นความสูงของคนจากพื้นจริง (Height from Floor)
  float raw_h = input_val;
  if (input_val > 0.05 && mount_height > input_val) {
    raw_h = mount_height - input_val;
  }

  // 1. กรองค่ารบกวนที่หลุดช่วงระยะมนุษย์จริงจากพื้น (Valid Person Height Range: 0.20m - 2.30m)
  if (raw_h < 0.20 || raw_h > 2.30) {
    return; // ตัดขยะทิ้ง
  }

  // 2. ถ้าเป็นค่าแรกสุด หรือมีการขยับเปลี่ยนท่าทางใหญ่ (> 0.75m) ให้ซิงก์ค่าใหม่ทันที
  if (filtered_h < 0.1 || fabs(raw_h - filtered_h) > 0.75) {
    filtered_h = raw_h;
    last_pushed_h = filtered_h;
    last_push_time = now;
    Serial.printf("📊 [RADAR HEIGHT] Current Target Height: %.2f m\n", filtered_h);
  } else {
    // 3. Exponential Moving Average (EMA) เพื่อให้ตัวเลขขยับนุ่มนวล (Smooth 35% / 65%)
    filtered_h = (0.35f * raw_h) + (0.65f * filtered_h);
  }

  // 4. จำกัดความถี่การส่งข้อมูล (ส่งออกเมื่อขยับเกิน 2 cm หรือส่งสรุปอย่างน้อยทุก 2.5 วินาที)
  if (fabs(filtered_h - last_pushed_h) >= 0.02 || (now - last_push_time >= 2500)) {
    last_pushed_h = filtered_h;
    last_push_time = now;
    Serial.printf("📊 [RADAR HEIGHT] Current Target Height: %.2f m\n", filtered_h);
  }

  // 5. [ปิดการใช้งาน] ตรวจจับการล้มด้วยระดับความสูง (Fall Detection via Height Threshold Engine)
  // ปิดส่วนนี้ไว้เพื่อให้ระบบพึ่งพาการตัดสินใจจากอัลกอริทึมชิปเรดาร์ HLK-LD6002C โดยตรง (Method 1: is_fall frame)
  /*
  if (filtered_h > 0.05 && filtered_h <= fall_threshold) {
    if (!fall_alerted_this_session) {
      target_event = "fall";
      display_status = "ตรวจพบคนล้ม (เรดาร์/ระดับความสูง)";
      fall_alerted_this_session = true;
      Serial.println("🚨 [FALL ALERT] Target height dropped below fall threshold! Sending fall alert...");
    }
  } else if (filtered_h >= (fall_threshold + 0.35f)) {
    // เมื่อคนลุกขึ้นยืนกลับมาสูงกว่าเกณฑ์ (> 1.10m) ให้รีเซ็ตสถานะการล้มเพื่อรองรับการตรวจจับครั้งถัดไป
    if (fall_alerted_this_session) {
      fall_alerted_this_session = false;
      target_event = "enter";
      display_status = "กำลังใช้งาน (เรดาร์)";
      Serial.println("🟢 [FALL RECOVERED] Target stood back up above fall threshold.");
    }
  }
  */
}

// ================= HLK-LD6002C RADAR PARSER =================
// ฟังก์ชันอ่านข้อมูลและถอดรหัสโปรโตคอลของเซนเซอร์เรดาร์ HLK-LD6002C
void readRadar() {
  // บล็อกวินิจฉัยปัญหาการต่อสายและข้อมูลดิบ
  static unsigned long last_avail_check = 0;
  static unsigned long last_valid_packet_time = 0;
  if (last_valid_packet_time == 0) last_valid_packet_time = millis();

  // ระบบสลับขาสัญญาณอัจฉริยะ (Smart Auto-Pin Swapper):
  // จะสลับหาขาที่ถูกต้องเฉพาะตอนบูตเครื่องหากต่อสายผิด แต่เมื่อเจอกล่องข้อมูลที่ถูกต้องครั้งแรกแล้ว
  // จะทำการ LOCK ขานั้นไว้ถาวรทันที ป้องกันสัญญาณหลุดตอนห้องว่าง
  static unsigned long last_pin_swap = 0;
  static bool is_pins_swapped = false;
  static bool pins_locked = false;

  if (last_valid_packet_time > millis() - 1000) {
    pins_locked = true; // ล็อกขาถาวรเมื่อพบแพ็กเกจถูกต้องล่าสุด
  }

  if (!pins_locked && millis() > 10000 && millis() - last_pin_swap > 8000 && RadarSerial.available() > 0) {
    last_pin_swap = millis();
    is_pins_swapped = !is_pins_swapped;
    int rx = is_pins_swapped ? RADAR_TX_PIN : RADAR_RX_PIN;
    int tx = is_pins_swapped ? RADAR_RX_PIN : RADAR_TX_PIN;
    RadarSerial.begin(115200, SERIAL_8N1, rx, tx);
    Serial.printf("🔄 [RADAR SMART-PIN] ยังไม่พบแพ็กเกจถูกต้องตอนบูต -> ทดลองสลับขาซอฟต์แวร์เป็น RX=%d, TX=%d...\n", rx, tx);
  }

  if (millis() - last_avail_check > 4000) {
    last_avail_check = millis();
    int avail = RadarSerial.available();
    Serial.printf("📡 [RADAR HEARTBEAT] readRadar() is running. Buffer: %d bytes\n", avail);
    if (avail == 0) {
      Serial.println("ℹ️ [RADAR DIAGNOSTIC] ไม่พบสัญญาณข้อมูลจากเรดาร์เข้ามาเลย (0 bytes) -> ให้เช็กสายไฟเลี้ยง, ขา GND หรือขา P19 ว่าต่อลงกราวด์แน่นสนิทดีแล้วหรือยัง");
    } else {
      // มีข้อมูลใน Buffer แต่ถ้าไม่ได้รับแพ็กเกจที่ถูกต้องเลยเกิน 10 วินาที
      if (millis() - last_valid_packet_time > 10000) {
        Serial.printf("⚠️ [RADAR DIAGNOSTIC] ได้รับข้อมูลดิบค้างในบัฟเฟอร์ (%d bytes) แต่ถอดรหัสแพ็กเกจไม่สำเร็จ -> อาจเกิดจากความเร็ว Baud rate ผิดพลาด หรือสาย RX/TX ต่อสลับ/หลวม\n", avail);
      }
    }
  }

  String target_event = current_event;
  String display_status = "";
  bool got_valid_packet = false;

  // 1. อ่านทุกแพ็กเกจที่ค้างอยู่ใน Buffer ตอนนี้เพื่ออัปเดตเป็นสถานะล่าสุดจริงๆ
  while (RadarSerial.available() > 0) {
    uint8_t topByte = RadarSerial.peek();
    
    // หากไบต์แรกในบัฟเฟอร์ไม่ใช่ 0x01 (SOF) ให้ลบทิ้งทีละ 1 ไบต์ พร้อมรายงานค่าไบต์ที่พบทางหน้าจอทุกๆ 2 วินาที
    if (topByte != 0x01) {
      static unsigned long last_non_sof_log = 0;
      if (millis() - last_non_sof_log >= 2000) {
        last_non_sof_log = millis();
        Serial.printf("🔍 [RADAR PEEK] Top byte in buffer is 0x%02X (discarding non-0x01 byte)\n", topByte);
      }
      RadarSerial.read();
      continue;
    }
    
    // หากพบ 0x01 แต่ข้อมูลมายังไม่ครบส่วนหัว 8 ไบต์ ให้หยุดรอรอบถัดไป
    if (RadarSerial.available() < 8) {
      break;
    }

    // อ่าน 0x01 (SOF) ออกจากบัฟเฟอร์
    RadarSerial.read();
    
    uint8_t header[7];
    // อ่านข้อมูลส่วนหัวที่เหลืออีก 7 ไบต์
    RadarSerial.readBytes(header, 7);
      
      // คำนวณหา Checksum ของส่วนหัว
      uint8_t xorSum = 0x01;
      for (int i = 0; i < 6; i++) {
        xorSum ^= header[i];
      }
      uint8_t calculated_head_cksum = ~xorSum;
      
      // ปริ้นต์ตรวจสอบค่าไบต์ของส่วนหัวแบบเรียลไทม์ทุกๆ 2 วินาที
      static unsigned long last_peek_time = 0;
      if (millis() - last_peek_time >= 2000) {
        last_peek_time = millis();
        Serial.printf("🔍 [HEADER PEEK] H[0..5]: %02X %02X %02X %02X %02X %02X | H[6]: 0x%02X | Calc: 0x%02X\n",
                      header[0], header[1], header[2], header[3], header[4], header[5], header[6], calculated_head_cksum);
      }
      
      // ยอมรับส่วนหัวหาก Checksum ตรง หรือหากพบรหัสประเภทเรดาร์ (0x0E) ในส่วนหัว
      bool isHeadChecksumValid = (calculated_head_cksum == header[6]) || (xorSum == header[6]) || (header[0] == 0x0E || header[4] == 0x0E);
      if (isHeadChecksumValid) {
        last_valid_packet_time = millis(); // อัปเดตเวลาที่ได้รับข้อมูลถูกต้องล่าสุด
        uint16_t dataLen = (header[2] << 8) | header[3];
        uint16_t totalPayloadBytes = dataLen + 1; // ข้อมูล + DATA_CKSUM 1 ไบต์
        
        // ป้องกันบัฟเฟอร์โอเวอร์โฟลว์ อ่านข้อมูลทั้งหมดที่มากับเฟรมอย่างปลอดภัยไม่เกิน 256 ไบต์
        if (totalPayloadBytes > 0 && totalPayloadBytes <= 256) {
          uint8_t dataBuf[256];
          RadarSerial.readBytes(dataBuf, totalPayloadBytes);
          
          // สแกนคีย์ประเภทเฟรมข้อมูลจากทั้งสองตำแหน่งลอจิก (bytes 0-1 และ bytes 4-5) เพื่อรองรับโปรโตคอลเรดาร์ทุกเวอร์ชัน
          uint16_t typeA = (header[0] << 8) | header[1];
          uint16_t typeB = (header[4] << 8) | header[5];
          
          bool isFallFrame = (typeA == 0x0E02 || typeA == 0x020E || typeB == 0x0E02 || typeB == 0x020E);
          bool isHeightFrame = (typeA == 0x0E0E || typeB == 0x0E0E);
          bool isConfigResponse = (typeA == 0x0E04 || typeA == 0x0E08 || typeB == 0x0E04 || typeB == 0x0E08 || typeA == 0x0E0C || typeB == 0x0E0C);

          // ตรวจสอบประเภทเฟรมรายงานผลการตรวจจับการล้ม (TYPE = 0x0E02)
          if (isFallFrame) {
            uint8_t is_fall = dataBuf[0];    
            uint8_t data_cksum = dataBuf[dataLen]; 
            
            // ตรวจสอบความถูกต้องของ Checksum ข้อมูล (ถ้ามี 1 ไบต์) หรือยอมรับเฟรมถ้าข้อมูลสั้น
            if (dataLen == 1 || data_cksum == (uint8_t)~is_fall) {
              got_valid_packet = true;
              
              // อัปเดตเป้าหมายสถานะการล้มในลูปนี้
              if (is_fall == 0x01) {
                // หากห้องว่าง (exit) จะไม่ประมวลผลการล้ม เพื่อป้องกัน False Alert ตอนไม่มีคน
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
                if (is_falling_candidate) {
                  is_falling_candidate = false;
                  Serial.println("ℹ️ [RADAR] Fall canceled (transient noise or crouch detected).");
                }

                if (current_event == "fall") {
                  // หากกำลังอยู่ในสถานะคนล้ม แล้วเรดาร์ตรวจพบว่ายืนปกติ (is_fall == 0)
                  // ให้นับถอยหลังยืนยันว่าลุกขึ้นยืนจริง (Recovery Confirmation)
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

              // ปริ้นต์ข้อมูลดิบเมื่อมีการเปลี่ยนแปลงสถานะ หรือสรุปทุกๆ 3 วินาทีเพื่อไม่ให้ Log ล้นสาย
              static uint8_t last_printed_is_fall = 0xFF;
              unsigned long now = millis();
              if (is_fall != last_printed_is_fall || now - last_radar_log_time >= 3000) {
                last_printed_is_fall = is_fall;
                last_radar_log_time = now;
                Serial.print("📡 [RAW SENSOR] Direct output from radar -> is_fall: ");
                Serial.println(is_fall == 0x01 ? "1 (🚨 FALL / ล้ม)" : "0 (🟢 NORMAL / ปกติ)");
              }
              
              // ดึงค่าความสูงและส่งเข้าตัวกรองสัญญาณดิจิทัล (Digital Signal Filter)
              float current_height = 0.0;
              if (dataLen >= 4) {
                float temp_h = 0.0;
                memcpy(&temp_h, &dataBuf[1], 4);
                if (temp_h >= 0.1 && temp_h <= 10.0) current_height = temp_h;
                else {
                  uint32_t raw_h = 0;
                  memcpy(&raw_h, &dataBuf[1], 4);
                  if (raw_h > 500) current_height = (float)raw_h / 1000.0;
                  else if (raw_h > 50) current_height = (float)raw_h / 100.0;
                }
              } else if (dataLen >= 2) {
                uint8_t raw_h = dataBuf[1];
                if (raw_h > 50) current_height = (float)raw_h / 100.0;
                else if (raw_h > 0) current_height = (float)raw_h / 10.0;
              } else if (dataLen == 1) {
                // เฟรมสถานะปกติ 1 ไบต์จากชิปเรดาร์: 
                // หาก is_fall == 1 (ล้มลงนอนบนพื้น) -> กำหนดความสูงระดับพื้น (~0.25 m)
                // หาก is_fall == 0 (ยืนขยับปกติ) -> กำหนดความสูงยืนปกติ (~1.70 m)
                if (is_fall == 0x01) {
                  current_height = 0.25f;
                } else if (current_event == "exit") {
                  current_height = 0.00f;
                }
              }
              
              if (current_height > 0.0) {
                processFilteredHeight(current_height, target_event, display_status);
              }
            }
          } 
          else if (isHeightFrame) {
            float height_float = 0.0;
            if (dataLen == 1) {
              uint8_t raw_val = dataBuf[0];
              if (raw_val > 50) {
                height_float = (float)raw_val / 100.0;
              } else {
                height_float = (float)raw_val / 10.0;
              }
            } 
            else if (dataLen >= 4) {
              uint32_t raw_int = 0;
              memcpy(&raw_int, dataBuf, 4);
              
              float temp_float = 0.0;
              memcpy(&temp_float, dataBuf, 4);
              
              if (temp_float >= 0.1 && temp_float <= 10.0) {
                height_float = temp_float;
              } else {
                if (raw_int > 500) {
                  height_float = (float)raw_int / 1000.0;
                } else {
                  height_float = (float)raw_int / 100.0;
                }
              }
            }
            
            if (height_float > 0.0) {
              processFilteredHeight(height_float, target_event, display_status);
            }
          }
          else if (isConfigResponse) {
            Serial.print("📡 [RADAR RESPONSE] Received frame TYPE: 0x");
            Serial.print(typeA < 0x1000 ? "0" : "");
            Serial.print(typeA, HEX);
            Serial.print(" / 0x");
            Serial.print(typeB, HEX);
            Serial.print(" | Result Data: ");
            for (int i = 0; i < dataLen; i++) {
              Serial.print(dataBuf[i] < 0x10 ? "0" : "");
              Serial.print(dataBuf[i], HEX);
              Serial.print(" ");
            }
            Serial.println();
          }
          else {
            // กรณีเป็นเฟรมชนิดอื่นๆ ที่ส่งมาจากชิป ให้ปริ้นต์รายละเอียดออกมาเพื่อการวิเคราะห์
            static unsigned long last_raw_frame_time = 0;
            if (millis() - last_raw_frame_time >= 3000) {
              last_raw_frame_time = millis();
              Serial.printf("📡 [RAW FRAME] TypeA: 0x%04X | TypeB: 0x%04X | Len: %d | Data[0]: 0x%02X\n", typeA, typeB, dataLen, dataBuf[0]);
            }
          }
        }
      }
    }

  // 2. ส่งข้อมูลหลังบ้านเฉพาะเมื่อสถานะสุดท้ายที่แกะได้ มีการเปลี่ยนแปลงจากปัจจุบันจริงๆ
  if (got_valid_packet && target_event != current_event) {
    if (target_event == "fall") {
      Serial.println("\n🚨 [RADAR DETECTED] !!! PERSON FELL !!!");
      current_event = "fall";
      sendEvent("event", "fall", display_status.c_str());
      
      // เคลียร์ Buffer ขยะสะสมทิ้งทันทีหลังบล็อกส่ง HTTP POST เสร็จ
      while (RadarSerial.available() > 0) {
        RadarSerial.read();
      }
    } 
    else if (target_event == "enter" && current_event == "fall") {
      Serial.println("\n🟢 [RADAR DETECTED] Person is no longer in fall state.");
      current_event = "enter";
      sendEvent("event", "enter", display_status.c_str());
      
      // เคลียร์ Buffer ขยะสะสมทิ้งทันทีหลังบล็อกส่ง HTTP POST เสร็จ
      while (RadarSerial.available() > 0) {
        RadarSerial.read();
      }
    }
  }
}
#endif

#if USE_LD2410C
// ================= HLK-LD2410C PRESENCE PARSER (GPIO) =================
// ฟังก์ชันอ่านสถานะจากขา OUT ของเซนเซอร์ HLK-LD2410C เพื่อเปิด-ปิดการใช้งานห้องน้ำอัตโนมัติ
void readLD2410() {
  int presence = digitalRead(LD2410_OUT_PIN);
  static unsigned long last_ld2410_log_time = 0;
  
  // แสดงผลสถานะการมีอยู่ของมนุษย์ใน Terminal ทุกๆ 3 วินาที เพื่อช่วยทดสอบและติดตั้ง
  unsigned long now = millis();
  if (now - last_ld2410_log_time >= 3000) {
    last_ld2410_log_time = now;
    Serial.print("📡 [LD2410 DEBUG] Human presence state: ");
    Serial.println(presence == HIGH ? "OCCUPIED (มีคนอยู่)" : "EMPTY (ห้องว่าง)");
  }

  if (presence == HIGH) {
    // บันทึกเวลาล่าสุดที่ยังตรวจเจอสัญญาณของคน
    last_presence_time = millis();
    
    // หากสถานะปัจจุบันเป็นห้องว่าง (exit) ให้เปลี่ยนเป็นมีคนอยู่ (enter) ทันที
    if (current_event == "exit") {
      current_event = "enter";
      sendEvent("event", "enter", "กำลังใช้งาน (ตรวจพบคนเข้าห้องน้ำ)");
      fall_alerted_this_session = false;
      is_falling_candidate = false;
      is_recovering_candidate = false;
    }
  } else {
    // หากไม่มีคนอยู่ และสถานะเป็น "กำลังใช้งาน" (enter)
    // ให้คอยตรวจจับว่าสัญญาณหายไปนานเกินระยะเวลาหน่วงที่กำหนด (presence_timeout) หรือยัง
    // ⚠️ สำคัญมาก: หากสถานะเป็น "fall" (คนล้ม) จะไม่ตัดเป็น exit อัตโนมัติ เพราะคนที่ล้มอาจนอนนิ่ง หมดสติบนพื้น
    if (current_event == "enter" && (millis() - last_presence_time > presence_timeout)) {
      current_event = "exit";
      sendEvent("event", "exit", "ว่าง (ไม่มีคนอยู่)");
      fall_alerted_this_session = false; // รีเซ็ตการแจ้งเตือนล้มสำหรับผู้ใช้งานคนถัดไป
      is_falling_candidate = false;      // ล้างค่ายืนยันล้มเมื่อออกจากห้อง
      fall_start_time = 0;
      is_recovering_candidate = false;
    }
  }
}
#endif