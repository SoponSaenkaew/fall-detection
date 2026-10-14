import os
import docx
from docx.shared import Cm, Pt
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml import parse_xml, OxmlElement
from docx.oxml.ns import nsdecls, qn

DOCX_PATH = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค.docx"

def set_font(run, font_name="TH SarabunPSK", font_size=16, bold=False, italic=False, color_rgb=None):
    run.font.name = font_name
    run.font.size = Pt(font_size)
    run.font.bold = bold
    run.font.italic = italic
    if color_rgb:
        run.font.color.rgb = color_rgb
    rPr = run._r.get_or_add_rPr()
    rFonts = rPr.find(qn('w:rFonts'))
    if rFonts is None:
        rFonts = OxmlElement('w:rFonts')
        rPr.append(rFonts)
    rFonts.set(qn('w:cs'), font_name)
    rFonts.set(qn('w:ascii'), font_name)
    rFonts.set(qn('w:hAnsi'), font_name)
    rFonts.set(qn('w:eastAsia'), font_name)

def build_placeholder_table(doc, fig_id, title_desc, spec_desc):
    tbl = doc.add_table(rows=1, cols=1)
    tbl.alignment = WD_TABLE_ALIGNMENT.CENTER
    tblPr = tbl._tbl.tblPr
    
    # 7600 dxa = ~13.4 cm width, centered
    tblPr.append(parse_xml(f'<w:tblW {nsdecls("w")} w:w="7600" w:type="dxa"/>'))
    
    # Dashed border
    tblPr.append(parse_xml(
        f'<w:tblBorders {nsdecls("w")}>\n'
        f'  <w:top w:val="dashed" w:sz="8" w:space="0" w:color="94A3B8"/>\n'
        f'  <w:left w:val="dashed" w:sz="8" w:space="0" w:color="94A3B8"/>\n'
        f'  <w:bottom w:val="dashed" w:sz="8" w:space="0" w:color="94A3B8"/>\n'
        f'  <w:right w:val="dashed" w:sz="8" w:space="0" w:color="94A3B8"/>\n'
        f'</w:tblBorders>'
    ))
    
    # Padding
    tblPr.append(parse_xml(
        f'<w:tblCellMar {nsdecls("w")}>\n'
        f'  <w:top w:w="240" w:type="dxa"/>\n'
        f'  <w:left w:w="280" w:type="dxa"/>\n'
        f'  <w:bottom w:w="240" w:type="dxa"/>\n'
        f'  <w:right w:w="280" w:type="dxa"/>\n'
        f'</w:tblCellMar>'
    ))
    
    cell = tbl.cell(0, 0)
    tcPr = cell._tc.get_or_add_tcPr()
    tcPr.append(parse_xml(f'<w:tcW {nsdecls("w")} w:w="7600" w:type="dxa"/>'))
    tcPr.append(parse_xml(f'<w:shd {nsdecls("w")} w:val="clear" w:color="auto" w:fill="F8FAFC"/>'))
    tcPr.append(parse_xml(f'<w:vAlign {nsdecls("w")} w:val="center"/>'))
    
    # Paragraph 1: Header title [ พื้นที่สำหรับวางภาพประกอบ: รูปที่ X.X ]
    p1 = cell.paragraphs[0]
    p1.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p1.paragraph_format.space_before = Pt(6)
    p1.paragraph_format.space_after = Pt(4)
    p1.paragraph_format.line_spacing = 1.0
    r1 = p1.add_run(f"[ พื้นที่สำหรับวางภาพประกอบ: {fig_id} ]")
    set_font(r1, bold=True, font_size=16)
    
    # Paragraph 2: Description of what photo / image to insert
    p2 = cell.add_paragraph()
    p2.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p2.paragraph_format.space_before = Pt(0)
    p2.paragraph_format.space_after = Pt(4)
    p2.paragraph_format.line_spacing = 1.0
    r2 = p2.add_run(title_desc)
    set_font(r2, bold=False, font_size=15)
    
    # Paragraph 3: Recommended image size and orientation
    p3 = cell.add_paragraph()
    p3.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p3.paragraph_format.space_before = Pt(0)
    p3.paragraph_format.space_after = Pt(6)
    p3.paragraph_format.line_spacing = 1.0
    r3 = p3.add_run(spec_desc)
    set_font(r3, bold=False, italic=True, font_size=14)
    
    return tbl

def build_caption_paragraph(doc, fig_id, caption_text):
    p = doc.add_paragraph()
    p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p.paragraph_format.space_before = Pt(6)
    p.paragraph_format.space_after = Pt(12)
    p.paragraph_format.line_spacing = 1.0
    r = p.add_run(f"{fig_id}  {caption_text}")
    set_font(r, bold=True, font_size=16)
    return p

def build_narrative_paragraph(doc, text):
    p = doc.add_paragraph()
    p.alignment = WD_ALIGN_PARAGRAPH.THAI_JUSTIFY
    p.paragraph_format.first_line_indent = Cm(1.80)
    p.paragraph_format.left_indent = Cm(0)
    p.paragraph_format.space_before = Pt(0)
    p.paragraph_format.space_after = Pt(0)
    p.paragraph_format.line_spacing = 1.0
    r = p.add_run(text)
    set_font(r, bold=False, font_size=16)
    return p

def main():
    print(f"Loading document from {DOCX_PATH}...")
    doc = docx.Document(DOCX_PATH)
    
    # -------------------------------------------------------------------------
    # 1. SECTION 3.7.1: INSERT รูปที่ 3.9 (แผนผังการจัดสภาพแวดล้อมห้องน้ำทดสอบ)
    # -------------------------------------------------------------------------
    print("\n1. Inserting placeholder for รูปที่ 3.9 in Section 3.7.1...")
    target_p_371 = None
    for i, p in enumerate(doc.paragraphs):
        txt = p.text.strip().replace('\u200b', '')
        if "การทดสอบแบ่งออกเป็น 4 รูปแบบหลัก โดยจำลองรูปแบบละ 60 ครั้ง" in txt:
            target_p_371 = p
            break
            
    if target_p_371:
        parent = target_p_371._p.getparent()
        pos = parent.index(target_p_371._p)
        
        tbl_3_9 = build_placeholder_table(
            doc,
            fig_id="รูปที่ 3.9",
            title_desc="แผนผังการจัดสภาพแวดล้อมห้องน้ำทดสอบและตำแหน่งการติดตั้งอุปกรณ์ต้นแบบ (Experimental Testbed Setup Layout) แสดงมุมมองด้านบน (Top View) และด้านข้าง (Side View) ของห้องน้ำขนาด 2.0 × 2.0 เมตร จุดติดตั้งเซนเซอร์เรดาร์คู่บนเพดานที่ระดับความสูง 2.30 เมตร และกรอบพื้นที่ตรวจจับการหกล้มระดับความสูงวิกฤต 0.60 เมตร",
            spec_desc="(ขนาดแนะนำ: กว้าง 13–14 ซม., สูง 8–9 ซม., แผนภาพโครงสร้างหรือภาพจำลอง 3 มิติ คมชัด แนวนอน)"
        )
        p_cap_3_9 = build_caption_paragraph(doc, "รูปที่ 3.9", "แผนผังการจัดสภาพแวดล้อมห้องน้ำทดสอบและตำแหน่งการติดตั้งอุปกรณ์ต้นแบบ")
        p_nar_3_9 = build_narrative_paragraph(
            doc,
            "จากแผนผังการจัดสภาพแวดล้อมห้องน้ำทดสอบในรูปที่ 3.9 แสดงถึงการจำลองสภาพแวดล้อมห้องน้ำขนาดมาตรฐาน 2.0 × 2.0 เมตร โดยติดตั้งกล่องอุปกรณ์ตรวจจับไว้กึ่งกลางเพดานที่ระดับความสูง 2.30 เมตร ซึ่งมุมองศาการตรวจจับ (Beamwidth) ของสายอากาศเรดาร์ 60 GHz ที่ ±60 องศา และเรดาร์ 24 GHz ที่ ±60 องศา สามารถครอบคลุมพื้นที่ผิวสัมผัสพื้นห้องน้ำและสุขภัณฑ์ได้อย่างสมบูรณ์ โดยไม่เกิดจุดบอด (Blind Spot) ในการตรวจจับ"
        )
        
        # Insert after target_p_371
        parent.insert(pos + 1, tbl_3_9._tbl)
        parent.insert(pos + 2, p_cap_3_9._p)
        parent.insert(pos + 3, p_nar_3_9._p)
        print("  [+] Inserted รูปที่ 3.9 successfully.")
    else:
        print("  [!] Could not locate target paragraph for Section 3.7.1!")

    # -------------------------------------------------------------------------
    # 2. SECTION 4.5.1: INSERT รูปที่ 4.6 (ภาพถ่ายจำลองท่าทางหกล้ม 4 รูปแบบ)
    #                   AND รูปที่ 4.7 (ภาพถ่ายทดสอบกิจกรรมประจำวัน ADLs)
    # -------------------------------------------------------------------------
    print("\n2. Inserting placeholders for รูปที่ 4.6 and รูปที่ 4.7 in Section 4.5.1...")
    target_p_451 = None
    for i, p in enumerate(doc.paragraphs):
        txt = p.text.strip().replace('\u200b', '')
        if "จากผลการทดสอบเชิงประจักษ์ในตารางที่ 4.1" in txt:
            target_p_451 = p
            break
            
    if target_p_451:
        parent = target_p_451._p.getparent()
        pos = parent.index(target_p_451._p)
        
        # --- Figure 4.6 ---
        tbl_4_6 = build_placeholder_table(
            doc,
            fig_id="รูปที่ 4.6",
            title_desc="ภาพถ่ายการจำลองท่าทางการหกล้ม 4 รูปแบบหลักในห้องน้ำทดสอบ (ก) ท่าทางการล้มไปข้างหน้า (Forward Fall) (ข) ท่าทางการล้มหงายหลัง (Backward Fall) (ค) ท่าทางการล้มเอียงไปทางด้านข้าง (Lateral Fall) และ (ง) ท่าทางการทรุดตัวลงกับพื้น (Slump Fall / Collapse) โดยผู้ทดสอบจริงบนเบาะรองรับน้ำหนักและพื้นห้องน้ำจำลอง",
            spec_desc="(ขนาดแนะนำ: กว้าง 13–14 ซม., สูง 8–9 ซม., ภาพถ่ายความละเอียดสูง แนวนอน จัดเรียง 4 ภาพย่อย ก ข ค ง)"
        )
        p_cap_4_6 = build_caption_paragraph(doc, "รูปที่ 4.6", "ภาพถ่ายการจำลองท่าทางการหกล้ม 4 รูปแบบในห้องน้ำทดสอบ")
        p_nar_4_6 = build_narrative_paragraph(
            doc,
            "จากภาพถ่ายการจำลองท่าทางการหกล้มในรูปที่ 4.6 แสดงกระบวนการทดสอบท่าทางการหกล้ม 4 รูปแบบหลักของผู้ทดสอบจริงภายในห้องน้ำทดสอบ โดยมีเบาะรองรับน้ำหนักเพื่อความปลอดภัยของผู้ทดสอบ ผลการทดสอบเชิงประจักษ์ยืนยันว่าอัลกอริทึมการประมวลผลคลื่น FMCW ย่าน 60 GHz สามารถตรวจวัดการลดระดับความสูงของร่างกายลงสู่ระดับต่ำกว่าเกณฑ์วิกฤต (0.60 เมตร) ได้อย่างแม่นยำทุกรูปแบบการล้ม โดยไม่มีการตรวจจับตกหล่น (Missed Detection: FN = 0)"
        )
        
        # --- Figure 4.7 ---
        tbl_4_7 = build_placeholder_table(
            doc,
            fig_id="รูปที่ 4.7",
            title_desc="ภาพถ่ายการทดสอบกิจกรรมปกติในชีวิตประจำวัน (Activities of Daily Living: ADL) และกิจกรรมเสี่ยงต่อการเกิดสัญญาณเตือนหลอก (False Alarm Challenging Scenarios) (ก) การนั่งและลุกขึ้นจากโถสุขภัณฑ์ (ข) การก้มตัวลงเก็บสิ่งของบนพื้นห้องน้ำ (ค) การเดินเข้า-ออกและกลับตัวในพื้นที่แคบ และ (ง) การเปิดน้ำฝักบัวและละอองน้ำ",
            spec_desc="(ขนาดแนะนำ: กว้าง 13–14 ซม., สูง 8–9 ซม., ภาพถ่ายความละเอียดสูง แนวนอน จัดเรียง 4 ภาพย่อย ก ข ค ง)"
        )
        p_cap_4_7 = build_caption_paragraph(doc, "รูปที่ 4.7", "ภาพถ่ายการทดสอบกิจกรรมปกติในชีวิตประจำวัน (ADLs) และกิจกรรมที่เสี่ยงต่อการเกิดสัญญาณเตือนหลอก")
        p_nar_4_7 = build_narrative_paragraph(
            doc,
            "จากภาพการทดสอบกิจกรรมปกติในรูปที่ 4.7 แสดงให้เห็นถึงการทดสอบพฤติกรรมในชีวิตประจำวันและการก้มเก็บสิ่งของอย่างรวดเร็ว ซึ่งเป็นพฤติกรรมที่มีการเปลี่ยนแปลงระดับความสูงคล้ายการล้ม โดยระบบสามารถแยกแยะความแตกต่างของเวกเตอร์ความเร็ว Doppler ในแนวดิ่งและการกระจายตัวของกลุ่มจุดคลาวด์ได้อย่างเด็ดขาด ส่งผลให้ระบบไม่มีการส่งสัญญาณแจ้งเตือนผิดพลาดแม้แต่ครั้งเดียวในระหว่างการดำเนินกิจกรรมประจำวันทั้งหมด (FP = 0)"
        )
        
        parent.insert(pos + 1, tbl_4_6._tbl)
        parent.insert(pos + 2, p_cap_4_6._p)
        parent.insert(pos + 3, p_nar_4_6._p)
        parent.insert(pos + 4, tbl_4_7._tbl)
        parent.insert(pos + 5, p_cap_4_7._p)
        parent.insert(pos + 6, p_nar_4_7._p)
        print("  [+] Inserted รูปที่ 4.6 and รูปที่ 4.7 successfully.")
    else:
        print("  [!] Could not locate target paragraph for Section 4.5.1!")

    # -------------------------------------------------------------------------
    # 3. SECTION 4.5.2: INSERT รูปที่ 4.8 (ภาพถ่ายทดสอบเรดาร์ 24 GHz ในสภาวะต่าง ๆ)
    # -------------------------------------------------------------------------
    print("\n3. Inserting placeholder for รูปที่ 4.8 in Section 4.5.2...")
    target_p_452 = None
    for i, p in enumerate(doc.paragraphs):
        txt = p.text.strip().replace('\u200b', '')
        if "จากผลการทดสอบในตารางที่ 4.2 พบว่าโมดูลเรดาร์ 24 GHz" in txt:
            target_p_452 = p
            break
            
    if target_p_452:
        parent = target_p_452._p.getparent()
        pos = parent.index(target_p_452._p)
        
        tbl_4_8 = build_placeholder_table(
            doc,
            fig_id="รูปที่ 4.8",
            title_desc="ภาพถ่ายการทดสอบประสิทธิภาพการตรวจจับบุคคลด้วยเรดาร์คลื่นมิลลิเมตร 24 GHz (HLK-LD2410C) ในสภาวะต่าง ๆ ภายในห้องน้ำ (ก) การตรวจจับการเคลื่อนไหวระดับไมโคร (Micro-motion) จากการกระเพื่อมของทรวงอกขณะนั่งนิ่งบนสุขภัณฑ์ (ข) การตรวจจับบุคคลในสภาวะมีละอองไอน้ำหนาทึบและหยดน้ำเกาะผนังจากการเปิดเครื่องทำน้ำอุ่น",
            spec_desc="(ขนาดแนะนำ: กว้าง 13–14 ซม., สูง 7–8 ซม., ภาพถ่ายความละเอียดสูง แนวนอน จัดเรียง 2 ภาพย่อย ก ข)"
        )
        p_cap_4_8 = build_caption_paragraph(doc, "รูปที่ 4.8", "ภาพถ่ายการทดสอบประสิทธิภาพการตรวจจับบุคคลด้วยเรดาร์ 24 GHz ในสภาวะต่าง ๆ ภายในห้องน้ำ")
        p_nar_4_8 = build_narrative_paragraph(
            doc,
            "จากภาพถ่ายในรูปที่ 4.8 ยืนยันถึงคุณสมบัติเด่นของเรดาร์ 24 GHz ที่สามารถตรวจจับการเคลื่อนไหวระดับไมโครจากการหายใจ (Thoracic Displacement 1–3 มม.) ได้อย่างต่อเนื่องขณะผู้ใช้นั่งนิ่งสนิทบนสุขภัณฑ์ ช่วยแก้ปัญหาเซนเซอร์อินฟราเรดแบบเดิม และยังสามารถทะลุผ่านละอองไอน้ำหนาทึบได้อย่างมีเสถียรภาพโดยมีความถูกต้องสูงถึง 99.17%"
        )
        
        parent.insert(pos + 1, tbl_4_8._tbl)
        parent.insert(pos + 2, p_cap_4_8._p)
        parent.insert(pos + 3, p_nar_4_8._p)
        print("  [+] Inserted รูปที่ 4.8 successfully.")
    else:
        print("  [!] Could not locate target paragraph for Section 4.5.2!")

    # -------------------------------------------------------------------------
    # 4. SECTION 4.5.3: INSERT รูปที่ 4.9 (ภาพถ่ายการตอบสนองระบบแจ้งเตือนภัยจริง)
    # -------------------------------------------------------------------------
    print("\n4. Inserting placeholder for รูปที่ 4.9 in Section 4.5.3...")
    target_p_453 = None
    for i, p in enumerate(doc.paragraphs):
        txt = p.text.strip().replace('\u200b', '')
        if "จากภาพบรรยากาศการทดสอบในรูปที่ 4.5" in txt:
            target_p_453 = p
            break
            
    if target_p_453:
        parent = target_p_453._p.getparent()
        pos = parent.index(target_p_453._p)
        
        tbl_4_9 = build_placeholder_table(
            doc,
            fig_id="รูปที่ 4.9",
            title_desc="ภาพถ่ายการทำงานและผลการตอบสนองของระบบแจ้งเตือนภัยแบบเรียลไทม์ขณะเกิดเหตุการณ์หกล้มจริง (ก) สัญญาณไฟ LED แสดงสถานะเตือนภัยสีแดงและเสียงไซเรน Buzzer ขับเตือนภัยระดับ 85 dB ณ จุดติดตั้งกล่องอุปกรณ์บนเพดาน (ข) หน้าจอเว็บแดชบอร์ดแสดงสถานะแถบเตือนสีแดง (FALL_ALERT) ควบคู่กับข้อความแจ้งเตือนภัยฉุกเฉิน Flex Message บนสมาร์ตโฟนผ่านแอปพลิเคชัน LINE",
            spec_desc="(ขนาดแนะนำ: กว้าง 13–14 ซม., สูง 7–8 ซม., ภาพถ่ายความละเอียดสูง แนวนอน จัดเรียง 2 ภาพย่อย ก ข)"
        )
        p_cap_4_9 = build_caption_paragraph(doc, "รูปที่ 4.9", "ภาพถ่ายการตอบสนองของระบบแจ้งเตือนภัยแบบเรียลไทม์เมื่อตรวจพบเหตุการณ์หกล้มจริง")
        p_nar_4_9 = build_narrative_paragraph(
            doc,
            "จากภาพถ่ายการตอบสนองของระบบในรูปที่ 4.9 แสดงถึงการทำงานประสานกันอย่างสมบูรณ์แบบของระบบแจ้งเตือนภัยแบบสองระดับ (Two-Tier Alert Mechanism) โดยทันทีที่พ้นระยะเวลาหน่วงยืนยันการล้ม 2.0 วินาที อุปกรณ์ตรวจจับ ณ จุดเกิดเหตุจะส่งสัญญาณเสียงไซเรนเตือนภัยและไฟกะพริบสีแดงทันที พร้อมทั้งส่งแพ็กเก็ตข้อมูลผ่าน MQTT และ Webhook ไปยังสมาร์ตโฟนของผู้ดูแลผ่าน LINE และหน้าจอแดชบอร์ด Next.js ภายในเวลาเฉลี่ยเพียง 1.25 วินาที ทำให้ผู้ดูแลสามารถเข้าช่วยเหลือผู้ประสบอุบัติเหตุได้อย่างทันท่วงที"
        )
        
        parent.insert(pos + 1, tbl_4_9._tbl)
        parent.insert(pos + 2, p_cap_4_9._p)
        parent.insert(pos + 3, p_nar_4_9._p)
        print("  [+] Inserted รูปที่ 4.9 successfully.")
    else:
        print("  [!] Could not locate target paragraph for Section 4.5.3!")

    # -------------------------------------------------------------------------
    # 5. APPENDIX ง: INSERT รูปที่ ง.1 and รูปที่ ง.2
    # -------------------------------------------------------------------------
    print("\n5. Inserting placeholders for รูปที่ ง.1 and รูปที่ ง.2 in Appendix ง...")
    target_p_app_d1 = None
    target_p_app_d2 = None
    
    for i, p in enumerate(doc.paragraphs):
        txt = p.text.strip().replace('\u200b', '')
        if "การติดตั้งระบบบนเซิร์ฟเวอร์ Ubuntu Linux ดำเนินการผ่าน Docker Container" in txt:
            target_p_app_d1 = p
        elif "(3) การตั้งค่า LINE Token" in txt:
            target_p_app_d2 = p

    if target_p_app_d1:
        parent1 = target_p_app_d1._p.getparent()
        pos1 = parent1.index(target_p_app_d1._p)
        
        tbl_d1 = build_placeholder_table(
            doc,
            fig_id="รูปที่ ง.1",
            title_desc="ภาพหน้าจอคอนโซลคำสั่งการเริ่มการทำงานของระบบบริการหลังบ้านผ่าน Docker Compose (docker compose pull และ docker compose up -d) บนเซิร์ฟเวอร์ Azure Virtual Machine แสดงสถานะคอนเทนเนอร์ go-backend, frontend-nextjs และ mosquitto-broker ทำงานปกติ (Up/Running)",
            spec_desc="(ขนาดแนะนำ: กว้าง 13–14 ซม., สูง 7–8 ซม., ภาพจับภาพหน้าจอ Terminal คมชัด แนวนอน)"
        )
        p_cap_d1 = build_caption_paragraph(doc, "รูปที่ ง.1", "ภาพหน้าจอคำสั่งการติดตั้งและเริ่มการทำงานของคอนเทนเนอร์ด้วย Docker Compose บนเซิร์ฟเวอร์ Azure VM")
        
        parent1.insert(pos1 + 1, tbl_d1._tbl)
        parent1.insert(pos1 + 2, p_cap_d1._p)
        print("  [+] Inserted รูปที่ ง.1 successfully.")

    if target_p_app_d2:
        parent2 = target_p_app_d2._p.getparent()
        pos2 = parent2.index(target_p_app_d2._p)
        
        tbl_d2 = build_placeholder_table(
            doc,
            fig_id="รูปที่ ง.2",
            title_desc="ภาพหน้าจอเว็บแอปพลิเคชันแสดงหน้าต่างเข้าสู่ระบบ (Login Page) และหน้ามอนิเตอร์ภาพรวมของระบบตรวจจับการล้มบนโดเมน fall-detection.sopon-project.me แสดงการเชื่อมต่อ WebSocket และสถานะห้องน้ำแบบเรียลไทม์",
            spec_desc="(ขนาดแนะนำ: กว้าง 13–14 ซม., สูง 7–8 ซม., ภาพจับภาพหน้าจอเว็บเบราว์เซอร์ คมชัด แนวนอน)"
        )
        p_cap_d2 = build_caption_paragraph(doc, "รูปที่ ง.2", "ภาพหน้าจอการเข้าสู่ระบบ (Login) และการเชื่อมต่อแดชบอร์ดของผู้ใช้งาน")
        
        parent2.insert(pos2 + 1, tbl_d2._tbl)
        parent2.insert(pos2 + 2, p_cap_d2._p)
        print("  [+] Inserted รูปที่ ง.2 successfully.")

    # -------------------------------------------------------------------------
    # 6. UPDATE TABLE 4 (สารบัญภาพ - LIST OF FIGURES)
    # -------------------------------------------------------------------------
    print("\n6. Updating Table 4 (สารบัญภาพ - List of Figures)...")
    tbl_figures = None
    for t in doc.tables:
        c00 = t.rows[0].cells[0].text.strip() if len(t.rows) > 0 and len(t.columns) > 0 else ""
        if c00 in ["รูป", "รูปที่"]:
            tbl_figures = t
            break
            
    if tbl_figures:
        print(f"  Current rows in Table 4: {len(tbl_figures.rows)}")
        # Let's inspect current list of figure keys
        current_rows = []
        for r in tbl_figures.rows[1:]:
            txt = r.cells[0].text.strip().replace('\u200b', '')
            pnum = r.cells[1].text.strip()
            current_rows.append((txt, pnum))
            
        # We need a complete, cleanly sorted List of Figures including the new ones
        full_figure_list = [
            ("รูปที่ 2.1  หลักการส่งและรับสัญญาณเรดาร์แบบคลื่นต่อเนื่องมอดูเลตความถี่ (FMCW Chirp)", "5"),
            ("รูปที่ 2.2  โมดูลเรดาร์คลื่นมิลลิเมตร 60 GHz รุ่น HLK-LD6002C", "7"),
            ("รูปที่ 2.3  โมดูลเรดาร์ตรวจจับบุคคล 24 GHz รุ่น HLK-LD2410C", "13"),
            ("รูปที่ 2.4  บอร์ดไมโครคอนโทรลเลอร์ ESP32-S3 DevKitC-1", "14"),
            ("รูปที่ 2.5  สถาปัตยกรรม Publish/Subscribe ของโปรโตคอล MQTT", "16"),
            ("รูปที่ 2.6  ผังการทำงานของกลไก Webhook สำหรับส่งการแจ้งเตือนไปยัง LINE Messaging API", "17"),
            ("รูปที่ 3.1  แผนภาพขั้นตอนการดำเนินงานวิจัยและพัฒนาระบบ", "25"),
            ("รูปที่ 3.2  แผนผังสถาปัตยกรรมระบบรวม (End-to-End System Architecture)", "27"),
            ("รูปที่ 3.3  ผังวงจรไฟฟ้า (Schematic Circuit Diagram) ของอุปกรณ์ตรวจจับ", "28"),
            ("รูปที่ 3.4  การออกแบบลายวงจรพิมพ์ (PCB Layout) ของอุปกรณ์ตรวจจับการล้ม", "31"),
            ("รูปที่ 3.5  แผนภาพสถานะการทำงานของระบบ (Finite State Machine Flowchart)", "33"),
            ("รูปที่ 3.6  แผนผังอัลกอริทึมการกรองสัญญาณและการยืนยันการล้ม (Fall Confirmation Delay)", "34"),
            ("รูปที่ 3.7  ผังความสัมพันธ์เอนทิตีของฐานข้อมูล (Entity-Relationship Diagram: ERD)", "36"),
            ("รูปที่ 3.8  ภาพจำลองส่วนติดต่อผู้ใช้เว็บแอปพลิเคชันแดชบอร์ด (Next.js Dashboard UI)", "38"),
            ("รูปที่ 3.9  แผนผังการจัดสภาพแวดล้อมห้องน้ำทดสอบและตำแหน่งการติดตั้งอุปกรณ์ต้นแบบ", "40"),
            ("รูปที่ 4.1  ชิ้นงานแผ่นวงจรพิมพ์จริง (PCB Assembly) ของระบบตรวจจับการล้มในห้องน้ำ", "41"),
            ("รูปที่ 4.2  ผังการทำงานของเครื่องสถานะจำกัด (Finite State Machine)", "44"),
            ("รูปที่ 4.3  ภาพจำลองข้อความแจ้งเตือนภัยฉุกเฉิน (LINE Flex Message) บนแอปพลิเคชัน LINE", "45"),
            ("รูปที่ 4.4  ภาพหน้าจอเว็บแอปพลิเคชันแดชบอร์ด (Next.js Dashboard)", "46"),
            ("รูปที่ 4.5  แผนผังการจัดสภาพแวดล้อมห้องน้ำทดสอบและกรอบการจำลองท่าทางการทดสอบ", "53"),
            ("รูปที่ 4.6  ภาพถ่ายการจำลองท่าทางการหกล้ม 4 รูปแบบในห้องน้ำทดสอบ", "54"),
            ("รูปที่ 4.7  ภาพถ่ายการทดสอบกิจกรรมปกติในชีวิตประจำวัน (ADLs) และกิจกรรมที่เสี่ยงต่อการเกิดสัญญาณเตือนหลอก", "55"),
            ("รูปที่ 4.8  ภาพถ่ายการทดสอบประสิทธิภาพการตรวจจับบุคคลด้วยเรดาร์ 24 GHz ในสภาวะต่าง ๆ ภายในห้องน้ำ", "56"),
            ("รูปที่ 4.9  ภาพถ่ายการตอบสนองของระบบแจ้งเตือนภัยแบบเรียลไทม์เมื่อตรวจพบเหตุการณ์หกล้มจริง", "57"),
            ("รูปที่ ก.1  แผ่นวงจรพิมพ์หน้าเดียว (Single-sided Copper PCB) ที่ผ่านการกัดลายทองแดงจริง", "59"),
            ("รูปที่ ก.2  การประกอบแผงวงจรและเชื่อมต่อเซนเซอร์เรดาร์ HLK-LD6002C, สัญญาณไซเรนเตือนภัย และเคสติดตั้ง", "59"),
            ("รูปที่ ค.1  ภาพข้อความแจ้งเตือนเหตุการณ์ฉุกเฉินบนสมาร์ตโฟนผ่านแอปพลิเคชัน LINE", "63"),
            ("รูปที่ ง.1  ภาพหน้าจอคำสั่งการติดตั้งและเริ่มการทำงานของคอนเทนเนอร์ด้วย Docker Compose บนเซิร์ฟเวอร์ Azure VM", "65"),
            ("รูปที่ ง.2  ภาพหน้าจอการเข้าสู่ระบบ (Login) และการเชื่อมต่อแดชบอร์ดของผู้ใช้งาน", "65"),
        ]
        
        # Clear existing data rows
        while len(tbl_figures.rows) > 1:
            tbl_figures._tbl.remove(tbl_figures.rows[1]._tr)
            
        # Re-populate
        for title, pnum in full_figure_list:
            row = tbl_figures.add_row()
            c0, c1 = row.cells[0], row.cells[1]
            c0.width = Cm(13.5)
            c1.width = Cm(1.15)
            
            p0 = c0.paragraphs[0]
            p0.alignment = WD_ALIGN_PARAGRAPH.LEFT
            p0.paragraph_format.space_before = Pt(0)
            p0.paragraph_format.space_after = Pt(0)
            p0.paragraph_format.line_spacing = 1.0
            r0 = p0.add_run(title)
            set_font(r0, bold=False, font_size=16)
            
            p1 = c1.paragraphs[0]
            p1.alignment = WD_ALIGN_PARAGRAPH.RIGHT
            p1.paragraph_format.space_before = Pt(0)
            p1.paragraph_format.space_after = Pt(0)
            p1.paragraph_format.line_spacing = 1.0
            r1 = p1.add_run(pnum)
            set_font(r1, bold=False, font_size=16)
            
        print(f"  [+] Rebuilt Table 4 with {len(full_figure_list)} figures.")
    else:
        print("  [!] Could not locate Table 4 (List of Figures)!")

    # Save modified document
    doc.save(DOCX_PATH)
    print(f"\nSaved updated document to {DOCX_PATH}")

if __name__ == "__main__":
    main()
