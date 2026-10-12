@echo off
setlocal EnableExtensions
chcp 65001 >nul
title Windows fix: UTF-8 + Long Path

:: =====================================================================
::  01 set-UTF8-Fix-windows.bat
::  1. Bật UTF-8 cho Python / terminal (cho tài khoản hiện tại)
::  2. Mở giới hạn độ dài đường dẫn (Long Path > 260 ký tự)
::     - Windows: HKLM\...\FileSystem\LongPathsEnabled = 1 (cần quyền Admin)
::     - Git: core.longpaths = true (nếu có Git)
:: =====================================================================

:: Tự xin quyền Administrator (cần cho Long Path)
net session >nul 2>&1
if errorlevel 1 (
    echo [INFO] Dang xin quyen Administrator...
    powershell -NoProfile -ExecutionPolicy Bypass -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
    if errorlevel 1 (
        echo [ERROR] Khong lay duoc quyen Administrator. Hay chuot phai file nay - Run as administrator.
        pause
    )
    exit /b
)

echo =====================================================================
echo  [1/2] UTF-8
echo =====================================================================
setx PYTHONUTF8 1 >nul
setx PYTHONIOENCODING utf-8 >nul
setx LANG en_US.UTF-8 >nul
setx LC_ALL en_US.UTF-8 >nul
echo [OK] PYTHONUTF8, PYTHONIOENCODING, LANG, LC_ALL
reg query "HKCU\Software\Microsoft\Command Processor" /v AutoRun >nul 2>&1
if errorlevel 1 (
    reg add "HKCU\Software\Microsoft\Command Processor" /v AutoRun /t REG_SZ /d "chcp 65001>nul" /f >nul
    echo [OK] cmd tu dong dung code page 65001 ^(UTF-8^)
) else (
    echo [SKIP] Da co AutoRun cho cmd - giu nguyen.
)

echo.
echo =====================================================================
echo  [2/2] Long Path (duong dan dai hon 260 ky tu)
echo =====================================================================
reg add "HKLM\SYSTEM\CurrentControlSet\Control\FileSystem" /v LongPathsEnabled /t REG_DWORD /d 1 /f >nul
if errorlevel 1 (
    echo [ERROR] Khong ghi duoc LongPathsEnabled.
) else (
    echo [OK] Windows LongPathsEnabled = 1
)
where git >nul 2>&1
if errorlevel 1 (
    echo [SKIP] Khong tim thay Git trong PATH.
) else (
    git config --system core.longpaths true >nul 2>&1 || git config --global core.longpaths true
    echo [OK] Git core.longpaths = true
)

echo.
echo Xong. Dong tat ca terminal va mo lai. Long Path co hieu luc day du sau khi khoi dong lai Windows.
pause
