package register

import (
	"net/http"

	"github.com/gin-gonic/gin"
)

type Handler struct {
	regService Service
}

func NewHandler(s Service) *Handler {
	return &Handler{regService: s}
}

func (h *Handler) Handle(c *gin.Context) {
	var input struct {
		Email    string `json:"email" binding:"required"`
		Username string `json:"username" binding:"required"`
		Password string `json:"password" binding:"required"`
	}

	if err := c.ShouldBindJSON(&input); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "ข้อมูลไม่ครบค่ะเซนเซย์!"})
		return
	}

	if err := h.regService.Execute(input.Email, input.Username, input.Password); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "สมัครสมาชิกไม่สำเร็จ"})
		return
	}

	c.JSON(http.StatusCreated, gin.H{"message": "ลงทะเบียนเรียบร้อย! ✨"})
}
