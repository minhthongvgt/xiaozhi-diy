@echo off
chcp 65001 >nul
set PYTHONUTF8=1
set PYTHONIOENCODING=utf-8
set MSYSTEM=
title Xiaozhi-ESP32 Web Configurator Launcher

echo =====================================================================
echo    XIAOZHI-ESP32 WEB CONFIGURATOR (ESP32-S3-N16R8)
echo =====================================================================
echo.
echo Đang khởi động Web Configurator trên máy chủ cục bộ...
echo (Yêu cầu môi trường localhost để cấp quyền Web File System Access API)
echo.

cd /d "%~dp0"

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
    if defined PY_CMD (
        echo [OK] Tự động phát hiện Python từ ESP-IDF: %PY_CMD%
    )
)

:: Tìm vị trí tệp configurator_server.py (ngang hàng, cha 1 cấp, hoặc cha 2 cấp)
set "SERVER_SCRIPT="
if exist "%~dp0configurator_server.py" set "SERVER_SCRIPT=%~dp0configurator_server.py"
if not defined SERVER_SCRIPT if exist "%~dp0..\configurator_server.py" set "SERVER_SCRIPT=%~dp0..\configurator_server.py"
if not defined SERVER_SCRIPT if exist "%~dp0..\..\configurator_server.py" set "SERVER_SCRIPT=%~dp0..\..\configurator_server.py"

if defined SERVER_SCRIPT (
    echo [OK] Tìm thấy máy chủ Configurator: %SERVER_SCRIPT%
    echo [OK] Đang khởi chạy với thư mục web: %~dp0
    "%PY_CMD%" "%SERVER_SCRIPT%" --web-dir "%~dp0"
    goto end
)

:: Nếu không tìm thấy server script, mở trực tiếp HTML bằng trình duyệt
echo [INFO] Đang mở trực tiếp bằng trình duyệt mặc định...
start "" "%~dp0index.html"

:end
pause
