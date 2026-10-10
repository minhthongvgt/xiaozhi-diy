@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"
echo ========================================================
echo   DONG BO WEB CONFIGURATOR VOI MENUCONFIG (Kconfig.projbuild)
echo ========================================================
echo.

set "PYTHONUTF8=1"
set "IDF_PY="
if defined IDF_PYTHON_ENV_PATH if exist "%IDF_PYTHON_ENV_PATH%\Scripts\python.exe" set "IDF_PY=%IDF_PYTHON_ENV_PATH%\Scripts\python.exe"
if not defined IDF_PY if exist "C:\Espressif\tools\python\v6.1\venv\Scripts\python.exe" set "IDF_PY=C:\Espressif\tools\python\v6.1\venv\Scripts\python.exe"
if not defined IDF_PY (
    echo [ERROR] Khong tim thay Python cua ESP-IDF. Dat bien IDF_PYTHON_ENV_PATH hoac cai ESP-IDF v6.1.
    exit /b 1
)
set "ROOT=%~dp0..\.."

echo [1/5] Xuat cay menuconfig ra kconfig_schema.json
"%IDF_PY%" -I "%ROOT%\scripts\gen_kconfig_schema.py"
if errorlevel 1 goto error

echo [2/5] Cap nhat Danh_Sach_Kconfig_WebUI.md
"%IDF_PY%" -I "%ROOT%\scripts\gen_kconfig_webui_doc.py"
if errorlevel 1 goto error

echo [3/5] Sinh lai cac panel giao dien
"%IDF_PY%" -I "%ROOT%\scripts\gen_webui_panels.py"
if errorlevel 1 goto error

echo [4/5] Kiem tra dong bo giao dien
node test_kconfig_parity.mjs
if errorlevel 1 goto error

echo [5/5] Doi chieu sdkconfig sinh ra voi kconfiglib
"%IDF_PY%" -I "%ROOT%\scripts\check_webui_kconfig.py"
if errorlevel 1 goto error

echo.
echo [SUCCESS] Web Configurator da dong bo voi menuconfig.
exit /b 0

:error
echo.
echo [ERROR] Dong bo that bai, xem thong bao phia tren.
exit /b 1
