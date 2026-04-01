# 🛰️ IoT Firmware (ESP32-S3)

ส่วนควบคุม Hardware สำหรับอ่านค่าจากเซนเซอร์ mmWave และสื่อสารกับ Backend API

## 🚀 การเริ่มต้นใช้งาน
1. เปิดโฟลเดอร์นี้ด้วย **VS Code** ที่ติดตั้ง **PlatformIO IDE**
2. ตรวจสอบไฟล์ `include/network_utils.h` และตั้งค่า `baseUrl` ให้ตรงกับ Backend ของคุณ
3. กดปุ่ม **Build** (รูปเครื่องหมายถูก) เพื่อติดตั้ง Library และคอมไพล์โค้ด

## 🎮 การจำลองสถานการณ์ (Simulation)
- โปรเจกต์นี้รองรับการรันบน **Wokwi Simulator** ภายใน VS Code
- ใช้ไฟล์ `diagram.json` เพื่อจำลองการต่อวงจรปุ่มกดและ ESP32-S3
- รันคำสั่ง `Wokwi: Start Simulator` เพื่อทดสอบ Logic การส่งข้อมูลโดยไม่ต้องมีบอร์ดจริง

## 📦 ไลบรารีที่สำคัญ
- `ArduinoJson`: สำหรับจัดการข้อมูล JSON ระหว่างอุปกรณ์และ Server
- `HTTPClient`: สำหรับส่ง Event และดึง Config