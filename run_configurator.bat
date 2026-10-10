@echo off
setlocal EnableExtensions
chcp 65001 >nul
title Xiaozhi-ESP32 Web Configurator Launcher

set "PYTHONUTF8=1"
set "PYTHONIOENCODING=utf-8"
set "PYTHONUNBUFFERED=1"
set "MSYSTEM="

echo =====================================================================
echo    XIAOZHI-ESP32 WEB CONFIGURATOR (ESP32-S3-N16R8)
echo =====================================================================
echo.

set "PROJECT_DIR=%~dp0"
if "%PROJECT_DIR:~-1%"=="\" set "PROJECT_DIR=%PROJECT_DIR:~0,-1%"
cd /d "%PROJECT_DIR%"
echo [OK] Thư mục dự án: %PROJECT_DIR%
echo.

set "PY_CMD="

:: 1. Kiểm tra python trong PATH
python --version >nul 2>&1
if not errorlevel 1 (
    set "PY_CMD=python"
    goto python_found
)

:: 2. Kiểm tra py launcher
py -3 --version >nul 2>&1
if not errorlevel 1 (
    set "PY_CMD=py -3"
    goto python_found
)

:: 3. Kiểm tra biến môi trường IDF_TOOLS_PATH
if defined IDF_TOOLS_PATH (
    if exist "%IDF_TOOLS_PATH%\python.exe" (
        set "PY_CMD=%IDF_TOOLS_PATH%\python.exe"
        goto python_found
    )
)

:: 4. Quét các ổ đĩa tìm Espressif tools python
for %%D in (C D E F) do (
    if not defined PY_CMD if exist "%%D:\Espressif\tools\python\python.exe" (
        set "PY_CMD=%%D:\Espressif\tools\python\python.exe"
        goto python_found
    )
)

:: 5. Quét Espressif idf-python
for %%D in (C D E F) do (
    if not defined PY_CMD if exist "%%D:\Espressif\tools\idf-python\" (
        for /f "delims=" %%F in ('dir /b /s "%%D:\Espressif\tools\idf-python\python.exe" 2^>nul') do (
            if not defined PY_CMD if exist "%%F" (
                set "PY_CMD=%%F"
                goto python_found
            )
        )
    )
)

:: 6. Quét .espressif trong User Profile
if exist "%USERPROFILE%\.espressif\python_env\" (
    for /f "delims=" %%F in ('dir /b /s "%USERPROFILE%\.espressif\python_env\python.exe" 2^>nul') do (
        if not defined PY_CMD if exist "%%F" (
            set "PY_CMD=%%F"
            goto python_found
        )
    )
)

:missing_python
echo [CẢNH BÁO] Không tìm thấy Python trên hệ thống.
echo Đang mở giao diện Web Configurator chế độ Standalone...
if exist "%PROJECT_DIR%\tools\web-configurator\index.html" (
    start "" "%PROJECT_DIR%\tools\web-configurator\index.html"
) else (
    echo [LỖI] Không tìm thấy file index.html tại tools\web-configurator\index.html
)
goto end

:python_found
echo [OK] Tự động phát hiện Python: %PY_CMD%
echo [OK] Đang khởi động Configurator Server và tự động nạp cấu hình...
echo.

"%PY_CMD%" "%PROJECT_DIR%\configurator_server.py" --project "%PROJECT_DIR%"

:end
echo.
pause
