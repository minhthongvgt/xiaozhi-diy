@echo off
chcp 65001 >nul
title Xiaozhi-ESP32 Web Configurator Launcher

echo =====================================================================
echo    XIAOZHI-ESP32 WEB CONFIGURATOR (ESP32-S3-N16R8)
echo =====================================================================
echo.
echo Đang nhận diện vị trí thư mục mã nguồn thực tế...
cd /d "%~dp0"
echo [OK] Thư mục dự án: %CD%
echo.

:: Kiểm tra Python
where python >nul 2>nul
if %ERRORLEVEL% equ 0 (
    echo [OK] Đã tìm thấy Python trên hệ thống.
    echo [OK] Đang khởi động Configurator Server và tự động nạp cấu hình...
    echo.
    python "%~dp0configurator_server.py"
    goto end
)

:: Nếu không tìm thấy Python
echo [CẢNH BÁO] Không tìm thấy Python. Đang chuyển sang chế độ Standalone (Mở trực tiếp HTML)...
start "" "%~dp0tools\web-configurator\index.html"

:end
pause
