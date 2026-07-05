#include <Arduino.h>
#include "network_utils.h"

// กำหนดตัวแปรจริง (ผูกกับ extern ใน network_utils.h)
float fallThreshold = 0.0, mountHeight = 0.0, roomWidth = 0.0, roomLength = 0.0;

// ขาปุ่มกดตามการกำหนดพินในไฟล์ diagram.json
const int btnConfig = 1, btnEnter = 2, btnFall = 4, btnExit = 5, btnOnline = 6;

void setup() {
    Serial.begin(115200);
    pinMode(btnConfig, INPUT_PULLUP);
    pinMode(btnEnter,  INPUT_PULLUP);
    pinMode(btnFall,   INPUT_PULLUP);
    pinMode(btnExit,   INPUT_PULLUP);
    pinMode(btnOnline, INPUT_PULLUP);

    WiFi.begin(ssid, password);
    Serial.print("Connecting WiFi");
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println("\r\nWiFi Connected! ✨");

    // ดึง Config จาก HTTP ทันทีที่เปิดเครื่อง
    fetchDeviceSettings();
}

void loop() {
    // ให้ MQTT สแตนด์บายทำงานอยู่เบื้องหลัง
    if (WiFi.status() == WL_CONNECTED && !mqttClient.connected()) {
        connectMQTT();
    }
    mqttClient.loop();

    // ตรวจเช็คปุ่มกดแต่ละอัน (จำลองเซ็นเซอร์)
    if (digitalRead(btnConfig) == LOW) { 
        Serial.println("\r\n[BTN] Action: Fetching Config...");
        fetchDeviceSettings(); 
        delay(1000); // หน่วงเวลาป้องกันการกดซ้ำ
    }
    if (digitalRead(btnEnter) == LOW)  { 
        sendEvent("enter", "user_in"); 
        delay(1000); 
    }
    if (digitalRead(btnFall) == LOW)   { 
        sendEvent("fall", "detected"); 
        delay(1000); 
    }
    if (digitalRead(btnExit) == LOW)   { 
        sendEvent("exit", "user_out"); 
        delay(1000); 
    }
    if (digitalRead(btnOnline) == LOW) { 
        sendEvent("online", "ready"); 
        delay(1000); 
    }
}