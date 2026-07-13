# โครงสร้างและข้อมูลระบบ Azure ของคุณ Sopon (Project Context)

ไฟล์นี้สร้างขึ้นเพื่อบันทึกประวัติการตั้งค่าระบบและสภาพแวดล้อม (Environment) ทั้งหมดที่คุณ Sopon ได้ทำการเซ็ตอัปไว้ เพื่อให้ AI เพื่อนคู่คิด (Coding Assistant) ในเซสชันถัดๆ ไป สามารถอ่านข้อมูลนี้และเข้าใจบริบทได้ทันทีโดยไม่ต้องเริ่มต้นอธิบายใหม่ครับ

---

## 🌐 1. โดเมนเนมและการเชื่อมโยง (Domain & Routing)
- **โดเมนหลัก (Main Domain):** `sopon-project.me`
- **โปรเจกต์ที่ 1 (Smart Fall Detection):** `fall-detection.sopon-project.me`
- **โปรเจกต์ที่ 2 ในอนาคต (Live Poll & Voting):** `poll.sopon-project.me`
- **โปรเจกต์ที่ 3 ในอนาคต (Smart Expense Parser):** `expense.sopon-project.me`

---

## 🖥️ 2. ข้อมูลเครื่องเซิร์ฟเวอร์ (Azure VM B1s)
- **สเปกเครื่อง VM:** Burstable B1s (1 vCPU, 1 GiB RAM) ระบบปฏิบัติการ Ubuntu
- **การจัดการแรมเสมือน (Swap File):** ติดตั้งแรมเสมือนขนาด **4 GiB** ไว้เพื่อป้องกันปัญหาหน่วยความจำเต็ม (OOM)
- **การจัดการช่องทางเข้าออก (Reverse Proxy):** 
  - ใช้ **Nginx Proxy** แยกเดี่ยวๆ รันอยู่ที่โฟลเดอร์ `~/nginx-proxy/`
  - คอยถือครองพอร์ต `80` และ `443`
  - ผูกใบรับรองความปลอดภัย **HTTPS (SSL)** ของ Let's Encrypt ผ่าน Certbot บนโดเมน `fall-detection.sopon-project.me` เรียบร้อยแล้ว
- **ระบบเครือข่าย Docker:** เชื่อมโยงทุกโครงการผ่าน External Network ร่วมกันในชื่อ `proxy-network`

---

## 🛢️ 3. ฐานข้อมูล (Azure Database for PostgreSQL)
- **ประเภทบริการ:** Flexible Server (B1ms, 2 GiB RAM)
- **โฮสต์ปลายทาง (DB_HOST):** `sopon-postgres.postgres.database.azure.com`
- **พอร์ต (DB_PORT):** `5432` (เชื่อมต่อแบบบังคับใช้ SSL `sslmode=require`)
- **ชื่อแอดมิน (DB_USER):** `postgres`
- **ชื่อฐานข้อมูลปัจจุบัน (DB_NAME):** `postgres` (ฐานข้อมูลเริ่มต้นของระบบ)

---

## 📦 4. คลังภาพซอฟต์แวร์ (Azure Container Registry - ACR)
- **ชื่อคลังเก็บ (ACR Name):** `soponproject`
- **ชื่อโดเมนรีจิสทรี (Registry Endpoint):** `soponproject.azurecr.io`
- **อิมเมจปัจจุบัน:**
  - หลังบ้าน: `soponproject.azurecr.io/fall-detection-backend:latest`
  - หน้าบ้าน: `soponproject.azurecr.io/fall-detection-frontend:latest`

---

## 🔄 5. ขั้นตอนการแก้ไขและอัปเดตโค้ด (Deployment Workflow)
1. แก้ไขโค้ดหน้าบ้าน/หลังบ้านในเครื่องคอมพิวเตอร์ของคุณ
2. คอมไพล์เป็นอิมเมจในเครื่องคอมคุณ:
   - หลังบ้าน: `docker build -t soponproject.azurecr.io/fall-detection-backend:latest ./backend`
   - หน้าบ้าน: `docker build -t soponproject.azurecr.io/fall-detection-frontend:latest ./frontend`
3. อัปโหลดขึ้นคลัง:
   - `docker push soponproject.azurecr.io/fall-detection-backend:latest`
   - `docker push soponproject.azurecr.io/fall-detection-frontend:latest`
4. อัปเดตบน VM (ผ่าน SSH):
   - `cd ~/fall-detection && docker compose pull && docker compose up -d`
