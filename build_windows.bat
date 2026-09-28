@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "PYTHONUTF8=1"
set "PYTHONIOENCODING=utf-8"

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
for %%D in (
    "C:\Espressif\v6.1\esp-idf"
    "%USERPROFILE%\esp\v6.1\esp-idf"
    "%USERPROFILE%\esp\esp-idf"
    "C:\Espressif\frameworks\esp-idf-v6.1"
    "C:\Espressif\frameworks\esp-idf-v6.1.0"
    "C:\esp\v6.1\esp-idf"
) do if not defined IDF_PATH if exist "%%~D\export.bat" set "IDF_PATH=%%~D"
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