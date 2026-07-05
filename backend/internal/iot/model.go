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
	DeviceID            string `gorm:"unique;not null" json:"device_id"` // รหัส Hardware ID ของเซนเซอร์
	Name                string `json:"name"`                             // ชื่อเรียก เช่น "ห้องนอน 1"
	Status              string `gorm:"default:'online'" json:"status"`   // สถานะเครื่อง
	LatestEvent         string `json:"latest_event"`                     // ค่าที่อาจจะเก็บไว้ เช่น "fall", "enter", "exit" หรือค่าอื่นๆ ที่ส่งมาจากอุปกรณ์
	LatestEventMetadata string `json:"latest_event_metadata"`            // เก็บข้อมูล Metadata ล่าสุด เช่น พิกัดหรือเส้นทางเดิน
	GroupID             uint   `json:"group_id"`                         // สังกัดกลุ่มไหน
}

type DeviceEvent struct {
	gorm.Model
	DeviceID uint   `gorm:"index;not null" json:"device_id"`
	Type     string `gorm:"index;not null" json:"type"` // "event" หรือ "status"
	Name     string `gorm:"index;not null" json:"name"` // "fall", "enter", "exit", "online"
	Value    string `json:"value"`                      // ค่าที่ส่งมา (ถ้ามี)
	Metadata string `gorm:"type:jsonb" json:"metadata"` // เก็บข้อมูล Custom อื่นๆ เป็น JSON
}

type DeviceSetting struct {
	gorm.Model
	DeviceID      string  `gorm:"uniqueIndex;not null" json:"device_id"`
	FallThreshold float64 `gorm:"default:0.5" json:"fall_threshold"` // เกณฑ์การล้ม (เช่น ความเร็วหรือระยะที่เปลี่ยน)
	MountHeight   float64 `gorm:"default:2.0" json:"mount_height"`   // ความสูงของเซนเซอร์จากพื้น (เมตร)
	RoomWidth     float64 `gorm:"default:4.0" json:"room_width"`     // ความกว้างของห้อง (เมตร)
	RoomLength    float64 `gorm:"default:4.0" json:"room_length"`    // ความยาวของห้อง (เมตร)
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
