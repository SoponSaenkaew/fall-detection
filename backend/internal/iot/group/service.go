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
	// Preload เพื่อดึงข้อมูล Device ที่สังกัดกลุ่มนี้ออกมาด้วยค่ะ
	err := s.db.Where("user_id = ?", userID).Preload("Devices").Find(&groups).Error
	return groups, err
}
