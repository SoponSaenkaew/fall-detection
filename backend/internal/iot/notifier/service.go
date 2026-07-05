package notifier

import (
	"bytes"
	"encoding/json"
	"fmt"
	"net/http"
)

// ฟังก์ชันกลางสำหรับกระจายข่าว
func SendSimpleNotification(notifType, target, message string) {
	switch notifType {
	case "discord":
		sendDiscord(target, message)
	case "webhook":
		sendWebhook(target, message)
	}
}

func SendLineNotification(notifType, target, GroupID, message string) {
	switch notifType {
	case "line":
		sendLine(target, GroupID, message)
	}
}

func sendWebhook(url, msg string) {
	payload := map[string]string{"event": "fall_detected", "message": msg}
	body, _ := json.Marshal(payload)
	http.Post(url, "application/json", bytes.NewBuffer(body))
}

func sendDiscord(url, msg string) {
	payload := map[string]string{"content": msg}
	body, _ := json.Marshal(payload)
	http.Post(url, "application/json", bytes.NewBuffer(body))
}

func sendLine(LineToken, LineGroupID, message string) {
	// 1. เตรียม Endpoint ของ LINE Messaging API
	url := "https://api.line.me/v2/bot/message/push"

	// 2. สร้าง Payload ตามรูปแบบที่ LINE กำหนด
	// targetID ในที่นี้คือ User ID หรือ Group ID ของ LINE ค่ะ
	payload := map[string]interface{}{
		"to": LineGroupID,
		"messages": []map[string]interface{}{
			{
				"type": "text",
				"text": message,
			},
		},
	}

	jsonData, err := json.Marshal(payload)
	if err != nil {
		fmt.Printf("Error marshal LINE payload: %v\n", err)
		return
	}

	// 3. สร้าง Request
	req, err := http.NewRequest("POST", url, bytes.NewBuffer(jsonData))
	if err != nil {
		fmt.Printf("Error creating LINE request: %v\n", err)
		return
	}

	// 4. ใส่ Header ที่จำเป็น (สำคัญมากค่ะ!)
	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+LineToken)

	// 5. ส่งข้อมูล
	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		fmt.Printf("Error sending to LINE: %v\n", err)
		return
	}
	defer resp.Body.Close()

	// ตรวจสอบผลลัพธ์การส่งการแจ้งเตือน
	if resp.StatusCode != http.StatusOK {
		fmt.Printf("LINE API returned non-200 status: %d\n", resp.StatusCode)
	} else {
		fmt.Println("Successfully sent LINE notification")
	}
}
