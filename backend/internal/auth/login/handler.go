package login

import (
	"net/http"

	"github.com/gin-gonic/gin"
)

type Handler struct {
	service *Service
}

func NewHandler(s *Service) *Handler {
	return &Handler{service: s}
}

// internal/auth/login/handler.go
func (h *Handler) Handle(c *gin.Context) {
	var input struct {
		Email    string `json:"email" binding:"required,email"` // ตรวจสอบรูปแบบอีเมล
		Password string `json:"password" binding:"required"`
	}

	if err := c.ShouldBindJSON(&input); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "กรุณากรอกอีเมลให้ถูกต้อง"})
		return
	}

	// ส่ง email ไปให้ service
	token, err := h.service.Execute(input.Email, input.Password)
	if err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": err.Error()})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"message": "เข้าสู่ระบบด้วย Email สำเร็จแล้วค่ะ! ✨",
		"token":   token,
	})
}
