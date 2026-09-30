@echo off
chcp 65001 >nul
set "PYTHONUTF8=1"
set "PYTHONIOENCODING=utf-8"
set "MSYSTEM="
title Xiaozhi-ESP32 Web Configurator Launcher

echo =====================================================================
echo    XIAOZHI-ESP32 WEB CONFIGURATOR (ESP32-S3-N16R8)
echo =====================================================================
echo.
echo Đang nhận diện vị trí thư mục mã nguồn thực tế...
cd /d "%~dp0"
echo [OK] Thư mục dự án: %CD%
echo.

:: Kiểm tra Python (hệ thống hoặc tự động dò tìm trong môi trường ESP-IDF trên mọi ổ đĩa)
set "PY_CMD=python"
where python >nul 2>nul
if %ERRORLEVEL% neq 0 (
    set "PY_CMD="
    if defined IDF_TOOLS_PATH if exist "%IDF_TOOLS_PATH%" (
        for /f "delims=" %%F in ('dir /b /s "%IDF_TOOLS_PATH%\python.exe" 2^>nul') do if not defined PY_CMD set "PY_CMD=%%F"
    )
    if not defined PY_CMD (
        for %%D in (C D E F) do (
            if not defined PY_CMD if exist "%%D:\Espressif\tools\python\" (
                for /f "delims=" %%F in ('dir /b /s "%%D:\Espressif\tools\python\python.exe" 2^>nul') do if not defined PY_CMD set "PY_CMD=%%F"
            )
        )
    )
    if not defined PY_CMD if exist "%USERPROFILE%\.espressif\python_env\" (
        for /f "delims=" %%F in ('dir /b /s "%USERPROFILE%\.espressif\python_env\python.exe" 2^>nul') do if not defined PY_CMD set "PY_CMD=%%F"
    )
    if not defined PY_CMD (
        echo [CẢNH BÁO] Không tìm thấy Python. Đang chuyển sang chế độ Standalone (Mở trực tiếp HTML)...
        start "" "%~dp0tools\web-configurator\index.html"
        goto end
    ) else (
        echo [OK] Tự động phát hiện Python từ ESP-IDF Toolchain: %PY_CMD%
    )
) else (
    echo [OK] Đã tìm thấy Python trên hệ thống.
)

echo [OK] Đang khởi động Configurator Server và tự động nạp cấu hình...
echo.
"%PY_CMD%" "%~dp0configurator_server.py" --project "%~dp0"
goto end

:end
pause
