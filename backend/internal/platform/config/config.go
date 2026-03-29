package config

import (
	"os"

	"github.com/joho/godotenv"
)

// LoadConfig โหลดไฟล์ .env และคืนค่า DSN สำหรับ GORM
func GetDSN() string {
	godotenv.Load(".env") // โหลดไฟล์ .env (ถ้าไม่มีมันจะข้ามไปดู Environment หลักค่ะ)

	host := os.Getenv("DB_HOST")
	port := os.Getenv("DB_PORT")
	user := os.Getenv("DB_USER")
	pass := os.Getenv("DB_PASSWORD")
	name := os.Getenv("DB_NAME")
	ssl := os.Getenv("DB_SSLMODE")

	// รวมร่างเป็น Connection String
	return "host=" + host + " user=" + user + " password=" + pass + " dbname=" + name + " port=" + port + " sslmode=" + ssl
}
