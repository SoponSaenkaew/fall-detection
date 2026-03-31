package device

import (
	"fmt"
	"net/http"

	"backend/internal/iot"

	"github.com/gin-gonic/gin"
)

type Handler struct {
	service *Service
}

func NewHandler(s *Service) *Handler {
	return &Handler{service: s}
}

// ลงทะเบียนอุปกรณ์เข้ากับกลุ่ม (ใช้โดย User)
func (h *Handler) Register(c *gin.Context) {
	var input struct {
		GroupID  uint   `json:"group_id" binding:"required"`
		DeviceID string `json:"device_id" binding:"required"`
		Name     string `json:"name" binding:"required"`
	}

	if err := c.ShouldBindJSON(&input); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "ข้อมูลอุปกรณ์ไม่ครบค่ะ"})
		return
	}

	if err := h.service.Register(input.GroupID, input.DeviceID, input.Name); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "ลงทะเบียนอุปกรณ์ไม่สำเร็จ"})
		return
	}

	c.JSON(http.StatusCreated, gin.H{"message": "ลงทะเบียนอุปกรณ์เรียบร้อย! ✨"})
}

// รับข้อมูลจากตัวเซนเซอร์ (Enter, Fall, Exit)
func (h *Handler) ReceiveEvent(c *gin.Context) {
	var input struct {
		DeviceID string `json:"device_id" binding:"required"`
		Type     string `json:"type" binding:"required"` // event/status
		Name     string `json:"name" binding:"required"` // fall/enter/exit
		Value    string `json:"value"`
		Metadata string `json:"metadata"`
	}

	if err := c.ShouldBindJSON(&input); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "ข้อมูล Event ไม่ถูกต้อง"})
		return
	}

	if err := h.service.ProcessEvent(input.DeviceID, input.Type, input.Name, input.Value, input.Metadata); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "บันทึกข้อมูลไม่สำเร็จ"})
		return
	}

	c.JSON(http.StatusOK, gin.H{"message": "รับข้อมูลเรียบร้อย!"})
}

// เพิ่มใน internal/iot/device/handler.go

func (h *Handler) AddNotificationConfig(c *gin.Context) {
	var input iot.NotificationConfig
	if err := c.ShouldBindJSON(&input); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "ข้อมูลไม่ถูกต้องนะคะเซนเซย์"})
		return
	}

	if err := h.service.CreateNotificationConfig(input); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "บันทึกการตั้งค่าไม่สำเร็จ"})
		return
	}

	c.JSON(http.StatusCreated, gin.H{"message": "เพิ่มช่องทางแจ้งเตือนสำเร็จแล้วค่ะ! ✨"})
}

func (h *Handler) Delete(c *gin.Context) {
	deviceID := c.Param("device_id")
	if err := h.service.Delete(deviceID); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "ลบอุปกรณ์ไม่สำเร็จค่ะ"})
		return
	}
	c.JSON(http.StatusOK, gin.H{"message": "ลบอุปกรณ์เรียบร้อยแล้วค่ะ! ✨"})
}

func (h *Handler) GetNotificationConfigs(c *gin.Context) {
	groupIDStr := c.Query("group_id") // รับ group_id จาก Query Param
	var groupID uint
	fmt.Sscanf(groupIDStr, "%d", &groupID)

	configs, err := h.service.GetNotificationConfigs(groupID) //
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "ดึงข้อมูลไม่สำเร็จ"})
		return
	}
	c.JSON(http.StatusOK, configs)
}

// backend/internal/iot/device/handler.go
func (h *Handler) Update(c *gin.Context) {
	originalID := c.Param("device_id")
	var input iot.Device
	if err := c.ShouldBindJSON(&input); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "ข้อมูลที่ส่งมาไม่ถูกต้องค่ะ"})
		return
	}

	if err := h.service.Update(originalID, input); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "อัปเดตข้อมูลอุปกรณ์ไม่สำเร็จ"})
		return
	}
	c.JSON(http.StatusOK, gin.H{"message": "แก้ไขข้อมูลเซนเซอร์เรียบร้อยแล้วค่ะ! ✨"})
}
