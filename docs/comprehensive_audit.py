import docx, re, fitz

doc = docx.Document('docs/เอกสารประกอบโปรเจกต์จบ/รายงานเชิงเทคนิก/รายงานเชิงเทคนิค.docx')
pdf = fitz.open('docs/เอกสารประกอบโปรเจกต์จบ/รายงานเชิงเทคนิก/รายงานเชิงเทคนิค_final_render.pdf')

print("="*60)
print("AUDIT REPORT: CURRENT DOCX & PDF VS MANUAL RULES")
print("="*60)

# 1. Check Margins of all sections
print("\n[1] SECTION MARGINS:")
for idx, s in enumerate(doc.sections):
    top = s.top_margin.cm
    bottom = s.bottom_margin.cm
    left = s.left_margin.cm
    right = s.right_margin.cm
    header = s.header_distance.cm
    different_first = s.different_first_page_header_footer
    print(f"  Section {idx}: top={top:.2f}cm, bottom={bottom:.2f}cm, left={left:.2f}cm, right={right:.2f}cm | header_dist={header:.2f}cm | diff_first={different_first}")

# 2. Check Keep With Next on Headings
print("\n[2] HEADING 'KEEP WITH NEXT' STATUS:")
orphan_risks = []
for idx, p in enumerate(doc.paragraphs):
    txt = p.text.strip()
    if re.match(r'^[1-5]\.\d+', txt) or re.match(r'^บทที่\s+[1-5]', txt):
        kwn = p.paragraph_format.keep_with_next
        if not kwn:
            orphan_risks.append((idx, txt[:40]))
print(f"  Headings WITHOUT keep_with_next: {len(orphan_risks)}")
for o in orphan_risks[:10]:
    print(f"    P{o[0]}: {o[1]}")

# 3. Check Headings Formatting (Font, Size, Bold, Indents)
print("\n[3] HEADINGS FORMATTING AUDIT:")
h2_list = []
h3_list = []
for idx, p in enumerate(doc.paragraphs):
    txt = p.text.strip()
    if re.match(r'^[1-5]\.[1-9]\s+', txt):
        h2_list.append((idx, txt[:30], p.paragraph_format.left_indent.cm if p.paragraph_format.left_indent else 0, p.paragraph_format.first_line_indent.cm if p.paragraph_format.first_line_indent else 0, set(r.bold for r in p.runs)))
    elif re.match(r'^[1-5]\.\d+\.\d+\s+', txt):
        h3_list.append((idx, txt[:30], p.paragraph_format.left_indent.cm if p.paragraph_format.left_indent else 0, p.paragraph_format.first_line_indent.cm if p.paragraph_format.first_line_indent else 0, set(r.bold for r in p.runs)))

print(f"  H2 total: {len(h2_list)}")
print(f"    Sample H2: {h2_list[0] if h2_list else 'None'}")
print(f"  H3 total: {len(h3_list)}")
print(f"    Sample H3: {h3_list[0] if h3_list else 'None'}")
bolds_in_h3 = set(b for h in h3_list for b in h[4])
print(f"    All bold states in H3: {bolds_in_h3}")

# 4. Check Spacing before/after H2 and H3
print("\n[4] SPACING BEFORE/AFTER HEADINGS:")
h2_sb = set(p.paragraph_format.space_before.pt if p.paragraph_format.space_before else 0 for p in doc.paragraphs if re.match(r'^[1-5]\.[1-9]\s+', p.text.strip()))
h3_sb = set(p.paragraph_format.space_before.pt if p.paragraph_format.space_before else 0 for p in doc.paragraphs if re.match(r'^[1-5]\.\d+\.\d+\s+', p.text.strip()))
print(f"  H2 space_before values: {h2_sb}")
print(f"  H3 space_before values: {h3_sb}")

# 5. Check PDF Page Number Locations and Values
print("\n[5] PDF PAGE NUMBERS (Top-Right):")
for pno in [4, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19]:
    page = pdf[pno]
    # Check top right quadrant: y from 0 to 100, x from 400 to 600
    rect = fitz.Rect(400, 20, 580, 100)
    text_in_header = page.get_text('text', clip=rect).strip()
    print(f"  PDF Page {pno+1} Header (clip x:400-580, y:20-100): '{text_in_header}'")

# 6. Check Table Format (Borders, fonts)
print("\n[6] TABLES IN DOCX:")
for idx, tbl in enumerate(doc.tables):
    rows = len(tbl.rows)
    cols = len(tbl.columns)
    c0 = tbl.rows[0].cells[0].text.strip()[:25] if rows and cols else ""
    print(f"  Table {idx}: {rows}x{cols} | cell(0,0)='{c0}'")
