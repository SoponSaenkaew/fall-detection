#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// --- การตั้งค่า (Constants) ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* baseUrl = "http://rico-concord-mary-beast.trycloudflare.com/api/v1"; 
const char* deviceId = "LD6002C_MASTER_01";

// --- ตัวแปร Global ---
extern float fallThreshold;
extern float mountHeight;
extern float roomWidth;
extern float roomLength;

// --- ฟังก์ชันดึง Config (GET) ---
void fetchDeviceSettings() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String fullUrl = String(baseUrl) + "/config/" + String(deviceId);
        
        http.begin(fullUrl);
        http.setTimeout(5000);
        int httpCode = http.GET();

        if (httpCode == 200) {
            StaticJsonDocument<512> doc;
            deserializeJson(doc, http.getString());
            
            fallThreshold = doc["fall_threshold"];
            mountHeight   = doc["mount_height"];
            roomWidth     = doc["room_width"];
            roomLength    = doc["room_length"];
            
            // ✨ ใช้ \r\n เพื่อบังคับให้ขึ้นบรรทัดใหม่แบบชิดซ้ายเป๊ะๆ ค่ะ
            Serial.print("\r\n====================================");
            Serial.print("\r\n   [SYSTEM CONFIGURATION UPDATED]   ");
            Serial.print("\r\n====================================");
            Serial.printf("\r\n > Device ID      : %s", deviceId);
            Serial.printf("\r\n > Fall Threshold : %.2f", fallThreshold);
            Serial.printf("\r\n > Mount Height   : %.2f m", mountHeight);
            Serial.printf("\r\n > Room Dimension : %.2f x %.2f m", roomWidth, roomLength);
            Serial.print("\r\n====================================\r\n");
        } else {
            Serial.printf("\r\n[SYSTEM] Get Config Failed (Code: %d)\r\n", httpCode);
        }
        http.end();
    }
}

// --- ฟังก์ชันส่ง Event (POST) ---
void sendEvent(String eventName, String value) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(String(baseUrl) + "/events");
        http.setTimeout(3000); // รอแค่ 3 วินาทีพอค่ะ จะได้ไม่หน่วง
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Connection", "keep-alive"); // ช่วยให้ส่งครั้งต่อไปไวขึ้นค่ะ

        StaticJsonDocument<200> doc;
        doc["device_id"] = deviceId;
        doc["type"] = (eventName == "online") ? "status" : "event";
        doc["name"] = eventName;
        doc["value"] = value;
        doc["metadata"] = "PlatformIO Test";

        String json;
        serializeJson(doc, json);
        int httpCode = http.POST(json);
        
        // ✨ ปรับการแสดงผล Log ให้เรียงบรรทัดสวยงาม
        Serial.printf("\r\n[EVENT] %-6s | Value: %-10s | Result: %d\r\n", 
                      eventName.c_str(), value.c_str(), httpCode);
        
        http.end();
    }
}

#endif