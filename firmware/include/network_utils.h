#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h> 

// --- การตั้งค่า (Constants) ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* baseUrl = "http://192.168.1.92:8080/api/v1"; 
const char* deviceId = "LD6002C_MASTER_01";

// กำหนด IP ของตู้ไปรษณีย์ MQTT (คอมพิวเตอร์ของเซนเซย์)
// const char* mqtt_server = "192.168.1.92"; 
const char* mqtt_server = "broker.hivemq.com";

// --- ตัวแปร Global ---
extern float fallThreshold;
extern float mountHeight;
extern float roomWidth;
extern float roomLength;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// --- ฟังก์ชันเชื่อมต่อ MQTT ---
void connectMQTT() {
    mqttClient.setServer(mqtt_server, 1883);
    while (!mqttClient.connected()) {
        Serial.print("\r\n[MQTT] Connecting to Broker...");
        if (mqttClient.connect(deviceId)) { 
            Serial.println(" Connected! 🌐");
        } else {
            Serial.print(" Failed! Retrying in 5 sec...");
            delay(5000);
        }
    }
}

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

// --- ฟังก์ชันส่ง Event ผ่าน MQTT 🚀 ---
void sendEvent(String eventName, String value) {
    if (!mqttClient.connected()) {
        connectMQTT();
    }
    mqttClient.loop(); // สั่งให้ MQTT ทำงาน

    // แพ็กข้อมูลใส่กล่อง JSON
    StaticJsonDocument<200> doc;
    doc["device_id"] = deviceId;
    doc["type"] = (eventName == "online") ? "status" : "event";
    doc["name"] = eventName;
    doc["value"] = value;
    doc["metadata"] = "{}";

    String json;
    serializeJson(doc, json);
    
    bool success = mqttClient.publish("sensor/events", json.c_str());
    
    Serial.printf("\r\n[MQTT EVENT] %-6s | Value: %-10s | Success: %d\r\n", 
                  eventName.c_str(), value.c_str(), success);
}

#endif