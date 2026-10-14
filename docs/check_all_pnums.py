import pymupdf as fitz
import re

pdf_path = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค_final_render.pdf"
doc = fitz.open(pdf_path)

print("=== CHECKING PRINTED PAGE NUMBERS ACROSS ALL 75 PAGES ===")
for i, page in enumerate(doc):
    blocks = page.get_text("blocks")
    top_blocks = [b for b in blocks if b[1] < 90]
    pnum = None
    for b in top_blocks:
        txt = b[4].strip()
        lines = [l.strip() for l in txt.split('\n') if l.strip()]
        for l in lines:
            if re.match(r"^(\d+|[ก-ฮ])$", l):
                pnum = l
                break
        if pnum:
            break
    
    first_line = page.get_text().strip().split('\n')[0].strip() if page.get_text().strip() else "[BLANK]"
    print(f"PDF P.{i+1:02d}: Printed Page Num = {str(pnum):>5s} | First line = {first_line[:40]}")
