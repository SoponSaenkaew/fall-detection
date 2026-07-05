package main

import (
	"bufio"
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"net/http/cookiejar"
	"os"
	"strings"
	"time"
)

const (
	baseURL    = "http://localhost:8080/api/v1"
	testEmail  = "simulated_user@example.com"
	testUser   = "simulated_user"
	testPass   = "Password123!"
	deviceID   = "LD6002C_TEST_01"
	deviceName = "Sensor ห้องทดสอบจำลอง"
)

type Client struct {
	httpClient *http.Client
	token      string
	groupID    uint
}

func main() {
	fmt.Println("==================================================")
	fmt.Println("🚨 เครื่องมือจำลองเหตุการณ์เซนเซอร์ตรวจจับการล้ม (Simulator) 🚨")
	fmt.Println("==================================================")

	jar, _ := cookiejar.New(nil)
	client := &Client{
		httpClient: &http.Client{
			Timeout: 10 * time.Second,
			Jar:     jar,
		},
	}

	// 1. ลงทะเบียนและเข้าสู่ระบบเพื่อจำลองผู้ใช้งานจริง
	if err := client.setupUserAndDevice(); err != nil {
		fmt.Printf("❌ เกิดข้อผิดพลาดในการติดตั้งระบบจำลอง: %v\n", err)
		return
	}

	// 2. ลูปเมนูโต้ตอบสำหรับการส่งข้อมูลจำลอง
	reader := bufio.NewReader(os.Stdin)
	for {
		fmt.Println("\n--------------------------------------------------")
		fmt.Println("เลือกเหตุการณ์ที่ต้องการส่งจำลองไปยังเซิร์ฟเวอร์:")
		fmt.Println("1) จำลองผู้สูงอายุเดินเข้าห้อง (Event: enter)")
		fmt.Println("2) จำลองผู้สูงอายุล้มลง! (Event: fall) **[แจ้งเตือน LINE]**")
		fmt.Println("3) จำลองผู้สูงอายุเดินออกจากห้อง (Event: exit)")
		fmt.Println("4) จำลองอุปกรณ์ออนไลน์ (Status: online)")
		fmt.Println("5) จำลองอุปกรณ์ออฟไลน์ (Status: offline)")
		fmt.Println("6) รันสถานการณ์จำลองต่อเนื่อง (Enter -> Fall -> Exit)")
		fmt.Println("0) ออกจากโปรแกรมจำลอง")
		fmt.Print("เลือกข้อ (0-6): ")

		input, err := reader.ReadString('\n')
		if err != nil {
			break
		}
		input = strings.TrimSpace(input)

		if input == "0" {
			fmt.Println("👋 ปิดโปรแกรมจำลองเรียบร้อยแล้ว")
			break
		}

		switch input {
		case "1":
			client.sendEvent("event", "enter", "1")
		case "2":
			client.sendEvent("event", "fall", "1")
		case "3":
			client.sendEvent("event", "exit", "1")
		case "4":
			client.sendEvent("status", "online", "")
		case "5":
			client.sendEvent("status", "offline", "")
		case "6":
			fmt.Println("🎬 เริ่มสถานการณ์จำลองแบบต่อเนื่อง...")
			fmt.Println("⏱️ 1. ส่งสถานะ อุปกรณ์เริ่ม Online...")
			client.sendEvent("status", "online", "")
			time.Sleep(2 * time.Second)

			fmt.Println("⏱️ 2. ส่งเหตุการณ์ มีคนก้าวเข้ามาในพื้นที่ (Enter)...")
			client.sendEvent("event", "enter", "1")
			time.Sleep(3 * time.Second)

			fmt.Println("⏱️ 3. ส่งเหตุการณ์ ตรวจพบการล้มลง (Fall)!...")
			client.sendEvent("event", "fall", "1")
			time.Sleep(4 * time.Second)

			fmt.Println("⏱️ 4. ส่งเหตุการณ์ คนลุกขึ้นและเดินออกไป (Exit)...")
			client.sendEvent("event", "exit", "1")
			fmt.Println("🏁 สิ้นสุดสถานการณ์จำลองแบบต่อเนื่อง")
		default:
			fmt.Println("⚠️ ตัวเลือกไม่ถูกต้อง กรุณาป้อนตัวเลข 0 ถึง 6")
		}
	}
}

// setupUserAndDevice ดำเนินการสมัครสมาชิก เข้าสู่ระบบ สร้างสถานที่ และสร้างอุปกรณ์เพื่อรองรับการจำลอง
func (c *Client) setupUserAndDevice() error {
	fmt.Println("🔄 1. กำลังสร้างบัญชีผู้ใช้ทดสอบ...")
	regPayload := map[string]string{
		"email":    testEmail,
		"username": testUser,
		"password": testPass,
	}
	_, _ = c.postJSON("/register", regPayload, false) // ละเว้นข้อผิดพลาดกรณีอีเมลซ้ำ

	fmt.Println("🔄 2. กำลังเข้าสู่ระบบเพื่อรับสิทธิ์เข้าถึง...")
	loginPayload := map[string]string{
		"email":    testEmail,
		"password": testPass,
	}
	loginResp, err := c.postJSON("/login", loginPayload, false)
	if err != nil {
		return fmt.Errorf("เข้าสู่ระบบไม่สำเร็จ: %w", err)
	}

	var loginResult struct {
		Token string `json:"token"`
	}
	if err := json.Unmarshal(loginResp, &loginResult); err != nil || loginResult.Token == "" {
		return fmt.Errorf("ไม่ได้รับสิทธิ์โทเค็น (Token)")
	}
	c.token = loginResult.Token
	fmt.Println("✅ เข้าสู่ระบบและได้รับสิทธิ์สำเร็จ")

	fmt.Println("🔄 3. กำลังตรวจสอบสถานที่ทดสอบ...")
	groupsData, err := c.getJSON("/iot/groups")
	if err != nil {
		return fmt.Errorf("ดึงข้อมูลสถานที่ล้มเหลว: %w", err)
	}

	var groups []struct {
		ID   uint   `json:"id"`
		Name string `json:"name"`
	}
	_ = json.Unmarshal(groupsData, &groups)

	foundGroup := false
	for _, g := range groups {
		if g.Name == "ห้องทดสอบระบบ" {
			c.groupID = g.ID
			foundGroup = true
			break
		}
	}

	if !foundGroup {
		fmt.Println("🔄 3.1 ไม่พบสถานที่ทดสอบ กำลังสร้างสถานที่ 'ห้องทดสอบระบบ'...")
		grpPayload := map[string]string{"name": "ห้องทดสอบระบบ"}
		_, err = c.postJSON("/iot/groups", grpPayload, true)
		if err != nil {
			return fmt.Errorf("สร้างสถานที่ทดสอบล้มเหลว: %w", err)
		}

		// ดึงข้อมูลกลุ่มอีกครั้งเพื่อเอา ID
		groupsData, _ = c.getJSON("/iot/groups")
		_ = json.Unmarshal(groupsData, &groups)
		for _, g := range groups {
			if g.Name == "ห้องทดสอบระบบ" {
				c.groupID = g.ID
				break
			}
		}
	}
	fmt.Printf("✅ ใช้รหัสสถานที่กลุ่ม ID: %d\n", c.groupID)

	fmt.Println("🔄 4. กำลังลงทะเบียนอุปกรณ์ตรวจจับทดสอบ...")
	devPayload := map[string]interface{}{
		"group_id":  c.groupID,
		"device_id": deviceID,
		"name":      deviceName,
	}
	// ลงทะเบียนอุปกรณ์ (ละเว้นข้อผิดพลาดหากอุปกรณ์ถูกลงทะเบียนไว้แล้ว)
	_, _ = c.postJSON("/iot/devices", devPayload, true)
	fmt.Printf("✅ อุปกรณ์ตรวจจับไอดี: %s พร้อมสำหรับการจำลอง\n", deviceID)

	return nil
}

// sendEvent ส่งข้อมูลเหตุการณ์จำลองไปยังเซิร์ฟเวอร์
func (c *Client) sendEvent(evType, name, val string) {
	fmt.Printf("\n📤 กำลังส่งข้อมูล: [Type=%s, Name=%s, Value=%s] ...\n", evType, name, val)
	payload := map[string]string{
		"device_id": deviceID,
		"type":      evType,
		"name":      name,
		"value":     val,
		"metadata":  "{}",
	}

	resp, err := c.postJSON("/events", payload, false)
	if err != nil {
		fmt.Printf("❌ ส่งข้อมูลล้มเหลว: %v\n", err)
		return
	}

	var res struct {
		Message string `json:"message"`
		Error   string `json:"error"`
	}
	_ = json.Unmarshal(resp, &res)

	if res.Error != "" {
		fmt.Printf("❌ เซิร์ฟเวอร์ตอบกลับข้อผิดพลาด: %s\n", res.Error)
	} else {
		fmt.Printf("✅ ส่งเรียบร้อย: %s\n", res.Message)
	}
}

func (c *Client) postJSON(endpoint string, payload interface{}, useAuth bool) ([]byte, error) {
	data, err := json.Marshal(payload)
	if err != nil {
		return nil, err
	}

	req, err := http.NewRequest("POST", baseURL+endpoint, bytes.NewBuffer(data))
	if err != nil {
		return nil, err
	}

	req.Header.Set("Content-Type", "application/json")
	if useAuth && c.token != "" {
		req.Header.Set("Authorization", "Bearer "+c.token)
	}

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, err
	}

	if resp.StatusCode >= 400 && !strings.Contains(endpoint, "register") && !strings.Contains(endpoint, "devices") {
		return body, fmt.Errorf("HTTP error: %s (Body: %s)", resp.Status, string(body))
	}

	return body, nil
}

func (c *Client) getJSON(endpoint string) ([]byte, error) {
	req, err := http.NewRequest("GET", baseURL+endpoint, nil)
	if err != nil {
		return nil, err
	}

	if c.token != "" {
		req.Header.Set("Authorization", "Bearer "+c.token)
	}

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, err
	}

	if resp.StatusCode >= 400 {
		return nil, fmt.Errorf("HTTP error: %s", resp.Status)
	}

	return body, nil
}
