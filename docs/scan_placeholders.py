import pymupdf as fitz
import re

pdf_path = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค_final_render.pdf"
doc = fitz.open(pdf_path)

patterns = [
    (r"\[\s*พื้นที่สำหรับวางภาพประกอบ[^\]]*\]", "Placeholder Figure"),
    (r"[-–—]\s*%", "Missing % value"),
    (r"[-–—]\s*วินาที", "Missing seconds value"),
    (r"ร้อยละ\s*[-–—]", "Missing percentage"),
]

for i, page in enumerate(doc):
    txt = page.get_text()
    for pat, desc in patterns:
        for m in re.finditer(pat, txt):
            start = max(0, m.start() - 30)
            end = min(len(txt), m.end() + 30)
            snip = txt[start:end].replace('\n', ' ')
            print(f"Page {i+1}: {desc} -> ...{snip}...")
