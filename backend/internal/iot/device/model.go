package device

import (
	"gorm.io/gorm"
)

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
