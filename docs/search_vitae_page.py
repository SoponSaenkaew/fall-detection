import fitz

doc = fitz.open(r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\คู่มือรูปเล่ม\คู่มือจัดพิมพ์เล่มปริญญานิพนธ์ 2566 V2.31(คณะครุศาสตร์อุตสาหกรรม).pdf")

print("Searching for ประวัติผู้เขียน sample pages...")
for i, page in enumerate(doc):
    text = page.get_text()
    if "ประวัติผู้เขียน" in text:
        print(f"Found on PDF page {i+1} (printed page might be ~{i-3})")
