package device

import (
	"fmt"
	"net/http"
	"strconv"

	"backend/internal/iot"
	"backend/internal/ws"

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
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
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
	ws.BroadcastEvent(input)

	c.JSON(http.StatusOK, gin.H{"message": "รับข้อมูลเรียบร้อย!"})
}

// เพิ่มใน internal/iot/device/handler.go

func (h *Handler) AddNotificationConfig(c *gin.Context) {
	var input iot.NotificationConfig
	if err := c.ShouldBindJSON(&input); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "ข้อมูลไม่ถูกต้อง"})
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
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	c.JSON(http.StatusOK, gin.H{"message": "แก้ไขข้อมูลเซนเซอร์เรียบร้อยแล้ว"})
}

func (h *Handler) UpdateSettings(c *gin.Context) {
	var input iot.DeviceSetting
	if err := c.ShouldBindJSON(&input); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "ข้อมูลการตั้งค่าไม่ถูกต้อง"})
		return
	}

	if input.DeviceID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "กรุณาระบุรหัสอุปกรณ์ (device_id)"})
		return
	}
	if input.StayTimeoutMinutes < 1.0 || input.StayTimeoutMinutes > 60.0 {
		c.JSON(http.StatusBadRequest, gin.H{"error": "ระยะเวลาแจ้งเตือนอยู่นานเกินกำหนดต้องอยู่ระหว่าง 1.0 ถึง 60.0 นาที"})
		return
	}
	if input.FallThreshold < 0.2 || input.FallThreshold > 0.8 {
		c.JSON(http.StatusBadRequest, gin.H{"error": "เกณฑ์ตรวจจับการล้มตามสเปกเรดาร์ต้องอยู่ระหว่าง 0.20 ถึง 0.80 เมตร"})
		return
	}
	if input.MountHeight < 2.0 || input.MountHeight > 3.0 {
		c.JSON(http.StatusBadRequest, gin.H{"error": "ความสูงติดตั้งตามสเปกเซนเซอร์ HLK-LD6002C ต้องอยู่ระหว่าง 2.0 ถึง 3.0 เมตร"})
		return
	}
	if input.RoomWidth < 0.8 || input.RoomWidth > 4.0 {
		c.JSON(http.StatusBadRequest, gin.H{"error": "ความกว้างห้องต้องอยู่ระหว่าง 0.8 ถึง 4.0 เมตร (อิงรัศมีตรวจจับเรดาร์สูงสุด 2.0 เมตร)"})
		return
	}
	if input.RoomLength < 0.8 || input.RoomLength > 4.0 {
		c.JSON(http.StatusBadRequest, gin.H{"error": "ความยาวห้องต้องอยู่ระหว่าง 0.8 ถึง 4.0 เมตร (อิงรัศมีตรวจจับเรดาร์สูงสุด 2.0 เมตร)"})
		return
	}

	if err := h.service.UpdateSettings(input); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "บันทึกค่าพารามิเตอร์ไม่สำเร็จ"})
		return
	}
	c.JSON(http.StatusOK, gin.H{"message": "อัปเดตเกณฑ์การตรวจจับเรียบร้อยแล้ว"})
}

func (h *Handler) GetSettings(c *gin.Context) {
	deviceID := c.Param("device_id")
	settings, err := h.service.GetSettings(deviceID)
	if err != nil {
		// ถ้ายังไม่มีการตั้งค่า ให้คืนค่า Default แนะนำตามสเปกเซนเซอร์ (2.2m, 2.0m, 2.0m)
		c.JSON(http.StatusOK, iot.DeviceSetting{
			DeviceID:           deviceID,
			MountHeight:        2.2,
			RoomWidth:          2.0,
			RoomLength:         2.0,
			StayTimeoutMinutes: 30.0,
			FallThreshold:      0.5,
		})
		return
	}
	c.JSON(http.StatusOK, settings)
}

// GET /api/v1/iot/events
func (h *Handler) GetEvents(c *gin.Context) {
	var filter EventFilter

	// ดึง userID จาก Context (ถ้ามี AuthMiddleware)
	if val, exists := c.Get("user_id"); exists {
		switch v := val.(type) {
		case float64:
			filter.UserID = uint(v)
		case uint:
			filter.UserID = v
		case int:
			filter.UserID = uint(v)
		}
	}

	if gID := c.Query("group_id"); gID != "" {
		if id, err := strconv.ParseUint(gID, 10, 32); err == nil {
			filter.GroupID = uint(id)
		}
	}

	filter.Name = c.Query("name")
	filter.Status = c.Query("status")
	filter.Search = c.Query("search")

	limit := 50
	if l := c.Query("limit"); l != "" {
		if l == "all" || l == "-1" {
			limit = -1
		} else if val, err := strconv.Atoi(l); err == nil && val > 0 {
			limit = val
		}
	}
	filter.Limit = limit

	offset := 0
	if limit > 0 {
		if p := c.Query("page"); p != "" {
			if page, err := strconv.Atoi(p); err == nil && page > 1 {
				offset = (page - 1) * limit
			}
		}
	}
	filter.Offset = offset

	events, total, err := h.service.GetEvents(filter)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "ไม่สามารถดึงข้อมูลประวัติเหตุการณ์ได้: " + err.Error()})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"events": events,
		"total":  total,
		"limit":  limit,
	})
}

// PUT /api/v1/iot/events/:id/resolve
func (h *Handler) ResolveEvent(c *gin.Context) {
	idParam := c.Param("id")
	eventID, err := strconv.ParseUint(idParam, 10, 32)
	if err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "รหัสเหตุการณ์ไม่ถูกต้อง"})
		return
	}

	var req struct {
		Status         string `json:"status" binding:"required"` // "resolved" หรือ "false_alarm"
		ResolvedBy     string `json:"resolved_by"`
		ResolutionNote string `json:"resolution_note"`
	}

	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "ข้อมูลสถานะไม่ครบถ้วน"})
		return
	}

	// ถ้าไม่ระบุผู้ช่วยเหลือ ให้ดึงจาก Token Username
	if req.ResolvedBy == "" {
		if val, exists := c.Get("username"); exists {
			if uName, ok := val.(string); ok {
				req.ResolvedBy = uName
			}
		}
		if req.ResolvedBy == "" {
			req.ResolvedBy = "ผู้ดูแลระบบ"
		}
	}

	if err := h.service.ResolveEvent(uint(eventID), req.Status, req.ResolvedBy, req.ResolutionNote); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "อัปเดตสถานะไม่สำเร็จ: " + err.Error()})
		return
	}

	c.JSON(http.StatusOK, gin.H{"message": "บันทึกการเข้าช่วยเหลือเรียบร้อยแล้ว! ✨"})
}

// GET /api/v1/iot/events/stats
func (h *Handler) GetStats(c *gin.Context) {
	var userID uint
	if val, exists := c.Get("user_id"); exists {
		switch v := val.(type) {
		case float64:
			userID = uint(v)
		case uint:
			userID = v
		case int:
			userID = uint(v)
		}
	}

	var groupID uint
	if gID := c.Query("group_id"); gID != "" {
		if id, err := strconv.ParseUint(gID, 10, 32); err == nil {
			groupID = uint(id)
		}
	}

	stats, err := h.service.GetEventStats(userID, groupID)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "ดึงสถิติไม่สำเร็จ: " + err.Error()})
		return
	}

	c.JSON(http.StatusOK, stats)
}
