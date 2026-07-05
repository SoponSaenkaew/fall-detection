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
		c.JSON(http.StatusBadRequest, gin.H{"error": "กรุณาระบุชื่อกลุ่ม"})
		return
	}

	// ดึงข้อมูลรหัสผู้ใช้งาน (user_id) ดีฟอลต์ = 1 สำหรับระบบ Simple No-Auth
	userID := uint(1)

	if err := h.service.Create(input.Name, userID); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "สร้างกลุ่มไม่สำเร็จ: " + err.Error()})
		return
	}
	c.JSON(http.StatusCreated, gin.H{"message": "สร้างสถานที่สำเร็จแล้ว"})
}

// เพิ่มใน backend/internal/iot/group/handler.go
func (h *Handler) GetAll(c *gin.Context) {
	// ดึงข้อมูลรหัสผู้ใช้งาน (user_id) ดีฟอลต์ = 1 สำหรับระบบ Simple No-Auth
	userID := uint(1)

	groups, err := h.service.GetAll(userID)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	c.JSON(http.StatusOK, groups)
}

func (h *Handler) Delete(c *gin.Context) {
	idStr := c.Param("id") // รับ ID จาก URL
	// ดึงข้อมูลรหัสผู้ใช้งาน (user_id) ดีฟอลต์ = 1 สำหรับระบบ Simple No-Auth
	userID := uint(1)

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

	// ดึงข้อมูลรหัสผู้ใช้งาน (user_id) ดีฟอลต์ = 1 สำหรับระบบ Simple No-Auth
	userID := uint(1)

	if err := h.service.Update(uint(castID(idStr)), userID, input.Name); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "แก้ไขชื่อกลุ่มไม่สำเร็จค่ะ"})
		return
	}
	c.JSON(http.StatusOK, gin.H{"message": "อัปเดตชื่อสถานที่เรียบร้อย! ✨"})
}
