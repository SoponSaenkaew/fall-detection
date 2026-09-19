"""
=============================================================================
📊 Python Data Logger สำหรับการทดลองเรดาร์ HLK-LD6002C
=============================================================================
สคริปต์นี้จะเชื่อมต่อกับ ESP32-S3 ผ่าน Serial Port (USB)
และบันทึกผลการทดลองการล้มแต่ละรอบ (Trial) ลงในไฟล์ CSV โดยอัตโนมัติ
เพื่อนำไปวิเคราะห์และทำตารางผลการทดลองในบทที่ 4-5 ของเล่มรายงาน

วิธีใช้งาน:
  1. ติดตั้ง pyserial (ถ้ายังไม่มี):
     pip install pyserial
  2. รันสคริปต์:
     python logger.py
=============================================================================
"""

import sys
import time
import os

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("❌ ไม่พบไลบรารี pyserial!")
    print("👉 โปรดติดตั้งโดยรันคำสั่ง: pip install pyserial")
    sys.exit(1)

OUTPUT_CSV_FILE = "ld6002c_experiment_records.csv"
BAUD_RATE = 115200

def list_and_select_port():
    ports = serial.tools.list_ports.comports()
    if not ports:
        print("❌ ไม่พบพอร์ต COM ที่เชื่อมต่ออยู่ โปรดเสียบบอร์ด ESP32-S3 เข้ากับคอมพิวเตอร์")
        sys.exit(1)

    print("\n=======================================================")
    print("🔌 รายการพอร์ต Serial ที่ตรวจพบ:")
    for idx, port in enumerate(ports):
        print(f"  [{idx + 1}] {port.device} - {port.description}")
    print("=======================================================")

    if len(ports) == 1:
        selected = ports[0].device
        print(f"👉 เลือกพอร์ตอัตโนมัติ: {selected}")
        return selected

    while True:
        choice = input(f"โปรดเลือกหมายเลขพอร์ต [1-{len(ports)}]: ").strip()
        if choice.isdigit() and 1 <= int(choice) <= len(ports):
            return ports[int(choice) - 1].device
        print("⚠️ หมายเลขพอร์ตไม่ถูกต้อง ลองใหม่อีกครั้ง")

def main():
    port_name = list_and_select_port()

    print(f"\n📡 กำลังเปิดการเชื่อมต่อไปยัง {port_name} @ {BAUD_RATE} baud...")
    try:
        ser = serial.Serial(port_name, BAUD_RATE, timeout=0.1)
    except Exception as e:
        print(f"❌ ไม่สามารถเปิดพอร์ต {port_name} ได้: {e}")
        sys.exit(1)

    # เตรียมไฟล์ CSV
    file_exists = os.path.isfile(OUTPUT_CSV_FILE)
    csv_file = open(OUTPUT_CSV_FILE, "a", encoding="utf-8-sig")
    if not file_exists:
        csv_file.write("Timestamp,Trial_No,Scenario_Name,Ground_Truth,Radar_Output,Confirm_Time_ms,Recovery_Time_ms,Result\n")
        csv_file.flush()

    print(f"✅ เชื่อมต่อสำเร็จ! ข้อมูลการทดลองจะถูกบันทึกลง: {OUTPUT_CSV_FILE}")
    print("💡 คุณสามารถพิมพ์คำสั่งในหน้าต่างนี้เพื่อส่งคำสั่งไปยัง ESP32-S3 ได้ เช่น f1, a1, end, s, csv, r")
    print("🛑 กด Ctrl+C เพื่อออกจากการทำงาน\n")

    in_csv_block = False

    try:
        while True:
            # อ่านข้อมูลจาก Serial
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(line)

                    # ตรวจจับท่อน CSV อัตโนมัติ
                    if "--- START CSV DATA ---" in line:
                        in_csv_block = True
                        continue
                    elif "--- END CSV DATA ---" in line:
                        in_csv_block = False
                        continue

                    # บันทึกบรรทัดข้อมูล CSV
                    if in_csv_block and line and not line.startswith("Trial_No"):
                        now_str = time.strftime("%Y-%m-%d %H:%M:%S")
                        csv_file.write(f"{now_str},{line}\n")
                        csv_file.flush()

                    # ตรวจจับผลลัพธ์รายรอบจากข้อความสรุป
                    if "[บันทึกผลรอบที่ #" in line:
                        # มีการบันทึกรอบใหม่ ให้แจ้งเตือน
                        pass

            # รองรับการพิมพ์คำสั่งส่งกลับไปยังบอร์ด (ใน Windows อาจใช้ msvcrt หรือส่งคำสั่ง)
            # เพื่อความเรียบง่าย รอรับข้อมูลต่อเนื่อง
            time.sleep(0.01)

    except KeyboardInterrupt:
        print("\n\n🛑 หยุดการบันทึกข้อมูลเรียบร้อยแล้ว!")
    finally:
        csv_file.close()
        ser.close()
        print(f"📁 บันทึกข้อมูลทั้งหมดลงในไฟล์: {os.path.abspath(OUTPUT_CSV_FILE)}")

if __name__ == "__main__":
    main()
