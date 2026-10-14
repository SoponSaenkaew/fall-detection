#pragma once

#include "SystemConfig.h"

#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

inline WiFiManager wm;
inline WiFiClient espClient;
inline PubSubClient mqttClient(espClient);
inline unsigned long last_mqtt_reconnect_attempt = 0;

// ล้างข้อมูล Wi-Fi ที่บันทึกไว้ใน Flash Memory แล้วรีบูตเข้าโหมด AP Portal
inline void resetWiFiSettings() {
  Serial.println("\n🔄 [WIFI RESET] ทำการล้างข้อมูล Wi-Fi ที่บันทึกไว้ใน Flash Memory...");
  setRGBColor(255, 100, 0);
  wm.resetSettings();
  delay(1000);
  Serial.println("🔄 [WIFI RESET] รีสตาร์ตบอร์ดเพื่อเปิดโหมดตั้งค่าใหม่ (AP Portal)...");
  ESP.restart();
}

// ตรวจสอบการกดปุ่ม BOOT (GPIO 0) ค้าง 3 วินาที เพื่อล้างการตั้งค่า Wi-Fi
inline void checkResetButton() {
  if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
    unsigned long pressStart = millis();
    Serial.println("\n🔘 [BOOT BUTTON] กดปุ่ม BOOT... (หากต้องการรีเซ็ต Wi-Fi ให้กดค้าง 3 วินาที)");
    while (digitalRead(BOOT_BUTTON_PIN) == LOW) {
      if (millis() - pressStart >= 3000) {
        Serial.println("⚠️ [BOOT BUTTON] ครบ 3 วินาที! เริ่มกระบวนการล้างค่า Wi-Fi...");
        resetWiFiSettings();
      }
      delay(50);
    }
  }
}

// ================= DIAGNOSTIC HELPERS =================
inline String getWiFiStatusString(uint8_t status) {
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

inline String getMQTTErrorString(int rc) {
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

inline String getHTTPErrorString(int code) {
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
      case 404: return "404 Not Found (ไม่พบปลายทาง: ลิงก์ URL ของ API ผิดพลาด)";
      case 500: return "500 Internal Server Error (ระบบหลังบ้านเกิดข้อผิดพลาด: โปรดตรวจสอบว่าลงทะเบียน ID ในฐานข้อมูลแล้ว)";
      case 502: return "502 Bad Gateway (Nginx Proxy เปิดอยู่ แต่เซิร์ฟเวอร์ Go Backend ด้านในยังไม่ได้รัน!)";
      case 504: return "504 Gateway Timeout (Proxy หมดเวลารอการตอบรับจากระบบหลังบ้าน)";
      default:  return "HTTP Response Code " + String(code);
    }
  }
}

// ================= CUSTOM LIGHTWEIGHT JSON PARSER =================
inline float getJsonFloatValue(String json, String key) {
  int keyIndex = json.indexOf("\"" + key + "\":");
  if (keyIndex == -1) return -1.0; 
  
  int valueStartIndex = keyIndex + key.length() + 3;
  
  int commaIndex = json.indexOf(",", valueStartIndex);
  int braceIndex = json.indexOf("}", valueStartIndex);
  int endIndex = -1;
  
  if (commaIndex != -1 && braceIndex != -1) {
    endIndex = (commaIndex < braceIndex ? commaIndex : braceIndex);
  } else if (commaIndex != -1) {
    endIndex = commaIndex;
  } else {
    endIndex = braceIndex;
  }
  
  if (endIndex == -1) return -1.0;
  
  String valStr = json.substring(valueStartIndex, endIndex);
  valStr.replace("\"", "");
  valStr.trim();
  
  Serial.print("🔍 [JSON PARSE] Key: ");
  Serial.print(key);
  Serial.print(" | Raw String: '");
  Serial.print(valStr);
  Serial.print("' | Float Value: ");
  Serial.println(valStr.toFloat());
  
  return valStr.toFloat();
}

inline void connectWiFi() {
  Serial.println("\n📶 [WIFI] กำลังตรวจสอบและเชื่อมต่อ Wi-Fi ด้วย WiFiManager...");
  
  // ตั้งค่า timeout ของหน้า Portal เป็น 180 วินาที (3 นาที)
  // หากไม่มีผู้ใช้เข้ามาเชื่อมต่อเพื่อตั้งค่าภายใน 3 นาที ให้บอร์ดข้ามไปทำงานส่วนอื่นต่อได้
  wm.setConfigPortalTimeout(180);

  // เพิ่มช่องกรอก Device ID ในหน้า Captive Portal เผื่อต้องการเปลี่ยนรหัสประจำเครื่อง
  WiFiManagerParameter custom_device_id("device_id", "Device ID (เช่น LD6002C_TEST_01)", device_id, 32);
  wm.addParameter(&custom_device_id);

  // ตั้ง Callback แจ้งเตือนเมื่อบอร์ดเข้าสู่โหมด Access Point เพื่อรอการตั้งค่า
  wm.setAPCallback([](WiFiManager *myWiFiManager) {
    Serial.println("\n========================================================");
    Serial.println("📡 [WIFI PORTAL] บอร์ดกำลังเปิด Hotspot สำหรับตั้งค่า Wi-Fi");
    Serial.print("👉 ชื่อ Wi-Fi (SSID) : ");
    Serial.println(myWiFiManager->getConfigPortalSSID());
    Serial.println("👉 รหัสผ่าน Wi-Fi    : 12345678");
    Serial.print("👉 ไอพีสำหรับตั้งค่า : http://");
    Serial.println(WiFi.softAPIP());
    Serial.println("========================================================\n");
    setRGBColor(180, 0, 220); // แสดงไฟสีม่วง (Purple) สำหรับโหมดตั้งค่า Wi-Fi Portal ไม่ซ้ำกับสถานะมีคนใช้งาน (สีฟ้าคราม)
  });

  // สร้างชื่อ Hotspot ของบอร์ด เช่น ESP32S3-LD6002C_TEST_01
  String apName = "ESP32S3-" + String(device_id);

  // สั่งเชื่อมต่อ Wi-Fi ที่บันทึกไว้ใน NVS ถ้าไม่มีจะเปิด AP ชื่อ apName รหัส 12345678
  bool isConnected = wm.autoConnect(apName.c_str(), "12345678");

  if (isConnected) {
    Serial.println("\n✅ [WIFI] WiFi Connected Successfully!");
    Serial.print("👉 [WIFI] Connected SSID : ");
    Serial.println(WiFi.SSID());
    Serial.print("👉 [WIFI] IP Address     : ");
    Serial.println(WiFi.localIP());

    // หากมีการแก้ไข Device ID ผ่านหน้า Portal ให้บันทึกลง Flash
    if (strlen(custom_device_id.getValue()) > 0 && strcmp(device_id, custom_device_id.getValue()) != 0) {
      Serial.printf("💾 [CONFIG] บันทึก Device ID ใหม่: %s\n", custom_device_id.getValue());
      saveDeviceIDToNVS(custom_device_id.getValue());
    }
  } else {
    Serial.println("\n⚠️ [WIFI] Config Portal Timeout หรือเชื่อมต่อไม่สำเร็จ กำลังรันต่อแบบ Offline...");
  }
}

inline void connectMQTTNonBlocking() {
  unsigned long now = millis();
  if (now - last_mqtt_reconnect_attempt > 5000) {
    last_mqtt_reconnect_attempt = now;
    setRGBColor(100, 100, 0);
    Serial.print("📡 [MQTT] Connecting to Broker at ");
    Serial.print(mqtt_broker);
    Serial.print(":");
    Serial.print(mqtt_port);
    Serial.print("... ");
    
    String clientId = "ESP32Client-" + String(device_id);
    String willTopic = mqtt_topic;
    String willPayload = "{\"device_id\":\"" + String(device_id) + "\",\"type\":\"status\",\"name\":\"offline\",\"value\":\"\"}";
    
    if (mqttClient.connect(clientId.c_str(), NULL, NULL, willTopic.c_str(), 0, false, willPayload.c_str())) {
      Serial.println("✅ CONNECTED!");

      // Subscribe รับการตั้งค่าจากหน้าเว็บแบบ Realtime
      String configTopic = "device/" + String(device_id) + "/config";
      if (mqttClient.subscribe(configTopic.c_str())) {
        Serial.printf("📡 [MQTT] Subscribed to [%s] for realtime config!\n", configTopic.c_str());
      } else {
        Serial.printf("❌ [MQTT] Failed to subscribe to [%s]\n", configTopic.c_str());
      }

      sendEvent("status", "online", "ออนไลน์");
      setRGBColor(0, 150, 0);
    } else {
      Serial.print("❌ FAILED. Error code: ");
      Serial.println(getMQTTErrorString(mqttClient.state()));
    }
  }
}

inline bool isWiFiConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

inline bool isMqttConnected() {
  return mqttClient.connected();
}

inline void sendEvent(const char* type, const char* name, const char* display_status) {
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
    clientSecure.setInsecure();
    
    HTTPClient http;
    Serial.println("📤 [HTTP] Sending POST (HTTPS) to: https://40.81.24.21/api/v1/events");
    Serial.print("📝 [HTTP] Payload: ");
    Serial.println(jsonPayload);
    
    http.begin(clientSecure, "40.81.24.21", 443, "/api/v1/events");
    http.addHeader("Host", "fall-detection.sopon-project.me");
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

// ================= DEVICE CONFIGURATION & REALTIME MQTT SYNC =================
inline bool applyDeviceSettings(const String& payload, const char* source = "CONFIG") {
  Serial.printf("\n⚙️ [%s] Applying updated parameters...\n", source);
  Serial.print("📥 [PAYLOAD]: ");
  Serial.println(payload);

  float temp_threshold = getJsonFloatValue(payload, "fall_threshold");
  float temp_height    = getJsonFloatValue(payload, "mount_height");
  float temp_width     = getJsonFloatValue(payload, "room_width");
  float temp_length    = getJsonFloatValue(payload, "room_length");
  float temp_stay_timeout = getJsonFloatValue(payload, "stay_timeout_minutes");

  bool updated = false;

  if (temp_threshold >= 0) {
    fall_threshold = temp_threshold;
#if USE_LD6002C
    sendConfigToRadar(0x0E08, fall_threshold);
    delay(50);
#endif
    updated = true;
  }
  if (temp_height >= 0) {
    mount_height = temp_height;
#if USE_LD6002C
    sendConfigToRadar(0x0E04, mount_height);
    delay(50);
#endif
    updated = true;
  }
  bool sizeChanged = false;
  if (temp_width >= 0) {
    room_width = temp_width;
    sizeChanged = true;
    updated = true;
  }
  if (temp_length >= 0) {
    room_length = temp_length;
    sizeChanged = true;
    updated = true;
  }

#if USE_LD6002C
  if (sizeChanged) {
    sendRoomSizeToRadar(room_width, room_length);
    delay(100);
  }
#endif

  if (temp_stay_timeout > 0) {
    stay_timeout_minutes = temp_stay_timeout;
    prolonged_stay_timeout = (unsigned long)(stay_timeout_minutes * 60.0 * 1000.0);
    Serial.print("⏱️ [CONFIG] Updated stay_timeout_minutes: ");
    Serial.print(stay_timeout_minutes);
    Serial.println(" min");
    updated = true;
  }

  if (updated) {
    saveConfigToNVS(fall_threshold, mount_height, room_width, room_length, stay_timeout_minutes);
    Serial.printf("✅ [%s] Settings applied & saved to Flash successfully!\n\n", source);
  }
  return updated;
}

inline void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  message.reserve(length);
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println("\n---------------------------------------------");
  Serial.printf("📩 [MQTT RX] Topic: %s\n", topic);

  String configTopic = "device/" + String(device_id) + "/config";
  if (String(topic) == configTopic) {
    applyDeviceSettings(message, "MQTT REALTIME");
  }
  Serial.println("---------------------------------------------\n");
}

inline bool fetchDeviceSettings() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ [CONFIG] WiFi disconnected. Cannot fetch settings.");
    return false;
  }

  WiFiClientSecure clientSecure;
  clientSecure.setInsecure();
  
  HTTPClient http;
  String config_path = "/api/v1/config/" + String(device_id);
  
  Serial.println("\n📡 [CONFIG] Fetching latest parameters from web...");
  Serial.print("🔗 [CONFIG] Endpoint: https://40.81.24.21");
  Serial.println(config_path);
  
  http.begin(clientSecure, "40.81.24.21", 443, config_path);
  http.addHeader("Host", "fall-detection.sopon-project.me");
  int httpResponseCode = http.GET();
  
  if (httpResponseCode == 200) {
    String payload = http.getString();
    applyDeviceSettings(payload, "HTTP BOOT");
    http.end();
    return true;
  } else {
    Serial.print("❌ [CONFIG] Fetch Failed. HTTP response: ");
    Serial.println(getHTTPErrorString(httpResponseCode));
    http.end();
    return false;
  }
}

inline void setupNetwork() {
  connectWiFi();

  // ซิงค์เวลาจากอินเทอร์เน็ตผ่าน NTP Server (UTC+7)
  configTzTime("ICT-7", "pool.ntp.org", "time.nist.gov");

  // ArduinoOTA สำหรับอัปเดตแบบไร้สาย
  ArduinoOTA.setHostname("esp32s3-fall");
  ArduinoOTA.begin();

  if (USE_MQTT) {
    mqttClient.setServer(mqtt_broker, mqtt_port);
    mqttClient.setBufferSize(512);
    mqttClient.setCallback(mqttCallback);
  }
}

inline void handleNetwork() {
  ArduinoOTA.handle();

  static bool wasConnected = false;
  bool isConnected = (WiFi.status() == WL_CONNECTED);

  if (!isConnected) {
    wasConnected = false;
    static unsigned long last_wifi_retry = 0;
    if (millis() - last_wifi_retry > 10000) {
      last_wifi_retry = millis();
      Serial.println("📶 [WIFI] Reconnecting in background (Non-blocking)...");
      WiFi.disconnect();
      WiFi.reconnect();
    }
  } else {
    if (!wasConnected) {
      wasConnected = true;
      Serial.println("✅ [WIFI] Reconnected successfully in background!");
      Serial.print("👉 [WIFI] IP Address: ");
      Serial.println(WiFi.localIP());
    }

    if (USE_MQTT) {
      if (!mqttClient.connected()) {
        connectMQTTNonBlocking();
      } else {
        mqttClient.loop();
      }
    }
  }
}
