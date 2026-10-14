import pymupdf as fitz
import os

pdf_path = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค_final_render.pdf"
doc = fitz.open(pdf_path)

out_dir = r"D:\Final project\fall-detection\docs\debug_pages"
os.makedirs(out_dir, exist_ok=True)

targets = ["รูปที่ 3.9", "รูปที่ 4.5", "รูปที่ 4.6", "รูปที่ 4.7", "รูปที่ 4.8", "รูปที่ 4.9", "รูปที่ ง.1", "รูปที่ ง.2"]

saved_pages = set()
for i, page in enumerate(doc):
    text = page.get_text()
    for fig in targets:
        if fig in text and i not in saved_pages:
            saved_pages.add(i)
            pix = page.get_pixmap(dpi=150)
            fname = f"page_{i+1}.png"
            pix.save(os.path.join(out_dir, fname))
            print(f"Saved page {i+1} (has matches like {fig})")

print("Done saving debug pages.")
