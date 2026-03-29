package group

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

func (h *Handler) Create(c *gin.Context) {
	var input struct {
		Name string `json:"name" binding:"required"`
	}
	if err := c.ShouldBindJSON(&input); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "ชื่อกลุ่มห้ามว่างนะคะเซนเซย์!"})
		return
	}

	userID, _ := c.Get("user_id") // ดึงจาก Middleware
	if err := h.service.Create(input.Name, userID.(uint)); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "สร้างกลุ่มไม่สำเร็จค่ะ"})
		return
	}
	c.JSON(http.StatusCreated, gin.H{"message": "สร้างสถานที่สำเร็จแล้ว! ✨"})
}
