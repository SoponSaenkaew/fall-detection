$wdFormatPDF = 17
$word = New-Object -ComObject Word.Application
$word.Visible = $false
try {
    $docxPath = "D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค.docx"
    $pdfPath = "D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\รายงานเชิงเทคนิก\รายงานเชิงเทคนิค_final_render.pdf"
    $doc = $word.Documents.Open($docxPath)
    $doc.SaveAs([ref]$pdfPath, [ref]$wdFormatPDF)
    $doc.Close()
    Write-Host "PDF rendered successfully!"
} catch {
    Write-Host "Error: $_"
} finally {
    $word.Quit()
}
