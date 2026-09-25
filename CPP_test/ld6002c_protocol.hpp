#pragma once

#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <functional>
#include <string>

namespace LD6002C {

// ==========================================
// 1. Constants & Protocol Definitions
// ==========================================
constexpr uint8_t SOF_BYTE = 0x01; // Start of Frame (TinyFrame standard)

// Message Types ตามเอกสาร HLK-LD6002C Communication Protocol
enum MessageType : uint16_t {
    MSG_FALL_STATUS      = 0x0E02, // เรดาร์รายงานผลการตรวจจับการล้ม (1 byte: 0=normal, 1=fall)
    MSG_SET_HEIGHT       = 0x0E04, // ตั้งค่าความสูงติดตั้ง (float 4 bytes: 1.0 - 5.0m)
    MSG_GET_HEIGHT       = 0x0E06, // อ่านค่าพารามิเตอร์เรดาร์
    MSG_SET_THRESHOLD    = 0x0E08, // ตั้งค่าเกณฑ์ความสูงการล้ม (float/uint8)
    MSG_SET_SENSITIVITY  = 0x0E0A, // ตั้งค่าความไวตรวจจับ (uint8)
    MSG_SET_ALARM_AREA   = 0x0E0C, // กำหนดขอบเขตห้อง (float 4 ค่า: XL, XR, ZF, ZB)
    MSG_INIT_PARAMS      = 0x2110  // ตั้งค่าเริ่มต้นพร้อมกันทั้งหมด
};

// ==========================================
// 2. Checksum Calculator
// ==========================================
// ตามคู่มือ HLK-LD6002C: นำข้อมูลทุกไบต์มา XOR กัน แล้ว Bitwise NOT (~)
inline uint8_t calculate_checksum(const uint8_t* data, size_t len) {
    uint8_t ret = 0;
    for (size_t i = 0; i < len; ++i) {
        ret ^= data[i];
    }
    return static_cast<uint8_t>(~ret);
}

// ==========================================
// 3. Parsed Frame Data Structure
// ==========================================
struct Frame {
    uint16_t id = 0;
    uint16_t type = 0;
    std::vector<uint8_t> payload;
    bool is_valid = false;

    // Helper: แปลง Type เป็นชื่อที่มนุษย์เข้าใจง่าย
    std::string get_type_name() const {
        switch (type) {
            case MSG_FALL_STATUS:     return "MSG_IND_FALL_STATUS (0x0E02)";
            case MSG_SET_HEIGHT:      return "MSG_IND_FALL_RES / SET_HEIGHT (0x0E04)";
            case MSG_GET_HEIGHT:      return "MSG_IND_FALL_GET_HIGH (0x0E06)";
            case MSG_SET_THRESHOLD:   return "MSG_IND_FALL_SET_THRESHOLD (0x0E08)";
            case MSG_SET_SENSITIVITY: return "MSG_IND_FALL_SET_SENSITIVITY (0x0E0A)";
            case MSG_SET_ALARM_AREA:  return "MSG_IND_FALL_SET_ALARM_AREA (0x0E0C)";
            default: {
                char buf[32];
                snprintf(buf, sizeof(buf), "UNKNOWN (0x%04X)", type);
                return std::string(buf);
            }
        }
    }

    // Helper: ตรวจสอบสถานะการล้ม (เฉพาะเมื่อ Type == 0x0E02)
    bool is_fall() const {
        return (type == MSG_FALL_STATUS && !payload.empty() && payload[0] == 0x01);
    }

    // Helper: ดึงค่า Float 4 ไบต์แบบ Little-Endian
    float get_payload_float(size_t offset = 0) const {
        if (payload.size() < offset + 4) return 0.0f;
        float val;
        std::memcpy(&val, &payload[offset], 4);
        return val;
    }
};

// ==========================================
// 4. Frame Builder (สร้างเฟรมสำหรับส่งเข้าเรดาร์)
// ==========================================
class FrameBuilder {
public:
    // ฟังก์ชันพื้นฐานในการแพ็กเฟรม TinyFrame
    static std::vector<uint8_t> build(uint16_t type, const std::vector<uint8_t>& payload, uint16_t id = 0x0000) {
        std::vector<uint8_t> frame;
        uint16_t len = static_cast<uint16_t>(payload.size());

        // Header 7 ไบต์ (ก่อน Checksum)
        frame.push_back(SOF_BYTE);               // Byte 0: SOF (0x01)
        frame.push_back((id >> 8) & 0xFF);        // Byte 1: ID High
        frame.push_back(id & 0xFF);               // Byte 2: ID Low
        frame.push_back((len >> 8) & 0xFF);       // Byte 3: LEN High
        frame.push_back(len & 0xFF);              // Byte 4: LEN Low
        frame.push_back((type >> 8) & 0xFF);      // Byte 5: TYPE High
        frame.push_back(type & 0xFF);             // Byte 6: TYPE Low

        // Byte 7: HEAD_CKSUM
        uint8_t head_cksum = calculate_checksum(frame.data(), 7);
        frame.push_back(head_cksum);

        // Byte 8..8+len-1: Payload DATA
        if (len > 0) {
            frame.insert(frame.end(), payload.begin(), payload.end());
            // Byte 8+len: DATA_CKSUM
            uint8_t data_cksum = calculate_checksum(payload.data(), len);
            frame.push_back(data_cksum);
        }

        return frame;
    }

    // คำสั่ง 0x0E04: ตั้งความสูงติดตั้ง (เมตร)
    static std::vector<uint8_t> set_mount_height(float height_m) {
        std::vector<uint8_t> payload(4);
        std::memcpy(payload.data(), &height_m, 4);
        return build(MSG_SET_HEIGHT, payload);
    }

    // คำสั่ง 0x0E08: ตั้งเกณฑ์ความสูงล้ม (เมตร)
    static std::vector<uint8_t> set_fall_threshold(float threshold_m) {
        std::vector<uint8_t> payload(4);
        std::memcpy(payload.data(), &threshold_m, 4);
        return build(MSG_SET_THRESHOLD, payload);
    }

    // คำสั่ง 0x0E0C: ตั้งขอบเขตห้อง (XL, XR, ZF, ZB)
    static std::vector<uint8_t> set_room_area(float width_m, float length_m) {
        float xl = -width_m;
        float xr = width_m;
        float zf = length_m;
        float zb = -length_m;

        std::vector<uint8_t> payload(16);
        std::memcpy(&payload[0],  &xl, 4);
        std::memcpy(&payload[4],  &xr, 4);
        std::memcpy(&payload[8],  &zf, 4);
        std::memcpy(&payload[12], &zb, 4);
        return build(MSG_SET_ALARM_AREA, payload);
    }
};

// ==========================================
// 5. Streaming UART Parser (State Machine)
// ==========================================
class Parser {
public:
    enum State {
        STATE_WAIT_SOF,
        STATE_READ_HEADER,
        STATE_READ_PAYLOAD
    };

    using FrameCallback = std::function<void(const Frame&)>;

    Parser(FrameCallback cb = nullptr) : callback(cb) {
        reset();
    }

    void set_callback(FrameCallback cb) {
        callback = cb;
    }

    void reset() {
        state = STATE_WAIT_SOF;
        rx_buffer.clear();
        expected_len = 0;
        current_frame = Frame();
    }

    // ป้อนข้อมูลไบต์ทีละตัว (เหมาะกับการอ่านจาก UART Serial Stream)
    bool parse_byte(uint8_t byte) {
        switch (state) {
            case STATE_WAIT_SOF:
                if (byte == SOF_BYTE) {
                    rx_buffer.clear();
                    rx_buffer.push_back(byte);
                    state = STATE_READ_HEADER;
                }
                break;

            case STATE_READ_HEADER:
                rx_buffer.push_back(byte);
                // ต้องการ 8 ไบต์: SOF(1) + ID(2) + LEN(2) + TYPE(2) + HEAD_CKSUM(1)
                if (rx_buffer.size() == 8) {
                    uint8_t calc_head_cksum = calculate_checksum(rx_buffer.data(), 7);
                    uint8_t received_head_cksum = rx_buffer[7];

                    if (calc_head_cksum != received_head_cksum) {
                        // Checksum หัวเฟรมไม่ตรง -> เลื่อนหา SOF ใหม่
                        handle_error("Header checksum mismatch! Expected: 0x" + 
                                     to_hex(calc_head_cksum) + ", Got: 0x" + to_hex(received_head_cksum));
                        resync_stream();
                        return false;
                    }

                    // แกะ Header
                    current_frame.id   = (rx_buffer[1] << 8) | rx_buffer[2];
                    expected_len       = (rx_buffer[3] << 8) | rx_buffer[4];
                    current_frame.type = (rx_buffer[5] << 8) | rx_buffer[6];
                    current_frame.payload.clear();

                    if (expected_len == 0) {
                        // ไม่มี Payload -> ถือว่าได้เฟรมสมบูรณ์
                        current_frame.is_valid = true;
                        if (callback) callback(current_frame);
                        reset();
                        return true;
                    } else if (expected_len > 512) {
                        // ขนาดผิดปกติเกินขนาดเรดาร์
                        handle_error("Payload length too large: " + std::to_string(expected_len));
                        reset();
                        return false;
                    } else {
                        state = STATE_READ_PAYLOAD;
                    }
                }
                break;

            case STATE_READ_PAYLOAD:
                rx_buffer.push_back(byte);
                // ข้อมูล Payload ทั้งหมด + DATA_CKSUM 1 ไบต์
                if (rx_buffer.size() == static_cast<size_t>(8 + expected_len + 1)) {
                    const uint8_t* payload_start = &rx_buffer[8];
                    uint8_t calc_data_cksum = calculate_checksum(payload_start, expected_len);
                    uint8_t received_data_cksum = rx_buffer[8 + expected_len];

                    if (calc_data_cksum != received_data_cksum) {
                        handle_error("Data checksum mismatch! Expected: 0x" + 
                                     to_hex(calc_data_cksum) + ", Got: 0x" + to_hex(received_data_cksum));
                        reset();
                        return false;
                    }

                    // ถอดรหัสสำเร็จ!
                    current_frame.payload.assign(payload_start, payload_start + expected_len);
                    current_frame.is_valid = true;
                    if (callback) callback(current_frame);
                    reset();
                    return true;
                }
                break;
        }
        return false;
    }

    // ป้อนข้อมูลเป็น Chunk บัฟเฟอร์
    size_t parse_buffer(const uint8_t* data, size_t len) {
        size_t frames_decoded = 0;
        for (size_t i = 0; i < len; ++i) {
            if (parse_byte(data[i])) {
                frames_decoded++;
            }
        }
        return frames_decoded;
    }

    // Helper: แสดงข้อความ Error
    void set_debug(bool enable) { debug_mode = enable; }

private:
    State state = STATE_WAIT_SOF;
    std::vector<uint8_t> rx_buffer;
    uint16_t expected_len = 0;
    Frame current_frame;
    FrameCallback callback;
    bool debug_mode = true;

    void handle_error(const std::string& msg) {
        if (debug_mode) {
            std::cerr << "⚠️ [PARSER ERROR] " << msg << std::endl;
        }
    }

    void resync_stream() {
        // หา SOF ตัวถัดไปในบัฟเฟอร์ที่มีอยู่
        for (size_t i = 1; i < rx_buffer.size(); ++i) {
            if (rx_buffer[i] == SOF_BYTE) {
                std::vector<uint8_t> new_buf(rx_buffer.begin() + i, rx_buffer.end());
                reset();
                for (uint8_t b : new_buf) {
                    parse_byte(b);
                }
                return;
            }
        }
        reset();
    }

    static std::string to_hex(uint8_t b) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%02X", b);
        return std::string(buf);
    }
};

} // namespace LD6002C
