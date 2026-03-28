package sheets

import (
	"bytes"
	"encoding/json"
	"net/http"
)

type Client struct {
	URL string
}

type Payload struct {
	DeviceID  string `json:"device_id"`
	EventType string `json:"event_type"`
	Timestamp int64  `json:"timestamp"`
}

func NewClient(url string) *Client {
	return &Client{URL: url}
}

func (c *Client) Send(deviceID, eventType string, timestamp int64) error {
	payload := Payload{
		DeviceID:  deviceID,
		EventType: eventType,
		Timestamp: timestamp,
	}

	data, _ := json.Marshal(payload)

	_, err := http.Post(c.URL, "application/json", bytes.NewBuffer(data))
	return err
}
