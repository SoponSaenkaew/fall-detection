import docx
from PIL import Image
import io

TEST_DOCX = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\test_ch2.docx"
orig_docx = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค.docx"

doc = docx.Document(orig_docx)

# 1. Rotate image4.png blob 180 degrees
for r_id, rel in doc.part.rels.items():
    if "image4.png" in rel.target_ref:
        img = Image.open(io.BytesIO(rel.target_part.blob))
        rotated = img.rotate(180, expand=True)
        buf = io.BytesIO()
        rotated.save(buf, format="PNG")
        rel.target_part._blob = buf.getvalue()
        print("Rotated image4.png blob 180 deg")

# 2. Fix P142, P159, P164
for p_idx in [142, 159, 164]:
    p = doc.paragraphs[p_idx]
    p.alignment = docx.enum.text.WD_ALIGN_PARAGRAPH.CENTER
    for d in p._p.xpath(".//w:drawing"):
        # remove rot if any
        for xfrm in d.xpath(".//a:xfrm"):
            if "rot" in xfrm.attrib:
                del xfrm.attrib["rot"]
                print(f"P{p_idx}: deleted rot attribute")
        for ext in d.xpath(".//wp:extent"):
            ext.set("cx", "2700000")
            ext.set("cy", "2700000")
        for a_ext in d.xpath(".//a:ext"):
            if a_ext.get("cx") is not None and a_ext.get("cy") is not None:
                a_ext.set("cx", "2700000")
                a_ext.set("cy", "2700000")
    print(f"P{p_idx}: set to 7.5x7.5 cm centered")

doc.save(TEST_DOCX)
print("Saved test_ch2.docx successfully")
