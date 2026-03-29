package user

import "gorm.io/gorm"

type User struct {
	gorm.Model        // ตัวนี้จะแถม ID, CreatedAt, UpdatedAt, DeletedAt มาให้ฟรีๆ เลยค่ะ!
	Email      string `gorm:"unique;not null"`
	Username   string `gorm:"unique;not null"`
	Password   string `gorm:"not null"`
}

// internal/user/model.go
func GetUserByEmail(db *gorm.DB, email string) (*User, error) {
	var user User
	// ค้นหาแถวที่มี email ตรงกับที่ส่งมาค่ะ
	err := db.Where("email = ?", email).First(&user).Error
	return &user, err
}
