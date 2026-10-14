import docx

doc = docx.Document(r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค.docx")

# Table 1 is TOC
toc_table = doc.tables[1]
print(f"Total rows in TOC: {len(toc_table.rows)}")
for r in toc_table.rows:
    c0 = r.cells[0].text.strip().replace('\n', ' ')
    c1 = r.cells[1].text.strip()
    print(f"{c0:70s} | {c1}")
