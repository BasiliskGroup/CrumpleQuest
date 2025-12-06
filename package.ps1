# CrumpleQuest Windows Packaging Script
# This script builds the game in Release mode and packages it for distribution

param(
    [string]$OutputName = "CrumpleQuest-Windows",
    [switch]$SkipBuild = $false
)

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "  CrumpleQuest Packaging Script" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# Check if build directory exists
if (-not (Test-Path "build")) {
    Write-Host "Error: build directory not found!" -ForegroundColor Red
    Write-Host "Please run CMake to configure the project first:" -ForegroundColor Yellow
    Write-Host "  mkdir build" -ForegroundColor Yellow
    Write-Host "  cd build" -ForegroundColor Yellow
    Write-Host "  cmake .." -ForegroundColor Yellow
    exit 1
}

# Build in Release mode (unless skipped)
if (-not $SkipBuild) {
    Write-Host "[1/5] Building Release version..." -ForegroundColor Green
    Push-Location build
    cmake --build . --config Release
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Error: Build failed!" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    Pop-Location
    Write-Host "Build completed successfully!" -ForegroundColor Green
    Write-Host ""
} else {
    Write-Host "[1/5] Skipping build (using existing executable)..." -ForegroundColor Yellow
    Write-Host ""
}

# Check if executable exists
if (-not (Test-Path "build\Release\game.exe")) {
    Write-Host "Error: game.exe not found in build\Release\" -ForegroundColor Red
    Write-Host "Please build the project first or run without -SkipBuild flag" -ForegroundColor Yellow
    exit 1
}

# Clean up old package if it exists
if (Test-Path $OutputName) {
    Write-Host "[2/5] Removing old package directory..." -ForegroundColor Green
    Remove-Item -Recurse -Force $OutputName
}

# Create distribution directory
Write-Host "[2/5] Creating package directory..." -ForegroundColor Green
New-Item -ItemType Directory -Path $OutputName -Force | Out-Null

# Copy executable
Write-Host "[3/5] Copying executable..." -ForegroundColor Green
Copy-Item "build\Release\game.exe" -Destination "$OutputName\" -Force

# Copy resource folders
Write-Host "[4/5] Copying game resources..." -ForegroundColor Green
$resources = @("shaders", "textures", "models", "sounds", "art")
foreach ($folder in $resources) {
    if (Test-Path $folder) {
        Write-Host "  - Copying $folder..." -ForegroundColor Cyan
        Copy-Item -Recurse -Force $folder -Destination "$OutputName\$folder"
    } else {
        Write-Host "  - Warning: $folder not found, skipping..." -ForegroundColor Yellow
    }
}

# Copy DLL dependencies
Write-Host "  - Checking for DLL dependencies..." -ForegroundColor Cyan
$dllFiles = Get-ChildItem "build\Release\*.dll" -ErrorAction SilentlyContinue
if ($dllFiles) {
    foreach ($dll in $dllFiles) {
        Write-Host "    - Copying $($dll.Name)..." -ForegroundColor Gray
        Copy-Item $dll.FullName -Destination "$OutputName\" -Force
    }
} else {
    Write-Host "    - No DLLs found in build\Release\" -ForegroundColor Gray
}

# Check _deps for any required DLLs
$depDlls = Get-ChildItem "build\_deps\*\*\*.dll" -Recurse -ErrorAction SilentlyContinue
if ($depDlls) {
    Write-Host "    - Found dependency DLLs:" -ForegroundColor Gray
    foreach ($dll in $depDlls) {
        $destPath = Join-Path $OutputName $dll.Name
        if (-not (Test-Path $destPath)) {
            Write-Host "      - Copying $($dll.Name)..." -ForegroundColor Gray
            Copy-Item $dll.FullName -Destination "$OutputName\" -Force
        }
    }
}

# Create zip archive
Write-Host "[5/5] Creating zip archive..." -ForegroundColor Green
$zipPath = "$OutputName.zip"
if (Test-Path $zipPath) {
    Remove-Item $zipPath -Force
}
Compress-Archive -Path $OutputName -DestinationPath $zipPath -Force

Write-Host ""
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "  Package created successfully!" -ForegroundColor Green
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Package location:" -ForegroundColor Yellow
Write-Host "  Folder: $OutputName\" -ForegroundColor White
Write-Host "  Zip:    $zipPath" -ForegroundColor White
Write-Host ""
Write-Host "You can now distribute the zip file or the folder!" -ForegroundColor Green
Write-Host ""

# Show package size
$folderSize = (Get-ChildItem $OutputName -Recurse | Measure-Object -Property Length -Sum).Sum / 1MB
$zipSize = (Get-Item $zipPath).Length / 1MB
Write-Host "Package sizes:" -ForegroundColor Cyan
Write-Host "  Folder: $([math]::Round($folderSize, 2)) MB" -ForegroundColor White
Write-Host "  Zip:    $([math]::Round($zipSize, 2)) MB" -ForegroundColor White
Write-Host ""
