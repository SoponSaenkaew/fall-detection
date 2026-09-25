CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = output/main.exe
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC) ld6002c_protocol.hpp serial_port.hpp
	@if not exist output mkdir output
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)
	@echo ✅ Compilation successful! Run with: .\output\main.exe

clean:
	@if exist output\main.exe del /f /q output\main.exe
	@echo 🧹 Cleaned output.

run: $(TARGET)
	.\$(TARGET)

.PHONY: all clean run
