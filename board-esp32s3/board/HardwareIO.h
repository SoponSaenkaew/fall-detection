#pragma once

#include "SystemConfig.h"

inline void setupHardwareIO() {
  pinMode(BUTTON_ENTER_PIN, INPUT_PULLUP);
  pinMode(BUTTON_FALL_PIN, INPUT_PULLUP);
  pinMode(BUTTON_EXIT_PIN, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH); // สั่ง HIGH ปิดรีเลย์แบบ Low Trigger ป้องกันเสียงไซเรนดังตอนเปิดเครื่อง
}

inline void setRGBColor(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(RGB_LED_PIN, r * RGB_BRIGHTNESS, g * RGB_BRIGHTNESS, b * RGB_BRIGHTNESS);
}

inline void updateRGBStatus(bool wifiConnected, bool mqttConnected) {
  static unsigned long last_flash_time = 0;
  static bool flash_state = false;

  if (!wifiConnected) {
    // WiFi หลุด -> ไฟสีน้ำเงินกะพริบ
    unsigned long current_time = millis();
    if (current_time - last_flash_time >= 500) {
      last_flash_time = current_time;
      flash_state = !flash_state;
      if (flash_state) setRGBColor(0, 0, 100);
      else setRGBColor(0, 0, 0);
    }
    return;
  }

  if (USE_MQTT && !mqttConnected) {
    // กำลังเชื่อมต่อ MQTT -> ไฟสีเหลืองค้าง
    setRGBColor(100, 100, 0);
    return;
  }

  if (current_event == "enter") {
    // กำลังใช้งานห้องน้ำ -> ไฟสีฟ้าครามค้าง
    setRGBColor(0, 100, 200);
    digitalWrite(BUZZER_PIN, HIGH);
  } 
  else if (current_event == "prolonged_stay") {
    // อยู่นานเกินกำหนด -> ไฟสีส้มค้าง และปิดรีเลย์ไซเรน (HIGH) ไม่ให้มีเสียงรบกวน
    setRGBColor(255, 100, 0);
    digitalWrite(BUZZER_PIN, HIGH);
  }
  else if (current_event == "fall") {
    // เกิดเหตุคนล้มฉุกเฉิน -> ไซเรนดังค้าง (Active LOW) + ไฟสีแดงกะพริบถี่
    digitalWrite(BUZZER_PIN, LOW); // สั่งรีเลย์ ON แช่ค้าง ให้ไซเรนทำงานเต็มที่

    unsigned long current_time = millis();
    if (current_time - last_flash_time >= 200) {
      last_flash_time = current_time;
      flash_state = !flash_state;
      if (flash_state) {
        setRGBColor(255, 0, 0);
      } else {
        setRGBColor(0, 0, 0);
      }
    }
  } 
  else {
    // ห้องว่าง -> ไฟสีเขียวค้าง
    setRGBColor(0, 150, 0);
    digitalWrite(BUZZER_PIN, HIGH);
  }
}

inline void checkSimulationButtons() {
  static unsigned long last_button_press = 0;
  unsigned long now = millis();
  if (now - last_button_press < 400) {
    return; // Non-blocking Debounce 400ms ป้องกันการกระตุกของระบบ
  }

  if (digitalRead(BUTTON_ENTER_PIN) == LOW) {
    last_button_press = now;
    current_event = "enter";
    enter_start_time = millis();
    prolonged_stay_alerted = false;
    resetRadarFallFlags();
    sendEvent("event", "enter", "กำลังใช้งาน");
  }
  else if (digitalRead(BUTTON_FALL_PIN) == LOW) {
    last_button_press = now;
    if (current_event == "fall") {
      // ถ้าไซเรนกำลังดังอยู่แล้วกดซ้ำ -> ยกเลิก/สลับเป็นห้องว่าง (Toggle ทดสอบง่ายด้วยปุ่มเดียว)
      current_event = "exit";
      enter_start_time = 0;
      prolonged_stay_alerted = false;
      resetRadarFallFlags();
      sendEvent("event", "exit", "ยกเลิกเตือน/ห้องว่าง");
    } else {
      // สั่งคนล้มฉุกเฉิน (เปิดรีเลย์ไซเรนดัง)
      current_event = "fall";
      setRadarFallAlerted(true);
      sendEvent("event", "fall", "ตรวจพบคนล้ม");
    }
  }
  else if (digitalRead(BUTTON_EXIT_PIN) == LOW) {
    last_button_press = now;
    current_event = "exit";
    enter_start_time = 0;
    prolonged_stay_alerted = false;
    resetRadarFallFlags();
    sendEvent("event", "exit", "ว่าง");
  }
}
