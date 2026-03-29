package database

import (
	"backend/internal/platform/config"
	"backend/internal/user"

	"gorm.io/driver/postgres"
	"gorm.io/gorm"
)

func ConnectDB() (*gorm.DB, error) {
	dsn := config.GetDSN()
	db, err := gorm.Open(postgres.Open(dsn), &gorm.Config{})
	if err != nil {
		return nil, err // ถ้า Error ให้ส่ง nil กลับไปพร้อม Error ค่ะ
	}

	// สั่งให้ GORM สร้าง/อัปเดต Table ตาม Struct ของเราอัตโนมัติ!
	db.AutoMigrate(&user.User{})

	return db, err
}
