package model

type Event struct {
	DeviceID  string `json:"device_id"`
	EventType string `json:"type"`
	Timestamp int64  `json:"timestamp"`
}
