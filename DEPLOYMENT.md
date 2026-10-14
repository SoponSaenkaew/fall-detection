# 🚀 คู่มือขั้นตอนการ Deploy และการเตรียมความพร้อมระบบ (Deployment Guide)

คู่มือนี้สรุปรายการสิ่งที่ต้องเตรียมก่อน (Prerequisites) และขั้นตอนการ Deploy ระบบ **Smart Fall Detection System** ขึ้นสู่เครื่องเซิร์ฟเวอร์ (Production Server / Cloud เช่น Microsoft Azure, VPS หรือ Linux Server) อย่างละเอียดและเป็นลำดับขั้นตอน

---

## 📑 สารบัญ
1. [สิ่งที่ต้องเตรียมล่วงหน้า (Prerequisites Checklist)](#1-สิ่งที่ต้องเตรียมล่วงหน้า-prerequisites-checklist)
2. [การตั้งค่าตัวแปรสภาพแวดล้อม (Environment Variables Setup)](#2-การตั้งค่าตัวแปรสภาพแวดล้อม-environment-variables-setup)
3. [การตั้งค่าเซิร์ฟเวอร์ครั้งแรก (Initial Server Provisioning)](#3-การตั้งค่าเซิร์ฟเวอร์ครั้งแรก-initial-server-provisioning)
4. [ขั้นตอนการ Build และ Push Docker Image](#4-ขั้นตอนการ-build-และ-push-docker-image)
5. [ขั้นตอนการ Deploy และรันบริการบน Server](#5-ขั้นตอนการ-deploy-และรันบริการบน-server)
6. [ขั้นตอนการตั้งค่าบอร์ดฮาร์ดแวร์ ESP32-S3](#6-ขั้นตอนการตั้งค่าบอร์ดฮาร์ดแวร์-esp32-s3)
7. [ขั้นตอนการอัปเดตระบบในอนาคต (Maintenance & Update Workflow)](#7-ขั้นตอนการอัปเดตระบบในอนาคต-maintenance--update-workflow)

---

## 1. สิ่งที่ต้องเตรียมล่วงหน้า (Prerequisites Checklist)

ก่อนเริ่มทำการ Deploy ให้จัดเตรียมทรัพยากรเหล่านี้ให้พร้อม:

### 🖥️ 1.1 เซิร์ฟเวอร์ (Cloud VM หรือ VPS)
- **ระบบปฏิบัติการ:** Ubuntu 22.04 LTS (หรือ Linux Distribution อื่น)
- **สเปกขั้นต่ำ:** 1 vCPU, 1 GiB RAM (แนะนำตั้งค่า Swap Space เพิ่ม 2–4 GiB)
- **พอร์ตที่ต้องเปิด (Firewall / Network Security Group):**
  - `80` (HTTP สำหรับ Certbot / Nginx)
  - `443` (HTTPS สำหรับ Dashboard & API)
  - `1883` (MQTT สำหรับ ESP32-S3 ส่งข้อมูลเหตุการณ์)
  - `22` (SSH สำหรับการจัดการเซิร์ฟเวอร์)

### 🌐 1.2 โดเมนเนมและการตั้งค่า DNS (Domain & DNS)
- มีโดเมนเนมเป็นของตนเอง (เช่น `your-domain.com`)
- ชี้ระเบียน DNS (A Record) ดังนี้:
  - `fall-detection.your-domain.com` ➔ ชี้ไปที่ **Public IP ของเซิร์ฟเวอร์**

### 🛢️ 1.3 ฐานข้อมูล (PostgreSQL Database)
- ฐานข้อมูล PostgreSQL เวอร์ชัน 15 ขึ้นไป (รองรับทั้ง Managed Database เช่น Azure Database for PostgreSQL Flexible Server หรือรันบน Docker)
- รองรับการเชื่อมต่อแบบบังคับใช้ SSL (`sslmode=require`)

### 📦 1.4 คลังภาพซอฟต์แวร์ (Container Registry)
- บัญชีคลังภาพสำหรับจัดเก็บ Docker Images เช่น:
  - **Azure Container Registry (ACR)**: เช่น `myacr.azurecr.io`
  - หรือ **Docker Hub** / **GitHub Container Registry (GHCR)**

### 📲 1.5 ข้อมูลการแจ้งเตือน (Notification Configs)
- **LINE Messaging API หรือ LINE Notify Token**: สำหรับใช้ส่งแจ้งเตือนภัยเข้ามือถือผู้ดูแล

---

## 2. การตั้งค่าตัวแปรสภาพแวดล้อม (Environment Variables Setup)

บนเครื่องเซิร์ฟเวอร์ ให้สร้างไฟล์ `.env` ไว้ในไดเรกทอรีโครงการ (เช่น `~/fall-detection/.env`) เพื่อเก็บค่า Credentials และการเชื่อมต่อฐานข้อมูล **(ห้ามนำไฟล์นี้ขึ้น Git เด็ดขาด)**:

```bash
# ==========================================
# 📦 CONTAINER REGISTRY
# ==========================================
ACR_NAME=youracrname

# ==========================================
# 🗄️ DATABASE CONFIGURATION (POSTGRESQL)
# ==========================================
DB_HOST=your-db-host.postgres.database.azure.com
DB_PORT=5432
DB_USER=your_postgres_admin
DB_PASSWORD=your_super_strong_password
DB_NAME=fall_detection_db
DB_SSLMODE=require

# ==========================================
# 🔑 SECURITY & AUTH
# ==========================================
JWT_SECRET=your_random_and_secure_jwt_secret_key_at_least_32_chars
ALLOWED_ORIGINS=*
```

---

## 3. การตั้งค่าเซิร์ฟเวอร์ครั้งแรก (Initial Server Provisioning)

### 3.1 เพิ่มพื้นที่ Swap Memory (จำเป็นมากหากใช้ VM RAM 1 GiB)
```bash
# สร้างไฟล์ Swap ขนาด 4 GiB
sudo fallocate -l 4G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile

# บันทึกให้เริ่มทำงานอัตโนมัติเมื่อรีบูตเครื่อง
echo '/swapfile none swap sw 0 0' | sudo tee -a /etc/fstab
```

### 3.2 ติดตั้ง Docker และ Docker Compose
```bash
sudo apt update && sudo apt install -y docker.io docker-compose-plugin
sudo systemctl enable --now docker
sudo usermod -aG docker $USER
```

### 3.3 สร้าง Docker External Network ร่วม
```bash
docker network create proxy-network
```

### 3.4 ตั้งค่า Nginx Reverse Proxy และขอใบรับรอง HTTPS (SSL)
ใช้โฟลเดอร์ `~/nginx-proxy/` บนเซิร์ฟเวอร์เพื่อรัน Nginx แยกเดี่ยว:
```bash
# ตัวอย่างโครงสร้างโฟลเดอร์บนเซิร์ฟเวอร์
~/nginx-proxy/
├── docker-compose.yml
└── conf.d/
    └── default.conf
```
* รันขอใบรับรองความปลอดภัยฟรีผ่าน Certbot (Let's Encrypt)
* ตรวจสอบให้ Nginx ส่งผ่านเส้นทาง:
  - `/` ➔ ส่งต่อไปยัง `http://frontend:80`
  - `/api/` ➔ ส่งต่อไปยัง `http://backend:8080`
  - `/ws` ➔ ส่งต่อไปยัง `http://backend:8080` (รองรับ WebSocket Upgrade)

---

## 4. ขั้นตอนการ Build และ Push Docker Image

ทำจากเครื่องคอมพิวเตอร์ของคุณ (เครื่อง Dev):

### 4.1 เข้าสู่ระบบ Container Registry
```bash
# ตัวอย่างกรณีใช้ Azure Container Registry (ACR)
az acr login --name <YOUR_ACR_NAME>

# หรือกรณีใช้ Docker Hub
docker login
```

### 4.2 ทำการ Build Docker Images
```bash
# 1. หลังบ้าน (Go Backend)
docker build -t <YOUR_REGISTRY>/fall-detection-backend:latest ./backend

# 2. หน้าบ้าน (Next.js Frontend)
docker build -t <YOUR_REGISTRY>/fall-detection-frontend:latest ./frontend
```

### 4.3 อัปโหลด Images ขึ้นสู่ Registry
```bash
docker push <YOUR_REGISTRY>/fall-detection-backend:latest
docker push <YOUR_REGISTRY>/fall-detection-frontend:latest
```

---

## 5. ขั้นตอนการ Deploy และรันบริการบน Server

SSH เข้าสู่เซิร์ฟเวอร์ของคุณ:
```bash
ssh user@your-server-ip
```

### 5.1 เข้าสู่โฟลเดอร์โครงการ
```bash
cd ~/fall-detection
```

### 5.2 ตรวจสอบไฟล์ที่จำเป็น
ต้องมีไฟล์อย่างน้อย 2 ไฟล์ในไดเรกทอรี:
1. `docker-compose.prod.yml`
2. `.env` (ที่ใส่ค่าคอนฟิกตามข้อ 2 แล้ว)

### 5.3 ดึง Image ล่าสุดและสั่งรันระบบ
```bash
# เข้าสู่ระบบ Registry บนเซิร์ฟเวอร์ (หาก Image เป็นแบบ Private)
docker login <YOUR_REGISTRY>

# ดึง Image ล่าสุดจากคลัง
docker compose -f docker-compose.prod.yml pull

# สั่งรัน Container ในพื้นหลัง
docker compose -f docker-compose.prod.yml up -d
```

### 5.4 ตรวจสอบสถานะการทำงาน
```bash
# ดูสถานะคอนเทนเนอร์ทั้งหมด
docker compose -f docker-compose.prod.yml ps

# ดู Log ของระบบหลังบ้าน
docker logs -f prod_go_backend

# ดู Log ของระบบหน้าบ้าน
docker logs -f prod_nextjs_frontend
```

---

## 6. ขั้นตอนการตั้งค่าบอร์ดฮาร์ดแวร์ ESP32-S3

### 6.1 ตรวจสอบค่าพารามิเตอร์ในเฟิร์มแวร์
เปิดไฟล์ [board-esp32s3/board/SystemConfig.h](file:///d:/Final%20project/fall-detection/board-esp32s3/board/SystemConfig.h) และตรวจสอบค่า:
- `backend_url`: URL ของ API หลังบ้าน (เช่น `https://fall-detection.your-domain.com/api/v1/events`)
- `mqtt_broker`: IP หรือ Hostname ของ MQTT Broker (พอร์ต `1883`)

### 6.2 อัปโหลดเฟิร์มแวร์ลงบอร์ด
1. เปิดไฟล์ `board-esp32s3/board/board.ino` ใน Arduino IDE
2. เลือกบอร์ด **ESP32S3 Dev Module**
3. กด **Upload** ผ่านสาย USB

### 6.3 การเชื่อมต่อ Wi-Fi ของบอร์ดครั้งแรก
1. เมื่อเปิดบอร์ดครั้งแรก (หรือไม่มี Wi-Fi ที่เคยจำไว้) ไฟ LED จะแสดง **สีม่วง** และเปิด Hotspot ชื่อ:
   - **SSID:** `ESP32S3-LD6002C_TEST_01` (หรือตามรหัสเครื่อง)
   - **Password:** `12345678`
2. นำสมาร์ทโฟนหรือคอมพิวเตอร์เชื่อมต่อ Hotspot ดังกล่าว
3. หน้าต่าง Captive Portal จะเปิดขึ้นอัตโนมัติ (หรือเข้าเว็บ `http://192.168.4.1`)
4. สแกนเลือก Wi-Fi ประจำสถานที่และใส่รหัสผ่าน
5. กด **Save** บอร์ดจะรีบูตและจำค่าลง Flash Memory พร้อมเริ่มส่งข้อมูลเข้าเซิร์ฟเวอร์ทันที
> 💡 **การรีเซ็ต Wi-Fi:** กดปุ่ม **BOOT** บนบอร์ดค้างไว้ 3 วินาที บอร์ดจะล้างค่า Wi-Fi เดิมและกลับเข้าสู่โหมด Hotspot สีม่วงเพื่อให้ตั้งค่าใหม่ได้ตลอดเวลา

---

## 7. ขั้นตอนการอัปเดตระบบในอนาคต (Maintenance & Update Workflow)

เมื่อคุณมีการแก้ไขโค้ดหน้าบ้านหรือหลังบ้านในเครื่องคอมพิวเตอร์ ให้ทำตาม 3 ขั้นตอนนี้เพื่ออัปเดตขึ้น Production:

1. **คอมไพล์และอัปโหลดจากเครื่องคุณ:**
   ```bash
   docker build -t <YOUR_REGISTRY>/fall-detection-backend:latest ./backend
   docker build -t <YOUR_REGISTRY>/fall-detection-frontend:latest ./frontend
   docker push <YOUR_REGISTRY>/fall-detection-backend:latest
   docker push <YOUR_REGISTRY>/fall-detection-frontend:latest
   ```

2. **สั่งอัปเดตบนเซิร์ฟเวอร์ (ผ่าน SSH):**
   ```bash
   ssh user@your-server-ip "cd ~/fall-detection && docker compose -f docker-compose.prod.yml pull && docker compose -f docker-compose.prod.yml up -d"
   ```

3. **ตรวจสอบความเรียบร้อย:**
   - เปิดหน้าเว็บแดชบอร์ด `https://fall-detection.your-domain.com/dashboard`
   - เช็คว่าสถานะอุปกรณ์และข้อมูลอัปเดตเรียลไทม์เป็นปกติ
