import docx
import pymupdf as fitz
import re

pdf_path = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค_final_render.pdf"
docx_path = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค.docx"

doc_pdf = fitz.open(pdf_path)
doc_docx = docx.Document(docx_path)

# Let's find every page number in doc_pdf
# Section 1 (Front matter) has Thai page numbers: ก, ข, ค, ง, จ, ฉ, ช, ซ, ฌ, ญ, ฎ, ฏ, ฐ
# Section 2 (Body) has Arabic page numbers: 1, 2, 3, ...

print("=== PAGE MAPPING (PDF Page -> Document Page) ===")
pdf_to_doc_page = {}
# PDF P.1 = ปกนอก (no number)
# PDF P.2 = ปกในไทย (ก - unprinted)
# PDF P.3 = ปกในอังกฤษ (ข - unprinted)
# PDF P.4 = ใบรับรอง (ค - unprinted)
# PDF P.5 = บทคัดย่อไทย (ง)
# PDF P.6 = Abstract (จ)
# PDF P.7 = กิตติกรรมประกาศ (ฉ)
# PDF P.8 = สารบัญ (ช - unprinted)
# PDF P.9 = สารบัญ ต่อ (ซ)
# PDF P.10 = สารบัญตาราง (ฌ - unprinted)
# PDF P.11 = สารบัญรูป (ญ - unprinted)
# PDF P.12 = คำอธิบาย (ฎ)
# PDF P.13 = คำอธิบาย ต่อ (ฏ)
# PDF P.14 = บทที่ 1 (1 - unprinted)

for i in range(len(doc_pdf)):
    pdf_pno = i + 1
    if pdf_pno == 1:
        doc_p = "ปกนอก"
    elif pdf_pno <= 13:
        thai_letters = ["ก", "ข", "ค", "ง", "จ", "ฉ", "ช", "ซ", "ฌ", "ญ", "ฎ", "ฏ", "ฐ"]
        doc_p = thai_letters[pdf_pno - 2]
    else:
        doc_p = str(pdf_pno - 13)
    pdf_to_doc_page[pdf_pno] = doc_p

for p in range(1, len(doc_pdf)+1):
    # check if printed
    page = doc_pdf[p-1]
    blocks = page.get_text("blocks")
    top_right = [b[4].strip() for b in blocks if b[1] < 80 and b[0] > 400]
    printed = top_right[0] if top_right else "(ซ่อน)"
    print(f"PDF P.{p:02d} = หน้า {pdf_to_doc_page[p]:>3s} [บนขวา: {printed}]")
