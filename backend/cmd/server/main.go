package main

import (
	"backend/internal/auth/login"
	"backend/internal/auth/register"
	"backend/internal/database"
	"backend/internal/iot/device"
	"backend/internal/iot/group"
	"backend/internal/iot"
	"backend/internal/mqtt"
	"backend/internal/user"
	"backend/internal/ws"
	"os"
	"strings"

	"github.com/gin-contrib/cors"
	"github.com/gin-gonic/gin"
)

func main() {
	db, err := database.ConnectDB()
	if err != nil {
		panic("ไม่สามารถเชื่อมต่อฐานข้อมูลได้: " + err.Error())
	}

	// สร้างผู้ใช้ดีฟอลต์หากยังไม่มี
	var defaultUser user.User
	if err := db.Where("email = ?", "admin@example.com").First(&defaultUser).Error; err != nil {
		defaultUser = user.User{
			Email:    "admin@example.com",
			Username: "ผู้ดูแลระบบ (System Admin)",
			Password: "system_password",
		}
		db.Create(&defaultUser)
	}

	// ลงทะเบียนข้อมูลตัวอย่างสำหรับโหมดเดโม หากตารางกลุ่มยังว่างอยู่
	var groupCount int64
	db.Model(&iot.Group{}).Count(&groupCount)
	if groupCount == 0 {
		g1 := iot.Group{Name: "อาคาร 1", UserID: defaultUser.ID}
		db.Create(&g1)

		// 1. สร้างห้องน้ำสำหรับอาคาร 1 (ห้องน้ำ 101 - 105)
		// ห้องน้ำ 101: สถานะล้มฉุกเฉิน (มีพิกัดล้มใกล้ชักโครก)
		db.Create(&iot.Device{
			DeviceID:            "B1_R101",
			Name:                "ห้องน้ำ 101",
			GroupID:             g1.ID,
			Status:              "online",
			LatestEvent:         "fall",
			LatestEventMetadata: `{"fall_x": -0.3, "fall_y": 1.5}`,
		})
		db.Create(&iot.DeviceSetting{DeviceID: "B1_R101", FallThreshold: 0.5, MountHeight: 2.0, RoomWidth: 3.0, RoomLength: 4.0})

		// ห้องน้ำ 102: สถานะปกติ (มีประวัติเส้นทางการเดินเข้าหาชักโครกและเดินกลับออกมา)
		db.Create(&iot.Device{
			DeviceID:            "B1_R102",
			Name:                "ห้องน้ำ 102",
			GroupID:             g1.ID,
			Status:              "online",
			LatestEvent:         "exit",
			LatestEventMetadata: `{"path": [[0.0, 3.8], [0.0, 2.8], [-0.4, 2.0], [-0.2, 1.2], [0.0, 0.6], [0.0, 1.2], [0.2, 2.2], [0.0, 3.8]]}`,
		})
		db.Create(&iot.DeviceSetting{DeviceID: "B1_R102", FallThreshold: 0.5, MountHeight: 2.0, RoomWidth: 3.0, RoomLength: 4.0})

		// ห้องน้ำ 103: สถานะมีคนอยู่ (กำลังใช้งาน)
		db.Create(&iot.Device{
			DeviceID:    "B1_R103",
			Name:        "ห้องน้ำ 103",
			GroupID:     g1.ID,
			Status:      "online",
			LatestEvent: "enter",
		})
		db.Create(&iot.DeviceSetting{DeviceID: "B1_R103", FallThreshold: 0.5, MountHeight: 2.0, RoomWidth: 3.0, RoomLength: 4.0})

		// ห้องน้ำ 104: สถานะปกติ (มีประวัติเส้นทางสั้นๆ และเดินกลับออกมา)
		db.Create(&iot.Device{
			DeviceID:            "B1_R104",
			Name:                "ห้องน้ำ 104",
			GroupID:             g1.ID,
			Status:              "online",
			LatestEvent:         "exit",
			LatestEventMetadata: `{"path": [[0.0, 3.8], [0.2, 2.5], [0.0, 1.5], [0.0, 0.7], [0.0, 1.5], [-0.2, 2.5], [0.0, 3.8]]}`,
		})
		db.Create(&iot.DeviceSetting{DeviceID: "B1_R104", FallThreshold: 0.5, MountHeight: 2.0, RoomWidth: 3.0, RoomLength: 4.0})

		// ห้องน้ำ 105: สถานะเครื่องปิด (Offline)
		db.Create(&iot.Device{
			DeviceID:    "B1_R105",
			Name:        "ห้องน้ำ 105",
			GroupID:     g1.ID,
			Status:      "offline",
			LatestEvent: "exit",
		})
		db.Create(&iot.DeviceSetting{DeviceID: "B1_R105", FallThreshold: 0.5, MountHeight: 2.0, RoomWidth: 3.0, RoomLength: 4.0})
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
	// อ่านค่าที่อยู่ Broker จาก Environment Variable (ดีฟอลต์เป็น localhost:1883)
	mqttBrokerURI := os.Getenv("MQTT_BROKER_URI")
	if mqttBrokerURI == "" {
		mqttBrokerURI = "tcp://localhost:1883"
	}
	mqttClient := mqtt.SetupMQTT(mqttBrokerURI, devService)

	defer mqttClient.Disconnect(250)

	// อ่านค่า Allowed Origins สำหรับ CORS จาก Environment Variable
	allowedOriginsEnv := os.Getenv("ALLOWED_ORIGINS")
	allowedOrigins := []string{"http://localhost:3000"}
	if allowedOriginsEnv != "" {
		if allowedOriginsEnv == "*" {
			allowedOrigins = []string{"*"}
		} else {
			allowedOrigins = strings.Split(allowedOriginsEnv, ",")
		}
	}

	r := gin.Default()
	
	// ตั้งค่า CORS Configuration
	corsConfig := cors.Config{
		AllowMethods:     []string{"GET", "POST", "PUT", "PATCH", "DELETE", "OPTIONS"},
		AllowHeaders:     []string{"Origin", "Authorization", "Content-Type", "Accept"},
		ExposeHeaders:    []string{"Content-Length"},
		AllowCredentials: true,
	}
	
	if len(allowedOrigins) > 0 && allowedOrigins[0] == "*" {
		corsConfig.AllowAllOrigins = true
		corsConfig.AllowCredentials = false // เบราว์เซอร์ไม่อนุญาตให้ใช้ AllowCredentials ร่วมกับ wildcard '*'
	} else {
		corsConfig.AllowOrigins = allowedOrigins
	}
	
	r.Use(cors.New(corsConfig))

	// ✨ จุดสำคัญที่ 3: เพิ่ม Route สำหรับให้หน้าจอ Dashboard (Next.js) มาเชื่อมต่อ WebSocket
	r.GET("/ws", ws.HandleConnections)

	v1 := r.Group("/api/v1")
	{
		v1.POST("/register", regHandler.Handle)
		v1.POST("/login", loginHandler.Handle)

		protected := v1.Group("/iot")
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
				c.JSON(200, gin.H{
					"message": "ยินดีต้อนรับเข้าสู่ระบบข้อมูลส่วนตัว",
					"your_id": "ผู้ดูแลระบบ (System Admin)",
				})
			})
		}

		// API เดิมที่รับผ่าน HTTP สำหรับทดสอบผ่านเครื่องมือภายนอก (เช่น Postman)
		v1.POST("/events", devHandler.ReceiveEvent)
		v1.GET("/config/:device_id", devHandler.GetSettings)
	}

	r.Run(":8080")
}
