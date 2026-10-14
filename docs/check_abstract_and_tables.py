import pymupdf as fitz

pdf_path = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค_final_render.pdf"
doc = fitz.open(pdf_path)

print("=== ABSTRACT THAI (Page 5) ===")
print(doc[4].get_text())

print("=== ABSTRACT ENG (Page 6) ===")
print(doc[5].get_text())

print("=== TABLE 4.1 (Confusion Matrix, Page 58-59) ===")
print(doc[57].get_text())

print("=== TABLE 4.2 (24 GHz Presence, Page 60) ===")
print(doc[59].get_text())
