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

set "WEB_DIR=%~dp0"
if "%WEB_DIR:~-1%"=="\" set "WEB_DIR=%WEB_DIR:~0,-1%"
cd /d "%WEB_DIR%"

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

:python_found
:: Tìm vị trí tệp configurator_server.py
set "SERVER_SCRIPT="
if exist "%WEB_DIR%\configurator_server.py" set "SERVER_SCRIPT=%WEB_DIR%\configurator_server.py"
if not defined SERVER_SCRIPT if exist "%WEB_DIR%\..\configurator_server.py" set "SERVER_SCRIPT=%WEB_DIR%\..\configurator_server.py"
if not defined SERVER_SCRIPT if exist "%WEB_DIR%\..\..\configurator_server.py" set "SERVER_SCRIPT=%WEB_DIR%\..\..\configurator_server.py"

if defined SERVER_SCRIPT if defined PY_CMD (
    echo [OK] Tự động phát hiện Python: %PY_CMD%
    echo [OK] Tìm thấy máy chủ Configurator: %SERVER_SCRIPT%
    echo [OK] Đang khởi chạy với thư mục web: %WEB_DIR%
    echo.
    "%PY_CMD%" "%SERVER_SCRIPT%" --web-dir "%WEB_DIR%"
    goto end
)

:missing_server
echo [CẢNH BÁO] Không thể khởi động server. Đang mở trực tiếp bằng trình duyệt...
if exist "%WEB_DIR%\index.html" (
    start "" "%WEB_DIR%\index.html"
) else (
    echo [LỖI] Không tìm thấy index.html!
)

:end
echo.
pause
