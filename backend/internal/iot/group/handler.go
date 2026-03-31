package group

import (
	"fmt"
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

	// แก้ไขการดึง userID ตรงนี้ค่ะ
	val, exists := c.Get("user_id")
	if !exists {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "ไม่พบข้อมูลผู้ใช้งาน"})
		return
	}

	// แปลงจาก interface{} (ที่อาจเป็น float64) มาเป็น uint อย่างปลอดภัย
	var userID uint
	switch v := val.(type) {
	case float64:
		userID = uint(v)
	case uint:
		userID = v
	default:
		c.JSON(http.StatusInternalServerError, gin.H{"error": "ประเภทข้อมูล UserID ไม่ถูกต้อง"})
		return
	}

	if err := h.service.Create(input.Name, userID); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "สร้างกลุ่มไม่สำเร็จค่ะ: " + err.Error()})
		return
	}
	c.JSON(http.StatusCreated, gin.H{"message": "สร้างสถานที่สำเร็จแล้ว! ✨"})
}

// เพิ่มใน backend/internal/iot/group/handler.go
func (h *Handler) GetAll(c *gin.Context) {
	val, _ := c.Get("user_id")
	userID := uint(val.(float64)) // หรือใช้ switch-case ที่เซนเซย์แก้ไว้คราวก่อนค่ะ

	groups, err := h.service.GetAll(userID)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	c.JSON(http.StatusOK, groups)
}

func (h *Handler) Delete(c *gin.Context) {
	idStr := c.Param("id") // รับ ID จาก URL
	val, _ := c.Get("user_id")
	userID := uint(val.(float64))

	if err := h.service.Delete(uint(castID(idStr)), userID); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "ลบกลุ่มไม่สำเร็จค่ะ"})
		return
	}
	c.JSON(http.StatusOK, gin.H{"message": "ลบสถานที่เรียบร้อยแล้วค่ะ! ✨"})
}

// ฟังก์ชันช่วยแปลง ID
func castID(s string) int {
	var id int
	fmt.Sscanf(s, "%d", &id)
	return id
}
