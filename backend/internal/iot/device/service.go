package device

import (
	"backend/internal/iot"

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
	return s.db.Create(&iot.DeviceEvent{
		DeviceID: dev.ID,
		Type:     evType,
		Name:     name,
		Value:    val,
		Metadata: meta,
	}).Error
}
