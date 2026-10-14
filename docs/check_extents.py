import docx

doc = docx.Document(r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค.docx")

for p_idx in [142, 159, 164, 352]:
    p = doc.paragraphs[p_idx]
    for d in p._p.xpath(".//w:drawing"):
        for ext in d.xpath(".//wp:extent"):
            print(f"P{p_idx} wp:extent cx={ext.get('cx')} cy={ext.get('cy')}")
        for a_ext in d.xpath(".//a:ext"):
            print(f"P{p_idx} a:ext cx={a_ext.get('cx')} cy={a_ext.get('cy')}")
