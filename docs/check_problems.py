import pymupdf as fitz
import re

pdf_path = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค_final_render.pdf"
doc = fitz.open(pdf_path)

print("=== CHECKING PAGE 45-47 (Figure 3.7) ===")
for pno in [45, 46, 47]:
    print(f"--- Page {pno} ---")
    print(doc[pno-1].get_text())

print("\n=== CHECKING PAGE 60-62 (Accuracy percentage) ===")
for pno in [60, 61, 62]:
    print(f"--- Page {pno} ---")
    print(doc[pno-1].get_text())
