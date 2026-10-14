import pymupdf as fitz
import re

manual_path = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\คู่มือรูปเล่ม\คู่มือจัดพิมพ์เล่มปริญญานิพนธ์ 2566 V2.31(คณะครุศาสตร์อุตสาหกรรม).pdf"
manual_doc = fitz.open(manual_path)

print(f"Total manual pages: {len(manual_doc)}")

print("\n--- SEARCHING FOR 'ประวัติผู้เขียน' IN MANUAL ---")
for i, page in enumerate(manual_doc):
    txt = page.get_text()
    if "ประวัติผู้เขียน" in txt:
        print(f"Page {i+1}:")
        for line in txt.split("\n"):
            if any(k in line for k in ["ประวัติผู้เขียน", "ประวัติผู้จัดทำ", "หน้าคั่น", "ภาคผนวก"]):
                print("   ", line.strip())

print("\n--- SEARCHING FOR 'หน้าคั่น' or 'แผ่นคั่น' IN MANUAL ---")
for i, page in enumerate(manual_doc):
    txt = page.get_text()
    if "หน้าคั่น" in txt or "แผ่นคั่น" in txt:
        print(f"Page {i+1}:")
        for line in txt.split("\n"):
            if "หน้าคั่น" in line or "แผ่นคั่น" in line:
                print("   ", line.strip())
