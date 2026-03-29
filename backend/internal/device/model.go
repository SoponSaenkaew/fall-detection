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
