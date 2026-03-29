package main

import (
	"backend/internal/auth/login"    // import โฟลเดอร์ใหม่
	"backend/internal/auth/register" // import โฟลเดอร์ใหม่
	"backend/internal/database"
	"backend/internal/middleware"
	"net/http"

	"github.com/gin-gonic/gin"
)

func main() {
	db, err := database.ConnectDB()
	if err != nil {
		panic("เชื่อมต่อ Database ไม่ได้ค่ะเซนเซย์!: " + err.Error())
	}

	regService := register.NewService(db)
	regHandler := register.NewHandler(regService)

	loginService := login.NewService(db)
	loginHandler := login.NewHandler(loginService)

	r := gin.Default()

	v1 := r.Group("/api/v1")
	{
		v1.POST("/register", regHandler.Handle)
		v1.POST("/login", loginHandler.Handle)

		// *** ต้องมีส่วนนี้ด้วยนะคะเซนเซย์! ***
		protected := v1.Group("/user").Use(middleware.AuthMiddleware())
		{
			protected.GET("/profile", func(c *gin.Context) {
				userID, _ := c.Get("user_id")
				c.JSON(http.StatusOK, gin.H{
					"message": "ยินดีต้อนรับเข้าสู่ห้องส่วนตัวค่ะเซนเซย์!",
					"your_id": userID,
				})
			})
		}
	}

	r.Run(":8080")
}
