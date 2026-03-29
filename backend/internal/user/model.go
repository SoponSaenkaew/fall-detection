package user

import "gorm.io/gorm"

type User struct {
	gorm.Model        // ตัวนี้จะแถม ID, CreatedAt, UpdatedAt, DeletedAt มาให้ฟรีๆ เลยค่ะ!
	Email      string `gorm:"unique;not null"`
	Username   string `gorm:"unique;not null"`
	Password   string `gorm:"not null"`
}
