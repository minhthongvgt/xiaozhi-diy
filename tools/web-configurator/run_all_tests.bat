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

node test_dom.js
if errorlevel 1 goto error

node test_project_integration.mjs
if errorlevel 1 goto error

echo.
echo ========================================================
echo   [SUCCESS] ALL TESTS PASSED (100% KCONFIG PARITY)!
echo ========================================================
exit /b 0

:error
echo.
echo [ERROR] Test suite failed!
exit /b 1
