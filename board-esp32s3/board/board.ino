#include "SystemConfig.h"
#include "HardwareIO.h"
#include "NetworkManager.h"
#include "RadarSensors.h"

void setup() {
  Serial.begin(115200);
  delay(2000);

  // 1. โหลดการตั้งค่าพารามิเตอร์ล่าสุดจาก NVS Flash
  loadSavedConfig();

  Serial.println("\n=============================================");
  Serial.println("Starting ESP32-S3 Production Node (Unified)...");
  Serial.print("Target Device ID: ");
  Serial.println(device_id);
  Serial.print("Communication mode: ");
  Serial.println(USE_MQTT ? "MQTT (QoS 0)" : "HTTP POST (HTTPS Secure)");
  Serial.println("=============================================\n");

  // 2. เริ่มต้นระบบ I/O ฮาร์ดแวร์ (ไฟ RGB, ไซเรน/รีเลย์, ปุ่มกด)
  setupHardwareIO();

  // 3. เริ่มต้นระบบเครือข่าย (WiFi, NTP Time Sync, ArduinoOTA, MQTT)
  setupNetwork();

  // 4. เริ่มต้นเซนเซอร์เรดาร์ตรวจจับการล้ม HLK-LD6002C
#if USE_LD6002C
  setupRadarLD6002C();
#endif

  // 5. เริ่มต้นเซนเซอร์ตรวจจับคนเข้า-ออก HLK-LD2410C
#if USE_LD2410C
  setupRadarLD2410C();
#endif

  // 6. ดึงค่าคอนฟิกล่าสุดจากเว็บหลังบ้าน
  fetchDeviceSettings();
}

void loop() {
  // 1. จัดการงานระบบเครือข่าย (OTA, ตรวจสอบ WiFi, จัดการ MQTT)
  handleNetwork();

  // 2. อ่านค่าจากเซนเซอร์เรดาร์ตรวจจับการล้ม
#if USE_LD6002C
  readRadar();
#endif

  // 3. อ่านค่าจากเซนเซอร์เรดาร์ตรวจจับคนเข้า-ออก
#if USE_LD2410C
  readLD2410();
#endif

  // 4. ตรวจสอบปุ่มกดจำลองสถานะ
  checkSimulationButtons();

  // 5. อัปเดตไฟแสดงสถานะ RGB และเสียง Buzzer/Relay
  updateRGBStatus(isWiFiConnected(), isMqttConnected());

  delay(1);
}
