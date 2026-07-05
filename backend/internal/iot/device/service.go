package device

import (
	"backend/internal/iot"
	"backend/internal/iot/notifier"

	"gorm.io/gorm"
)

type Service struct {
	db *gorm.DB
}

func NewService(db *gorm.DB) *Service {
	return &Service{db: db}
}

func (s *Service) Register(groupID uint, deviceID, name string) error {
	return s.db.Create(&iot.Device{
		DeviceID: deviceID,
		Name:     name,
		GroupID:  groupID,
	}).Error
}

// ฟังก์ชันรับ Event ที่ยืดหยุ่นรองรับอนาคตค่ะ
func (s *Service) ProcessEvent(devID, evType, name, val, meta string) error {
	var dev iot.Device
	if err := s.db.Where("device_id = ?", devID).First(&dev).Error; err != nil {
		return err
	}
	if meta == "" {
		meta = "{}"
	}

	// บันทึก Event ลง DB ปกติ
	event := iot.DeviceEvent{DeviceID: dev.ID, Type: evType, Name: name, Value: val, Metadata: meta}
	s.db.Create(&event)

	updates := make(map[string]interface{})

	switch evType {
	case "status":
		updates["status"] = name // เช่น online / offline
	case "event":
		updates["latest_event"] = name          // เช่น fall / enter / exit
		updates["latest_event_metadata"] = meta // บันทึก Metadata ล่าสุดด้วย
	}

	// ✨ ถ้ามีการล้ม ให้ดึง Config การแจ้งเตือนมาทำงาน
	// ใน service.go ของ device
	// backend/internal/iot/device/service.go
	if name == "fall" {
		var configs []iot.NotificationConfig
		// ดึงการตั้งค่าทั้งหมดที่เปิดใช้งาน (Enabled) ของกลุ่มนี้
		s.db.Where("group_id = ? AND enabled = ?", dev.GroupID, true).Find(&configs)

		for _, cfg := range configs {
			msg := "[แจ้งเตือนระบบตรวจจับการล้ม] ตรวจพบเหตุการณ์การล้มในพื้นที่: " + dev.Name
			switch cfg.Type {
			case "line":
				// ส่งผ่าน LINE Messaging API
				go notifier.SendLineNotification(cfg.Type, cfg.LineToken, cfg.LineGroupID, msg)
			case "webhook":
				// ส่งผ่าน Webhook ทั่วไป (เช่น Discord)
				go notifier.SendSimpleNotification("webhook", cfg.TargetURL, msg)
			}
		}
	}
	return s.db.Model(&dev).Updates(updates).Error
}

// เพิ่มใน internal/iot/device/service.go

func (s *Service) CreateNotificationConfig(config iot.NotificationConfig) error {
	return s.db.Create(&config).Error
}

func (s *Service) GetNotificationConfigs(groupID uint) ([]iot.NotificationConfig, error) {
	var configs []iot.NotificationConfig
	err := s.db.Where("group_id = ?", groupID).Find(&configs).Error
	return configs, err
}

func (s *Service) Delete(deviceID string) error {
	return s.db.Where("device_id = ?", deviceID).Delete(&iot.Device{}).Error
}

// backend/internal/iot/device/service.go
func (s *Service) Update(originalID string, newData iot.Device) error {
	return s.db.Model(&iot.Device{}).
		Where("device_id = ?", originalID).
		Updates(map[string]interface{}{
			"device_id": newData.DeviceID,
			"name":      newData.Name,
		}).Error
}

func (s *Service) UpdateSettings(settings iot.DeviceSetting) error {
	// ใช้ Save เพื่อทำ Upsert (ถ้ามี ID เดิมจะอัปเดต ถ้าไม่มีจะสร้างใหม่ค่ะ)
	return s.db.Where("device_id = ?", settings.DeviceID).
		Assign(settings).
		FirstOrCreate(&iot.DeviceSetting{DeviceID: settings.DeviceID}).Error
}

func (s *Service) GetSettings(deviceID string) (iot.DeviceSetting, error) {
	var settings iot.DeviceSetting
	err := s.db.Where("device_id = ?", deviceID).First(&settings).Error
	return settings, err
}
