import docx

doc = docx.Document(r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค.docx")

rels = doc.part.rels
images = []
for rel in rels.values():
    if "image" in rel.target_ref:
        images.append((rel.target_ref, len(rel.target_part.blob)))

print(f"Total embedded images in docx: {len(images)}")
for ref, size in images:
    print(f"  Image: {ref} ({size/1024:.1f} KB)")
