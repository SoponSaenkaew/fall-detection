# สคริปต์คอมไพล์โปรแกรม C++ ด้วย g++ (MinGW)
$ErrorActionPreference = "Stop"

$OutputDir = Join-Path $PSScriptRoot "output"
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

$OutputFile = Join-Path $OutputDir "main.exe"
$SourceFile = Join-Path $PSScriptRoot "main.cpp"

Write-Host "🔨 Compiling main.cpp with g++..." -ForegroundColor Cyan

g++ -std=c++17 -Wall -Wextra -O2 $SourceFile -o $OutputFile

if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Compilation Successful! Binary created at: $OutputFile" -ForegroundColor Green
    Write-Host "🚀 Running main.exe..." -ForegroundColor Yellow
    & $OutputFile
} else {
    Write-Host "❌ Compilation Failed!" -ForegroundColor Red
}
