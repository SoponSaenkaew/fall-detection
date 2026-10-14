import fitz
import re

pdf_path = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค_final_render.pdf"
doc = fitz.open(pdf_path)

print("=== EXTRACTING TABLE OF CONTENTS (สารบัญ) ===")
# Pages 8 and 9 are TOC
toc_p8 = doc[7].get_text()
toc_p9 = doc[8].get_text()
print("--- TOC PAGE 8 ---")
print(toc_p8)
print("--- TOC PAGE 9 ---")
print(toc_p9)

print("\n=== EXTRACTING LIST OF TABLES (สารบัญตาราง) ===")
# Page 10
print(doc[9].get_text())

print("\n=== EXTRACTING LIST OF FIGURES (สารบัญรูป) ===")
# Page 11
print(doc[10].get_text())
