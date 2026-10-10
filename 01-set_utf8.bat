@echo off
setx PYTHONUTF8 1
setx PYTHONIOENCODING utf-8
setx LANG en_US.UTF-8
setx LC_ALL en_US.UTF-8
reg query "HKCU\Software\Microsoft\Command Processor" /v AutoRun >nul 2>&1
if errorlevel 1 reg add "HKCU\Software\Microsoft\Command Processor" /v AutoRun /t REG_SZ /d "chcp 65001>nul" /f
echo Done. Close all terminals and open a new one.
pause
