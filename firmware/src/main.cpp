#include <Arduino.h>
#include "network_utils.h"

// กำหนดตัวแปรจริง
float fallThreshold = 0.0, mountHeight = 0.0, roomWidth = 0.0, roomLength = 0.0;

// ขาปุ่มกดตามที่ต่อไว้ใน diagram.json
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

    fetchDeviceSettings();
}

void loop() {
    // ตรวจเช็คปุ่มกดแต่ละอัน
    if (digitalRead(btnConfig) == LOW) { 
        Serial.println("Action: Fetching Config...");
        fetchDeviceSettings(); 
        delay(500); 
    }
    if (digitalRead(btnEnter) == LOW)  { 
        sendEvent("enter", "user_in"); 
        delay(500); 
    }
    if (digitalRead(btnFall) == LOW)   { 
        sendEvent("fall", "detected"); 
        delay(500); 
    }
    if (digitalRead(btnExit) == LOW)   { 
        sendEvent("exit", "user_out"); 
        delay(500); 
    }
    if (digitalRead(btnOnline) == LOW) { 
        sendEvent("online", "ready"); 
        delay(500); 
    }
}