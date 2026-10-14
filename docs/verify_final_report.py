import pymupdf as fitz
import re

PDF_PATH = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค_final_render.pdf"
doc = fitz.open(PDF_PATH)
print(f"=== FINAL PDF VERIFICATION AUDIT (Total Pages: {len(doc)}) ===")

# 1. Search for placeholders or drafting patterns
print("\n[CHECK 1] Scanning for Placeholders / Incomplete Draft Markers...")
patterns = [
    r"\[\s*พื้นที่สำหรับวางภาพประกอบ[^\]]*\]",
    r"[-–—]\s*%",
    r"[-–—]\s*วินาที",
    r"ร้อยละ\s*[-–—]",
    r"TODO",
    r"FIXME",
]
found_issues = 0
for i, page in enumerate(doc):
    txt = page.get_text()
    for pat in patterns:
        for m in re.finditer(pat, txt):
            start = max(0, m.start() - 30)
            end = min(len(txt), m.end() + 30)
            print(f"  [!] Found '{pat}' on Page {i+1}: {txt[start:end].replace(chr(10), ' ')}")
            found_issues += 1

if found_issues == 0:
    print("  [PASSED] 0 placeholders or incomplete markers found in document!")

# 2. Check Front Matter and Page Numbers
print("\n[CHECK 2] Verifying Front Matter Pagination Sequence...")
for i in range(16):
    txt = doc[i].get_text()
    lines = [l.strip() for l in txt.split('\n') if l.strip()]
    top_num = None
    for b in doc[i].get_text('blocks'):
        if b[1] < 90:
            for l in b[4].split('\n'):
                if l.strip():
                    top_num = l.strip()
                    break
            if top_num:
                break
    fl = lines[0] if lines else ""
    print(f"  PDF P.{i+1:02d}: Printed Header Num = {str(top_num):>5s} | First line = {fl[:40]}")

# 3. Check Separator Pages (แผ่นคั่น) for Appendixes and Vitae
print("\n[CHECK 3] Verifying Separator Pages (แผ่นคั่น - Must NOT display page numbers)...")
for i, page in enumerate(doc):
    txt = page.get_text().strip()
    first_line = txt.split('\n')[0].strip() if txt else ""
    if any(k in first_line for k in ["ภาคผนวก ก", "ภาคผนวก ข", "ภาคผนวก ค", "ภาคผนวก ง", "ภาคผนวก จ", "ประวัติผู้เขียน"]) and len(txt.split('\n')) <= 3:
        top_num = None
        for b in page.get_text('blocks'):
            if b[1] < 90:
                for l in b[4].split('\n'):
                    if l.strip().isdigit():
                        top_num = l.strip()
        print(f"  PDF P.{i+1:02d} Separator '{first_line}': Top printed num = {top_num} (Expected: None)")

# 4. Compare TOC against Physical Pages
print("\n[CHECK 4] Verifying Table of Contents (Pages 8 and 9) against Scanned Positions...")
# Page 8 text (TOC p1)
toc_p1_text = doc[7].get_text()
# Page 9 text (TOC p2)
toc_p2_text = doc[8].get_text()

print("TOC Page 1 Sample Lines:")
for line in toc_p1_text.split('\n')[:15]:
    if line.strip():
        print("  ", line.strip())

print("\nTOC Page 2 Sample Lines:")
for line in toc_p2_text.split('\n')[:15]:
    if line.strip():
        print("  ", line.strip())

print("\nAudit check completed.")
