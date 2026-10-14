package group

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

func (s *Service) Create(name string, userID uint) error {
	return s.db.Create(&iot.Group{Name: name, UserID: userID}).Error
}

func (s *Service) GetAll(userID uint) ([]iot.Group, error) {
	var groups []iot.Group
	// Preload เพื่อดึงข้อมูล Device และพารามิเตอร์การตั้งค่าที่สังกัดกลุ่มนี้ออกมาด้วย
	err := s.db.Where("user_id = ?", userID).Preload("Devices").Preload("Devices.Setting").Find(&groups).Error
	return groups, err
}

func (s *Service) Delete(groupID uint, userID uint) error {
	// Soft delete อุปกรณ์และการแจ้งเตือนในกลุ่มนี้ (ประวัติเหตุการณ์ยังคงอยู่ใน DB ไม่สูญหาย)
	s.db.Where("group_id = ?", groupID).Delete(&iot.Device{})
	s.db.Where("group_id = ?", groupID).Delete(&iot.NotificationConfig{})
	return s.db.Where("id = ? AND user_id = ?", groupID, userID).Delete(&iot.Group{}).Error
}

func (s *Service) Update(groupID uint, userID uint, name string) error {
	// ตรวจสอบความเป็นเจ้าของกลุ่มก่อนอัปเดตเพื่อความปลอดภัยค่ะ
	return s.db.Model(&iot.Group{}).
		Where("id = ? AND user_id = ?", groupID, userID).
		Update("name", name).Error
}
