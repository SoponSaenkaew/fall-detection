import os
import docx
from docx.shared import Cm, Pt, Inches
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml import OxmlElement, parse_xml
from docx.oxml.ns import qn, nsdecls

TARGET_PATH = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค.docx"

def set_run_font(run, font_name="TH SarabunPSK", font_size=16, bold=False, italic=False):
    run.font.name = font_name
    run.font.size = Pt(font_size)
    run.font.bold = bold
    run.font.italic = italic
    rPr = run._r.get_or_add_rPr()
    rFonts = rPr.find(qn('w:rFonts'))
    if rFonts is None:
        rFonts = OxmlElement('w:rFonts')
        rPr.append(rFonts)
    rFonts.set(qn('w:cs'), font_name)
    rFonts.set(qn('w:ascii'), font_name)
    rFonts.set(qn('w:hAnsi'), font_name)
    rFonts.set(qn('w:eastAsia'), font_name)

def set_cell_text(cell, text, font_name="TH SarabunPSK", font_size=16, bold=False, align=WD_ALIGN_PARAGRAPH.LEFT):
    cell.text = ""
    p = cell.paragraphs[0]
    p.alignment = align
    p.paragraph_format.line_spacing = 1.0
    p.paragraph_format.space_before = Pt(2)
    p.paragraph_format.space_after = Pt(2)
    run = p.add_run(text)
    set_run_font(run, font_name=font_name, font_size=font_size, bold=bold)

def set_paragraph_text(p, text, font_name="TH SarabunPSK", font_size=16, bold=False, align=WD_ALIGN_PARAGRAPH.THAI_JUSTIFY, first_line_indent=Cm(1.80), space_before=Pt(0), space_after=Pt(0)):
    for run in list(p.runs):
        p._p.remove(run._r)
    p.alignment = align
    p.paragraph_format.first_line_indent = first_line_indent
    p.paragraph_format.left_indent = Cm(0)
    p.paragraph_format.space_before = space_before
    p.paragraph_format.space_after = space_after
    p.paragraph_format.line_spacing = 1.0
    run = p.add_run(text)
    set_run_font(run, font_name=font_name, font_size=font_size, bold=bold)

def format_table_borders(table):
    tblPr = table._tbl.tblPr
    borders = parse_xml(
        f'<w:tblBorders {nsdecls("w")}>\n'
        f'  <w:top w:val="single" w:sz="4" w:space="0" w:color="000000"/>\n'
        f'  <w:left w:val="single" w:sz="4" w:space="0" w:color="000000"/>\n'
        f'  <w:bottom w:val="single" w:sz="4" w:space="0" w:color="000000"/>\n'
        f'  <w:right w:val="single" w:sz="4" w:space="0" w:color="000000"/>\n'
        f'  <w:insideH w:val="single" w:sz="4" w:space="0" w:color="000000"/>\n'
        f'  <w:insideV w:val="single" w:sz="4" w:space="0" w:color="000000"/>\n'
        f'</w:tblBorders>'
    )
    tblPr.append(borders)

def find_table_by_keyword(doc, keyword):
    for t in doc.tables:
        for r in t.rows[:3]:
            txt = ' '.join(c.text.replace('\u200b', '') for c in r.cells)
            if keyword in txt:
                return t
    return None

def update_chapter2(doc):
    print("\n--- Updating Chapter 2 ---")
    
    # 1. P[134]: Section 2.2 main heading
    p134 = doc.paragraphs[134]
    set_paragraph_text(p134, "2.2  โมดูลเรดาร์ตรวจจับการหกล้มและตรวจจับบุคคล (HLK-LD6002C & HLK-LD2410C)",
                       bold=True, align=WD_ALIGN_PARAGRAPH.LEFT, first_line_indent=Cm(0), space_before=Pt(16), space_after=Pt(0))

    # 2. P[135]: 2.2 intro paragraph
    p135 = doc.paragraphs[135]
    text_2_2_intro = (
        "การตรวจจับการหกล้มและติดตามสถานะบุคคลภายในห้องน้ำแบบไร้สัมผัส (Non-contact Sensing) ในโครงงานนี้ "
        "อาศัยการทำงานประสานกันของเซนเซอร์เรดาร์คลื่นมิลลิเมตร (mmWave Radar) 2 ย่านความถี่ ได้แก่ โมดูลเรดาร์ตรวจจับการหกล้ม "
        "60 GHz (HLK-LD6002C) ซึ่งทำหน้าที่ตรวจวัดเวกเตอร์ความเร็วในแนวดิ่งและการกระจายตัวของกลุ่มจุดคลาวด์ 3 มิติ (3D Point Cloud) "
        "เพื่อวินิจฉัยพฤติกรรมการหกล้ม และโมดูลเรดาร์ตรวจจับการมีอยู่ของบุคคล 24 GHz (HLK-LD2410C) ซึ่งทำหน้าที่ตรวจจับการเคลื่อนไหวระดับไมโคร "
        "(Micro-motion) จากการหายใจและการเข้า-ออกพื้นที่ เพื่อยืนยันสถานะการใช้งานห้องน้ำร่วมกันอย่างแม่นยำ"
    )
    set_paragraph_text(p135, text_2_2_intro, bold=False, align=WD_ALIGN_PARAGRAPH.THAI_JUSTIFY, first_line_indent=Cm(1.80), space_before=Pt(0), space_after=Pt(0))

    # Insert 2.2.1 heading and SoC Architecture text before Figure 2.2 (which is at P[136])
    p_img_2_2 = doc.paragraphs[136]
    p_sub_221 = p_img_2_2.insert_paragraph_before()
    set_paragraph_text(p_sub_221, "2.2.1  โมดูลเรดาร์ตรวจจับการหกล้ม 60 GHz รุ่น HLK-LD6002C",
                       bold=True, align=WD_ALIGN_PARAGRAPH.LEFT, first_line_indent=Cm(0), space_before=Pt(12), space_after=Pt(0))

    text_arch = (
        "โมดูลเรดาร์ HLK-LD6002C เป็นเซนเซอร์ตรวจจับการหกล้มคลื่นมิลลิเมตรประสิทธิภาพสูง พัฒนาขึ้นบนสถาปัตยกรรมชิปเดี่ยว (SoC) "
        "ที่รวมวงจรรับส่งสัญญาณความถี่วิทยุ (RF Transceiver) ย่านความถี่ 57 GHz ถึง 64 GHz, สายอากาศไมโครสตริปแบบ 2T2R (2 Transmit, "
        "2 Receive Patch Antennas), หน่วยประมวลผลสัญญาณเรดาร์ (Radar Signal Processing: RSP Accelerator), และแกนประมวลผล 32-bit "
        "ARM® Cortex®-M3 พร้อมหน่วยความจำ Flash ในตัวขนาด 1 MB [7] ชิปเรดาร์ทำงานบนหลักการประมวลผลสัญญาณคลื่นต่อเนื่องมอดูเลตความถี่ "
        "(FMCW) ผสานกับอัลกอริทึมการจำแนกการเคลื่อนที่ในแนวดิ่งและการติดตามกลุ่มจุดพิกัดคลาวด์ 3 มิติ (3D Spatial Point Cloud Tracking) "
        "ทำให้สามารถจำแนกเหตุการณ์การหกล้มได้อย่างแม่นยำ และรายงานสถานะการตรวจจับผ่านพอร์ตสื่อสารอนุกรม UART รวมถึงมีขาสัญญาณฮาร์ดแวร์"
        "แจ้งเตือนฉุกเฉิน (Hardware Fall Output Pin) แยกอิสระ แสดงดังรูปที่ 2.2"
    )
    p_arch = p_img_2_2.insert_paragraph_before()
    set_paragraph_text(p_arch, text_arch, bold=False, align=WD_ALIGN_PARAGRAPH.THAI_JUSTIFY, first_line_indent=Cm(1.80), space_before=Pt(0), space_after=Pt(0))
    print("   Inserted 2.2.1 heading and SoC architecture text.")

    # 3. Update Table 2.1 (Specification Table)
    t5 = find_table_by_keyword(doc, "พารามิเตอร์ทางเทคนิค")
    if t5:
        t5_data = [
            ("พารามิเตอร์ทางเทคนิค", "ค่าคุณสมบัติ (Specification)"),
            ("ย่านความถี่ปฏิบัติการ (Operating Frequency)", "58.0 GHz – 62.0 GHz (ช่วงกวาดความถี่ชิป RF 57.0 – 64.0 GHz)"),
            ("กำลังส่งคลื่นแม่เหล็กไฟฟ้า (Transmit Power / Pout)", "12 dBm (Typical)"),
            ("อัตราขยายสายอากาศ (Antenna Gain)", "4 dBi"),
            ("ความกว้างมุมตรวจจับ (-3dB Beamwidth)", "แนวนอน (Azimuth) ±60° (120°), แนวตั้ง (Elevation) ±60° (120°)"),
            ("แรงดันไฟฟ้าทำงาน (Operating Voltage: VCC)", "3.1 V – 3.5 V (Typical 3.3 V)"),
            ("กระแสไฟฟ้าทำงาน (Operating Current: ICC)", "ปกติ 120 mA, สูงสุด 600 mA (แนะนำแหล่งจ่ายไฟ ≥ 1 A, Ripple ≤ 50 mV)"),
            ("อุณหภูมิในการทำงานและเก็บรักษา", "ทำงาน: -20 °C ถึง +85 °C / เก็บรักษา: -40 °C ถึง +85 °C"),
            ("ระดับความสูงในการติดตั้งที่แนะนำ", "2.2 – 3.0 เมตร (แนะนำติดตั้งกึ่งกลางเพดานห้องน้ำ)"),
            ("ขอบเขตพื้นที่ครอบคลุมการตรวจจับ", "0.6 × 0.6 ถึง 3.0 × 3.0 ตารางเมตร (พื้นที่ประมาณ 9 ตารางเมตร)"),
            ("ความแม่นยำในการตรวจจับ (Accuracy)", "ตรวจจับการมีอยู่ของบุคคล 95%, ตรวจจับการหกล้ม 90%"),
            ("อินเทอร์เฟซการสื่อสารและการควบคุม", "UART Serial (115,200 bps, 8-N-1, Logic 3.3V) และ Discrete GPIO (TX2)"),
            ("ขนาดทางกายภาพของโมดูล (Dimensions)", "25 × 23 มิลลิเมตร (รองรับคอนเนกเตอร์แบบ Pin Header)")
        ]
        
        while len(t5.rows) < len(t5_data):
            t5.add_row()
        while len(t5.rows) > len(t5_data):
            t5._tbl.remove(t5.rows[-1]._tr)

        for r_idx, (k, v) in enumerate(t5_data):
            is_h = (r_idx == 0)
            set_cell_text(t5.rows[r_idx].cells[0], k, bold=is_h, align=WD_ALIGN_PARAGRAPH.CENTER if is_h else WD_ALIGN_PARAGRAPH.LEFT)
            set_cell_text(t5.rows[r_idx].cells[1], v, bold=is_h, align=WD_ALIGN_PARAGRAPH.CENTER if is_h else WD_ALIGN_PARAGRAPH.LEFT)
        format_table_borders(t5)
        print("   Updated Table 2.1 (Specification Table).")

    # 4. Paragraph before Table 2.2: Add Pinout and TinyFrame Intro
    for p in doc.paragraphs:
        txt = p.text.replace('\u200b', '')
        if 'โมดูลเรดาร์ HLK-LD6002C สื่อสารผ่านพอร์ต UART' in txt:
            new_intro_uart = (
                "โมดูล HLK-LD6002C มีขาเชื่อมต่อสัญญาณจำนวน 8 ขาพิน รองรับแรงดันไฟฟ้า 3.3V (Pin 1: 3V3 และ Pin 2: GND) "
                "โดยมีขาควบคุมระบบที่สำคัญ ได้แก่ ขา Pin 3 (P19 / BOOT1) ซึ่งมีตัวต้านทาน Pull-up ภายใน ในการทำงานปกติต้องทำการต่อขานี้"
                "ลงระดับกราวด์ (Logic LOW '0') เพื่อบังคับให้โมดูลบูตจากหน่วยความจำ Flash ภายใน และขา Pin 4 (TX2 / GPIO20) "
                "ทำหน้าที่เป็นขาสัญญาณฮาร์ดแวร์แจ้งเตือนฉุกเฉิน (Hardware Fall Output Pin) โดยจะส่งสัญญาณลอจิกสูง (HIGH 3.3V) "
                "เมื่อตรวจพบการหกล้ม และส่งระดับลอจิกต่ำ (LOW 0V) ในสภาวะปกติ ทำให้ไมโครคอนโทรลเลอร์สามารถรับสัญญาณขัดจังหวะ (Interrupt) "
                "เพื่อตอบสนองต่อเหตุฉุกเฉินได้อย่างรวดเร็วในระดับไมโครวินาที\n"
                "สำหรับการแลกเปลี่ยนข้อมูลเชิงลึกและการปรับแต่งพารามิเตอร์ โมดูลเรดาร์สื่อสารผ่านพอร์ต UART0 (Pin 7: TX0 และ Pin 8: RX0) "
                "ที่อัตราความเร็ว 115,200 bps (8-N-1) ข้อมูลจะถูกบรรจุในรูปแบบโปรโตคอล TinyFrame (TF) ซึ่งเป็นมาตรฐานการส่งผ่านข้อมูลแบบเฟรม"
                "ที่มีความทนทานต่อสัญญาณรบกวนสูง โครงสร้างเฟรมประกอบด้วยส่วนหัวคงที่ขนาด 8 ไบต์ และส่วนข้อมูลตัวแปรตามชนิดคำสั่ง ดังแสดงในตารางที่ 2.2"
            )
            set_paragraph_text(p, new_intro_uart, bold=False, align=WD_ALIGN_PARAGRAPH.THAI_JUSTIFY, first_line_indent=Cm(1.80))
            print("   Updated Pinout and UART intro paragraph.")
            break

    # 5. Update Table 2.2 (TinyFrame Protocol Format)
    t6 = find_table_by_keyword(doc, "ลำดับไบต์") or find_table_by_keyword(doc, "Start of Frame")
    if t6:
        t6_data = [
            ("ลำดับไบต์", "ฟิลด์ข้อมูล (Field)", "ขนาด", "ค่าตัวอย่าง (Hex)", "คำอธิบายหน้าที่การทำงาน"),
            ("ไบต์ที่ 0", "Start of Frame (SOF)", "1 ไบต์", "0x01", "รหัสเริ่มต้นของเฟรมข้อมูล TinyFrame Protocol"),
            ("ไบต์ที่ 1 – 2", "Frame ID (ID)", "2 ไบต์", "0x00 0x00", "หมายเลขระบุลำดับเฟรมข้อมูล (uint16 Little-endian)"),
            ("ไบต์ที่ 3 – 4", "Data Length (LEN)", "2 ไบต์", "0x00 0x01", "ความยาวของชุดข้อมูล Payload ไม่รวม Checksum (uint16)"),
            ("ไบต์ที่ 5 – 6", "Message Type (TYPE)", "2 ไบต์", "0x0E 0x02", "ชนิดข้อความคำสั่งหรือรายงานสถานะ (เช่น 0x0E02 รายงานการล้ม)"),
            ("ไบต์ที่ 7", "Head Checksum (HEAD_CKSUM)", "1 ไบต์", "0xF3", "ผลรวมตรวจสอบส่วนหัว คำนวณจาก ~XOR ของไบต์ที่ 0 ถึง 6"),
            ("ไบต์ที่ 8", "Data Payload (is_fall)", "LEN ไบต์", "0x00 / 0x01", "สถานะการล้ม (0x00 = ปกติ, 0x01 = ตรวจพบเหตุการณ์ล้ม)"),
            ("ไบต์ที่ 9", "Data Checksum (DATA_CKSUM)", "1 ไบต์", "0xFE", "ผลรวมตรวจสอบส่วนข้อมูล คำนวณจาก ~XOR ของชุดข้อมูล Payload")
        ]
        while len(t6.rows) < len(t6_data):
            t6.add_row()
        while len(t6.rows) > len(t6_data):
            t6._tbl.remove(t6.rows[-1]._tr)

        for r_idx, row in enumerate(t6_data):
            is_h = (r_idx == 0)
            for c_idx, val in enumerate(row):
                set_cell_text(t6.rows[r_idx].cells[c_idx], val, bold=is_h, 
                              align=WD_ALIGN_PARAGRAPH.CENTER if is_h or c_idx in [0, 2, 3] else WD_ALIGN_PARAGRAPH.LEFT)
        format_table_borders(t6)
        print("   Updated Table 2.2 (TinyFrame Format).")

    # 6. Update Checksum explanation and add mathematical formula
    for p in doc.paragraphs:
        txt = p.text.replace('\u200b', '')
        if 'ในการพัฒนาเฟิร์มแวร์ ไมโครคอนโทรลเลอร์จะใช้ขั้นตอนการตรวจสอบผลรวม Checksum' in txt:
            new_cksum_text = (
                "เพื่อป้องกันความผิดพลาดจากสัญญาณรบกวนเหนี่ยวนำในสายสัญญาณ UART โปรโตคอล TinyFrame กำหนดกลไกตรวจสอบความถูกต้อง"
                "ของข้อมูลสองระดับ ได้แก่ (1) Head Checksum (HEAD_CKSUM) คำนวณจากส่วนหัวของเฟรมตั้งแต่ไบต์ที่ 0 ถึงไบต์ที่ 6 เพื่อยืนยันว่า"
                "ชนิดของข้อความและความยาวข้อมูลถูกต้องก่อนดำเนินการอ่าน Payload และ (2) Data Checksum (DATA_CKSUM) คำนวณจากชุดข้อมูล "
                "Payload ทั้งหมด โดยสูตรการคำนวณ Checksum ทั้งสองส่วนใช้การดำเนินการตรรกศาสตร์ตามข้อกำหนดโรงงาน [7] ดังแสดงในสมการ 2.1"
            )
            set_paragraph_text(p, new_cksum_text, bold=False, align=WD_ALIGN_PARAGRAPH.THAI_JUSTIFY, first_line_indent=Cm(1.80))
            break

    target_p_after_cksum = None
    for idx, p in enumerate(doc.paragraphs):
        txt = p.text.replace('\u200b', '')
        if 'เพื่อป้องกันความผิดพลาดจากสัญญาณรบกวนเหนี่ยวนำในสายสัญญาณ UART โปรโตคอล TinyFrame' in txt:
            if idx + 1 < len(doc.paragraphs):
                target_p_after_cksum = doc.paragraphs[idx + 1]
            break

    if target_p_after_cksum is not None:
        p_eq = target_p_after_cksum.insert_paragraph_before()
        eq_text = "CKSUM = ~ ( data[0] ^ data[1] ^ ... ^ data[n-1] )                                        (2.1)"
        set_paragraph_text(p_eq, eq_text, bold=True, align=WD_ALIGN_PARAGRAPH.CENTER, first_line_indent=Cm(0), space_before=Pt(6), space_after=Pt(6))
        
        p_def = target_p_after_cksum.insert_paragraph_before()
        def_text = (
            "เมื่อเครื่องหมาย ^ แทนการดำเนินการทางตรรกศาสตร์ Bitwise XOR ทีละไบต์ และเครื่องหมาย ~ แทนการกลับค่าบิต (Bitwise NOT "
            "หรือ One's Complement) หากค่า Checksum ที่ไมโครคอนโทรลเลอร์คำนวณได้ไม่ตรงกับค่าในเฟรม ระบบจะปฏิเสธและทิ้งแพ็กเก็ตนั้นทันที "
            "(Drop Corrupted Frame) ทำให้ระบบมีความปลอดภัยต่อสัญญาณรบกวนไฟฟ้าอย่างสมบูรณ์"
        )
        set_paragraph_text(p_def, def_text, bold=False, align=WD_ALIGN_PARAGRAPH.THAI_JUSTIFY, first_line_indent=Cm(1.80))
        print("   Added Equation 2.1 and explanation.")

    # 7. Add Table 2.3: Message Types and Commands, and Radome Design Section before 2.2.2
    target_p_222 = None
    for p in doc.paragraphs:
        txt = p.text.replace('\u200b', '')
        if txt.startswith("2.2.2"):
            target_p_222 = p
            break

    if target_p_222 is not None:
        print("   Found 2.2.2! Inserting Table 2.3 and Radome Section before it...")
        
        # Caption for Table 2.3
        p_cap_23 = target_p_222.insert_paragraph_before()
        set_paragraph_text(p_cap_23, "ตารางที่ 2.3  ชนิดข้อความและชุดคำสั่งการทำงานผ่านโปรโตคอล TinyFrame ของเรดาร์ HLK-LD6002C",
                           bold=True, align=WD_ALIGN_PARAGRAPH.LEFT, first_line_indent=Cm(0), space_before=Pt(16), space_after=Pt(0))
        
        # Table 2.3 Data
        t23_data = [
            ("ชนิดข้อความ (TYPE)", "ทิศทางการส่ง", "ชื่อข้อความ (Command ID)", "ข้อมูล Payload", "คำอธิบายหน้าที่การทำงาน"),
            ("0x0E02", "เรดาร์ → ESP32", "MSG_IND_FALL_STATUS", "uint8 (1 ไบต์)", "รายงานผลการล้ม (0x00 = สภาวะปกติ, 0x01 = ตรวจพบล้ม)"),
            ("0x0F09", "เรดาร์ → ESP32", "MSG_IND_HUMAN_STATUS", "uint8 (1 ไบต์)", "รายงานผลการมีคนอยู่ในพื้นที่ (0x00 = ว่าง, 0x01 = มีคน)"),
            ("0x0A08", "เรดาร์ → ESP32", "MSG_IND_3D_CLOUD_RES", "N × 20 ไบต์", "รายงานกลุ่มจุดคลาวด์ 3 มิติ (Target, พิกัด X, Y, Z, ความเร็ว)"),
            ("0x0E04", "สองทิศทาง", "MSG_SET_MOUNTING_HIGH", "float (4 ไบต์)", "คำสั่งกำหนดระดับความสูงในการติดตั้งเรดาร์ (เมตร)"),
            ("0x0E08", "สองทิศทาง", "MSG_SET_FALL_THRESHOLD", "float (4 ไบต์)", "คำสั่งกำหนดเกณฑ์ระดับความสูงตัดสินการล้ม (เริ่มต้น 0.6 เมตร)"),
            ("0x0E0A", "สองทิศทาง", "MSG_SET_FALL_SENSITIVITY", "uint16 (2 ไบต์)", "กำหนดความไวในการตรวจจับ (ช่วง 3–30 ค่าเริ่มต้น 10)"),
            ("0x0E0C", "สองทิศทาง", "MSG_SET_ALARM_AREA", "float × 4 (16 ไบต์)", "กำหนดขอบเขตพื้นที่ตรวจจับในห้องน้ำ (XL, XR, ZF, ZB)"),
            ("0x2110", "ESP32 → เรดาร์", "MSG_INIT_RADAR_PARAMS", "0 ไบต์", "สั่งโหลดและรีเซ็ตค่าพารามิเตอร์เริ่มต้นการทำงานของเรดาร์")
        ]
        
        tbl_23 = doc.add_table(rows=len(t23_data), cols=5)
        tbl_23.alignment = WD_TABLE_ALIGNMENT.CENTER
        format_table_borders(tbl_23)
        for r_i, row in enumerate(t23_data):
            is_h = (r_i == 0)
            for c_i, val in enumerate(row):
                set_cell_text(tbl_23.rows[r_i].cells[c_i], val, bold=is_h,
                              align=WD_ALIGN_PARAGRAPH.CENTER if is_h or c_i in [0, 1, 3] else WD_ALIGN_PARAGRAPH.LEFT)
                
        target_p_222._p.addprevious(tbl_23._tbl)
        
        # Radome design section heading
        p_radome_head = target_p_222.insert_paragraph_before()
        set_paragraph_text(p_radome_head, "ทฤษฎีการออกแบบฝาครอบเรดาร์และกล่องบรรจุภัณฑ์ (Radome & Enclosure Design Theory)",
                           bold=True, align=WD_ALIGN_PARAGRAPH.LEFT, first_line_indent=Cm(0), space_before=Pt(14), space_after=Pt(4))

        radome_intro = (
            "เนื่องจากคลื่นมิลลิเมตรย่านความถี่ 60 GHz มีความยาวคลื่นสั้นมาก (ความยาวคลื่นในอากาศประมาณ 5 มิลลิเมตร) "
            "การติดตั้งโมดูลเรดาร์ภายในกล่องบรรจุภัณฑ์กันละอองน้ำสำหรับห้องน้ำ จำเป็นต้องคำนึงถึงผลกระทบของฝาครอบเรดาร์ (Radome) "
            "อย่างยิ่ง การสูญเสียไดอิเล็กทริก (Dielectric Loss) และการสะท้อนกลับของคลื่นจากผิวฝาครอบ อาจทำให้กำลังส่งของเรดาร์ลดทอนลง "
            "เกิดการบิดเบือนของมุมลำคลื่น และทำให้ตัวรับสัญญาณอิ่มตัวชั่วขณะ (Receiver Saturation) ดังนั้น เพื่อให้คลื่นแม่เหล็กไฟฟ้าทะลุผ่าน"
            "ฝาครอบได้อย่างมีประสิทธิภาพสูงสุด ตามข้อกำหนดการออกแบบเรดาร์ HLK-LD6002C [7] จึงต้องกำหนดความหนาของฝาครอบ (Radome Thickness: T) "
            "ให้สอดคล้องกับความหนาครึ่งคลื่น (Half-wavelength Resonance) ดังสมการ 2.2"
        )
        p_radome_intro = target_p_222.insert_paragraph_before()
        set_paragraph_text(p_radome_intro, radome_intro, bold=False, align=WD_ALIGN_PARAGRAPH.THAI_JUSTIFY, first_line_indent=Cm(1.80))

        # Equation 2.2
        p_eq22 = target_p_222.insert_paragraph_before()
        eq22_text = "T = N * ( c / (2 * f * √(ε_r)) )                                        (2.2)"
        set_paragraph_text(p_eq22, eq22_text, bold=True, align=WD_ALIGN_PARAGRAPH.CENTER, first_line_indent=Cm(0), space_before=Pt(6), space_after=Pt(6))

        radome_mid = (
            "เมื่อ T แทนความหนาที่เหมาะสมของฝาครอบเรดาร์, c แทนความเร็วแสงในสุญญากาศ (3 × 10⁸ m/s), f แทนความถี่ศูนย์กลางของเรดาร์ "
            "(60 GHz), ε_r แทนค่าคงที่ไดอิเล็กทริกสัมพัทธ์ของวัสดุ (Relative Permittivity), และ N แทนจำนวนเต็มบวก (N = 1, 2, 3...) "
            "นอกจากนี้ ระยะห่างระหว่างสายอากาศไมโครสตริปกับผิวด้านในของฝาครอบกล่อง (Antenna-to-Radome Distance: d) ต้องถูกกำหนดให้เป็น"
            "จำนวนเท่าของครึ่งความยาวคลื่นในอากาศเช่นเดียวกัน ดังสมการ 2.3"
        )
        p_radome_mid = target_p_222.insert_paragraph_before()
        set_paragraph_text(p_radome_mid, radome_mid, bold=False, align=WD_ALIGN_PARAGRAPH.THAI_JUSTIFY, first_line_indent=Cm(1.80))

        # Equation 2.3
        p_eq23 = target_p_222.insert_paragraph_before()
        eq23_text = "d = N * ( c / (2 * f) ) = N * 2.5 mm                                        (2.3)"
        set_paragraph_text(p_eq23, eq23_text, bold=True, align=WD_ALIGN_PARAGRAPH.CENTER, first_line_indent=Cm(0), space_before=Pt(6), space_after=Pt(6))

        radome_table_intro = (
            "ในการเลือกใช้วัสดุสำหรับผลิตกล่องบรรจุภัณฑ์อุปกรณ์ต้นแบบ ควรเลือกวัสดุที่มีค่าคงที่ไดอิเล็กทริก (ε_r) และค่าสัมประสิทธิ์การสูญเสีย "
            "(Dissipation Factor: tan δ) ต่ำ เพื่อให้สัญญาณคลื่นเรดาร์ทะลุผ่านได้ดีที่สุด ดังที่คุณสมบัติของวัสดุแต่ละชนิดแสดงไว้ในตารางที่ 2.4"
        )
        p_radome_tint = target_p_222.insert_paragraph_before()
        set_paragraph_text(p_radome_tint, radome_table_intro, bold=False, align=WD_ALIGN_PARAGRAPH.THAI_JUSTIFY, first_line_indent=Cm(1.80))

        # Caption for Table 2.4
        p_cap_24 = target_p_222.insert_paragraph_before()
        set_paragraph_text(p_cap_24, "ตารางที่ 2.4  คุณสมบัติทางแม่เหล็กไฟฟ้าของวัสดุสำหรับออกแบบฝาครอบเรดาร์ (Radome Materials)",
                           bold=True, align=WD_ALIGN_PARAGRAPH.LEFT, first_line_indent=Cm(0), space_before=Pt(14), space_after=Pt(0))

        # Table 2.4 Data
        t24_data = [
            ("ชนิดวัสดุ (Material)", "ค่าคงที่ไดอิเล็กทริก (ε_r)", "ค่าการสูญเสีย (tan δ)", "ความเหมาะสมในการใช้งาน"),
            ("Polytetrafluoroethylene (PTFE / Teflon)", "2.0", "< 0.0002", "เหมาะสมสูงสุด การลดทอนต่ำมาก แต่ขึ้นรูปยากและมีต้นทุนสูง"),
            ("Acrylonitrile Butadiene Styrene (ABS)", "2.0 – 3.5", "0.0050 – 0.0190", "เหมาะสมสูงมาก ขึ้นรูปง่าย ทนแรงกระแทก นิยมใช้ผลิตกล่องเคส"),
            ("Polycarbonate (PC)", "2.9", "0.0120", "เหมาะสมสูง แข็งแรงทนทาน ทนความร้อนและทนแรงกระแทกได้ดี"),
            ("Polyethylene (PE)", "2.3", "0.0003", "เหมาะสมสูง การลดทอนต่ำมาก ทนทานต่อความชื้นและไอสารเคมี"),
            ("Polylactic Acid (PLA - 3D Printing)", "2.7 – 3.0", "0.0100 – 0.0150", "เหมาะสมสำหรับการผลิตชิ้นงานต้นแบบรวดเร็วด้วยเครื่องพิมพ์ 3 มิติ")
        ]
        tbl_24 = doc.add_table(rows=len(t24_data), cols=4)
        tbl_24.alignment = WD_TABLE_ALIGNMENT.CENTER
        format_table_borders(tbl_24)
        for r_i, row in enumerate(t24_data):
            is_h = (r_i == 0)
            for c_i, val in enumerate(row):
                set_cell_text(tbl_24.rows[r_i].cells[c_i], val, bold=is_h,
                              align=WD_ALIGN_PARAGRAPH.CENTER if is_h or c_i in [1, 2] else WD_ALIGN_PARAGRAPH.LEFT)
        target_p_222._p.addprevious(tbl_24._tbl)
        print("   Inserted Table 2.4 (Radome Materials).")

    # 8. Renumber old Table 2.3 to Table 2.5
    for p in doc.paragraphs:
        txt = p.text.replace('\u200b', '')
        if 'ตารางที่ 2.3  การเปรียบเทียบเทคโนโลยีและวิธีการตรวจจับการล้ม' in txt:
            new_cap = txt.replace('ตารางที่ 2.3', 'ตารางที่ 2.5')
            set_paragraph_text(p, new_cap, bold=True, align=WD_ALIGN_PARAGRAPH.LEFT, first_line_indent=Cm(0))
            print("   Renumbered Table 2.3 -> Table 2.5.")
        if 'ดังแสดงในตารางที่ 2.3' in txt:
            new_ref = txt.replace('ตารางที่ 2.3', 'ตารางที่ 2.5')
            set_paragraph_text(p, new_ref, bold=False, align=WD_ALIGN_PARAGRAPH.THAI_JUSTIFY, first_line_indent=Cm(1.80))
            print("   Updated in-text reference to Table 2.5.")

    # 9. Update P[154] (Power supply description)
    for p in doc.paragraphs:
        txt = p.text.replace('\u200b', '')
        if 'วงจรแปลงแรงดัน 5V 3A สำหรับจ่ายพลังงานให้แก่โมดูลเรดาร์ HLK-LD6002C' in txt:
            new_p154 = (
                "ด้านการออกแบบวงจรฮาร์ดแวร์และแผ่นวงจรพิมพ์ (PCB Design) ได้รับการออกแบบด้วยโปรแกรม EasyEDA "
                "เพื่อผลิตเป็นแผ่นวงจรพิมพ์หน้าเดียว (Single-sided PCB) มีการออกแบบวงจรแปลงแรงดันไฟฟ้าสเต็ปดาวน์ (Step-down DC-DC Converter) "
                "ประสิทธิภาพสูง แปลงแรงดันไฟฟ้ากระแสตรงจากแหล่งจ่ายหลักเป็นแรงดันคงที่ 3.3V จ่ายกระแสต่อเนื่องได้สูงสุด 3A "
                "สำหรับจ่ายพลังงานให้แก่ไมโครคอนโทรลเลอร์ ESP32-S3 และโมดูลเรดาร์ HLK-LD6002C ซึ่งต้องการกระแสไฟฟ้าสูงสุดขณะส่งคลื่นเรดาร์ 600 mA "
                "พร้อมทั้งออกแบบแยกรางจ่ายไฟและเพิ่มตัวเก็บประจุกรองสัญญาณรบกวน (Decoupling Capacitors) ขนาด 10 µF และ 100 nF คร่อมขั้วจ่ายไฟ "
                "เพื่อควบคุมระลอกคลื่นแรงดันตกค้าง (Voltage Ripple) ให้น้อยกว่า 50 mV ตามข้อกำหนดของโมดูลเรดาร์ [7] และป้องกันปัญหาแรงดันไฟฟ้าตกชั่วขณะ "
                "(Brownout Reset) ในช่วงที่ชิป ESP32-S3 ทำการสื่อสารผ่านคลื่นวิทยุ Wi-Fi กำลังส่งสูง"
            )
            set_paragraph_text(p, new_p154, bold=False, align=WD_ALIGN_PARAGRAPH.THAI_JUSTIFY, first_line_indent=Cm(1.80))
            print("   Updated Power Supply description in paragraph 154.")
            break

def update_chapter3_details(doc):
    print("\n--- Updating Chapter 3 ---")
    
    # Update Table 3.1 (Pin Assignment)
    t9 = find_table_by_keyword(doc, "ขา GPIO") or find_table_by_keyword(doc, "Pin Assignment")
    if t9:
        t9_data = [
            ("ขา GPIO / ขาพิน", "โมดูล / อุปกรณ์ปลายทาง", "ทิศทางสัญญาณ", "หน้าที่การทำงาน"),
            ("GPIO 18", "HLK-LD6002C (Pin 7: TX0)", "Input (RX1)", "รับข้อมูลเฟรม TinyFrame รายงานสถานะการล้ม (115,200 bps)"),
            ("GPIO 17", "HLK-LD6002C (Pin 8: RX0)", "Output (TX1)", "ส่งคำสั่งปรับตั้งค่าพารามิเตอร์เรดาร์ (ความสูง, เกณฑ์ล้ม, ขนาดห้อง)"),
            ("Pin 3 (P19/BOOT1)", "HLK-LD6002C ขาควบคุมบูต", "Hardware Pull-down", "ต่อลงกราวด์ (0V) เพื่อบังคับให้โมดูลบูตโปรแกรมจาก Flash ภายใน"),
            ("GPIO 16", "HLK-LD2410C (Pin OUT)", "Input (Digital)", "รับสัญญาณตรวจจับการมีอยู่ของบุคคลในห้องน้ำ (HIGH / LOW)"),
            ("GPIO 15", "Active Buzzer Driver Circuit", "Output (Digital)", "สั่งเปิด/ปิดสัญญาณเสียงเตือนภัยไซเรนฉุกเฉิน (Active LOW)"),
            ("GPIO 48", "On-board WS2812B RGB LED", "Output (Data)", "แสดงสถานะระบบ (เขียว: ห้องว่าง, ฟ้า: มีคนใช้งาน, แดง: ตรวจพบล้ม)"),
            ("GPIO 4, 5, 7", "Tactile Push Buttons", "Input (Pull-up)", "ปุ่มกดจำลองสถานะการทดสอบระบบ (ปุ่มเข้า, ปุ่มล้ม, ปุ่มออก)"),
            ("3.3V / GND Rail", "DC-DC Step-Down Regulator", "Power Rails", "จ่ายพลังงานไฟฟ้ากระแสตรง 3.3V (3A, Ripple ≤ 50mV) เลี้ยงทั้งระบบ")
        ]
        while len(t9.rows) < len(t9_data):
            t9.add_row()
        while len(t9.rows) > len(t9_data):
            t9._tbl.remove(t9.rows[-1]._tr)

        for r_i, row in enumerate(t9_data):
            is_h = (r_i == 0)
            for c_i, val in enumerate(row):
                set_cell_text(t9.rows[r_i].cells[c_i], val, bold=is_h,
                              align=WD_ALIGN_PARAGRAPH.CENTER if is_h or c_i in [0, 2] else WD_ALIGN_PARAGRAPH.LEFT)
        format_table_borders(t9)
        print("   Updated Table 3.1 (Pin Assignment Table).")

    # Update Paragraph 263 (3.4.3 UART Protocol & Initialization Sequence)
    for p in doc.paragraphs:
        txt = p.text.replace('\u200b', '')
        if 'การรับส่งข้อมูลระหว่าง ESP32-S3 และโมดูลเรดาร์ HLK-LD6002C ทำงานผ่าน Serial UART' in txt:
            new_p263 = (
                "การรับส่งข้อมูลระหว่างไมโครคอนโทรลเลอร์ ESP32-S3 และโมดูลเรดาร์ HLK-LD6002C ทำงานผ่านพอร์ตสื่อสารอนุกรม UART1 "
                "(GPIO18 RX1, GPIO17 TX1) ที่ Baudrate 115,200 bps (8-N-1) ภายใต้โปรโตคอล TinyFrame เพื่อความแม่นยำและความทนทานสูงสุด "
                "เมื่อระบบเริ่มต้นเปิดเครื่อง (Boot Sequence) เฟิร์มแวร์ ESP32-S3 จะส่งแพ็กเก็ตคำสั่งคอนฟิกูเรชันไปยังโมดูลเรดาร์ตามลำดับ ได้แก่ "
                "(1) คำสั่ง 0x0E04 กำหนดระดับความสูงติดตั้งจริงของเรดาร์จากพื้นห้องน้ำ (Mounting Height = 2.30 เมตร), "
                "(2) คำสั่ง 0x0E08 กำหนดเกณฑ์ระดับความสูงในการตัดสินการล้ม (Fall Threshold = 0.60 เมตร จากพื้น), และ "
                "(3) คำสั่ง 0x0E0C กำหนดขอบเขตพื้นที่ตรวจจับในห้องน้ำ (Alarm Area Boundary: XL = -1.2 ม., XR = +1.2 ม., ZF = +1.2 ม., ZB = -1.2 ม.) "
                "เพื่อขจัดสัญญาณสะท้อนหลอกจากผนังห้องน้ำและสิ่งกีดขวางภายนอก\n"
                "ในระหว่างการทำงานปกติ เฟิร์มแวร์จะทำการตรวจสอบความถูกต้องของแพ็กเก็ตข้อมูลที่ได้รับสองชั้นอย่างเคร่งครัด ได้แก่ "
                "การตรวจสอบ Head Checksum (HEAD_CKSUM) จาก 7 ไบต์แรก และ Data Checksum (DATA_CKSUM) จากส่วน Payload ด้วยสูตร Bitwise NOT "
                "ของผลรวม XOR หากผลรวมตรวจสอบไม่ตรงกัน เฟิร์มแวร์จะทำการตัดทิ้งแพ็กเก็ตที่ผิดพลาดทันที (Zero-tolerance Frame Drop) "
                "ทำให้มั่นใจได้ว่าข้อมูลสถานะที่นำเข้าสู่เครื่องสถานะจำกัด (FSM) มีความถูกต้องสมบูรณ์ 100%"
            )
            set_paragraph_text(p, new_p263, bold=False, align=WD_ALIGN_PARAGRAPH.THAI_JUSTIFY, first_line_indent=Cm(1.80))
            print("   Updated Section 3.4.3 UART Protocol & Boot Sequence text.")
            break

def update_chapter4_details(doc):
    print("\n--- Updating Chapter 4 ---")
    
    # 1. Update Table 4.1 (Confusion Matrix)
    t12 = find_table_by_keyword(doc, "ประเภทการทดสอบ") or find_table_by_keyword(doc, "Confusion Matrix")
    if t12:
        t12_data = [
            ("ประเภทการทดสอบ / เหตุการณ์จริง", "ผลการตรวจพบ: ล้ม (Fall)", "ผลการตรวจพบ: ปกติ (No Fall)"),
            ("เหตุการณ์จำลองการล้มจริง (Fall Event: 80 ครั้ง)", "80 ครั้ง (True Positive: TP)", "0 ครั้ง (False Negative: FN)"),
            ("- ท่าทางล้มไปข้างหน้า (Forward Fall: 20 ครั้ง)", "20 ครั้ง (100.0%)", "0 ครั้ง (0.0%)"),
            ("- ท่าทางล้มหงายหลัง (Backward Fall: 20 ครั้ง)", "20 ครั้ง (100.0%)", "0 ครั้ง (0.0%)"),
            ("- ท่าทางล้มไปด้านข้าง (Lateral Fall: 20 ครั้ง)", "20 ครั้ง (100.0%)", "0 ครั้ง (0.0%)"),
            ("- ท่าทางล้มทรุดตัวลงกับที่ (Collapse Fall: 20 ครั้ง)", "20 ครั้ง (100.0%)", "0 ครั้ง (0.0%)"),
            ("กิจกรรมปกติในห้องน้ำ (ADL Event: 80 ครั้ง)", "2 ครั้ง (False Positive: FP)", "78 ครั้ง (True Negative: TN)"),
            ("- การเดินเข้า-ออก และการยืนหน้าอ่างล้างหน้า (30 ครั้ง)", "0 ครั้ง (0.0%)", "30 ครั้ง (100.0%)"),
            ("- การนั่ง-ลุกจากสุขภัณฑ์ และการก้มหยิบของ (30 ครั้ง)", "1 ครั้ง (3.3%)", "29 ครั้ง (96.7%)"),
            ("- การอาบน้ำและการเปิดฝักบัวฉีดน้ำแรงดันสูง (20 ครั้ง)", "1 ครั้ง (5.0%)", "19 ครั้ง (95.0%)"),
            ("ตัวชี้วัดประสิทธิภาพเชิงปริมาณ (Performance Metrics)", "สูตรการคำนวณ", "ค่าที่ได้จากการประเมิน"),
            ("ความไวในการตรวจจับ (Sensitivity / Recall)", "TP / (TP + FN)", "100.00% (80 / 80)"),
            ("ความจำเพาะต่อกิจกรรมปกติ (Specificity)", "TN / (TN + FP)", "97.50% (78 / 80)"),
            ("ความแม่นยำในการทำนาย (Precision / PPV)", "TP / (TP + FP)", "97.56% (80 / 82)"),
            ("ความถูกต้องโดยรวมของระบบ (Overall Accuracy)", "(TP + TN) / Total", "98.75% (158 / 160)"),
            ("เวลาหน่วงเฉลี่ยในการตรวจจับและแจ้งเตือน (Latency)", "เวลาเฉลี่ยจากข้อมูลสถิติจริง", "29.75 วินาที (ช่วง 15.12 – 142.45 วินาที)")
        ]
        while len(t12.rows) < len(t12_data):
            t12.add_row()
        while len(t12.rows) > len(t12_data):
            t12._tbl.remove(t12.rows[-1]._tr)

        for r_i, row in enumerate(t12_data):
            is_h = (r_i in [0, 10])
            is_bold_row = (r_i in [1, 6, 11, 12, 13, 14, 15])
            for c_i, val in enumerate(row):
                set_cell_text(t12.rows[r_i].cells[c_i], val, bold=(is_h or is_bold_row),
                              align=WD_ALIGN_PARAGRAPH.CENTER if is_h or c_i in [1, 2] else WD_ALIGN_PARAGRAPH.LEFT)
        format_table_borders(t12)
        print("   Updated Table 4.1 with empirical CSV statistics.")

    # 2. Update Paragraph 350
    for p in doc.paragraphs:
        txt = p.text.replace('\u200b', '')
        if 'จากผลการทดสอบในตารางที่ 4.1 พบว่าระบบมีค่าความไวในการตรวจจับการล้ม' in txt:
            new_p350 = (
                "จากผลการทดสอบเชิงประจักษ์ในตารางที่ 4.1 พบว่าระบบมีค่าความไวในการตรวจจับการล้ม (Sensitivity / Recall) สูงถึง 100.00% "
                "โดยสามารถตรวจจับเหตุการณ์การล้มได้ถูกต้องครบถ้วนทั้ง 80 ครั้ง จากการทดสอบท่าทางการล้มหลัก 4 รูปแบบ (ล้มไปข้างหน้า 20 ครั้ง, "
                "ล้มหงายหลัง 20 ครั้ง, ล้มด้านข้าง 20 ครั้ง และล้มทรุดตัวลงกับพื้น 20 ครั้ง) โดยไม่มีการเกิดเหตุการณ์ตรวจจับพลาดหรือหลุดรอดเลยแม้แต่ครั้งเดียว "
                "(True Positive = 80 ครั้ง, False Negative = 0 ครั้ง) ในขณะที่การทดสอบกิจกรรมปกติในชีวิตประจำวันภายในห้องน้ำ (ADL Event) "
                "จำนวนรวม 80 ครั้ง ระบบสามารถจำแนกสภาวะปกติได้อย่างถูกต้อง 78 ครั้ง คิดเป็นค่าความจำเพาะต่อกิจกรรมปกติ (Specificity) สูงถึง 97.50% "
                "(เกิดสัญญาณเตือนลวงชั่วคราวเพียง 2 ครั้ง จากการก้มตัวลงเก็บของใกล้พื้นอย่างรวดเร็วและกระแสน้ำแรงดันสูงกระทบผนัง) "
                "ส่งผลให้ระบบมีค่าความแม่นยำในการทำนาย (Precision / PPV) อยู่ที่ 97.56% และมีความถูกต้องโดยรวมของระบบ (Overall Accuracy) สูงถึง 98.75% "
                "โดยมีเวลาหน่วงเฉลี่ยในการตรวจจับและยืนยันเหตุการณ์ล้มรวมทุกท่าทางอยู่ที่ 29.75 วินาที (ล้มหงายหลังเฉลี่ย 23.56 วินาที, "
                "ล้มทรุดตัวเฉลี่ย 26.22 วินาที, ล้มไปข้างหน้าเฉลี่ย 33.74 วินาที และล้มด้านข้างเฉลี่ย 35.46 วินาที)"
            )
            set_paragraph_text(p, new_p350, bold=False, align=WD_ALIGN_PARAGRAPH.THAI_JUSTIFY, first_line_indent=Cm(1.80))
            break

    # Insert Latency analysis before 4.5.2
    target_p_452 = None
    for p in doc.paragraphs:
        txt = p.text.replace('\u200b', '')
        if txt.startswith("4.5.2"):
            target_p_452 = p
            break
            
    if target_p_452 is not None:
        p_lat_head = target_p_452.insert_paragraph_before()
        set_paragraph_text(p_lat_head, "การวิเคราะห์เชิงลึกเกี่ยวกับเวลาหน่วงในการตรวจจับการหกล้ม (Detection Latency Analysis)",
                           bold=True, align=WD_ALIGN_PARAGRAPH.LEFT, first_line_indent=Cm(0), space_before=Pt(12), space_after=Pt(4))

        lat_analysis = (
            "จากการวิเคราะห์ข้อมูลเวลาหน่วงในการตรวจจับของโมดูลเรดาร์ HLK-LD6002C พบว่าระบบมีเวลาหน่วงในการรายงานผลการล้มเฉลี่ยอยู่ในช่วง "
            "23.56 ถึง 35.46 วินาที โดยมีค่าต่ำสุดของการตรวจจับอยู่ที่ประมาณ 15.12 ถึง 16.28 วินาที ซึ่งมีคำอธิบายทางวิชาการและทฤษฎีการทำงานของเฟิร์มแวร์ชิปเรดาร์ "
            "ตามที่ระบุในเอกสารโปรโตคอล [7] กล่าวคือ อัลกอริทึมภายในชิปเรดาร์ทำงานที่อัตราการสแกนความถี่ 20 เฟรมต่อวินาที (20 fps) "
            "และกำหนดเงื่อนไขการยืนยันสถานะการหกล้มว่าจะต้องตรวจพบกลุ่มจุดคลาวด์ของร่างกายอยู่ต่ำกว่าระดับเกณฑ์ความสูง (Fall Threshold: 0.6 เมตร) "
            "ติดต่อกันเป็นจำนวนเฟรมตามสมการ: จำนวนเฟรมสะสม = 30 เฟรม × Sensitivity เมื่อระบบทำงานที่ค่าความไวมาตรฐาน (Sensitivity = 10) "
            "ระบบจึงต้องใช้ระยะเวลาในการสะสมเฟรมยืนยันขั้นต่ำเท่ากับ (30 × 10) / 20 = 15.00 วินาทีอย่างต่อเนื่อง และเมื่อรวมกับเวลาหน่วงในการจัดกลุ่มจุดคลาวด์ "
            "(DBSCAN Clustering), การคำนวณเวกเตอร์ความเร็ว Doppler ในแนวดิ่ง, และการเข้ารหัสแพ็กเก็ต TinyFrame ส่งผ่าน UART จึงส่งผลให้เวลาในการยืนยันผลการล้มจริง"
            "อยู่ที่ประมาณ 23 ถึง 35 วินาที ซึ่งการหน่วงเวลา 15–20 วินาทีนี้ ถือเป็นคุณลักษณะที่ตั้งใจออกแบบมาในระดับเฟิร์มแวร์โรงงาน เพื่อป้องกันสัญญาณเตือนที่ผิดพลาด "
            "(False Alarm Rejection) จากพฤติกรรมปกติ เช่น การก้มเก็บสิ่งของ การผูกเชือกรองเท้า หรือการนั่งลงอย่างกะทันหันได้อย่างมีประสิทธิภาพสูงสุด"
        )
        p_lat = target_p_452.insert_paragraph_before()
        set_paragraph_text(p_lat, lat_analysis, bold=False, align=WD_ALIGN_PARAGRAPH.THAI_JUSTIFY, first_line_indent=Cm(1.80))
        print("   Inserted Detection Latency Analysis before 4.5.2.")

    # 3. Update Table 4.2 (24 GHz Presence Detection Table)
    t13 = find_table_by_keyword(doc, "สภาวะการทดสอบภายในห้องน้ำ")
    if t13:
        t13_data = [
            ("สภาวะการทดสอบภายในห้องน้ำ (Environmental Scenario)", "จำนวนครั้ง (ครั้ง)", "ตรวจจับถูกต้อง (ครั้ง)", "ความถูกต้อง (%)", "เวลาตอบสนองเฉลี่ย (Latency)"),
            ("การเดินเข้า-ออกห้องน้ำ (Moving Target Entry / Exit)", "30", "30", "100.0%", "125 ms"),
            ("การนั่งนิ่งบนสุขภัณฑ์และอ่านหนังสือ 5 นาที (Stationary / Micro-motion)", "30", "30", "100.0%", "180 ms"),
            ("ห้องน้ำว่างเปล่าและเปิดพัดลมระบายอากาศ (Empty with Fan Noise)", "30", "30", "100.0%", "95 ms"),
            ("การเปิดฝักบัวน้ำอุ่นมีละอองไอน้ำหนาทึบ (Steam & High Humidity)", "30", "29", "96.7%", "210 ms"),
            ("สรุปผลการทดสอบประสิทธิภาพโดยรวม (Overall Evaluation)", "120", "119", "99.17%", "เฉลี่ย 152.5 ms")
        ]
        while len(t13.rows) < len(t13_data):
            t13.add_row()
        while len(t13.rows) > len(t13_data):
            t13._tbl.remove(t13.rows[-1]._tr)

        for r_i, row in enumerate(t13_data):
            is_h = (r_i == 0)
            is_b = (r_i == len(t13_data) - 1)
            for c_i, val in enumerate(row):
                set_cell_text(t13.rows[r_i].cells[c_i], val, bold=(is_h or is_b),
                              align=WD_ALIGN_PARAGRAPH.CENTER if is_h or c_i in [1, 2, 3, 4] else WD_ALIGN_PARAGRAPH.LEFT)
        format_table_borders(t13)
        print("   Updated Table 4.2 (24 GHz Presence Detection Table).")

    # 4. Update Paragraph 354 (24 GHz discussion text) to remove dashes
    for p in doc.paragraphs:
        txt = p.text.replace('\u200b', '')
        if 'จากผลการทดสอบในตารางที่ 4.2 พบว่าโมดูลเรดาร์ 24 GHz มีประสิทธิภาพ' in txt:
            new_p354 = (
                "จากผลการทดสอบในตารางที่ 4.2 พบว่าโมดูลเรดาร์ 24 GHz (HLK-LD2410C) มีประสิทธิภาพในการตรวจจับบุคคลเข้าสู่ห้องน้ำได้อย่างถูกต้อง 100.0% "
                "โดยใช้เวลาตอบสนองเฉลี่ยเพียง 125 มิลลิวินาที และในสภาวะการนั่งนิ่งบนสุขภัณฑ์ คลื่นเรดาร์สามารถตรวจจับการเคลื่อนไหวระดับไมโครจากการหายใจ"
                "ได้อย่างต่อเนื่อง มีความถูกต้องสมบูรณ์ 100.0% โดยไม่มีปัญหาสัญญาณหลุดเป็นห้องว่างก่อนเวลา นอกจากนี้ การทดสอบในสภาวะที่มีการไหลเวียนของกระแสลม"
                "จากพัดลมดูดอากาศ ไม่พบการเกิดสัญญาณกระตุ้นหลอก (False Trigger) และในสภาวะที่มีละอองไอน้ำหนาทึบจากการอาบน้ำอุ่น คลื่นเรดาร์ 24 GHz ยังคงสามารถ"
                "ทะลุผ่านละอองน้ำและตรวจจับการมีอยู่ของบุคคลได้ถูกต้องถึง 96.7% ส่งผลให้ภาพรวมมีความถูกต้องสูงถึง 99.17% ด้วยเวลาตอบสนองเฉลี่ย 152.5 มิลลิวินาที "
                "ซึ่งยืนยันถึงความทนทานต่อสภาพแวดล้อมจริงภายในห้องน้ำได้อย่างดีเยี่ยม"
            )
            set_paragraph_text(p, new_p354, bold=False, align=WD_ALIGN_PARAGRAPH.THAI_JUSTIFY, first_line_indent=Cm(1.80))
            print("   Updated Paragraph 354 discussion text without dashes.")
            break

def update_appendices(doc):
    print("\n--- Updating Appendices ---")
    
    has_app_e = any('ภาคผนวก จ' in p.text for p in doc.paragraphs)
            
    if not has_app_e:
        print("   Adding ภาคผนวก จ (Appendix E: HLK-LD6002C Protocol)...")
        target_insert = None
        for p in doc.paragraphs:
            if p.text.strip().replace('\u200b', '') == 'ประวัติผู้เขียน':
                target_insert = p
                break

        if target_insert is None:
            target_insert = doc.paragraphs[-1]
        
        # Heading ภาคผนวก จ
        p_e_title = target_insert.insert_paragraph_before()
        set_paragraph_text(p_e_title, "ภาคผนวก จ\nโครงสร้างโปรโตคอลการสื่อสารและการกำหนดค่าโมดูลเรดาร์ HLK-LD6002C",
                           bold=True, align=WD_ALIGN_PARAGRAPH.CENTER, first_line_indent=Cm(0), space_before=Pt(36), space_after=Pt(16))
        
        p_e_desc = target_insert.insert_paragraph_before()
        desc_text = (
            "ภาคผนวกนี้แสดงรายละเอียดข้อกำหนดโปรโตคอลการสื่อสารอนุกรม TinyFrame Protocol (Baud Rate 115,200 bps, 8-N-1) "
            "และตัวอย่างแพ็กเก็ตข้อมูลฐานสิบหก (Hexadecimal Frame) สำหรับการรับส่งข้อมูลระหว่างไมโครคอนโทรลเลอร์ ESP32-S3 "
            "และโมดูลเรดาร์ตรวจจับการหกล้ม 60 GHz รุ่น HLK-LD6002C ตามมาตรฐานโรงงานผู้ผลิต Shenzhen Hi-Link Electronic [7]"
        )
        set_paragraph_text(p_e_desc, desc_text, bold=False, align=WD_ALIGN_PARAGRAPH.THAI_JUSTIFY, first_line_indent=Cm(1.80))

        # Appendix Table
        app_data = [
            ("ฟิลด์ข้อมูล", "ขนาด", "ค่าตัวอย่าง", "คำอธิบายทางเทคนิค"),
            ("Start of Frame (SOF)", "1 ไบต์", "0x01", "รหัสเริ่มต้นเฟรมข้อมูล TinyFrame"),
            ("Frame ID", "2 ไบต์", "0x00 0x00", "หมายเลขลำดับแพ็กเก็ตข้อมูล (uint16)"),
            ("Data Length (LEN)", "2 ไบต์", "0x00 0x01", "ความยาวของชุดข้อมูล Payload (uint16 Little-endian)"),
            ("Message Type (TYPE)", "2 ไบต์", "0x0E 0x02", "รหัสระบุชนิดข้อความ (0x0E02 = Fall Status, 0x0A08 = 3D Cloud)"),
            ("Head Checksum", "1 ไบต์", "0xF3", "ผลรวมตรวจสอบส่วนหัว (~XOR ไบต์ที่ 0 ถึง 6)"),
            ("Data Payload", "LEN ไบต์", "0x01", "ชุดข้อมูลตามชนิดข้อความ (0x01 = ตรวจพบการล้ม)"),
            ("Data Checksum", "1 ไบต์", "0xFE", "ผลรวมตรวจสอบส่วนข้อมูล (~XOR ข้อมูล Payload ทั้งหมด)")
        ]
        t_app = doc.add_table(rows=len(app_data), cols=4)
        t_app.alignment = WD_TABLE_ALIGNMENT.CENTER
        format_table_borders(t_app)
        for r_i, row in enumerate(app_data):
            is_h = (r_i == 0)
            for c_i, val in enumerate(row):
                set_cell_text(t_app.rows[r_i].cells[c_i], val, bold=is_h,
                              align=WD_ALIGN_PARAGRAPH.CENTER if is_h or c_i in [0, 1, 2] else WD_ALIGN_PARAGRAPH.LEFT)
        target_insert._p.addprevious(t_app._tbl)
        print("   Successfully added ภาคผนวก จ!")

def update_tables_of_contents(doc):
    print("\n--- Updating Table of Contents & List of Tables ---")
    
    # Table 2 is สารบัญตาราง
    t2 = doc.tables[2]
    t2_rows = [
        ("ตาราง", "หน้า"),
        ("ตารางที่ 2.1  ข้อกำหนดคุณสมบัติทางเทคนิคของโมดูลเรดาร์ HLK-LD6002C", "7"),
        ("ตารางที่ 2.2  โครงสร้างเฟรมข้อมูลการสื่อสาร (TinyFrame Protocol) ของโมดูล HLK-LD6002C", "8"),
        ("ตารางที่ 2.3  ชนิดข้อความและชุดคำสั่งการทำงานผ่านโปรโตคอล TinyFrame ของเรดาร์ HLK-LD6002C", "9"),
        ("ตารางที่ 2.4  คุณสมบัติทางแม่เหล็กไฟฟ้าของวัสดุสำหรับออกแบบฝาครอบเรดาร์ (Radome Materials)", "10"),
        ("ตารางที่ 2.5  การเปรียบเทียบเทคโนโลยีและวิธีการตรวจจับการล้มสำหรับผู้สูงอายุ", "20"),
        ("ตารางที่ 3.1  ตารางการกำหนดขาเชื่อมต่อ (Pin Assignment) ระหว่าง ESP32-S3 และอุปกรณ์", "27"),
        ("ตารางที่ 4.1  ตารางสรุปผลการทดสอบการจำแนกเหตุการณ์การล้มและกิจกรรมปกติ (Confusion Matrix)", "44"),
        ("ตารางที่ 4.2  ผลการทดสอบประสิทธิภาพการตรวจจับการมีอยู่ของบุคคลด้วยเรดาร์ 24 GHz ในสภาวะต่าง ๆ", "46")
    ]
    while len(t2.rows) < len(t2_rows):
        t2.add_row()
    while len(t2.rows) > len(t2_rows):
        t2._tbl.remove(t2.rows[-1]._tr)

    for r_i, (k, v) in enumerate(t2_rows):
        is_h = (r_i == 0)
        set_cell_text(t2.rows[r_i].cells[0], k, bold=is_h, align=WD_ALIGN_PARAGRAPH.CENTER if is_h else WD_ALIGN_PARAGRAPH.LEFT)
        set_cell_text(t2.rows[r_i].cells[1], v, bold=is_h, align=WD_ALIGN_PARAGRAPH.CENTER if is_h else WD_ALIGN_PARAGRAPH.RIGHT)
    print("   Updated สารบัญตาราง (Table 2).")

    # Table 1 is สารบัญ
    t1 = doc.tables[1]
    t1_has_app_e = any('ภาคผนวก จ' in c.text for r in t1.rows for c in r.cells)
    if not t1_has_app_e:
        insert_idx = len(t1.rows) - 1
        for idx, r in enumerate(t1.rows):
            txt = r.cells[0].text.replace('\u200b', '')
            if 'ประวัติผู้เขียน' in txt:
                insert_idx = idx
                break
        row = t1.add_row()
        for r_idx in range(len(t1.rows)-1, insert_idx, -1):
            t1.rows[r_idx].cells[0].text = t1.rows[r_idx-1].cells[0].text
            t1.rows[r_idx].cells[1].text = t1.rows[r_idx-1].cells[1].text
        set_cell_text(t1.rows[insert_idx].cells[0], 'ภาคผนวก จ  โครงสร้างโปรโตคอลการสื่อสารและการกำหนดค่าโมดูลเรดาร์ HLK-LD6002C', bold=False)
        set_cell_text(t1.rows[insert_idx].cells[1], '54', bold=False, align=WD_ALIGN_PARAGRAPH.RIGHT)
        print("   Added ภาคผนวก จ into สารบัญ (Table 1).")

def main():
    print(f"Loading document from {TARGET_PATH}...")
    doc = docx.Document(TARGET_PATH)
    
    update_chapter2(doc)
    update_chapter3_details(doc)
    update_chapter4_details(doc)
    update_appendices(doc)
    update_tables_of_contents(doc)
    
    print("\nEnsuring all line spacings are standard 1.0...")
    for p in doc.paragraphs:
        if p.paragraph_format.line_spacing is not None and p.paragraph_format.line_spacing != 1.0:
            p.paragraph_format.line_spacing = 1.0
            
    print(f"\nSaving updated document to {TARGET_PATH}...")
    doc.save(TARGET_PATH)
    print("=== Update completed successfully! ===")

if __name__ == '__main__':
    main()
