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

	// บันทึก Event ลง DB ปกติ
	event := iot.DeviceEvent{DeviceID: dev.ID, Type: evType, Name: name, Value: val, Metadata: meta}
	s.db.Create(&event)

	// ✨ ถ้ามีการล้ม ให้ดึง Config การแจ้งเตือนมาทำงาน
	// ใน service.go ของ device
	if name == "fall" {
		var configs []iot.NotificationConfig
		s.db.Where("group_id = ? AND enabled = ?", dev.GroupID, true).Find(&configs)

		for _, cfg := range configs {
			go notifier.SendLineNotification(cfg.Type, cfg.LineToken, cfg.LineGroupID, "เซนเซย์คะ! มีคนล้มในห้อง "+dev.Name)
		}
	}
	return nil
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
