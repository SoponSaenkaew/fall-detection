package main

import (
	"backend/internal/auth/login"
	"backend/internal/auth/register"
	"backend/internal/database"
	"backend/internal/iot/device"
	"backend/internal/iot/group"
	"backend/internal/middleware"
	"backend/internal/mqtt"
	"backend/internal/ws"

	"github.com/gin-contrib/cors"
	"github.com/gin-gonic/gin"
)

func main() {
	db, err := database.ConnectDB()
	if err != nil {
		panic("เชื่อมต่อ Database ไม่ได้ค่ะเซนเซย์!: " + err.Error())
	}

	// Setup Services & Handlers
	devService := device.NewService(db)
	regHandler := register.NewHandler(register.NewService(db))
	loginHandler := login.NewHandler(login.NewService(db))
	grpHandler := group.NewHandler(group.NewService(db))
	devHandler := device.NewHandler(devService)

	// ✨ จุดสำคัญที่ 1: เปิดระบบกระจายข้อความ WebSocket ให้ทำงานแบบ Background
	go ws.HandleMessages()

	// ✨ จุดสำคัญที่ 2: เชื่อมต่อ MQTT Broker เพื่อรอรับข้อมูลจาก ESP32
	// เซนเซย์อย่าลืมเช็ค Address ของ Broker (เช่น tcp://localhost:1883) ให้ตรงกับที่รันใน Docker นะค๊ะ
	// mqttClient := mqtt.SetupMQTT("tcp://localhost:1883", devService)
	mqttClient := mqtt.SetupMQTT("tcp://broker.hivemq.com:1883", devService)

	defer mqttClient.Disconnect(250)

	r := gin.Default()
	r.Use(cors.New(cors.Config{
		AllowOrigins:     []string{"http://localhost:3000"}, // พอร์ตของ Next.js
		AllowMethods:     []string{"GET", "POST", "PUT", "PATCH", "DELETE", "OPTIONS"},
		AllowHeaders:     []string{"Origin", "Authorization", "Content-Type", "Accept"},
		ExposeHeaders:    []string{"Content-Length"},
		AllowCredentials: true,
	}))

	// ✨ จุดสำคัญที่ 3: เพิ่ม Route สำหรับให้หน้าจอ Dashboard (Next.js) มาเชื่อมต่อ WebSocket
	r.GET("/ws", ws.HandleConnections)

	v1 := r.Group("/api/v1")
	{
		v1.POST("/register", regHandler.Handle)
		v1.POST("/login", loginHandler.Handle)

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

			// จัดการการแจ้งเตือน
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

		// API เดิมที่รับผ่าน HTTP (ยังเก็บไว้เผื่อเซนเซย์อยาก Test ผ่าน Postman ค่ะ)
		v1.POST("/events", devHandler.ReceiveEvent)
		v1.GET("/config/:device_id", devHandler.GetSettings)
	}

	r.Run(":8080")
}
