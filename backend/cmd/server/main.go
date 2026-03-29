package main

import (
	"backend/internal/auth/register" // import โฟลเดอร์ใหม่
	"backend/internal/database"

	"github.com/gin-gonic/gin"
)

func main() {
	db, err := database.ConnectDB()
	if err != nil {
		panic("เชื่อมต่อ Database ไม่ได้ค่ะเซนเซย์!: " + err.Error())
	}

	regService := register.NewService(db)
	regHandler := register.NewHandler(regService)

	r := gin.Default()

	v1 := r.Group("/api/v1")
	{
		v1.POST("/register", regHandler.Handle)
		// v1.POST("/login", loginHandler.Handle) <-- ใส่เพิ่มทีหลังได้ง่ายมาก!
	}

	r.Run(":8080")
}
