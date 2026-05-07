$MINGW_PATH = "D:/project/LVGL/LVGL_test/mingw64/bin"

# Setup MinGW environment
$env:PATH = "$MINGW_PATH;" + $env:PATH
$env:CC = "$MINGW_PATH/gcc.exe"

# Configure (first time or after CMakeLists.txt changes)
if (-not (Test-Path "build/CMakeCache.txt")) {
    Write-Host "==> Configuring CMake..." -ForegroundColor Cyan
    cmake -G "MinGW Makefiles" -B build -S .
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

# Build
Write-Host "==> Building..." -ForegroundColor Cyan
cmake --build build

if ($LASTEXITCODE -eq 0) {
    Write-Host "==> Build succeeded: build/ESTA_Simulator.exe" -ForegroundColor Green
}
