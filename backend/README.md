# ⚙️ Backend API (Go)

ระบบประมวลผลกลางสำหรับรับข้อมูลจากอุปกรณ์ IoT และจัดการฐานข้อมูลอุปกรณ์

## 🛣️ API Endpoints ที่สำคัญ
- `GET /api/v1/config/:device_id`: ดึงค่าการตั้งค่า (Threshold, Height) มายังอุปกรณ์
- `POST /api/v1/events`: รับข้อมูลเหตุการณ์ (ล้ม, เข้าห้อง, ออกห้อง) เพื่อบันทึกและแจ้งเตือน

## 🛠️ วิธีการรัน
1. ติดตั้ง Go Modules: `go mod tidy`
2. ตั้งค่าไฟล์ `.env` สำหรับฐานข้อมูลและ LINE Token
3. รันเซิร์ฟเวอร์: `go run cmd/server/main.go`