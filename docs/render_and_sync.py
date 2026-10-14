import subprocess
import time
import os
import re
import docx
from docx.shared import Cm, Pt
from docx.enum.text import WD_ALIGN_PARAGRAPH
import pymupdf as fitz

DOCX_PATH = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค.docx"
PDF_PATH = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค_final_render.pdf"

def render_pdf(docx_path, pdf_path):
    print(f"Rendering PDF via Word COM -> {os.path.basename(pdf_path)}...")
    subprocess.run(["powershell", "-Command", "Get-Process -Name WINWORD -ErrorAction SilentlyContinue | Stop-Process -Force"], capture_output=True)
    time.sleep(1)
    
    ps_script = f'''
$wdFormatPDF = 17
$word = New-Object -ComObject Word.Application
$word.Visible = $false
try {{
    $doc = $word.Documents.Open('{docx_path}')
    $doc.SaveAs([ref]'{pdf_path}', [ref]$wdFormatPDF)
    $doc.Close(0)
    Write-Host "PDF rendered successfully!"
}} finally {{
    $word.Quit()
    [System.Runtime.InteropServices.Marshal]::ReleaseComObject($word) | Out-Null
    [System.GC]::Collect()
    [System.GC]::WaitForPendingFinalizers()
}}
'''
    ps_file = os.path.join(os.path.dirname(docx_path), "temp_render.ps1")
    with open(ps_file, "w", encoding="utf-8") as f:
        f.write(ps_script)
        
    res = subprocess.run(["powershell", "-ExecutionPolicy", "Bypass", "-File", ps_file], capture_output=True, text=True)
    if os.path.exists(ps_file):
        os.remove(ps_file)
        
    if "PDF rendered successfully!" in res.stdout:
        print("  [+] PDF compiled successfully.")
        return True
    else:
        print("  [!] Render failed:", res.stdout, res.stderr)
        return False

def scan_pdf_pages(pdf_path):
    print(f"Scanning physical page numbers from {os.path.basename(pdf_path)}...")
    doc = fitz.open(pdf_path)
    total_pages = len(doc)
    print(f"  Total pages in rendered PDF: {total_pages}")
    
    # In RMUTI format:
    # Front matter pages are: 1..13 (Cover, Abstract, Ack, TOC, List of Tables, List of Figures, Symbols)
    # Body pages start at Chapter 1 with page number 1!
    # Let's locate the page index where Chapter 1 ("บทที่ 1") starts.
    ch1_p_idx = None
    for i in range(total_pages):
        txt = doc[i].get_text()
        if "บทที่ 1" in txt and "บทนำ" in txt:
            ch1_p_idx = i
            break
            
    print(f"  Chapter 1 starts at physical page index {ch1_p_idx} (Page {ch1_p_idx + 1})")
    
    # Map function to get printed academic page number from physical page index
    def to_academic(idx):
        return str(idx - ch1_p_idx + 1)
        
    # Search for headings
    headings_to_find = [
        ("บทที่ 1  บทนำ", r"บทที่\s*1\s+บทนำ"),
        ("1.1", r"1\.1\s+ความเป็นมา"),
        ("1.2", r"1\.2\s+วัตถุประสงค์"),
        ("1.3", r"1\.3\s+สมมติฐาน"),
        ("1.4", r"1\.4\s+ข้อตกลงเบื้องต้น"),
        ("1.5", r"1\.5\s+ขอบเขตของโครงงาน"),
        ("1.6", r"1\.6\s+ตัวแปรที่ศึกษา"),
        ("1.7", r"1\.7\s+ประโยชน์ที่คาดว่าจะได้รับ"),
        ("1.8", r"1\.8\s+คำนิยามศัพท์เฉพาะ"),
        ("บทที่ 2  ทฤษฎีและงานวิจัยที่เกี่ยวข้อง", r"บทที่\s*2\s+ทฤษฎี"),
        ("2.1", r"2\.1\s+เทคโนโลยีเรดาร์คลื่นมิลลิเมตร"),
        ("2.2", r"2\.2\s+โมดูลเรดาร์"),
        ("2.3", r"2\.3\s+ไมโครคอนโทรลเลอร์"),
        ("2.4", r"2\.4\s+โปรโตคอลการสื่อสาร"),
        ("2.5", r"2\.5\s+ระบบคลาวด์และฐานข้อมูล"),
        ("2.6", r"2\.6\s+เทคโนโลยีการพัฒนาเว็บแอปพลิเคชัน"),
        ("2.7", r"2\.7\s+งานวิจัยที่เกี่ยวข้อง"),
        ("บทที่ 3  วิธีดำเนินงาน", r"บทที่\s*3\s+วิธีดำเนินงาน"),
        ("3.1", r"3\.1\s+ขั้นตอน"),
        ("3.2", r"3\.2\s+การวิเคราะห์และออกแบบสถาปัตยกรรม"),
        ("3.3", r"3\.3\s+การออกแบบและพัฒนาฮาร์ดแวร์"),
        ("3.4", r"3\.4\s+การพัฒนาเฟิร์มแวร์"),
        ("3.5", r"3\.5\s+การพัฒนาระบบคลาวด์"),
        ("3.6", r"3\.6\s+การพัฒนาเว็บแอปพลิเคชัน"),
        ("3.7", r"3\.7\s+การออกแบบการทดสอบ"),
        ("บทที่ 4  ผลการดำเนินงาน", r"บทที่\s*4\s+ผลการดำเนินงาน"),
        ("4.1", r"4\.1\s+ผลการออกแบบและสร้างแผ่นวงจรพิมพ์"),
        ("4.2", r"4\.2\s+ผลการพัฒนาเฟิร์มแวร์"),
        ("4.3", r"4\.3\s+ผลการพัฒนาระบบบริการหลังบ้าน"),
        ("4.4", r"4\.4\s+ผลการพัฒนาเว็บแอปพลิเคชัน"),
        ("4.5", r"4\.5\s+ผลการทดสอบและประเมินประสิทธิภาพ"),
        ("บทที่ 5  สรุปผล และข้อเสนอแนะ", r"บทที่\s*5\s+สรุปผล"),
        ("5.1", r"5\.1\s+สรุปผลการดำเนินงาน"),
        ("5.2", r"5\.2\s+ปัญหาและอุปสรรค"),
        ("5.3", r"5\.3\s+ข้อเสนอแนะสำหรับ"),
        ("บรรณานุกรม", r"บรรณานุกรม"),
        ("ภาคผนวก ก", r"ภาคผนวก\s*ก"),
        ("ภาคผนวก ข", r"ภาคผนวก\s*ข"),
        ("ภาคผนวก ค", r"ภาคผนวก\s*ค"),
        ("ภาคผนวก ง", r"ภาคผนวก\s*ง"),
        ("ภาคผนวก จ", r"ภาคผนวก\s*จ"),
        ("ประวัติผู้เขียน", r"ประวัติผู้เขียน"),
    ]
    
    h_map = {}
    for key, pat in headings_to_find:
        for idx in range(ch1_p_idx, total_pages):
            if re.search(pat, doc[idx].get_text()):
                h_map[key] = to_academic(idx)
                break
                
    # Scan Tables
    tables_to_find = [
        ("ตารางที่ 2.1", r"ตารางที่\s*2\.1"),
        ("ตารางที่ 2.2", r"ตารางที่\s*2\.2"),
        ("ตารางที่ 2.3", r"ตารางที่\s*2\.3"),
        ("ตารางที่ 2.4", r"ตารางที่\s*2\.4"),
        ("ตารางที่ 2.5", r"ตารางที่\s*2\.5"),
        ("ตารางที่ 3.1", r"ตารางที่\s*3\.1"),
        ("ตารางที่ 4.1", r"ตารางที่\s*4\.1"),
        ("ตารางที่ 4.2", r"ตารางที่\s*4\.2"),
    ]
    t_map = {}
    for key, pat in tables_to_find:
        for idx in range(ch1_p_idx, total_pages):
            if re.search(pat, doc[idx].get_text()):
                t_map[key] = to_academic(idx)
                break

    # Scan Figures
    figures_to_find = [
        ("รูปที่ 2.1", r"รูปที่\s*2\.1"),
        ("รูปที่ 2.2", r"รูปที่\s*2\.2"),
        ("รูปที่ 2.3", r"รูปที่\s*2\.3"),
        ("รูปที่ 2.4", r"รูปที่\s*2\.4"),
        ("รูปที่ 2.5", r"รูปที่\s*2\.5"),
        ("รูปที่ 2.6", r"รูปที่\s*2\.6"),
        ("รูปที่ 3.1", r"รูปที่\s*3\.1"),
        ("รูปที่ 3.2", r"รูปที่\s*3\.2"),
        ("รูปที่ 3.3", r"รูปที่\s*3\.3"),
        ("รูปที่ 3.4", r"รูปที่\s*3\.4"),
        ("รูปที่ 3.5", r"รูปที่\s*3\.5"),
        ("รูปที่ 3.6", r"รูปที่\s*3\.6"),
        ("รูปที่ 3.7", r"รูปที่\s*3\.7"),
        ("รูปที่ 3.8", r"รูปที่\s*3\.8"),
        ("รูปที่ 3.9", r"รูปที่\s*3\.9"),
        ("รูปที่ 4.1", r"รูปที่\s*4\.1"),
        ("รูปที่ 4.2", r"รูปที่\s*4\.2"),
        ("รูปที่ 4.3", r"รูปที่\s*4\.3"),
        ("รูปที่ 4.4", r"รูปที่\s*4\.4"),
        ("รูปที่ 4.5", r"รูปที่\s*4\.5"),
        ("รูปที่ 4.6", r"รูปที่\s*4\.6"),
        ("รูปที่ 4.7", r"รูปที่\s*4\.7"),
        ("รูปที่ 4.8", r"รูปที่\s*4\.8"),
        ("รูปที่ 4.9", r"รูปที่\s*4\.9"),
        ("รูปที่ ก.1", r"รูปที่\s*ก\.1"),
        ("รูปที่ ก.2", r"รูปที่\s*ก\.2"),
        ("รูปที่ ค.1", r"รูปที่\s*ค\.1"),
        ("รูปที่ ง.1", r"รูปที่\s*ง\.1"),
        ("รูปที่ ง.2", r"รูปที่\s*ง\.2"),
    ]
    f_map = {}
    for key, pat in figures_to_find:
        for idx in range(ch1_p_idx, total_pages):
            if re.search(pat, doc[idx].get_text()):
                f_map[key] = to_academic(idx)
                break
                
    return h_map, t_map, f_map

def update_docx_tables(docx_path, h_map, t_map, f_map):
    print("Updating TOC and Table of Figures in docx...")
    doc = docx.Document(docx_path)
    
    # 1. Update Table 1 & Table 2 (TOC)
    tbl_toc1 = None
    tbl_toc2 = None
    tbl_tables = None
    tbl_figures = None
    
    for t in doc.tables:
        first_cell = t.rows[0].cells[0].text.strip() if len(t.rows) > 0 and len(t.columns) > 0 else ""
        r1_cell = t.rows[1].cells[0].text.strip() if len(t.rows) > 1 and len(t.columns) > 0 else ""
        if "บทคัดย่อภาษาไทย" in r1_cell:
            tbl_toc1 = t
        elif "บทที่ 3" in r1_cell:
            tbl_toc2 = t
        elif first_cell in ["ตาราง", "ตารางที่"]:
            tbl_tables = t
        elif first_cell in ["รูป", "รูปที่"]:
            tbl_figures = t
            
    # Update TOC2
    toc2_keys = [
        None,
        "บทที่ 3  วิธีดำเนินงาน",
        "3.1", "3.2", "3.3", "3.4", "3.5", "3.6", "3.7",
        "บทที่ 4  ผลการดำเนินงาน",
        "4.1", "4.2", "4.3", "4.4", "4.5",
        "บทที่ 5  สรุปผล และข้อเสนอแนะ",
        "5.1", "5.2", "5.3",
        "บรรณานุกรม",
        "ภาคผนวก ก", "ภาคผนวก ข", "ภาคผนวก ค", "ภาคผนวก ง", "ภาคผนวก จ",
        "ประวัติผู้เขียน",
    ]
    if tbl_toc2:
        for r_idx, key in enumerate(toc2_keys):
            if key is None or key not in h_map:
                continue
            pnum = h_map[key]
            if r_idx < len(tbl_toc2.rows):
                tbl_toc2.rows[r_idx].cells[1].paragraphs[0].text = pnum
                tbl_toc2.rows[r_idx].cells[1].paragraphs[0].alignment = WD_ALIGN_PARAGRAPH.RIGHT
                for r in tbl_toc2.rows[r_idx].cells[1].paragraphs[0].runs:
                    r.font.name = "TH SarabunPSK"
                    r.font.size = Pt(16)
                    r.font.bold = tbl_toc2.rows[r_idx].cells[0].paragraphs[0].runs[0].bold if tbl_toc2.rows[r_idx].cells[0].paragraphs[0].runs else False
        print("  [+] Synchronized TOC Table 2 page numbers.")
        
    # Update Table of Figures
    if tbl_figures:
        for r in tbl_figures.rows[1:]:
            txt = r.cells[0].text.strip().replace('\u200b', '')
            for f_key in f_map:
                if txt.startswith(f_key):
                    r.cells[1].paragraphs[0].text = f_map[f_key]
                    r.cells[1].paragraphs[0].alignment = WD_ALIGN_PARAGRAPH.RIGHT
                    for run in r.cells[1].paragraphs[0].runs:
                        run.font.name = "TH SarabunPSK"
                        run.font.size = Pt(16)
                        run.font.bold = False
                    break
        print("  [+] Synchronized Table of Figures (Table 4) page numbers.")

    doc.save(docx_path)
    print(f"  [+] Saved synchronized docx to {docx_path}")

def main():
    print("=== STEP 1: INITIAL RENDER ===")
    if not render_pdf(DOCX_PATH, PDF_PATH):
        return
        
    print("\n=== STEP 2: SCAN PDF PAGES ===")
    h_map, t_map, f_map = scan_pdf_pages(PDF_PATH)
    print(f"  Scanned: {len(h_map)} headings, {len(t_map)} tables, {len(f_map)} figures.")
    print("  Figures page map:", f_map)
    print("  Headings sample:", {k: h_map[k] for k in list(h_map.keys())[:8]})
    
    print("\n=== STEP 3: SYNC TO DOCX ===")
    update_docx_tables(DOCX_PATH, h_map, t_map, f_map)
    
    print("\n=== STEP 4: FINAL RE-RENDER WITH ACCURATE NUMBERS ===")
    render_pdf(DOCX_PATH, PDF_PATH)
    print("\n=== ALL SYNC AND RENDER OPERATIONS COMPLETED SUCCESSFULLY! ===")

if __name__ == "__main__":
    main()
