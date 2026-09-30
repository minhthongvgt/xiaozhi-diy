@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "PYTHONUTF8=1"
set "PYTHONIOENCODING=utf-8"
set "MSYSTEM="

if not defined IDF_PATH call :find_idf
if not defined IDF_PATH goto missing_idf
if not exist "%IDF_PATH%\export.bat" goto invalid_idf

echo [INFO] ESP-IDF: %IDF_PATH%
call "%IDF_PATH%\export.bat"
if errorlevel 1 goto activation_failed
where idf.py >nul 2>nul
if errorlevel 1 goto activation_failed

if "%~1"=="" (
    idf.py build
) else (
    idf.py %*
)
exit /b %ERRORLEVEL%

:find_idf
:: 1. Kiểm tra các thư mục chuẩn phổ biến
for %%D in (
    "%USERPROFILE%\esp\esp-idf"
    "%USERPROFILE%\.espressif\esp-idf"
    "C:\esp\esp-idf"
    "C:\esp-idf"
    "C:\idf"
    "D:\esp\esp-idf"
    "D:\esp-idf"
    "D:\idf"
) do if not defined IDF_PATH if exist "%%~D\export.bat" set "IDF_PATH=%%~D"

:: 2. Quét thư mục Espressif đa phiên bản trên các ổ đĩa
if not defined IDF_PATH (
    for %%V in (C D E F) do (
        if not defined IDF_PATH if exist "%%V:\Espressif\" (
            for /f "delims=" %%I in ('dir /b /s "%%V:\Espressif\export.bat" 2^>nul') do (
                if not defined IDF_PATH if exist "%%I" (
                    set "IDF_PATH=%%~dpI"
                )
            )
        )
    )
)
if defined IDF_PATH if "%IDF_PATH:~-1%"=="\" set "IDF_PATH=%IDF_PATH:~0,-1%"
exit /b 0

:missing_idf
echo [ERROR] ESP-IDF 6.x was not found.
echo Install ESP-IDF 6.1 for ESP32-S3, then set IDF_PATH to its folder or run this from an ESP-IDF terminal.
exit /b 2

:invalid_idf
echo [ERROR] IDF_PATH does not point to an ESP-IDF installation: %IDF_PATH%
echo The folder must contain export.bat.
exit /b 2

:activation_failed
echo [ERROR] ESP-IDF environment activation failed. Install its Python environment and tools, then retry.
exit /b 2