package device

import (
	"backend/internal/iot"
	"backend/internal/iot/notifier"
	"fmt"
	"time"

	"gorm.io/gorm"
)

type ConfigPublisherFunc func(deviceID string, settings iot.DeviceSetting) error

type Service struct {
	db              *gorm.DB
	configPublisher ConfigPublisherFunc
}

func NewService(db *gorm.DB) *Service {
	return &Service{db: db}
}

func (s *Service) SetConfigPublisher(fn ConfigPublisherFunc) {
	s.configPublisher = fn
}

func (s *Service) Register(groupID uint, deviceID, name string) error {
	// 1. ตรวจสอบว่ามีอุปกรณ์ที่ยังใช้งานอยู่ (ไม่ได้ถูกลบ) ใช้ Device ID นี้อยู่แล้วหรือไม่
	var activeDev iot.Device
	if err := s.db.Where("device_id = ?", deviceID).First(&activeDev).Error; err == nil {
		return fmt.Errorf("รหัสอุปกรณ์ '%s' มีอยู่ในระบบแล้ว กรุณาใช้รหัสอื่น", deviceID)
	}

	// 2. ถ้าเคยมี Device ID นี้แต่ถูกลบ (Soft Delete) ไปแล้ว สามารถกู้คืน (Restore) กลับมาใช้งานใหม่ได้
	var deletedDev iot.Device
	if err := s.db.Unscoped().Where("device_id = ? AND deleted_at IS NOT NULL", deviceID).First(&deletedDev).Error; err == nil {
		return s.db.Unscoped().Model(&deletedDev).Updates(map[string]interface{}{
			"deleted_at": nil,
			"group_id":   groupID,
			"name":       name,
			"status":     "offline",
		}).Error
	}

	// 3. ถ้าเป็น Device ID ใหม่ ให้สร้างแถวใหม่ตามปกติ
	return s.db.Create(&iot.Device{
		DeviceID: deviceID,
		Name:     name,
		GroupID:  groupID,
		Status:   "offline",
	}).Error
}

// ฟังก์ชันรับ Event ที่ยืดหยุ่นรองรับอนาคตค่ะ
func (s *Service) ProcessEvent(devID, evType, name, val, meta string) error {
	var dev iot.Device
	if err := s.db.Where("device_id = ?", devID).First(&dev).Error; err != nil {
		return err
	}
	if meta == "" {
		meta = "{}"
	}

	// บันทึก Event ลง DB พร้อมกำหนดสถานะ
	status := "normal"
	if name == "fall" || name == "prolonged_stay" {
		status = "pending"
	}
	event := iot.DeviceEvent{
		DeviceID: dev.ID,
		Type:     evType,
		Name:     name,
		Value:    val,
		Metadata: meta,
		Status:   status,
	}
	s.db.Create(&event)

	updates := make(map[string]interface{})

	switch evType {
	case "status":
		updates["status"] = name // เช่น online / offline
	case "event":
		updates["latest_event"] = name          // เช่น fall / enter / exit / prolonged_stay
		updates["latest_event_metadata"] = meta // บันทึก Metadata ล่าสุดด้วย
	}

	// ✨ ถ้ามีการล้ม ให้ดึง Config การแจ้งเตือนมาทำงาน
	// ใน service.go ของ device
	// backend/internal/iot/device/service.go
	if name == "fall" {
		var group iot.Group
		groupName := ""
		if err := s.db.First(&group, dev.GroupID).Error; err == nil && group.Name != "" {
			groupName = group.Name
		}

		var configs []iot.NotificationConfig
		// ดึงการตั้งค่าทั้งหมดที่เปิดใช้งาน (Enabled) ของกลุ่มนี้
		s.db.Where("group_id = ? AND enabled = ?", dev.GroupID, true).Find(&configs)

		loc := time.FixedZone("Asia/Bangkok", 7*3600)
		timeStr := time.Now().In(loc).Format("15:04 น. (02/01/2006)")

		var msg string
		if groupName != "" {
			msg = fmt.Sprintf("🚨 [แจ้งเตือนเหตุฉุกเฉิน: ตรวจพบคนหกล้ม!]\n📍 สถานที่: %s\n🚪 ห้อง/จุดติดตั้ง: %s\n⏰ เวลา: %s\n⚠️ กรุณาเข้าตรวจสอบและให้ความช่วยเหลือโดยด่วน!", groupName, dev.Name, timeStr)
		} else {
			msg = fmt.Sprintf("🚨 [แจ้งเตือนเหตุฉุกเฉิน: ตรวจพบคนหกล้ม!]\n🚪 ห้อง/จุดติดตั้ง: %s\n⏰ เวลา: %s\n⚠️ กรุณาเข้าตรวจสอบและให้ความช่วยเหลือโดยด่วน!", dev.Name, timeStr)
		}

		for _, cfg := range configs {
			switch cfg.Type {
			case "line":
				// ส่งผ่าน LINE Messaging API
				go notifier.SendLineNotification(cfg.Type, cfg.LineToken, cfg.LineGroupID, msg)
			case "webhook":
				// ส่งผ่าน Webhook ทั่วไป
				go notifier.SendSimpleNotification("webhook", cfg.TargetURL, msg)
			}
		}
	} else if name == "prolonged_stay" {
		var group iot.Group
		groupName := ""
		if err := s.db.First(&group, dev.GroupID).Error; err == nil && group.Name != "" {
			groupName = group.Name
		}

		var configs []iot.NotificationConfig
		s.db.Where("group_id = ? AND enabled = ?", dev.GroupID, true).Find(&configs)

		loc := time.FixedZone("Asia/Bangkok", 7*3600)
		timeStr := time.Now().In(loc).Format("15:04 น. (02/01/2006)")

		var msg string
		if groupName != "" {
			msg = fmt.Sprintf("⏳ [แจ้งเตือน: เข้าห้องน้ำนานเกินกำหนด]\n📍 สถานที่: %s\n🚪 ห้อง/จุดติดตั้ง: %s\n⏰ เวลา: %s\n⚠️ มีการใช้งานห้องน้ำต่อเนื่องนานเกินกำหนด กรุณาเข้าตรวจสอบความปลอดภัย", groupName, dev.Name, timeStr)
		} else {
			msg = fmt.Sprintf("⏳ [แจ้งเตือน: เข้าห้องน้ำนานเกินกำหนด]\n🚪 ห้อง/จุดติดตั้ง: %s\n⏰ เวลา: %s\n⚠️ มีการใช้งานห้องน้ำต่อเนื่องนานเกินกำหนด กรุณาเข้าตรวจสอบความปลอดภัย", dev.Name, timeStr)
		}

		for _, cfg := range configs {
			switch cfg.Type {
			case "line":
				go notifier.SendLineNotification(cfg.Type, cfg.LineToken, cfg.LineGroupID, msg)
			case "webhook":
				go notifier.SendSimpleNotification("webhook", cfg.TargetURL, msg)
			}
		}
	}
	return s.db.Model(&dev).Updates(updates).Error
}

// เพิ่มใน internal/iot/device/service.go

func (s *Service) CreateNotificationConfig(config iot.NotificationConfig) error {
	return s.db.Create(&config).Error
}

func (s *Service) GetNotificationConfigs(groupID uint) ([]iot.NotificationConfig, error) {
	var configs []iot.NotificationConfig
	err := s.db.Where("group_id = ?", groupID).Find(&configs).Error
	return configs, err
}

func (s *Service) Delete(deviceID string) error {
	// ทำการ Soft Delete ตัวอุปกรณ์ออกจากหน้ากลุ่ม แต่เก็บประวัติ DeviceEvent และ DeviceSetting ไว้ในระบบ ไม่สูญหาย
	return s.db.Where("device_id = ?", deviceID).Delete(&iot.Device{}).Error
}

// backend/internal/iot/device/service.go
func (s *Service) Update(originalID string, newData iot.Device) error {
	// ตรวจสอบว่าถ้ามีการเปลี่ยนรหัสอุปกรณ์ รหัสใหม่จะต้องไม่ซ้ำกับอุปกรณ์อื่นที่ใช้งานอยู่
	if newData.DeviceID != originalID {
		var activeDev iot.Device
		if err := s.db.Where("device_id = ? AND device_id <> ?", newData.DeviceID, originalID).First(&activeDev).Error; err == nil {
			return fmt.Errorf("รหัสอุปกรณ์ '%s' มีอยู่ในระบบแล้ว กรุณาใช้รหัสอื่น", newData.DeviceID)
		}
	}
	return s.db.Model(&iot.Device{}).
		Where("device_id = ?", originalID).
		Updates(map[string]interface{}{
			"device_id": newData.DeviceID,
			"name":      newData.Name,
		}).Error
}

func (s *Service) UpdateSettings(settings iot.DeviceSetting) error {
	// ใช้ Save เพื่อทำ Upsert (ถ้ามี ID เดิมจะอัปเดต ถ้าไม่มีจะสร้างใหม่ค่ะ)
	err := s.db.Where("device_id = ?", settings.DeviceID).
		Assign(settings).
		FirstOrCreate(&iot.DeviceSetting{DeviceID: settings.DeviceID}).Error
	if err != nil {
		return err
	}

	// บรอดแคสต์การตั้งค่าใหม่ไปยังอุปกรณ์ผ่าน MQTT แบบเรียลไทม์ทันที
	if s.configPublisher != nil {
		go func() {
			if pubErr := s.configPublisher(settings.DeviceID, settings); pubErr != nil {
				fmt.Printf("⚠️ [CONFIG PUBLISH] ไม่สามารถส่งค่าไปยัง MQTT ได้: %v\n", pubErr)
			}
		}()
	}

	return nil
}

func (s *Service) GetSettings(deviceID string) (iot.DeviceSetting, error) {
	var settings iot.DeviceSetting
	err := s.db.Where("device_id = ?", deviceID).First(&settings).Error
	return settings, err
}

// โครงสร้างข้อมูลสำหรับส่งออกประวัติเหตุการณ์ไปยัง Frontend
type EventItem struct {
	ID             uint       `json:"id"`
	CreatedAt      time.Time  `json:"created_at"`
	DeviceID       uint       `json:"device_id"`
	DeviceCode     string     `json:"device_code"`
	DeviceName     string     `json:"device_name"`
	GroupID        uint       `json:"group_id"`
	GroupName      string     `json:"group_name"`
	Type           string     `json:"type"`
	Name           string     `json:"name"`
	Value          string     `json:"value"`
	Metadata       string     `json:"metadata"`
	Status         string     `json:"status"`
	ResolvedAt     *time.Time `json:"resolved_at"`
	ResolvedBy     string     `json:"resolved_by"`
	ResolutionNote string     `json:"resolution_note"`
}

type EventFilter struct {
	UserID  uint
	GroupID uint
	Name    string
	Status  string
	Search  string
	Limit   int
	Offset  int
}

type EventStats struct {
	TotalFalls    int64 `json:"total_falls"`
	PendingFalls  int64 `json:"pending_falls"`
	ResolvedFalls int64 `json:"resolved_falls"`
	OnlineDevices int64 `json:"online_devices"`
	TotalDevices  int64 `json:"total_devices"`
}

// ดึงประวัติเหตุการณ์พร้อมข้อมูลชื่อห้องและกลุ่มสถานที่
func (s *Service) GetEvents(filter EventFilter) ([]EventItem, int64, error) {
	query := s.db.Table("device_events").
		Select(`device_events.id, 
		        device_events.created_at, 
		        device_events.device_id, 
		        COALESCE(devices.device_id, '') as device_code, 
		        COALESCE(devices.name, 'ไม่ทราบอุปกรณ์') as device_name, 
		        COALESCE(devices.group_id, 0) as group_id, 
		        COALESCE(groups.name, 'ทั่วไป') as group_name, 
		        device_events.type, 
		        device_events.name, 
		        device_events.value, 
		        device_events.metadata, 
		        COALESCE(device_events.status, 'normal') as status, 
		        device_events.resolved_at, 
		        device_events.resolved_by, 
		        device_events.resolution_note`).
		Joins("LEFT JOIN devices ON devices.id = device_events.device_id").
		Joins("LEFT JOIN groups ON groups.id = devices.group_id")

	// กรองตาม User ที่เป็นเจ้าของกลุ่ม (ถ้ามีระบุ)
	if filter.UserID > 0 {
		query = query.Where("(groups.user_id = ? OR groups.user_id IS NULL OR groups.user_id = 0)", filter.UserID)
	}

	if filter.GroupID > 0 {
		query = query.Where("devices.group_id = ?", filter.GroupID)
	}

	if filter.Name != "" && filter.Name != "all" {
		query = query.Where("device_events.name = ?", filter.Name)
	}

	if filter.Status != "" && filter.Status != "all" {
		query = query.Where("device_events.status = ?", filter.Status)
	}

	if filter.Search != "" {
		likeSearch := "%" + filter.Search + "%"
		query = query.Where("(devices.name ILIKE ? OR devices.device_id ILIKE ? OR groups.name ILIKE ?)", likeSearch, likeSearch, likeSearch)
	}

	var total int64
	if err := query.Count(&total).Error; err != nil {
		return nil, 0, err
	}

	if filter.Limit == 0 {
		filter.Limit = 50
	}

	var events []EventItem
	query = query.Order("device_events.created_at DESC")
	if filter.Limit > 0 {
		query = query.Limit(filter.Limit).Offset(filter.Offset)
	}

	err := query.Scan(&events).Error

	return events, total, err
}

// อัปเดตสถานะการเข้าช่วยเหลือ (Resolve / False Alarm)
func (s *Service) ResolveEvent(eventID uint, status, resolvedBy, note string) error {
	now := time.Now()
	updates := map[string]interface{}{
		"status":          status,
		"resolved_at":     &now,
		"resolved_by":     resolvedBy,
		"resolution_note": note,
	}
	return s.db.Model(&iot.DeviceEvent{}).Where("id = ?", eventID).Updates(updates).Error
}

// ดึงตัวเลขสถิติภาพรวมสำหรับ Dashboard สรุปผล
func (s *Service) GetEventStats(userID uint, groupID uint) (EventStats, error) {
	var stats EventStats

	baseEventQuery := s.db.Model(&iot.DeviceEvent{}).
		Joins("LEFT JOIN devices ON devices.id = device_events.device_id")

	if groupID > 0 {
		baseEventQuery = baseEventQuery.Where("devices.group_id = ?", groupID)
	} else if userID > 0 {
		baseEventQuery = baseEventQuery.
			Joins("LEFT JOIN groups ON groups.id = devices.group_id").
			Where("(groups.user_id = ? OR groups.user_id IS NULL OR groups.user_id = 0)", userID)
	}

	// นับเหตุการณ์ล้ม
	if err := baseEventQuery.Session(&gorm.Session{}).
		Where("device_events.name = ?", "fall").
		Count(&stats.TotalFalls).Error; err != nil {
		return stats, err
	}

	// นับเหตุการณ์ล้มที่รอการช่วยเหลือ
	if err := baseEventQuery.Session(&gorm.Session{}).
		Where("device_events.name = ? AND device_events.status = ?", "fall", "pending").
		Count(&stats.PendingFalls).Error; err != nil {
		return stats, err
	}

	// นับเหตุการณ์ล้มที่ช่วยเหลือแล้ว
	if err := baseEventQuery.Session(&gorm.Session{}).
		Where("device_events.name = ? AND device_events.status = ?", "fall", "resolved").
		Count(&stats.ResolvedFalls).Error; err != nil {
		return stats, err
	}

	// อุปกรณ์ออนไลน์ / ทั้งหมด
	devQuery := s.db.Model(&iot.Device{})
	if groupID > 0 {
		devQuery = devQuery.Where("devices.group_id = ?", groupID)
	} else if userID > 0 {
		devQuery = devQuery.Joins("LEFT JOIN groups ON groups.id = devices.group_id").
			Where("(groups.user_id = ? OR groups.user_id IS NULL OR groups.user_id = 0)", userID)
	}

	if err := devQuery.Session(&gorm.Session{}).Count(&stats.TotalDevices).Error; err != nil {
		return stats, err
	}

	if err := devQuery.Session(&gorm.Session{}).
		Where("devices.status = ?", "online").
		Count(&stats.OnlineDevices).Error; err != nil {
		return stats, err
	}

	return stats, nil
}
