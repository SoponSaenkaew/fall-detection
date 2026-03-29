package register

import (
	"backend/internal/user"

	"golang.org/x/crypto/bcrypt"
	"gorm.io/gorm"
)

type Service interface {
	Execute(email, username, password string) error
}

type service struct {
	db *gorm.DB
}

func NewService(db *gorm.DB) Service {
	return &service{db: db}
}

func (s *service) Execute(email, username, password string) error {
	hashedPassword, err := bcrypt.GenerateFromPassword([]byte(password), bcrypt.DefaultCost)
	if err != nil {
		return err
	}

	newUser := user.User{ //
		Email:    email,
		Username: username,
		Password: string(hashedPassword),
	}

	return s.db.Create(&newUser).Error
}
