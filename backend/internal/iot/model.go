package iot

import (
	"gorm.io/gorm"
)

type Group struct {
	gorm.Model
	Name        string   `gorm:"not null" json:"name"`              // ชื่อกลุ่ม เช่น "บ้านหลัก"
	Description string   `json:"description"`                       // รายละเอียดเพิ่มเติม
	UserID      uint     `json:"user_id"`                           // ใครเป็นเจ้าของกลุ่มนี้
	Devices     []Device `gorm:"foreignKey:GroupID" json:"devices"` // ในกลุ่มนี้มีอุปกรณ์อะไรบ้าง
}

type Device struct {
	gorm.Model
	DeviceID string `gorm:"unique;not null" json:"device_id"` // รหัส Hardware ID ของเซนเซอร์
	Name     string `json:"name"`                             // ชื่อเรียก เช่น "ห้องนอน 1"
	Status   string `gorm:"default:'online'" json:"status"`   // สถานะเครื่อง
	GroupID  uint   `json:"group_id"`                         // สังกัดกลุ่มไหน
}

type DeviceEvent struct {
	gorm.Model
	DeviceID uint   `gorm:"index;not null" json:"device_id"`
	Type     string `gorm:"index;not null" json:"type"` // "event" หรือ "status"
	Name     string `gorm:"index;not null" json:"name"` // "fall", "enter", "exit", "online"
	Value    string `json:"value"`                      // ค่าที่ส่งมา (ถ้ามี)
	Metadata string `gorm:"type:jsonb" json:"metadata"` // เก็บข้อมูล Custom อื่นๆ เป็น JSON
}

type NotificationConfig struct {
	gorm.Model
	GroupID     uint   `json:"group_id"`
	Type        string `json:"type"`          // "line", "discord", "webhook"
	TargetURL   string `json:"target_url"`    // สำหรับ Discord/Webhook
	LineToken   string `json:"line_token"`    // Channel Access Token
	LineGroupID string `json:"line_group_id"` // Group ID ผู้รับ
	Enabled     bool   `json:"enabled" gorm:"default:true"`
}
