#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <deque>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <thread>
#include <cstdarg>

// ================================================================
// Arduino Compatibility Layer for PC (Windows / MinGW / GCC)
// ทำให้เขียนโค้ดสไตล์ Arduino (Serial, millis, delay, String, HardwareSerial)
// บนเครื่องคอมพิวเตอร์ได้เหมือนอยู่บนบอร์ด ESP32-S3 100%
// ================================================================

typedef uint8_t byte;

#define HEX 16
#define DEC 10
#define BIN 2
#define OCT 8
#define SERIAL_8N1 0x06

// ---------------- 1. String Class (Arduino Compatible) ----------------
class String : public std::string {
public:
    String() : std::string() {}
    String(const char* s) : std::string(s ? s : "") {}
    String(const std::string& s) : std::string(s) {}
    String(int val) : std::string(std::to_string(val)) {}
    String(unsigned int val) : std::string(std::to_string(val)) {}
    String(long val) : std::string(std::to_string(val)) {}
    String(unsigned long val) : std::string(std::to_string(val)) {}
    String(float val) : std::string(std::to_string(val)) {}
    String(double val) : std::string(std::to_string(val)) {}

    bool operator==(const char* s) const { return this->compare(s ? s : "") == 0; }
    bool operator!=(const char* s) const { return !(*this == s); }
    bool operator==(const String& s) const { return this->compare(s) == 0; }
    bool operator!=(const String& s) const { return !(*this == s); }
};

// ---------------- 2. Time Functions ----------------
inline auto __arduino_start_time = std::chrono::steady_clock::now();

inline unsigned long millis() {
    auto now = std::chrono::steady_clock::now();
    return static_cast<unsigned long>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now - __arduino_start_time).count()
    );
}

inline void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// ---------------- 3. Serial (Console Output) ----------------
class ArduinoSerial {
public:
    void begin(unsigned long /*baud*/) {}

    void print(const char* s) { std::cout << (s ? s : ""); }
    void print(const String& s) { std::cout << s; }
    void print(char c) { std::cout << c; }
    void print(int val, int base = DEC) { print_num(val, base); }
    void print(unsigned int val, int base = DEC) { print_num(val, base); }
    void print(long val, int base = DEC) { print_num(val, base); }
    void print(unsigned long val, int base = DEC) { print_num(val, base); }
    void print(float val) { std::cout << val; }
    void print(double val) { std::cout << val; }

    void println() { std::cout << std::endl; }
    void println(const char* s) { std::cout << (s ? s : "") << std::endl; }
    void println(const String& s) { std::cout << s << std::endl; }
    void println(char c) { std::cout << c << std::endl; }
    void println(int val, int base = DEC) { print_num(val, base); std::cout << std::endl; }
    void println(unsigned int val, int base = DEC) { print_num(val, base); std::cout << std::endl; }
    void println(long val, int base = DEC) { print_num(val, base); std::cout << std::endl; }
    void println(unsigned long val, int base = DEC) { print_num(val, base); std::cout << std::endl; }
    void println(float val) { std::cout << val << std::endl; }
    void println(double val) { std::cout << val << std::endl; }

    void printf(const char* format, ...) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }

private:
    template<typename T>
    void print_num(T val, int base) {
        if (base == HEX) {
            std::cout << std::hex << std::uppercase << val << std::dec;
        } else {
            std::cout << val;
        }
    }
};

inline ArduinoSerial Serial;

// ---------------- 4. HardwareSerial (Stream Buffer for Radar) ----------------
class HardwareSerial {
public:
    explicit HardwareSerial(int port_num = 1) : port(port_num) {}

    void begin(unsigned long baud, uint32_t /*config*/ = 0, int rx_pin = -1, int tx_pin = -1) {
        rx_buffer.clear();
        (void)baud; (void)rx_pin; (void)tx_pin;
    }

    int available() {
        return static_cast<int>(rx_buffer.size());
    }

    int peek() {
        if (rx_buffer.empty()) return -1;
        return rx_buffer.front();
    }

    int read() {
        if (rx_buffer.empty()) return -1;
        uint8_t b = rx_buffer.front();
        rx_buffer.pop_front();
        return b;
    }

    size_t readBytes(uint8_t* buffer, size_t length) {
        size_t count = 0;
        while (count < length && !rx_buffer.empty()) {
            buffer[count++] = rx_buffer.front();
            rx_buffer.pop_front();
        }
        return count;
    }

    size_t write(const uint8_t* buffer, size_t size) {
        last_sent_frame.assign(buffer, buffer + size);
        return size;
    }

    size_t write(uint8_t b) {
        last_sent_frame.push_back(b);
        return 1;
    }

    // ฟังก์ชันเสริมสำหรับจำลองข้อมูลป้อนเข้า UART (Simulation Injector)
    void injectPacket(const uint8_t* data, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            rx_buffer.push_back(data[i]);
        }
    }

    void injectPacket(const std::vector<uint8_t>& data) {
        injectPacket(data.data(), data.size());
    }

    const std::vector<uint8_t>& getLastSentFrame() const {
        return last_sent_frame;
    }

private:
    int port;
    std::deque<uint8_t> rx_buffer;
    std::vector<uint8_t> last_sent_frame;
};
