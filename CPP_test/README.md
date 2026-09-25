# HLK-LD6002C Radar UART Parser Lab (Arduino Style 1:1)

โฟลเดอร์นี้จัดเตรียมสภาพแวดล้อมภาษา C++ ที่จำลองสภาพแวดล้อมของ **Arduino / ESP32-S3** เอาไว้บนคอมพิวเตอร์ 100% เพื่อให้คุณสามารถทดลองเขียนและแกะ UART ของเรดาร์ **HLK-LD6002C** ในรูปแบบเดียวกับบนบอร์ดจริง:

* ✅ ใช้คำสั่ง `Serial.print()`, `Serial.println()`, `Serial.printf()`
* ✅ ใช้สตรีม `RadarSerial.available()`, `RadarSerial.peek()`, `RadarSerial.read()`, `RadarSerial.readBytes()`
* ✅ ใช้ฟังก์ชันเวลา `millis()`, `delay()`
* ✅ มีโครงสร้าง `setup()` และ `loop()`
* 🚀 **โค้ดฟังก์ชัน `readRadar()` และ `sendConfigToRadar()` ใน `main.cpp` เขียนเหมือนใน `board-esp32s3/board/RadarSensors.h` ทุกประการ สามารถคัดลอกไปวางบนบอร์ดจริงได้ทันทีแบบไม่ต้องแปลงไวยากรณ์!**

---

## 📁 โครงสร้างไฟล์

| ไฟล์ | หน้าที่ |
| :--- | :--- |
| [`Arduino.h`](./Arduino.h) | เลเยอร์จำลองคำสั่ง Arduino (`HardwareSerial`, `Serial`, `String`, `millis()`, `delay()`) สำหรับรันบน PC |
| [`main.cpp`](./main.cpp) | โค้ดหลักที่เขียนสไตล์เดียวกับบนบอร์ด ESP32-S3 พร้อมตัวจำลองป้อนข้อมูล UART เพื่อทดสอบ |
| [`build.ps1`](./build.ps1) | สคริปต์ PowerShell คอมไพล์ด้วย `g++` และรันให้อัตโนมัติ |
| [`Makefile`](./Makefile) | สำหรับสั่งคอมไพล์ด้วย `mingw32-make` |

---

## 🚀 วิธีการทดสอบและสั่งรัน

เปิด PowerShell หรือ Terminal ที่โฟลเดอร์ `CPP_test` แล้วสั่ง:
```powershell
.\build.ps1
```
หรือคอมไพล์ด้วย g++ โดยตรง:
```bash
g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o output/main.exe
.\output\main.exe
```

---

## 🎮 เมนูจำลองสถานการณ์ในโปรแกรม

เมื่อสั่งรัน โปรแกรมจะเรียก `setup()` เพื่อจำลองการบูตเรดาร์และส่งค่า Config เริ่มต้น จากนั้นจะมีเมนูให้ทดลองป้อนแพ็กเก็ตจำลองเข้าสู่ `RadarSerial`:

```text
1. ส่งเฟรมล้ม (0x0E02, is_fall = 1)
2. ส่งเฟรมปกติ/ยืน (0x0E02, is_fall = 0)
3. ส่งสัญญาณรบกวน (Noise ขยะก่อนแล้วตามด้วยเฟรมล้ม)
4. ส่งเฟรมล้มค้างเกิน 2 วินาที (ทดสอบการหน่วงเวลายืนยันล้มจริง)
5. ส่งเฟรมลุกยืนค้างเกิน 2.5 วินาที (ทดสอบการ Recovery กลับสู่ปกติ)
6. ออกจากโปรแกรม
```

---

## ✍️ จุดที่คุณสามารถทดลองแก้ไขโค้ด

เข้าไปที่ไฟล์ [`main.cpp`](./main.cpp) แล้วลองแก้ไขฟังก์ชัน:
* **`void readRadar()`:** ปรับเปลี่ยนตรรกะการอ่านไบต์, Checksum, หรือการคัดกรองสัญญาณ
* **`void sendConfigToRadar(uint16_t type, float value)`:** ทดลองแพ็กเฟรมคำสั่งไบนารี
* เมื่อแก้ไขและทดสอบจนได้ผลลัพธ์ที่ต้องการแล้ว สามารถยกโค้ดฟังก์ชันดังกล่าวไปใส่ใน [`board-esp32s3/board/RadarSensors.h`](../board-esp32s3/board/RadarSensors.h) ได้ทันทีครับ
