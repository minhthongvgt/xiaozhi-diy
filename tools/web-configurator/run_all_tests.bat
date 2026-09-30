@echo off
setlocal EnableExtensions
cd /d "%~dp0"
echo ========================================================
echo   RUNNING ALL WEB CONFIGURATOR TESTS (130 TESTS)
echo ========================================================
echo.

node test_dom.js
if errorlevel 1 goto error

node test_runner.js
if errorlevel 1 goto error

node test_integration.js
if errorlevel 1 goto error

node test_kconfig_parity.js
if errorlevel 1 goto error

echo.
echo ========================================================
echo   [SUCCESS] ALL 130 TESTS PASSED (100% KCONFIG PARITY)!
echo ========================================================
exit /b 0

:error
echo.
echo [ERROR] Test suite failed!
exit /b 1
