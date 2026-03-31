package main

import (
	"backend/internal/auth/login"
	"backend/internal/auth/register"
	"backend/internal/database"
	"backend/internal/iot/device" // เพิ่ม
	"backend/internal/iot/group"  // เพิ่ม
	"backend/internal/middleware"

	"github.com/gin-contrib/cors"

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
	r.Use(cors.New(cors.Config{
		AllowOrigins:     []string{"http://localhost:3000"}, // พอร์ตของ Next.js
		AllowMethods:     []string{"GET", "POST", "PUT", "PATCH", "DELETE", "OPTIONS"},
		AllowHeaders:     []string{"Origin", "Authorization", "Content-Type", "Accept"},
		ExposeHeaders:    []string{"Content-Length"},
		AllowCredentials: true,
	}))

	v1 := r.Group("/api/v1")
	{
		v1.POST("/register", regHandler.Handle)
		v1.POST("/login", loginHandler.Handle)

		// กลุ่ม API ที่ต้อง Login (ตรวจบัตรผ่าน)
		protected := v1.Group("/iot").Use(middleware.AuthMiddleware())
		{
			// จัดการสถานที่
			protected.POST("/groups", grpHandler.Create)
			protected.PUT("/groups/:id", grpHandler.Update)
			protected.DELETE("/groups/:id", grpHandler.Delete)

			// จัดการอุปกรณ์
			protected.POST("/devices", devHandler.Register)
			protected.PUT("/devices/:device_id", devHandler.Update)
			protected.DELETE("/devices/:device_id", devHandler.Delete)
			protected.GET("/devices/:device_id/settings", devHandler.GetSettings)
			protected.PUT("/devices/settings", devHandler.UpdateSettings)
			protected.GET("/groups", grpHandler.GetAll)

			// จัดการการแจ้งเตือน (เพิ่ม/แก้ไข/ลบ) - ตัวอย่างนี้ทำแค่เพิ่มนะคะ
			protected.GET("/notifications", devHandler.GetNotificationConfigs)
			protected.POST("/notifications", devHandler.AddNotificationConfig)

			protected.GET("/profile", func(c *gin.Context) {
				userID, _ := c.Get("user_id")
				c.JSON(200, gin.H{
					"message": "ยินดีต้อนรับเข้าสู่ห้องส่วนตัวค่ะเซนเซย์!",
					"your_id": userID,
				})
			})
		}

		// API สำหรับตัว Device ส่งข้อมูล (ยังไม่ล็อกกุญแจเพื่อให้ Sensor ส่งง่ายค่ะ)
		v1.POST("/events", devHandler.ReceiveEvent)
		v1.GET("/config/:device_id", devHandler.GetSettings)
	}

	r.Run(":8080")
}
