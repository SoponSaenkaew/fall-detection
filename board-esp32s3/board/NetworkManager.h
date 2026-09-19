#pragma once

#include "SystemConfig.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

inline WiFiClient espClient;
inline PubSubClient mqttClient(espClient);
inline unsigned long last_mqtt_reconnect_attempt = 0;

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
  Serial.println();
  Serial.print("📶 [WIFI] Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, IPAddress(8, 8, 8, 8), IPAddress(1, 1, 1, 1));
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
    Serial.print("📥 [CONFIG] Server payload: ");
    Serial.println(payload);
    
    float temp_threshold = getJsonFloatValue(payload, "fall_threshold");
    float temp_height    = getJsonFloatValue(payload, "mount_height");
    float temp_width     = getJsonFloatValue(payload, "room_width");
    float temp_length    = getJsonFloatValue(payload, "room_length");
    
    if (temp_threshold >= 0) {
      fall_threshold = temp_threshold;
#if USE_LD6002C
      sendConfigToRadar(0x0E08, fall_threshold);
      delay(50);
#endif
    }
    if (temp_height >= 0) {
      mount_height = temp_height;
#if USE_LD6002C
      sendConfigToRadar(0x0E04, mount_height);
      delay(50);
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
    
    saveConfigToNVS(fall_threshold, mount_height, room_width, room_length);
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
      WiFi.begin(ssid, password);
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
