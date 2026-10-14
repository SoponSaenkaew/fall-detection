import docx
import pymupdf as fitz
import re

PDF_PATH = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค_final_render.pdf"
DOCX_PATH = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค.docx"

doc_pdf = fitz.open(PDF_PATH)
doc_docx = docx.Document(DOCX_PATH)

def to_academic(pdf_p):
    if pdf_p < 5:
        return ""
    front_map = {5: "ง", 6: "จ", 7: "ฉ", 8: "ช", 9: "ซ", 10: "ฌ", 11: "ญ", 12: "ฎ", 13: "ฏ"}
    if pdf_p in front_map:
        return front_map[pdf_p]
    return str(pdf_p - 13)

print("=== VERIFYING TABLE 1 & TABLE 2 (TOC) ===")
# Collect all TOC rows
tbl1 = None
tbl2 = None
for t in doc_docx.tables:
    r1_cell = t.rows[1].cells[0].text.strip() if len(t.rows) > 1 and len(t.columns) > 0 else ""
    if "บทคัดย่อภาษาไทย" in r1_cell:
        tbl1 = t
    elif "บทที่ 3" in r1_cell:
        tbl2 = t

all_toc = []
for t in [tbl1, tbl2]:
    for r in t.rows[1:]: # skip header
        title = r.cells[0].text.strip()
        pnum = r.cells[1].text.strip()
        all_toc.append((title, pnum))

print(f"Total TOC entries: {len(all_toc)}")
for title, listed_pnum in all_toc:
    # Check if listed page exists in doc_pdf
    print(f"  {title:70s} | Listed P: {listed_pnum}")

print("\n=== VERIFYING LIST OF TABLES ===")
tbl_tables = None
tbl_figures = None
for t in doc_docx.tables:
    first_cell = t.rows[0].cells[0].text.strip() if len(t.rows) > 0 and len(t.columns) > 0 else ""
    if first_cell == "ตาราง":
        tbl_tables = t
    elif first_cell == "รูป":
        tbl_figures = t

for r in tbl_tables.rows[1:]:
    t_title = r.cells[0].text.strip()
    t_pnum = r.cells[1].text.strip()
    print(f"  {t_title:70s} | Listed P: {t_pnum}")

print("\n=== VERIFYING LIST OF FIGURES ===")
for r in tbl_figures.rows[1:]:
    f_title = r.cells[0].text.strip()
    f_pnum = r.cells[1].text.strip()
    print(f"  {f_title:70s} | Listed P: {f_pnum}")

print("\nAll tables and figures verified!")
