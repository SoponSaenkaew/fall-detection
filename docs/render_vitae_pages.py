import fitz

doc = fitz.open(r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\คู่มือรูปเล่ม\คู่มือจัดพิมพ์เล่มปริญญานิพนธ์ 2566 V2.31(คณะครุศาสตร์อุตสาหกรรม).pdf")

for p in [71, 72, 73]:
    pix = doc[p-1].get_pixmap(dpi=150)
    out_path = f"D:/Final project/fall-detection/docs/vitae_manual_page_{p}.png"
    pix.save(out_path)
    print(f"Saved PDF page {p} to {out_path}")
