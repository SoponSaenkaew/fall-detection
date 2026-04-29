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

	// ✨ 1. เพิ่มระบบรายงานตัวตอนเริ่มเชื่อมต่อ
	opts.OnConnect = func(c mqtt.Client) {
		fmt.Println("🌐 [MQTT] Backend เชื่อมต่อกับ Broker สำเร็จแล้วพร้อมลุย!")
	}
	opts.OnConnectionLost = func(c mqtt.Client, err error) {
		fmt.Printf("⚠️ [MQTT] หลุดการเชื่อมต่อ: %v\n", err)
	}

	client := mqtt.NewClient(opts)
	if token := client.Connect(); token.Wait() && token.Error() != nil {
		panic(token.Error())
	}

	// ✨ 2. ผูกฟังก์ชันรับข้อมูลเข้ากับ Topic "sensor/events" ตรงๆ เลย! ชัวร์กว่า 100%
	token := client.Subscribe("sensor/events", 0, func(client mqtt.Client, msg mqtt.Message) {
		fmt.Println("\n📩 [MQTT] ได้รับข้อความดิบ:", string(msg.Payload()))

		var payload SensorPayload
		if err := json.Unmarshal(msg.Payload(), &interface{}{&payload}); err != nil {
			fmt.Println("❌ [MQTT] แปลงข้อมูล JSON ไม่สำเร็จ:", err)
			return
		}

		// ✨ 3. ลองบันทึกลงฐานข้อมูล พร้อมดักจับ Error
		err := devService.ProcessEvent(payload.DeviceID, payload.Type, payload.Name, payload.Value, "{}")
		if err != nil {
			fmt.Println("❌ [MQTT] บันทึกลง DB ไม่สำเร็จ (เซนเซย์เช็ครหัส Device ID ให้ตรงกับหน้าเว็บน้า):", err)
		} else {
			fmt.Println("✅ [MQTT] บันทึกลง DB สำเร็จ!")
		}

		// ✨ 4. บรอดแคสต์เข้าหน้าเว็บ
		ws.BroadcastEvent(payload)
		fmt.Println("📢 [MQTT] ส่งสัญญาณไฟกระพริบไปที่หน้า Dashboard แล้ว! ✨")
	})

	if token.Wait() && token.Error() != nil {
		panic(token.Error())
	}

	fmt.Println("📡 [MQTT] สแตนด์บายรอรับข้อมูลที่ Topic: sensor/events")
	return client
}