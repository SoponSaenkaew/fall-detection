package login

import (
	"backend/internal/user"
	"errors"
	"os"
	"time"

	"github.com/golang-jwt/jwt/v5"
	"golang.org/x/crypto/bcrypt"
	"gorm.io/gorm"
)

type Service struct {
	db *gorm.DB
}

func NewService(db *gorm.DB) *Service {
	return &Service{db: db}
}

func (s *Service) Execute(email, password string) (string, error) { // เปลี่ยนเป็น email
	// 1. หา User ใน DB ด้วย Email
	u, err := user.GetUserByEmail(s.db, email)
	if err != nil {
		return "", errors.New("ไม่พบผู้ใช้งานที่ใช้ Email นี้ค่ะเซนเซย์")
	}

	// 2. เช็ค Password (เทียบตัวที่ส่งมากับตัวที่ Hash ใน DB)
	err = bcrypt.CompareHashAndPassword([]byte(u.Password), []byte(password))
	if err != nil {
		return "", errors.New("รหัสผ่านไม่ถูกต้องนะคะ")
	}

	// 3. สร้าง JWT Token
	token := jwt.NewWithClaims(jwt.SigningMethodHS256, jwt.MapClaims{
		"user_id": u.ID,
		"exp":     time.Now().Add(time.Hour * 24).Unix(), // หมดอายุใน 24 ชม.
	})

	// ดึง Secret จาก .env
	return token.SignedString([]byte(os.Getenv("JWT_SECRET")))
}
