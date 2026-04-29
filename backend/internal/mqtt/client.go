package mqtt

import (
	"encoding/json"
	"fmt"

	"backend/internal/iot/device"
	"backend/internal/ws"

	mqtt "github.com/eclipse/paho.mqtt.golang"
)

type SensorPayload struct {
	DeviceID string `json:"device_id"`
	Type     string `json:"type"`
	Name     string `json:"name"`
	Value    string `json:"value"`
}

func SetupMQTT(brokerURI string, devService *device.Service) mqtt.Client {
	opts := mqtt.NewClientOptions().AddBroker(brokerURI).SetClientID("go_backend")

	// ตั้งค่าฟังก์ชันที่จะทำงานเมื่อมีข้อความเข้ามา
	opts.SetDefaultPublishHandler(func(client mqtt.Client, msg mqtt.Message) {
		var payload SensorPayload
		json.Unmarshal(msg.Payload(), &payload)

		// 1. นำข้อมูลไปบันทึกลง DB และส่งแจ้งเตือน LINE/Discord (ใช้ฟังก์ชัน ProcessEvent เดิมได้เลยค่ะ!)
		devService.ProcessEvent(payload.DeviceID, payload.Type, payload.Name, payload.Value, "")

		// 2. ส่งข้อมูลผ่าน WebSocket ไปให้ Dashboard (Next.js) อัปเดตแบบ Real-time
		ws.BroadcastEvent(payload)
		fmt.Println("ได้รับข้อมูลและบรอดแคสต์เรียบร้อย! ✨")
	})

	client := mqtt.NewClient(opts)
	if token := client.Connect(); token.Wait() && token.Error() != nil {
		panic(token.Error())
	}

	// ตั้ง Topic สำหรับดักฟังอุปกรณ์ ESP32
	client.Subscribe("sensor/events", 1, nil)
	return client
}
