package group

import (
	"backend/internal/iot/device"

	"gorm.io/gorm"
)

type Group struct {
	gorm.Model
	Name        string          `gorm:"not null" json:"name"`              // ชื่อกลุ่ม เช่น "บ้านหลัก"
	Description string          `json:"description"`                       // รายละเอียดเพิ่มเติม
	UserID      uint            `json:"user_id"`                           // ใครเป็นเจ้าของกลุ่มนี้
	Devices     []device.Device `gorm:"foreignKey:GroupID" json:"devices"` // ในกลุ่มนี้มีอุปกรณ์อะไรบ้าง
}
