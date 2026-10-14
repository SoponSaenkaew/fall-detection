import pymupdf as fitz
import re
import os

pdf_path = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค_final_render.pdf"
doc = fitz.open(pdf_path)

out_dir = r"D:\Final project\fall-detection\docs\final_snapshots"
os.makedirs(out_dir, exist_ok=True)

print(f"Total pages in final PDF: {len(doc)}")

ch1_idx = None
for i in range(len(doc)):
    txt = doc[i].get_text()
    if "บทที่ 1" in txt and "บทนำ" in txt and "สารบัญ" not in txt:
        ch1_idx = i
        break

print(f"Chapter 1 starts at PDF page {ch1_idx + 1}")

def to_acad(p_idx):
    return str(p_idx - ch1_idx + 1)

# Check all figure pages in body
figures = [
    "รูปที่ 2.1", "รูปที่ 2.2", "รูปที่ 2.3", "รูปที่ 2.4", "รูปที่ 2.5", "รูปที่ 2.6",
    "รูปที่ 3.1", "รูปที่ 3.2", "รูปที่ 3.3", "รูปที่ 3.4", "รูปที่ 3.5", "รูปที่ 3.6", "รูปที่ 3.7", "รูปที่ 3.8", "รูปที่ 3.9",
    "รูปที่ 4.1", "รูปที่ 4.2", "รูปที่ 4.3", "รูปที่ 4.4", "รูปที่ 4.5", "รูปที่ 4.6", "รูปที่ 4.7", "รูปที่ 4.8", "รูปที่ 4.9",
    "รูปที่ ก.1", "รูปที่ ก.2", "รูปที่ ค.1", "รูปที่ ง.1", "รูปที่ ง.2"
]

print("\n--- Physical vs Expected Locations of Figures ---")
f_map = {}
for f in figures:
    pat = re.escape(f)
    for idx in range(ch1_idx, len(doc)):
        if re.search(pat, doc[idx].get_text()):
            f_map[f] = (idx + 1, to_acad(idx))
            break
    print(f"  {f}: PDF Page {f_map.get(f, ('None','None'))[0]}, Academic Page {f_map.get(f, ('None','None'))[1]}")

# Export key pages to PNG for visual inspection
key_figs = ["รูปที่ 2.2", "รูปที่ 2.3", "รูปที่ 2.4", "รูปที่ 4.4", "รูปที่ 4.5", "รูปที่ 4.6", "รูปที่ 4.7", "รูปที่ 4.8", "รูปที่ 4.9"]
for k in key_figs:
    if k in f_map:
        pdf_p = f_map[k][0]
        pix = doc[pdf_p - 1].get_pixmap(dpi=150)
        fname = f"final_page_{pdf_p}_{k.replace(' ', '_').replace('.', '_')}.png"
        pix.save(os.path.join(out_dir, fname))

# Also export List of Figures pages
pix_lof1 = doc[10].get_pixmap(dpi=150) # Page 11 (สารบัญภาพ 1)
pix_lof1.save(os.path.join(out_dir, "final_list_of_figures_p11.png"))
pix_lof2 = doc[11].get_pixmap(dpi=150) # Page 12 (สารบัญภาพ 2)
pix_lof2.save(os.path.join(out_dir, "final_list_of_figures_p12.png"))

print("\nSaved final snapshots successfully.")
