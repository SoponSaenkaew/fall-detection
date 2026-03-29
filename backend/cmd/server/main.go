package main

import (
	"backend/internal/auth/login"
	"backend/internal/auth/register"
	"backend/internal/database"
	"backend/internal/iot/device" // เพิ่ม
	"backend/internal/iot/group"  // เพิ่ม
	"backend/internal/middleware"

	"github.com/gin-gonic/gin"
)

func main() {
	db, err := database.ConnectDB()
	if err != nil {
		panic("เชื่อมต่อ Database ไม่ได้ค่ะเซนเซย์!: " + err.Error())
	}

	// Setup Services & Handlers
	regHandler := register.NewHandler(register.NewService(db))
	loginHandler := login.NewHandler(login.NewService(db))

	grpHandler := group.NewHandler(group.NewService(db))   // เพิ่ม
	devHandler := device.NewHandler(device.NewService(db)) // เพิ่ม

	r := gin.Default()

	v1 := r.Group("/api/v1")
	{
		v1.POST("/register", regHandler.Handle)
		v1.POST("/login", loginHandler.Handle)

		// กลุ่ม API ที่ต้อง Login (ตรวจบัตรผ่าน)
		protected := v1.Group("/iot").Use(middleware.AuthMiddleware())
		{
			// จัดการสถานที่
			protected.POST("/groups", grpHandler.Create)

			// จัดการอุปกรณ์
			protected.POST("/devices", devHandler.Register)
		}

		// API สำหรับตัว Device ส่งข้อมูล (ยังไม่ล็อกกุญแจเพื่อให้ Sensor ส่งง่ายค่ะ)
		v1.POST("/events", devHandler.ReceiveEvent)
	}

	r.Run(":8080")
}
