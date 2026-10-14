import pymupdf as fitz
import re

pdf_path = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค_final_render.pdf"
doc = fitz.open(pdf_path)

print("=== 1. SEARCH FOR PLACEHOLDERS OR INCOMPLETE TEXT ===")
patterns = [
    r"\[\s*พื้นที่สำหรับวางภาพประกอบ[^\]]*\]",
    r"[-–—]\s*%",
    r"[-–—]\s*วินาที",
    r"ร้อยละ\s*[-–—]",
    r"\[\s*รูป[^\]]*\]",
    r"รูปที่\s*\d+\.\d+\s*$",
    r"TODO",
    r"FIXME",
    r"\.\.\.",
]

for i, page in enumerate(doc):
    txt = page.get_text()
    for pat in patterns:
        matches = list(re.finditer(pat, txt))
        if matches:
            for m in matches:
                # show context
                start = max(0, m.start() - 40)
                end = min(len(txt), m.end() + 40)
                snip = txt[start:end].replace('\n', ' ')
                print(f"Page {i+1}: Matched '{pat}' -> ...{snip}...")

print("\n=== 2. ALL FIGURE LABELS IN DOCUMENT ===")
for i, page in enumerate(doc):
    txt = page.get_text()
    for line in txt.split("\n"):
        if re.match(r"^\s*รูปที่\s+\d+\.\d+", line):
            print(f"Page {i+1} (Body P.{doc[i].get_text('blocks')[0][4].strip() if doc[i].get_text('blocks') else ''}): {line.strip()}")

print("\n=== 3. ALL TABLE LABELS IN DOCUMENT ===")
for i, page in enumerate(doc):
    txt = page.get_text()
    for line in txt.split("\n"):
        if re.match(r"^\s*ตารางที่\s+\d+\.\d+", line):
            print(f"Page {i+1}: {line.strip()}")
