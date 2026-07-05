#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* ssid = "you wifi name";
const char* password = "you wifi password";

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");
  
  // เชื่อมต่อ WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // --- ตั้งค่าสำหรับ ArduinoOTA ---
  // พี่สามารถตั้ง Port หรือ Password สำหรับความปลอดภัยเพิ่มได้นะคะ (เลือกเปิดใช้งานได้ค่ะ)
  // ArduinoOTA.setPort(3232);
  // ArduinoOTA.setHostname("my-esp32-device"); // ตั้งชื่ออุปกรณ์
  // ArduinoOTA.setPassword("admin"); // รหัสผ่านตอนกดอัพโหลดโค้ด

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) type = "sketch";
    else type = "filesystem"; // สำหรับ SPIFFS
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
  Serial.println("Ready for OTA!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // โค้ด Setup ของพี่ใส่ต่อตรงนี้ได้เลยค่ะ
  pinMode(2, OUTPUT); // ทดสอบกับไฟ LED บนบอร์ด
}

void loop() {
  // ⚠️ สำคัญมาก!! ห้ามลืมฟังก์ชันนี้ใน loop เด็ดขาดนะคะ ไม่งั้นบอร์ดจะไม่รับ OTA ครั้งต่อไปค่ะ
  ArduinoOTA.handle(); 

  // --- โค้ดทำงานปกติของพี่ (ห้ามใช้ delay แบบค้างนานๆ นะคะ) ---
  // แนะนำให้ใช้ millis() แทน delay เพื่อไม่ให้ไปบล็อกการทำงานของ OTA ค่ะ
  digitalWrite(2, HIGH);
  delay(1000); 
  digitalWrite(2, LOW);
  delay(1000);
}