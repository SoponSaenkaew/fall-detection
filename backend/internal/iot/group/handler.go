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
func getUserID(c *gin.Context) (uint, error) {
	val, exists := c.Get("user_id")
	if !exists {
		return 0, fmt.Errorf("ไม่พบข้อมูลผู้ใช้งาน")
	}
	switch v := val.(type) {
	case float64:
		return uint(v), nil
	case uint:
		return v, nil
	case int:
		return uint(v), nil
	default:
		return 0, fmt.Errorf("ประเภทข้อมูล UserID ไม่ถูกต้อง")
	}
}

func (h *Handler) Create(c *gin.Context) {
	var input struct {
		Name string `json:"name" binding:"required"`
	}
	if err := c.ShouldBindJSON(&input); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "กรุณาระบุชื่อกลุ่ม"})
		return
	}

	userID, err := getUserID(c)
	if err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": err.Error()})
		return
	}

	if err := h.service.Create(input.Name, userID); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "สร้างกลุ่มไม่สำเร็จ: " + err.Error()})
		return
	}
	c.JSON(http.StatusCreated, gin.H{"message": "สร้างสถานที่สำเร็จแล้ว"})
}

func (h *Handler) GetAll(c *gin.Context) {
	userID, err := getUserID(c)
	if err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": err.Error()})
		return
	}

	groups, err := h.service.GetAll(userID)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	c.JSON(http.StatusOK, groups)
}

func (h *Handler) Delete(c *gin.Context) {
	idStr := c.Param("id") // รับ ID จาก URL
	userID, err := getUserID(c)
	if err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": err.Error()})
		return
	}

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

func (h *Handler) Update(c *gin.Context) {
	idStr := c.Param("id")
	var input struct {
		Name string `json:"name" binding:"required"`
	}
	if err := c.ShouldBindJSON(&input); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "กรุณาระบุชื่อกลุ่มใหม่ด้วยค่ะ"})
		return
	}

	userID, err := getUserID(c)
	if err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": err.Error()})
		return
	}

	if err := h.service.Update(uint(castID(idStr)), userID, input.Name); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "แก้ไขชื่อกลุ่มไม่สำเร็จค่ะ"})
		return
	}
	c.JSON(http.StatusOK, gin.H{"message": "อัปเดตชื่อสถานที่เรียบร้อย! ✨"})
}
