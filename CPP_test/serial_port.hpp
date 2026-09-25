#pragma once

#ifdef _WIN32
#include <windows.h>
#include <string>
#include <vector>
#include <iostream>

namespace LD6002C {

class SerialPort {
public:
    SerialPort() : hSerial(INVALID_HANDLE_VALUE), is_connected(false) {}

    ~SerialPort() {
        close();
    }

    bool open(const std::string& port_name, int baud_rate = 115200) {
        close();

        std::string full_port_name = "\\\\.\\" + port_name;
        hSerial = CreateFileA(
            full_port_name.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (hSerial == INVALID_HANDLE_VALUE) {
            return false;
        }

        DCB dcbSerialParams;
        std::memset(&dcbSerialParams, 0, sizeof(dcbSerialParams));
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

        if (!GetCommState(hSerial, &dcbSerialParams)) {
            close();
            return false;
        }

        dcbSerialParams.BaudRate = baud_rate;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity   = NOPARITY;
        dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

        if (!SetCommState(hSerial, &dcbSerialParams)) {
            close();
            return false;
        }

        COMMTIMEOUTS timeouts;
        std::memset(&timeouts, 0, sizeof(timeouts));
        timeouts.ReadIntervalTimeout         = 50;
        timeouts.ReadTotalTimeoutConstant    = 50;
        timeouts.ReadTotalTimeoutMultiplier  = 10;
        timeouts.WriteTotalTimeoutConstant   = 50;
        timeouts.WriteTotalTimeoutMultiplier = 10;

        if (!SetCommTimeouts(hSerial, &timeouts)) {
            close();
            return false;
        }

        is_connected = true;
        return true;
    }

    void close() {
        if (is_connected && hSerial != INVALID_HANDLE_VALUE) {
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
            is_connected = false;
        }
    }

    int read(uint8_t* buffer, size_t max_bytes) {
        if (!is_connected) return -1;
        DWORD bytes_read = 0;
        if (ReadFile(hSerial, buffer, static_cast<DWORD>(max_bytes), &bytes_read, nullptr)) {
            return static_cast<int>(bytes_read);
        }
        return -1;
    }

    int write(const uint8_t* buffer, size_t bytes_to_write) {
        if (!is_connected) return -1;
        DWORD bytes_written = 0;
        if (WriteFile(hSerial, buffer, static_cast<DWORD>(bytes_to_write), &bytes_written, nullptr)) {
            return static_cast<int>(bytes_written);
        }
        return -1;
    }

    bool write(const std::vector<uint8_t>& data) {
        return write(data.data(), data.size()) == static_cast<int>(data.size());
    }

    bool is_open() const { return is_connected; }

private:
    HANDLE hSerial;
    bool is_connected;
};

} // namespace LD6002C
#endif
