# 🚨 ระบบตรวจจับการล้มอัจฉริยะ (Smart Fall Detection System) — รุ่น Demo No-Auth 🌟

ยินดีต้อนรับสู่โปรเจกต์ **ระบบตรวจจับการล้มอัจฉริยะ (Smart Fall Detection System)** ในสาขา **`demo-no-auth`** 

สาขานี้ถูกพัฒนาขึ้นมาเป็นพิเศษสำหรับการ**นำเสนองาน (Live Demo)** และการทดสอบระบบอย่างรวดเร็ว โดยมีการปรับปรุงโครงสร้างจากสาขาหลักดังนี้:
1. **ระบบ Simple No-Auth**: ปิดระบบการตรวจสอบสิทธิ์ผ่าน JWT Token ชั่วคราว ทำให้เปิดหน้าจอขึ้นมาก็เริ่มทำงานได้ทันทีโดยไม่ต้อง Login
2. **ระบบสร้างข้อมูลตัวอย่างอัตโนมัติ (Auto-Seeding)**: ทันทีที่รันระบบหลังบ้าน ระบบจะสร้างอาคารจำลองพร้อมห้องน้ำ 5 ห้องที่มีสถานะการใช้งานที่แตกต่างกันในฐานข้อมูลโดยอัตโนมัติ
3. **ระบบแจ้งเตือนแบบเรียลไทม์ (Real-time Event Streaming & Notification)**: หน้าจอติดตามห้องและสถานะเซนเซอร์แบบเรียลไทม์ผ่านระบบ WebSockets และระบบส่งข้อความเตือนภัยฉุกเฉินผ่านทาง LINE Notify ทันทีเมื่อเกิดสัญญาณล้ม
4. **โปรแกรมจำลองของฝั่ง Backend (CLI Event Simulator)**: เครื่องมือจำลองบอร์ดสำหรับยิงสัญญาณเหตุการณ์ต่าง ๆ เพื่อทดสอบการอัปเดตแบบเรียลไทม์บนหน้าจอแดชบอร์ด

---

## 📂 โครงสร้างโฟลเดอร์ของโปรเจกต์ (Project Structure)

```text
fall-detection/
├── backend/            # ระบบ API จัดการข้อมูล (Go/Gin) และ CLI Simulator
│   ├── cmd/
│   │   ├── server/     # ซอร์สโค้ดฝั่ง Server (ตัวหลักสำหรับเชื่อมฐานข้อมูล, WS และ MQTT)
│   │   └── simulator/  # [NEW] โปรแกรมจำลองการส่งข้อมูลบอร์ดผ่าน CLI Menu
│   └── internal/       # แพ็คเกจการประมวลผลภายใน (Database, MQTT, WebSocket, IoT)
├── frontend/           # ระบบ Dashboard ตรวจสอบสถานะ (Next.js/TS/Tailwind)
│   └── src/
│       ├── app/
│       │   ├── dashboard/   # หน้าจอตั้งค่าระยะติดตั้งและขอบเขตเซนเซอร์ (สำหรับแอดมิน)
│       │   └── buildings/   # [NEW] หน้าจอติดตามสถานะห้องและอุปกรณ์รายอาคารเรียลไทม์
│       └── components/      # ส่วน UI ย่อยต่าง ๆ
├── board-esp32s3/      # ซอร์สโค้ดควบคุมบอร์ด ESP32-S3 รูปแบบ (.ino) สำหรับ Arduino IDE
├── mosquitto/          # ไฟล์ตั้งค่าและสิทธิ์ของ MQTT Broker (Mosquitto)
├── docs/               # เอกสารข้อมูลทางเทคนิคและโปรโตคอลเซนเซอร์เรดาร์ mmWave
└── docker-compose.yml  # สคริปต์ Docker สำหรับเปิดฐานข้อมูล PostgreSQL และ MQTT Broker
```

---

## 🛠️ รายละเอียดฟีเจอร์เด่นในรุ่น Demo

### 1. ระบบติดตามสถานะห้องน้ำแบบเรียลไทม์ (Real-time Room Status & Alert Dashboard)
เมื่อเข้าสู่หน้าจอ **ผังอาคาร (Monitor)** ที่พาร์ท `/buildings` ระบบจะดึงและแสดงผลข้อมูลสถานะแบบเรียลไทม์ผ่าน WebSocket:
* **สถานะเชื่อมต่อ (Online Status)**: แสดงผลว่าอุปกรณ์ของแต่ละห้องน้ำพร้อมใช้งานหรือไม่ (`online` / `offline`)
* **สถานะความปลอดภัย (Presence & Alert Status)**: แสดงผลความเคลื่อนไหวล่าสุด เช่น ว่างปกติ (`🟢 ว่าง`), กำลังใช้งาน (`🚪 กำลังใช้งาน`), และการเกิดอุบัติเหตุล้ม (`คนล้ม!!! ⚠️`)
* **ระบบแจ้งเตือนภัยเร่งด่วน (Emergency Alert)**: เมื่อเซนเซอร์ตรวจพบการล้ม (Fall Event) ระบบจะส่งสัญญาณกะพริบสีแดงเตือนภัยขนาดใหญ่บน Dashboard ทันที พร้อมยิงข้อมูลพิกัดการล้มแจ้งเตือนผู้ดูแลระบบผ่าน LINE Notify และ Webhook

### 2. ข้อมูลจำลองตั้งต้น (Initial Seed Data)
เมื่อเปิดรัน Backend ครั้งแรก ระบบจะสร้างข้อมูลจำลองให้กับ `admin@example.com` (user_id = 1) ทันที:
* **อาคาร 1** (Group 1)
  * **ห้องน้ำ 101**: สถานะ `online` | เหตุการณ์ล่าสุด: `fall` (ตรวจพบการล้มฉุกเฉิน)
  * **ห้องน้ำ 102**: สถานะ `online` | เหตุการณ์ล่าสุด: `exit` (ปลอดภัยปกติ ไม่มีผู้ใช้งาน)
  * **ห้องน้ำ 103**: สถานะ `online` | เหตุการณ์ล่าสุด: `enter` (มีคนอยู่ในห้องน้ำกำลังใช้งาน)
  * **ห้องน้ำ 104**: สถานะ `online` | เหตุการณ์ล่าสุด: `exit` (ปลอดภัยปกติ ไม่มีผู้ใช้งาน)
  * **ห้องน้ำ 105**: สถานะ `offline` | บอร์ดปิดการเชื่อมต่อ

---

## 🚀 ขั้นตอนการเริ่มรันระบบเดโม (Quick Start Guide)

เซนเซย์สามารถรันระบบทดสอบได้ตามขั้นตอนดังต่อไปนี้:

### **ขั้นตอนที่ 1: รัน Docker Infrastructure**
เปิดใช้งานระบบฐานข้อมูล PostgreSQL และ MQTT Broker:
```bash
docker compose up -d
```
* **PostgreSQL DB** ทำงานที่พอร์ต `5432`
* **MQTT Broker (Mosquitto)** ทำงานที่พอร์ต `1883`

---

### **ขั้นตอนที่ 2: รันระบบหลังบ้าน (Backend API)**
1. ย้ายเข้าโฟลเดอร์ `backend`:
   ```bash
   cd backend
   ```
2. ติดตั้งโมดูล Go:
   ```bash
   go mod tidy
   ```
3. รันเซิร์ฟเวอร์หลัก:
   ```bash
   go run cmd/server/main.go
   ```
   * เซิร์ฟเวอร์จะเชื่อมต่อฐานข้อมูล Docker อัตโนมัติและสตรีมข้อมูลทางพอร์ต `8080`
   * ระบบจะทำการ Auto-Seed ข้อมูลห้องน้ำ 101 - 105 ให้ทันทีเมื่อตรวจพบว่าไม่มีข้อมูลกลุ่ม

---

### **ขั้นตอนที่ 3: รันระบบหน้าบ้าน (Frontend Dashboard)**
1. ย้ายเข้าโฟลเดอร์ `frontend`:
   ```bash
   cd frontend
   ```
2. ติดตั้งไลบรารี:
   ```bash
   npm install
   ```
3. รันโปรเจกต์ Next.js:
   ```bash
   npm run dev
   ```
4. เปิดเว็บเบราว์เซอร์แล้วเข้าไปที่: [http://localhost:3000/buildings](http://localhost:3000/buildings)
   * คุณจะเข้าสู่ **หน้าจอผังอาคาร (Monitor)** โดยไม่ต้องทำการ Login หรือกรอกรหัสผ่านใด ๆ ทั้งสิ้น!
   * สามารถสลับระหว่าง **ผังอาคาร (Monitor)** และ **ตั้งค่าอุปกรณ์ (Admin)** ได้ที่แถบเมนูด้านบนซ้าย

---

### **ขั้นตอนที่ 4: รันระบบจำลองเหตุการณ์ผ่าน Command Line (CLI Simulator)**
เพื่อสตรีมข้อมูลจำลองเหตุการณ์ต่าง ๆ เข้ามาโดยไม่ต้องใช้บอร์ดจริง:
1. เปิด Command Prompt / Terminal ใหม่แล้วย้ายเข้าโฟลเดอร์ `backend`:
   ```bash
   cd backend
   ```
2. รันคำสั่งเปิดเครื่องมือจำลอง:
   ```bash
   go run cmd/simulator/main.go
   ```
3. หน้าจอเทอร์มินัลจะปรากฏเมนูให้กดเลือกเลขสถานการณ์ต่าง ๆ:
   ```text
   เลือกเหตุการณ์ที่ต้องการส่งจำลองไปยังเซิร์ฟเวอร์:
   1) จำลองผู้สูงอายุเดินเข้าห้อง (Event: enter)
   2) จำลองผู้สูงอายุล้มลง! (Event: fall) **[แจ้งเตือน LINE]**
   3) จำลองผู้สูงอายุเดินออกจากห้อง (Event: exit)
   4) จำลองอุปกรณ์ออนไลน์ (Status: online)
   5) จำลองอุปกรณ์ออฟไลน์ (Status: offline)
   6) รันสถานการณ์จำลองต่อเนื่อง (Enter -> Fall -> Exit)
   0) ออกจากโปรแกรมจำลอง
   ```
4. ทดลองกดเลือกข้อ **`6`** จากนั้นเปิดหน้าจอเว็บเบราว์เซอร์ดูความเปลี่ยนแปลงแบบเรียลไทม์!

---

## 🛰️ ซอร์สโค้ดสำหรับบอร์ด ESP32-S3 (Arduino IDE)

หากมีฮาร์ดแวร์จริงหรือต้องการอัปโหลดโค้ดเข้าบอร์ด ESP32-S3:
1. เข้าไปที่โฟลเดอร์ [board-esp32s3](file:///d:/Final%20project/fall-detection/board-esp32s3) และเปิดไฟล์ `board.ino` ด้วยโปรแกรม Arduino IDE
2. ปรับแต่งชื่อ WiFi (`ssid`) และรหัสผ่าน (`password`) ในโค้ด
3. อัปโหลดโค้ดไปยังบอร์ดจริง โดยรองรับการอัปเกรดซอร์สโค้ดแบบไร้สาย (ArduinoOTA)

---

## ☁️ การติดตั้งระบบบนคลาวด์ Azure (Production Deployment)

ระบบเดโมและบอร์ดจริงสามารถเชื่อมต่อผ่านหน้าเว็บไซต์หลักที่เปิดบริการจริงบนคลาวด์:
*   **ลิงก์ระบบหน้าบ้าน (Dashboard Real-time):** [https://fall-detection.sopon-project.me/buildings](https://fall-detection.sopon-project.me/buildings)

### **สถาปัตยกรรมบนระบบ Production**
*   **คลาวด์เซิร์ฟเวอร์ (Azure VM B1s):** ระบบปฏิบัติการ Ubuntu (1 vCPU, 1 GiB RAM) มีการตั้งค่าหน่วยความจำเสมือน (Swap File) ขนาด 4 GiB เพื่อความเสถียรของระบบ
*   **บริการฐานข้อมูลคลาวด์ (Azure Database for PostgreSQL):** Flexible Server (`sopon-postgres.postgres.database.azure.com` พอร์ต 5432) บังคับเชื่อมต่อแบบปลอดภัยผ่าน SSL (`sslmode=require`)
*   **คลังจัดเก็บอิมเมจ (Azure Container Registry - ACR):** จัดเก็บ Docker Image ไว้ที่ `soponproject.azurecr.io`
*   **การจัดการช่องทางและใบรับรอง (Reverse Proxy & SSL):** ใช้ **Nginx Proxy** (`~/nginx-proxy/` บน VM) ผูกใบรับรองความปลอดภัย HTTPS (SSL) ของ Let's Encrypt ผ่าน Certbot บนโดเมน `fall-detection.sopon-project.me`

### **ขั้นตอนการพัฒนาและอัปเดตระบบ (Deployment Workflow)**
1. **เขียนโค้ดและทดสอบ:** พัฒนาและแก้ไขซอร์สโค้ดบนเครื่องคอมพิวเตอร์ของคุณจนผ่านการทดสอบ
2. **คอมไพล์เป็น Docker Image:**
    ```bash
    # คอมไพล์ฝั่งหลังบ้าน
    docker build -t soponproject.azurecr.io/fall-detection-backend:latest ./backend

    # คอมไพล์ฝั่งหน้าบ้าน
    docker build -t soponproject.azurecr.io/fall-detection-frontend:latest ./frontend
    ```
3. **อัปโหลด Image ขึ้นคลัง ACR:**
    ```bash
    docker push soponproject.azurecr.io/fall-detection-backend:latest
    docker push soponproject.azurecr.io/fall-detection-frontend:latest
    ```
4. **ดึงข้อมูลเพื่อรันอัปเดตบน VM (ผ่าน SSH):**
    ```bash
    cd ~/fall-detection
    docker compose pull
    docker compose up -d
    ```

---

## 🛣️ สรุปเส้นทางการส่งข้อมูล API (No-Auth API Reference)

ในรุ่น Demo นี้ ทุก Endpoint ที่เคยมี JWT Guard ถูกปลดออกทั้งหมด:

| Method | Route | Description | ตัวอย่าง Payload |
| :--- | :--- | :--- | :--- |
| **GET** | `/ws` | เปิดการเชื่อมต่อเรียลไทม์ (WebSockets) | (สำหรับ Dashboard รับข้อมูลแบบเรียลไทม์) |
| **GET** | `/api/v1/iot/groups` | ดึงข้อมูลสถานที่ทั้งหมด (ใช้สิทธิ์ user_id=1) | - |
| **GET** | `/api/v1/iot/devices/:device_id/settings` | ดึงการตั้งค่าห้อง ขนาด กว้าง x ยาว และค่าเกณฑ์ล้ม | - |
| **PUT** | `/api/v1/iot/devices/settings` | แก้ไขค่าเกณฑ์การล้มและระยะจำกัดเซนเซอร์ | `{"device_id": "...", "fall_threshold": 0.6}` |
| **POST** | `/api/v1/events` | บอร์ดส่งสัญญาณเหตุการณ์ (ล้ม, เข้า, ออก) แบบ HTTP | `{"device_id": "...", "event_type": "fall", "metadata": "{\"fall_x\": -0.2, \"fall_y\": 1.4}"}` |
| **POST** | `/api/v1/iot/notifications` | เพิ่ม Token ของ LINE Notify หรือ Webhook ปลายทาง | `{"name": "LINE ส่วนตัว", "type": "line", "token": "..."}` |

---

## 📄 แหล่งข้อมูลและคู่มือเซนเซอร์เพิ่มเติม
ดูคู่มือเอกสารอ้างอิงของ mmWave Radar โมดูล LD6002C เพิ่มเติมได้ในโฟลเดอร์ `docs/`
