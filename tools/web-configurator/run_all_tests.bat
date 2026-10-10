@echo off
setlocal EnableExtensions
cd /d "%~dp0"
echo ========================================================
echo   RUNNING ALL WEB CONFIGURATOR TESTS
echo ========================================================
echo.

node test_modules.mjs
if errorlevel 1 goto error

python test_sdkconfig_io.py
if errorlevel 1 goto error

node test_dom.cjs
if errorlevel 1 goto error

node test_project_integration.mjs
if errorlevel 1 goto error

node test_kconfig_parity.mjs
if errorlevel 1 goto error

if exist "node_modules\jsdom" (
    node test_gui_dom.mjs
    if errorlevel 1 goto error
) else (
    echo [SKIP] test_gui_dom.mjs: chay "npm install" trong tools\web-configurator de cai jsdom
)

echo.
echo ========================================================
echo   [SUCCESS] ALL TESTS PASSED (100% KCONFIG PARITY)!
echo ========================================================
exit /b 0

:error
echo.
echo [ERROR] Test suite failed!
exit /b 1
