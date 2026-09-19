/*
 * =========================================================================================
 * 📡 HLK-LD6002C LIVE FALL STATUS, HEIGHT MONITOR & WIRELESS OTA DASHBOARD
 * =========================================================================================
 * 🌟 คุณสมบัติระบบ:
 *    1. ตั้งค่ามาตรฐานเริ่มต้น:
 *       - ความสูงติดตั้งเรดาร์ (Mount Height): 2.20 เมตร
 *       - ขนาดพื้นที่ห้อง (Room Size): กว้าง 1.50 ม. x ยาว 1.50 ม.
 *       - เกณฑ์ความสูงการล้ม (Fall Threshold): 0.50 เมตร
 *    2. 📏 ตรวจวัดและแสดง "ความสูงของเป้าหมายสด (Target Height)" แบบเรียลไทม์:
 *       - ถอดรหัสเฟรม 0x0E0E จากเรดาร์
 *       - แสดงเกจวัดระดับความสูงเทียบกับเส้นเกณฑ์การล้มบนหน้าเว็บแบบสดๆ
 *    3. 🚀 ระบบอัปโหลดออนไลน์แบบไร้สาย (Dual OTA Upload):
 *       - แบบที่ 1 (ArduinoOTA): อัปโหลดตรงจากโปรแกรม Arduino IDE ผ่าน Network Port
 *       - แบบที่ 2 (Web Browser OTA): อัปโหลดไฟล์ .bin ผ่านหน้าเว็บเบราว์เซอร์ได้ทันที!
 *    4. 📶 การเชื่อมต่อ Wi-Fi แบบคู่ (AP + STA):
 *       - ปล่อย Hotspot ในตัว: SSID "LD6002C-DEBUG" (รหัสผ่าน 12345678) -> IP 192.168.4.1
 *       - เชื่อมต่อ Wi-Fi บ้านอัตโนมัติ (PINKU 8541) เพื่อความสะดวกในการอัปโหลด
 *    5. 🔀 เลือกระบบตรวจวัดได้: UART (TX0/RX0), Digital Pin (TX2 ต่อ GPIO 3), หรือ Dual View
 * 
 * การต่อสายฮาร์ดแวร์ (ESP32-S3):
 *  - ESP32-S3 RX (GPIO 18) -> ขา TX0 ของ LD6002C (UART Data)
 *  - ESP32-S3 TX (GPIO 17) -> ขา RX0 ของ LD6002C (UART Config)
 *  - ESP32-S3 GPIO 3        -> ขา TX2 ของ LD6002C (Hardware Fall Pin: 0=ปกติ, 1=ล้ม)
 *  - ESP32-S3 GPIO 15       -> ขา Buzzer (Active LOW)
 *  - ESP32-S3 GPIO 48       -> ไฟ RGB NeoPixel บนบอร์ด
 *  - GND ทั้งหมด            -> ต่อร่วมกัน (Common GND)
 * =========================================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <Update.h>

// ================= กำหนดขาใช้งาน =================
#define RADAR_RX_PIN       18     // UART RX (ต่อกับ TX0 ของ LD6002C)
#define RADAR_TX_PIN       17     // UART TX (ต่อกับ RX0 ของ LD6002C)
#define RADAR_DIGITAL_PIN  3      // Digital In (ต่อกับ TX2 ของ LD6002C)
#define BUZZER_PIN         15     // Buzzer (Active LOW)
#define RGB_LED_PIN        48     // ไฟ RGB NeoPixel บนบอร์ด
#define RGB_BRIGHTNESS     0.25   // ความสว่างไฟ RGB
#define RADAR_BAUD         115200 // ความเร็วบอดเรต

// ================= การตั้งค่าเริ่มต้น =================
float mount_height   = 2.20; // ความสูงติดตั้ง (เมตร)
float room_width     = 1.50; // ความกว้างห้อง (เมตร)
float room_length    = 1.50; // ความยาวห้อง (เมตร)
float fall_threshold = 0.50; // เกณฑ์ความสูงการล้ม (เมตร)
const unsigned long FALL_CONFIRM_MS = 2000; // หน่วง 2 วินาทียืนยันล้มจริง

// การตั้งค่า Wi-Fi (ทั้งแบบ Hotspot และเชื่อมต่อ Router)
const char* AP_SSID       = "LD6002C-DEBUG";
const char* AP_PASS       = "12345678";
const char* STA_SSID      = "PINKU 8541";       // Wi-Fi บ้าน/มือถือ
const char* STA_PASS      = "468Bt9@2";

// ฮาร์ดแวร์พอร์ตและเว็บเซิร์ฟเวอร์
HardwareSerial RadarSerial(1);
WebServer server(80);

// ================= ตัวแปรสถานะระบบ =================
// 1. สถานะ UART & ความสูง
bool uart_is_fall            = false;
float raw_radar_dist         = 0.0f; // ระยะจากเพดาน (เมตร) เช่น ยืน=0.64, นั่ง=1.14, นอน=1.98
float person_height          = 0.0f; // ความสูงจริงของคนจากพื้น = mount_height - raw_radar_dist
unsigned long last_height_ms = 0;
uint16_t last_frame_id       = 0;
unsigned long total_packets  = 0;
unsigned long packets_last_sec = 0;
float current_packet_hz      = 0.0;
unsigned long last_hz_calc   = 0;

// 2. สถานะ Digital Pin (TX2 / GPIO 3)
int digital_pin_state        = LOW;
unsigned long digital_trigger_count = 0;

// 3. สถานะการตัดสินใจและแจ้งเตือน
bool is_falling_candidate    = false;
bool fall_alerted            = false;
unsigned long fall_start_time = 0;

// ตรวจจับการล้มจากระดับความสูงจริง (Height-Based Fall Trigger)
bool height_falling_candidate = false;
bool height_fall_alerted     = false;
unsigned long height_fall_start_time = 0;

bool manual_buzzer_override  = false;

// 4. บันทึกประวัติ Log (Ring Buffer)
#define MAX_LOGS 35
String log_buffer[MAX_LOGS];
int log_head = 0;

void addLog(const String& msg) {
    Serial.println(msg);
    unsigned long s = millis() / 1000;
    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "[%02lu:%02lu] ", (s / 60) % 60, s % 60);
    log_buffer[log_head] = String(timeStr) + msg;
    log_head = (log_head + 1) % MAX_LOGS;
}

// คำนวณ Checksum (~XOR)
uint8_t calculateChecksum(const uint8_t* data, size_t len) {
    uint8_t ret = 0;
    for (size_t i = 0; i < len; ++i) ret ^= data[i];
    return (uint8_t)(~ret);
}

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
    neopixelWrite(RGB_LED_PIN, 
                  (uint8_t)(r * RGB_BRIGHTNESS), 
                  (uint8_t)(g * RGB_BRIGHTNESS), 
                  (uint8_t)(b * RGB_BRIGHTNESS));
}

void setBuzzer(bool active) {
    if (manual_buzzer_override) {
        digitalWrite(BUZZER_PIN, LOW);
    } else {
        digitalWrite(BUZZER_PIN, active ? LOW : HIGH);
    }
}

// ================= ฟังก์ชันส่งคำสั่งคอนฟิกเข้าเรดาร์ =================
void sendRadarFloat(uint16_t type, float value) {
    uint8_t frame[13];
    frame[0] = 0x01; // SOF
    frame[1] = 0x00; frame[2] = 0x00;
    frame[3] = 0x00; frame[4] = 0x04;
    frame[5] = (type >> 8) & 0xFF;
    frame[6] = type & 0xFF;
    frame[7] = calculateChecksum(frame, 7);

    uint8_t *valPtr = (uint8_t *)&value;
    frame[8]  = valPtr[0]; frame[9]  = valPtr[1];
    frame[10] = valPtr[2]; frame[11] = valPtr[3];
    frame[12] = calculateChecksum(&frame[8], 4);

    RadarSerial.write(frame, 13);
}

void sendRadarRoomSize(float width, float length) {
    float rect_XL = -width; float rect_XR = width;
    float rect_ZF = length; float rect_ZB = -length;

    uint8_t frame[25];
    frame[0] = 0x01;
    frame[1] = 0x00; frame[2] = 0x00;
    frame[3] = 0x00; frame[4] = 0x10;
    frame[5] = 0x0E; frame[6] = 0x0C;
    frame[7] = calculateChecksum(frame, 7);

    memcpy(&frame[8],  &rect_XL, 4);
    memcpy(&frame[12], &rect_XR, 4);
    memcpy(&frame[16], &rect_ZF, 4);
    memcpy(&frame[20], &rect_ZB, 4);
    frame[24] = calculateChecksum(&frame[8], 16);

    RadarSerial.write(frame, 25);
}

void applyRadarConfiguration() {
    addLog("⚙️ กำลังส่งพารามิเตอร์เข้าเรดาร์...");
    sendRadarFloat(0x0E04, mount_height);   // ความสูงติดตั้ง
    delay(100);
    sendRadarFloat(0x0E08, fall_threshold); // เกณฑ์ตัดสินล้ม
    delay(100);
    sendRadarRoomSize(room_width, room_length); // ขนาดห้อง
    delay(100);
    addLog("✅ บันทึกคอนฟิกเรดาร์สำเร็จ: สูง " + String(mount_height, 2) + "m | ขนาด " + String(room_width, 2) + "x" + String(room_length, 2) + "m");
}

// ================= HTML WEB INTERFACE =================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="th">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>HLK-LD6002C Fall & Height Dashboard</title>
  <style>
    :root {
      --bg: #0b1329; --card: #17233f; --border: #2c3e66;
      --text: #f1f5f9; --muted: #94a3b8; --green: #10b981;
      --orange: #f59e0b; --red: #ef4444; --blue: #38bdf8; --purple: #a855f7;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    body { background: var(--bg); color: var(--text); padding: 14px; }
    .container { max-width: 960px; margin: 0 auto; display: flex; flex-direction: column; gap: 14px; }
    
    .header { display: flex; justify-content: space-between; align-items: center; background: var(--card); padding: 14px 18px; border-radius: 12px; border: 1px solid var(--border); }
    .header h1 { font-size: 1.15rem; display: flex; align-items: center; gap: 8px; }
    .badge { padding: 4px 10px; border-radius: 9999px; font-size: 0.8rem; font-weight: 600; }
    .badge-online { background: rgba(16, 185, 129, 0.2); color: var(--green); border: 1px solid var(--green); }
    .badge-ota { background: rgba(168, 85, 247, 0.2); color: var(--purple); border: 1px solid var(--purple); cursor: pointer; text-decoration: none; }

    /* Alert Banner */
    .status-banner { padding: 18px; border-radius: 12px; text-align: center; font-size: 1.4rem; font-weight: bold; transition: all 0.3s; border: 2px solid transparent; }
    .status-normal { background: rgba(16, 185, 129, 0.15); color: var(--green); border-color: var(--green); }
    .status-candidate { background: rgba(245, 158, 11, 0.2); color: var(--orange); border-color: var(--orange); animation: pulse 1s infinite alternate; }
    .status-fall { background: rgba(239, 68, 68, 0.3); color: var(--red); border-color: var(--red); animation: blink 0.5s infinite alternate; }

    @keyframes pulse { from { opacity: 0.7; } to { opacity: 1; } }
    @keyframes blink { from { background: rgba(239, 68, 68, 0.6); } to { background: rgba(239, 68, 68, 0.2); } }

    /* Channel Selector Tabs */
    .tabs { display: grid; grid-template-columns: repeat(3, 1fr); gap: 6px; background: #070d1e; padding: 4px; border-radius: 10px; }
    .tab-btn { background: transparent; border: none; color: var(--muted); padding: 10px; font-size: 0.9rem; font-weight: 600; border-radius: 8px; cursor: pointer; transition: 0.2s; }
    .tab-btn.active { background: var(--card); color: var(--blue); border: 1px solid var(--border); box-shadow: 0 4px 6px rgba(0,0,0,0.3); }

    .grid-2 { display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 14px; }
    .card { background: var(--card); padding: 16px; border-radius: 12px; border: 1px solid var(--border); }
    .card-title { font-size: 0.95rem; color: var(--muted); margin-bottom: 10px; display: flex; align-items: center; justify-content: space-between; }

    .metric-value { font-size: 2.3rem; font-weight: bold; margin: 4px 0; }
    .metric-sub { font-size: 0.85rem; color: var(--muted); line-height: 1.5; }

    /* Height Visual Gauge */
    .gauge-box { margin-top: 12px; }
    .gauge-bar-bg { width: 100%; height: 18px; background: #070d1e; border-radius: 9px; overflow: hidden; position: relative; border: 1px solid var(--border); }
    .gauge-bar-fill { height: 100%; width: 0%; background: linear-gradient(90deg, var(--red) 0%, var(--orange) 30%, var(--green) 70%); transition: width 0.3s; border-radius: 9px; }
    .gauge-threshold-mark { position: absolute; top: 0; bottom: 0; width: 2px; background: #fff; z-index: 2; }
    .gauge-labels { display: flex; justify-content: space-between; font-size: 0.75rem; color: var(--muted); margin-top: 4px; }

    /* Forms & Buttons */
    .form-group { margin-bottom: 10px; }
    .form-group label { display: block; font-size: 0.8rem; color: var(--muted); margin-bottom: 3px; }
    .form-control { width: 100%; padding: 8px 12px; border-radius: 8px; border: 1px solid var(--border); background: #070d1e; color: #fff; font-size: 0.95rem; }
    .btn { width: 100%; padding: 10px; border: none; border-radius: 8px; font-size: 0.95rem; font-weight: bold; cursor: pointer; transition: 0.2s; }
    .btn-primary { background: #2563eb; color: #fff; }
    .btn-primary:hover { background: #1d4ed8; }
    .btn-purple { background: #9333ea; color: #fff; }

    /* Terminal Console */
    .console { background: #070d1e; border: 1px solid var(--border); border-radius: 8px; padding: 10px; height: 160px; overflow-y: auto; font-family: monospace; font-size: 0.82rem; color: #38bdf8; display: flex; flex-direction: column-reverse; }
    .log-line { margin-bottom: 3px; line-height: 1.35; border-bottom: 1px solid #1e293b; padding-bottom: 2px; }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>📡 HLK-LD6002C Fall & Height Monitor</h1>
      <div style="display:flex; gap:8px; align-items:center;">
        <span id="wifiBadge" class="badge badge-online">● Wi-Fi: PINKU 8541</span>
        <a href="#otaSection" class="badge badge-ota">🚀 อัปโหลดไร้สาย (OTA)</a>
      </div>
    </div>

    <!-- Live Overall Alert Banner -->
    <div id="statusBanner" class="status-banner status-normal">
      🟢 [สถานะปกติ] ไม่พบการล้ม
    </div>

    <!-- Source Selector Tabs -->
    <div class="tabs">
      <button id="tabDual" class="tab-btn active" onclick="setMode('dual')">🔀 Dual View (UART + Digital)</button>
      <button id="tabUart" class="tab-btn" onclick="setMode('uart')">📡 UART (TX0 / RX0)</button>
      <button id="tabDigital" class="tab-btn" onclick="setMode('digital')">⚡ Digital (TX2 / GPIO 3)</button>
    </div>

    <!-- Top Metric: Live Target Height (ความสูงจริงของคนจากพื้น) -->
    <div class="card" style="border-left: 4px solid var(--blue);">
      <div class="card-title">
        <span>📏 ความสูงจริงของคนจากพื้น (คำนวณจากเพดาน 2.20 ม.)</span>
        <span id="heightStatusText" class="badge" style="background:#0369a1; color:#e0f2fe;">ตรวจจับปกติ</span>
      </div>
      <div style="display: flex; align-items: baseline; gap: 10px;">
        <div class="metric-value" id="targetHeightVal" style="color:var(--blue)">-- m</div>
        <span class="metric-sub" id="heightCategory">กำลังรอข้อมูลความสูงจากเรดาร์...</span>
      </div>
      
      <!-- Visual Gauge Bar -->
      <div class="gauge-box">
        <div class="gauge-bar-bg">
          <div id="gaugeBar" class="gauge-bar-fill"></div>
          <div id="gaugeThreshold" class="gauge-threshold-mark" style="left: 22%;"></div>
        </div>
        <div class="gauge-labels">
          <span>0.0m (พื้นห้อง)</span>
          <span style="color:var(--orange)">▲ เกณฑ์ล้ม (<span id="lblThreshold">0.50</span>m)</span>
          <span>ความสูงติดตั้ง (<span id="lblMount">2.20</span>m)</span>
        </div>
      </div>
    </div>

    <!-- Dual / Conditional View Container -->
    <div class="grid-2">
      <!-- Card UART -->
      <div id="cardUart" class="card">
        <div class="card-title">
          <span>📡 สถานะ UART (TX0 / RX0)</span>
          <span id="uartHz" class="badge" style="background:#1e3a8a; color:#60a5fa;">0.0 Hz</span>
        </div>
        <div class="metric-value" id="uartStatus" style="color:var(--green)">0 (ปกติ)</div>
        <div class="metric-sub">
          เฟรมล่าสุด: <b id="uartFrameId">#0000</b> | Type: <b>0x0E02</b><br>
          จำนวนแพ็กเก็ต: <span id="uartTotalPackets">0</span>
        </div>
      </div>

      <!-- Card Digital Pin -->
      <div id="cardDigital" class="card">
        <div class="card-title">
          <span>⚡ สัญญาณขาดิจิตอล (TX2 / GPIO 3)</span>
          <span id="digitalTriggers" class="badge" style="background:#374151; color:#d1d5db;">0 Triggers</span>
        </div>
        <div class="metric-value" id="digitalStatus" style="color:var(--green)">LOW (0V)</div>
        <div class="metric-sub">
          สถานะลอจิก: <b id="digitalLogic">0</b> (ปกติ)<br>
          สายต่อ: <b>GPIO 3</b> ต่อเข้า <b>ขา TX2 เรดาร์</b>
        </div>
      </div>
    </div>

    <!-- Parameters & Settings Grid -->
    <div class="grid-2">
      <!-- Radar Configuration Form -->
      <div class="card">
        <div class="card-title">⚙️ กำหนดค่าเรดาร์ (บันทึกเข้าเซนเซอร์)</div>
        <div class="grid-2" style="gap:8px;">
          <div class="form-group">
            <label>ความสูงติดตั้ง (ม.):</label>
            <input type="number" step="0.1" id="cfgHeight" class="form-control" value="2.2">
          </div>
          <div class="form-group">
            <label>เกณฑ์ตัดสินล้ม (ม.):</label>
            <input type="number" step="0.05" id="cfgThreshold" class="form-control" value="0.5">
          </div>
        </div>
        <div class="grid-2" style="gap:8px;">
          <div class="form-group">
            <label>ความกว้างห้อง (ม.):</label>
            <input type="number" step="0.1" id="cfgWidth" class="form-control" value="1.5">
          </div>
          <div class="form-group">
            <label>ความยาวห้อง (ม.):</label>
            <input type="number" step="0.1" id="cfgLength" class="form-control" value="1.5">
          </div>
        </div>
        <button class="btn btn-primary" onclick="applyConfig()">🚀 ส่งค่าคอนฟิกเข้าเรดาร์</button>
      </div>

      <!-- Live Log & Hardware Test -->
      <div class="card">
        <div class="card-title">
          <span>📜 Live Console Log</span>
          <button onclick="toggleBuzzer()" class="badge" style="background:#e11d48; color:#fff; border:none; cursor:pointer; padding:4px 10px;">🔊 สลับเสียง Buzzer</button>
        </div>
        <div id="consoleLog" class="console">
          <div class="log-line">กำลังรอข้อมูลจาก ESP32-S3...</div>
        </div>
      </div>
    </div>

    <!-- Wireless OTA Firmware Update Form (อัปโหลดออนไลน์ผ่านหน้าเว็บ) -->
    <div id="otaSection" class="card" style="border: 1px solid var(--purple);">
      <div class="card-title" style="color:var(--purple); font-weight:bold;">
        <span>🚀 อัปโหลดโค้ดออนไลน์ไร้สาย (Web Browser OTA Firmware Update)</span>
        <span class="badge badge-ota">ไม่ต้องเสียบสาย USB</span>
      </div>
      <p style="font-size:0.85rem; color:var(--muted); margin-bottom:10px;">
        คุณสามารถคอมไพล์โค้ดใน Arduino IDE (เมนู <b>Sketch ➔ Export Compiled Binary</b> หรือกด <b>Ctrl+Alt+S</b>) แล้วนำไฟล์ <code>.bin</code> มาเลือกและกดอัปโหลดตรงนี้ได้ทันที:
      </p>
      <form method="POST" action="/update" enctype="multipart/form-data" id="upload_form">
        <div style="display:flex; gap:10px; flex-wrap:wrap; align-items:center;">
          <input type="file" name="update" id="file_input" accept=".bin" class="form-control" style="flex:1;">
          <input type="submit" value="⚡ เริ่มอัปโหลดออนไลน์" class="btn btn-purple" style="width:auto; padding:10px 20px;">
        </div>
      </form>
      <div id="otaProgress" style="display:none; margin-top:10px;">
        <div style="background:#070d1e; height:12px; border-radius:6px; overflow:hidden; border:1px solid var(--border);">
          <div id="otaProgressBar" style="height:100%; width:0%; background:var(--purple); transition:width 0.2s;"></div>
        </div>
        <span id="otaProgressText" style="font-size:0.8rem; color:var(--muted); margin-top:4px; display:inline-block;">กำลังส่งไฟล์เฟิร์มแวร์...</span>
      </div>
    </div>
  </div>

  <script>
    let currentMode = 'dual';

    function setMode(m) {
      currentMode = m;
      document.getElementById('tabDual').classList.toggle('active', m === 'dual');
      document.getElementById('tabUart').classList.toggle('active', m === 'uart');
      document.getElementById('tabDigital').classList.toggle('active', m === 'digital');

      const cardUart = document.getElementById('cardUart');
      const cardDigital = document.getElementById('cardDigital');

      if (m === 'dual') {
        cardUart.style.display = 'block';
        cardDigital.style.display = 'block';
      } else if (m === 'uart') {
        cardUart.style.display = 'block';
        cardDigital.style.display = 'none';
      } else if (m === 'digital') {
        cardUart.style.display = 'none';
        cardDigital.style.display = 'block';
      }
    }

    async function fetchStatus() {
      try {
        const res = await fetch('/api/status');
        const data = await res.json();

        // 0. Update Wi-Fi Badge
        const wifiBadge = document.getElementById('wifiBadge');
        if (data.sta_connected && data.sta_ip) {
          wifiBadge.className = 'badge badge-online';
          wifiBadge.textContent = '● Wi-Fi: PINKU 8541 (' + data.sta_ip + ')';
        } else {
          wifiBadge.className = 'badge';
          wifiBadge.style = 'background: rgba(245, 158, 11, 0.2); color: var(--orange); border: 1px solid var(--orange);';
          wifiBadge.textContent = '● Hotspot: 192.168.4.1';
        }

        // 1. Overall Status Banner
        const banner = document.getElementById('statusBanner');
        if (data.alert) {
          banner.className = 'status-banner status-fall';
          banner.innerHTML = '🚨🚨🚨 [ALERT CONFIRMED] ยืนยันคนล้มจริง! (ไซเรนดัง)';
        } else if (data.candidate) {
          banner.className = 'status-banner status-candidate';
          banner.innerHTML = '⚠️ [เฝ้าระวัง] ตรวจพบการล้ม กำลังนับถอยหลังยืนยัน 2 วินาที...';
        } else {
          banner.className = 'status-banner status-normal';
          banner.innerHTML = '🟢 [สถานะปกติ] สภาวะยืน / นั่ง (ไม่พบการล้ม)';
        }

        // 2. Target Height Metric & Visual Gauge
        const pH = data.person_height;
        const rawD = data.raw_distance;
        const hEl = document.getElementById('targetHeightVal');
        const hCat = document.getElementById('heightCategory');
        const hStatus = document.getElementById('heightStatusText');
        const gaugeFill = document.getElementById('gaugeBar');

        if (rawD > 0.1) {
          hEl.textContent = pH.toFixed(2) + ' ม.';
          const maxH = data.mount_height || 2.2;
          const pct = Math.min(Math.max((pH / maxH) * 100, 0), 100);
          gaugeFill.style.width = pct + '%';

          if (pH <= data.fall_threshold && pH > 0.05) {
            hEl.style.color = 'var(--red)';
            hCat.innerHTML = `🔴 <b>คนนอนราบกับพื้น! (ระยะจากเพดาน: ${rawD.toFixed(2)}m)</b>`;
            hStatus.textContent = '🚨 ล้มราบกับพื้น';
            hStatus.style.background = '#991b1b';
          } else if (pH <= 1.20 && pH > 0.45) {
            hEl.style.color = 'var(--orange)';
            hCat.innerHTML = `🟠 <b>คนนั่งเก้าอี้ / ชักโครก (ระยะจากเพดาน: ${rawD.toFixed(2)}m)</b>`;
            hStatus.textContent = 'นั่งปกติ';
            hStatus.style.background = '#9a3412';
          } else {
            hEl.style.color = 'var(--green)';
            hCat.innerHTML = `🟢 <b>คนยืนปกติ (ระยะจากเพดาน: ${rawD.toFixed(2)}m)</b>`;
            hStatus.textContent = 'ยืนปกติ';
            hStatus.style.background = '#065f46';
          }
        } else {
          hEl.textContent = '-- ม.';
          hCat.textContent = 'กำลังรอข้อมูลความสูงจากเรดาร์...';
          gaugeFill.style.width = '0%';
        }

        // Threshold indicator mark
        if (data.mount_height > 0) {
          const threshPct = (data.fall_threshold / data.mount_height) * 100;
          document.getElementById('gaugeThreshold').style.left = threshPct + '%';
          document.getElementById('lblThreshold').textContent = data.fall_threshold.toFixed(2);
          document.getElementById('lblMount').textContent = data.mount_height.toFixed(2);
        }

        // 3. UART Metrics
        const uartEl = document.getElementById('uartStatus');
        if (data.uart_fall === 1) {
          uartEl.textContent = '1 (ตรวจพบล้ม!)';
          uartEl.style.color = 'var(--red)';
        } else {
          uartEl.textContent = '0 (ปกติ)';
          uartEl.style.color = 'var(--green)';
        }
        document.getElementById('uartFrameId').textContent = '#' + data.frame_id;
        document.getElementById('uartHz').textContent = data.hz.toFixed(1) + ' Hz';
        document.getElementById('uartTotalPackets').textContent = data.total_packets;

        // 4. Digital Pin Metrics
        const digEl = document.getElementById('digitalStatus');
        const digLogic = document.getElementById('digitalLogic');
        if (data.digital === 1) {
          digEl.textContent = 'HIGH (3.3V)';
          digEl.style.color = 'var(--red)';
          digLogic.textContent = '1 (ล้ม)';
        } else {
          digEl.textContent = 'LOW (0V)';
          digEl.style.color = 'var(--green)';
          digLogic.textContent = '0 (ปกติ)';
        }
        document.getElementById('digitalTriggers').textContent = data.digital_triggers + ' Triggers';

        // 5. Update Logs
        if (data.logs && data.logs.length > 0) {
          const c = document.getElementById('consoleLog');
          c.innerHTML = data.logs.map(l => `<div class="log-line">${l}</div>`).join('');
        }
      } catch (err) {
        console.error('Fetch error:', err);
      }
    }

    async function applyConfig() {
      const h = document.getElementById('cfgHeight').value;
      const t = document.getElementById('cfgThreshold').value;
      const w = document.getElementById('cfgWidth').value;
      const l = document.getElementById('cfgLength').value;

      try {
        const res = await fetch(`/api/config?height=${h}&threshold=${t}&width=${w}&length=${l}`, { method: 'POST' });
        const resText = await res.text();
        alert(resText);
      } catch(e) {
        alert('เกิดข้อผิดพลาดในการส่งคอนฟิก');
      }
    }

    async function toggleBuzzer() {
      await fetch('/api/buzzer/toggle', { method: 'POST' });
    }

    // จัดการอัปโหลดไฟล์ OTA ผ่าน AJAX Form
    const uploadForm = document.getElementById('upload_form');
    uploadForm.addEventListener('submit', function(e) {
      e.preventDefault();
      const fileInput = document.getElementById('file_input');
      if (!fileInput.files.length) {
        alert('กรุณาเลือกไฟล์ .bin ก่อนครับ');
        return;
      }
      const file = fileInput.files[0];
      const formData = new FormData();
      formData.append('update', file);

      const xhr = new XMLHttpRequest();
      document.getElementById('otaProgress').style.display = 'block';
      const bar = document.getElementById('otaProgressBar');
      const txt = document.getElementById('otaProgressText');

      xhr.upload.addEventListener('progress', function(p) {
        if (p.lengthComputable) {
          const percent = Math.round((p.loaded / p.total) * 100);
          bar.style.width = percent + '%';
          txt.textContent = 'กำลังอัปโหลดไร้สาย: ' + percent + '%';
        }
      });

      xhr.onload = function() {
        if (xhr.status === 200) {
          bar.style.background = 'var(--green)';
          txt.textContent = '🎉 อัปโหลดสำเร็จ 100%! บอร์ด ESP32-S3 กำลังรีบูตอัตโนมัติใน 5 วินาที...';
          setTimeout(() => location.reload(), 6000);
        } else {
          bar.style.background = 'var(--red)';
          txt.textContent = '❌ อัปโหลดล้มเหลว: ' + xhr.responseText;
        }
      };

      xhr.open('POST', '/update');
      xhr.send(formData);
    });

    setInterval(fetchStatus, 350);
  </script>
</body>
</html>
)rawliteral";

// ================= API & OTA HANDLERS =================
void handleRoot() {
    server.send(200, "text/html", INDEX_HTML);
}

void handleStatusApi() {
    char frameHex[8];
    snprintf(frameHex, sizeof(frameHex), "%04X", last_frame_id);

    bool overall_alert = fall_alerted || height_fall_alerted;
    bool overall_candidate = is_falling_candidate || height_falling_candidate;

    String json = "{";
    json += "\"alert\":" + String(overall_alert ? "true" : "false") + ",";
    json += "\"candidate\":" + String(overall_candidate ? "true" : "false") + ",";
    json += "\"uart_fall\":" + String(uart_is_fall ? 1 : 0) + ",";
    json += "\"height_alert\":" + String(height_fall_alerted ? "true" : "false") + ",";
    json += "\"person_height\":" + String(person_height, 2) + ",";
    json += "\"raw_distance\":" + String(raw_radar_dist, 2) + ",";
    json += "\"frame_id\":\"" + String(frameHex) + "\",";
    json += "\"hz\":" + String(current_packet_hz, 1) + ",";
    json += "\"total_packets\":" + String(total_packets) + ",";
    json += "\"digital\":" + String(digital_pin_state) + ",";
    json += "\"digital_triggers\":" + String(digital_trigger_count) + ",";
    json += "\"mount_height\":" + String(mount_height, 2) + ",";
    json += "\"room_width\":" + String(room_width, 2) + ",";
    json += "\"room_length\":" + String(room_length, 2) + ",";
    json += "\"fall_threshold\":" + String(fall_threshold, 2) + ",";
    json += "\"sta_connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
    json += "\"sta_ip\":\"" + (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "") + "\",";

    json += "\"logs\":[";
    bool first = true;
    for (int i = 0; i < MAX_LOGS; i++) {
        int idx = (log_head - 1 - i + MAX_LOGS) % MAX_LOGS;
        if (log_buffer[idx].length() > 0) {
            if (!first) json += ",";
            String escaped = log_buffer[idx];
            escaped.replace("\"", "\\\"");
            json += "\"" + escaped + "\"";
            first = false;
        }
    }
    json += "]}";

    server.send(200, "application/json", json);
}

void handleConfigApi() {
    if (server.hasArg("height"))   mount_height   = server.arg("height").toFloat();
    if (server.hasArg("threshold")) fall_threshold = server.arg("threshold").toFloat();
    if (server.hasArg("width"))     room_width     = server.arg("width").toFloat();
    if (server.hasArg("length"))    room_length    = server.arg("length").toFloat();

    applyRadarConfiguration();
    server.send(200, "text/plain", "ส่งพารามิเตอร์เข้าเรดาร์เรียบร้อยแล้ว!");
}

void handleBuzzerToggle() {
    manual_buzzer_override = !manual_buzzer_override;
    setBuzzer(false);
    addLog(manual_buzzer_override ? "🔊 สั่งเปิด Buzzer ด้วยตัวเอง" : "🔇 สั่งปิด Buzzer กลับสู่โหมดอัตโนมัติ");
    server.send(200, "text/plain", "OK");
}

// ================= SETUP =================
void setup() {
    Serial.begin(115200);
    delay(1000);

    // 1. ฮาร์ดแวร์ I/O
    pinMode(RADAR_DIGITAL_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, HIGH); // ปิด Buzzer เริ่มต้น
    setRGB(0, 150, 0); // สีเขียวเริ่มต้น

    addLog("=================================================");
    addLog("🚀 ระบบมอนิเตอร์เรดาร์ HLK-LD6002C + Web OTA");
    addLog("=================================================");

    // 2. เริ่มต้น UART เรดาร์
    pinMode(RADAR_RX_PIN, INPUT_PULLUP);
    RadarSerial.setRxBufferSize(1024);
    RadarSerial.begin(RADAR_BAUD, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);
    RadarSerial.setTimeout(50);
    addLog("✅ พอร์ตเรดาร์เชื่อมต่อสำเร็จ (RX=18, TX=17, Digital=3)");

    // 3. เริ่มต้น Wi-Fi แบบคู่ (AP + STA)
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASS);
    IPAddress apIP = WiFi.softAPIP();
    addLog("📶 ปล่อย Wi-Fi Hotspot: " + String(AP_SSID) + " (IP: " + apIP.toString() + ")");

    // เชื่อมต่อไปยัง Wi-Fi คอม / Hotspot (PINKU 8541)
    WiFi.begin(STA_SSID, STA_PASS);
    Serial.printf("📡 กำลังเชื่อมต่อ Wi-Fi: %s ", STA_SSID);
    unsigned long startConn = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startConn < 12000) {
        delay(300);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        String staIP = WiFi.localIP().toString();
        addLog("🌐 เชื่อมต่อ Wi-Fi คอมสำเร็จ! IP: " + staIP);
        Serial.println("=================================================");
        Serial.printf("👉 เปิดดูหน้าเว็บจากคอมได้ที่: http://%s\n", staIP.c_str());
        Serial.println("=================================================");
    } else {
        addLog("⚠️ เชื่อมต่อ Wi-Fi " + String(STA_SSID) + " ไม่สำเร็จ (ยังเข้าผ่าน 192.168.4.1 ได้)");
    }

    // 4. mDNS (http://ld6002c.local)
    if (MDNS.begin("ld6002c")) {
        addLog("🌐 หรือเข้าผ่านชื่อโดเมน: http://ld6002c.local");
    }

    // 5. ติดตั้งระบบ ArduinoOTA สำหรับอัปโหลดไร้สายจาก Arduino IDE
    ArduinoOTA.setHostname("ld6002c-debug");
    ArduinoOTA.onStart([]() {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd OTA");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
    });
    ArduinoOTA.begin();
    addLog("🚀 พร้อมรับการอัปโหลดไร้สาย (ArduinoOTA Hostname: ld6002c-debug)");

    // 6. ผูก Route เว็บเซิร์ฟเวอร์ + ตัวรับไฟล์อัปโหลด .bin (Web Browser OTA)
    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/status", HTTP_GET, handleStatusApi);
    server.on("/api/config", HTTP_POST, handleConfigApi);
    server.on("/api/buzzer/toggle", HTTP_POST, handleBuzzerToggle);

    // Web Browser OTA Handler
    server.on("/update", HTTP_POST, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        delay(500);
        ESP.restart();
    }, []() {
        HTTPUpload& upload = server.upload();
        if (upload.status == UPLOAD_FILE_START) {
            Serial.printf("Update File: %s\n", upload.filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) {
                Serial.printf("Update Success: %u bytes\nRebooting...\n", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
        }
    });

    server.begin();

    // 7. ส่งคอนฟิกเริ่มต้นเข้าเรดาร์
    addLog("⏳ รอเรดาร์บูต 2 วินาที ก่อนส่งค่าเริ่มต้น...");
    delay(2000);
    applyRadarConfiguration();
}

// ================= MAIN LOOP =================
void loop() {
    unsigned long now = millis();
    server.handleClient();
    ArduinoOTA.handle(); // รองรับการอัปโหลดไร้สายตลอดเวลา

    // คำนวณความถี่ข้อมูล (Hz)
    if (now - last_hz_calc >= 1000) {
        current_packet_hz = (float)packets_last_sec;
        packets_last_sec = 0;
        last_hz_calc = now;
    }

    // -------------------------------------------------------------
    // 1. อ่านสัญญาณขาดิจิตอลตรงจากเรดาร์ (ขา TX2 / GPIO 3)
    // -------------------------------------------------------------
    static int prev_dig_state = -1;
    digital_pin_state = digitalRead(RADAR_DIGITAL_PIN);

    if (digital_pin_state != prev_dig_state) {
        prev_dig_state = digital_pin_state;
        if (digital_pin_state == HIGH) {
            digital_trigger_count++;
            addLog("⚡ [TX2 DIGITAL] เรดาร์ส่งสัญญาณ HIGH (3.3V) -> ตรวจพบล้ม!");
        } else {
            addLog("⚡ [TX2 DIGITAL] เรดาร์กลับสู่ LOW (0V) -> ปกติ");
        }
    }

    // -------------------------------------------------------------
    // 2. อ่านและถอดรหัสแพ็กเก็ต UART (TX0 / RX0)
    // -------------------------------------------------------------
    while (RadarSerial.available() > 0) {
        if (RadarSerial.peek() != 0x01) {
            RadarSerial.read();
            continue;
        }

        if (RadarSerial.available() < 8) break;

        uint8_t header[8];
        RadarSerial.readBytes(header, 8);

        uint16_t frameId  = (header[1] << 8) | header[2];
        uint16_t dataLen  = (header[3] << 8) | header[4];
        uint16_t msgType  = (header[5] << 8) | header[6];
        uint8_t  rxHeadCk = header[7];

        if (calculateChecksum(header, 7) != rxHeadCk) continue;

        if (dataLen > 0 && dataLen <= 64) {
            unsigned long waitStart = millis();
            while (RadarSerial.available() < (dataLen + 1) && (millis() - waitStart < 20)) {
                delayMicroseconds(50);
            }
            if (RadarSerial.available() < (dataLen + 1)) break;

            uint8_t payload[64];
            RadarSerial.readBytes(payload, dataLen);
            uint8_t rxDataCk = RadarSerial.read();

            if (calculateChecksum(payload, dataLen) != rxDataCk) continue;

            // บันทึกสถิติ
            total_packets++;
            packets_last_sec++;
            last_frame_id = frameId;

            // ก. เฟรมตรวจจับการล้ม (Type 0x0E02)
            if (msgType == 0x0E02) {
                uint8_t is_fall = payload[0];
                uart_is_fall = (is_fall == 0x01);

                if (uart_is_fall) {
                    if (!fall_alerted) {
                        if (!is_falling_candidate) {
                            is_falling_candidate = true;
                            fall_start_time = now;
                            setRGB(255, 120, 0); // สีส้ม
                            addLog("⚠️ [UART 0x0E02] ตรวจพบแนวโน้มการล้ม! เริ่มจับเวลา 2s...");
                        } else if (now - fall_start_time >= FALL_CONFIRM_MS) {
                            fall_alerted = true;
                            is_falling_candidate = false;
                            setRGB(255, 0, 0); // สีแดง
                            setBuzzer(true);
                            addLog("🚨🚨🚨 [UART ALERT] ยืนยันคนล้มจริงเกิน 2 วินาที! (ไซเรนดัง)");
                        }
                    }
                } else {
                    if (is_falling_candidate) {
                        is_falling_candidate = false;
                        setRGB(0, 150, 0);
                        addLog("🟢 [UART] ยกเลิกการเตือน (คนก้มตัวแล้วลุกขึ้น)");
                    }
                    if (fall_alerted) {
                        fall_alerted = false;
                        setRGB(0, 150, 0);
                        setBuzzer(false);
                        addLog("🟢 [UART] ผู้ใช้งานลุกขึ้นยืนแล้ว -> ปิดไซเรน");
                    }
                }
            }
            // ข. เฟรมความสูงเป้าหมายสด (Type 0x0E0E)
            else if (msgType == 0x0E0E && dataLen == 4) {
                float dist;
                memcpy(&dist, payload, 4);
                if (dist >= 0.1f && dist <= 5.0f) {
                    raw_radar_dist = dist;
                    // คำนวณความสูงจริงของคนจากพื้น: ความสูงติดตั้ง (2.2m) - ระยะจากเพดาน (เช่น ยืน 0.64m -> คนสูง 1.56m)
                    person_height = max(0.0f, mount_height - dist);
                    last_height_ms = now;

                    // ตรรกะตรวจจับการล้มจากระดับความสูงจริง (ฉุกเฉิน / สำรอง)
                    // ถ้าระดับความสูงคนต่ำกว่าเกณฑ์ (<= 0.45m) เช่น นอนราบ (0.22m)
                    if (person_height <= fall_threshold && person_height > 0.05f) {
                        if (!height_fall_alerted) {
                            if (!height_falling_candidate) {
                                height_falling_candidate = true;
                                height_fall_start_time = now;
                                setRGB(255, 120, 0); // สีส้ม
                                addLog("⚠️ [ตรวจพบล้มจากความสูง] ความสูงคนลดเหลือ " + String(person_height, 2) + "m! กำลังจับเวลา 2s...");
                            } else if (now - height_fall_start_time >= FALL_CONFIRM_MS) {
                                height_fall_alerted = true;
                                height_falling_candidate = false;
                                setRGB(255, 0, 0);   // สีแดง
                                setBuzzer(true);     // เปิดไซเรน
                                addLog("🚨🚨🚨 [ALERT CONFIRMED] ยืนยันคนนอนล้มราบกับพื้น! (ความสูง: " + String(person_height, 2) + "m) 🚨🚨🚨");
                            }
                        }
                    } else if (person_height > 0.85f) { // ผู้ใช้งานลุกขึ้นนั่งหรือยืน (> 0.85m)
                        if (height_falling_candidate) {
                            height_falling_candidate = false;
                            setRGB(0, 150, 0);
                            addLog("🟢 [ความสูง] ยกเลิกการเตือน (คนลุกขึ้นแล้ว ความสูง " + String(person_height, 2) + "m)");
                        }
                        if (height_fall_alerted) {
                            height_fall_alerted = false;
                            setRGB(0, 150, 0);
                            setBuzzer(false);
                            addLog("🟢 [ความสูง] ผู้ใช้งานลุกขึ้นยืนแล้ว (" + String(person_height, 2) + "m) -> ปิดไซเรน");
                        }
                    }
                }
            }
        }
    }

    // -------------------------------------------------------------
    // 3. เอฟเฟกต์ไฟกะพริบสีแดงขณะยืนยันการล้ม (ทั้งจากเรดาร์ หรือจากความสูง)
    // -------------------------------------------------------------
    bool active_alert = fall_alerted || height_fall_alerted;
    if (active_alert) {
        static unsigned long last_flash = 0;
        static bool flash = false;
        if (now - last_flash >= 200) {
            last_flash = now;
            flash = !flash;
            setRGB(flash ? 255 : 0, 0, 0);
        }
    }
}
