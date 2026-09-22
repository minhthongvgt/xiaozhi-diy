@echo off
chcp 65001 >nul
title Xiaozhi-ESP32 Web Configurator Launcher

echo =====================================================================
echo    XIAOZHI-ESP32 WEB CONFIGURATOR (ESP32-S3-N16R8)
echo =====================================================================
echo.
echo Đang khởi động Web Configurator trên máy chủ cục bộ...
echo (Yêu cầu môi trường localhost để cấp quyền Web File System Access API)
echo.

cd /d "%~dp0"

:: Kiểm tra Python
where python >nul 2>nul
if %ERRORLEVEL% equ 0 (
    echo [OK] Tìm thấy Python. Đang khởi chạy Configurator Server...
    if exist "%~dp0..\..\configurator_server.py" (
        python "%~dp0..\..\configurator_server.py"
    ) else (
        python -m http.server 8080
    )
    goto end
)

:: Nếu không có Python, mở trực tiếp bằng trình duyệt mặc định
echo [INFO] Không tìm thấy Python. Đang mở trực tiếp bằng trình duyệt mặc định...
start "" "%~dp0index.html"

:end
pause
