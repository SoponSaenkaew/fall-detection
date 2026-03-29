package middleware

import (
	"net/http"
	"os"
	"strings"

	"github.com/gin-gonic/gin"
	"github.com/golang-jwt/jwt/v5"
)

func AuthMiddleware() gin.HandlerFunc {
	return func(c *gin.Context) {
		// 1. ดึง Authorization Header ออกมา (รูปแบบคือ "Bearer <token>")
		authHeader := c.GetHeader("Authorization")
		if authHeader == "" {
			c.JSON(http.StatusUnauthorized, gin.H{"error": "กรุณาใส่ Token ด้วยนะเซนเซย์!"})
			c.Abort() // หยุดการทำงานทันที ไม่ให้ไปต่อค่ะ
			return
		}

		// 2. ตัดคำว่า "Bearer " ออกเพื่อเอาแต่ตัว Token
		tokenString := strings.TrimPrefix(authHeader, "Bearer ")

		// 3. ตรวจสอบ Token ด้วย JWT_SECRET ใน .env
		token, err := jwt.Parse(tokenString, func(token *jwt.Token) (interface{}, error) {
			return []byte(os.Getenv("JWT_SECRET")), nil
		})

		// 4. ถ้า Token ไม่ถูกต้องหรือหมดอายุ
		if err != nil || !token.Valid {
			c.JSON(http.StatusUnauthorized, gin.H{"error": "บัตรผ่านใช้ไม่ได้แล้วค่ะ!"})
			c.Abort()
			return
		}

		// 5. ถ้าผ่าน! เก็บข้อมูล user_id ไว้ใน Context เพื่อให้ Handler อื่นดึงไปใช้ได้ค่ะ
		if claims, ok := token.Claims.(jwt.MapClaims); ok {
			c.Set("user_id", claims["user_id"])
		}

		c.Next() // อนุญาตให้ผ่านด่านไปหา Handler ตัวจริงได้ค่ะ!
	}
}
