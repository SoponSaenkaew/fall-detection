import docx

doc = docx.Document(r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค_backup_before_full_compliance.docx")

tbl1 = doc.tables[1]
tbl1_elem = tbl1._element
parent = tbl1_elem.getparent()
idx = parent.index(tbl1_elem)

next_elem = parent[idx + 1]
print("Next elem xml:")
print(next_elem.xml[:400])
