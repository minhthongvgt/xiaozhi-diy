@echo off
setlocal EnableExtensions
chcp 65001 >nul
title Xiaozhi - set-target esp32s3 / menuconfig / build (ESP-IDF 6.1)
cd /d "%~dp0"

:: =====================================================================
::  02-build_esp32s3.bat
::  1. Tự tìm ESP-IDF 6.1 trên máy (không dùng đường dẫn cố định)
::  2. Tự nhận diện target: chưa là esp32s3 thì set-target, có rồi thì bỏ qua
::  3. Menu chọn: menuconfig / build / set-target lại
:: =====================================================================

set "TARGET=esp32s3"
set "WANT_MAJOR=6"
set "WANT_MINOR=1"

set "PYTHONUTF8=1"
set "PYTHONIOENCODING=utf-8"
set "MSYSTEM="

echo =====================================================================
echo    XIAOZHI - BUILD %TARGET% VOI ESP-IDF %WANT_MAJOR%.%WANT_MINOR%
echo =====================================================================
echo.

:: ---------------------------------------------------------------------
:: Tìm ESP-IDF 6.1
:: ---------------------------------------------------------------------
set "FOUND_IDF="
set "EIM_SCRIPT="

:: 0. Đang chạy trong terminal ESP-IDF 6.1 đã kích hoạt sẵn
if defined IDF_PATH (
    call :check_idf "%IDF_PATH%"
    if defined FOUND_IDF (
        where idf.py >nul 2>nul
        if not errorlevel 1 goto idf_ready
    )
)

:: 1. Bản cài bằng ESP-IDF Installation Manager (EIM, mặc định của IDF 6.x)
if not defined FOUND_IDF call :scan_eim

:: 2. Quét các thư mục cài đặt thông dụng trên mọi ổ đĩa
if not defined FOUND_IDF call :scan_root "%USERPROFILE%\esp"
if not defined FOUND_IDF call :scan_root "%USERPROFILE%\.espressif"
if not defined FOUND_IDF call :scan_root "%USERPROFILE%\.espressif\frameworks"
if not defined FOUND_IDF (
    for %%V in (C D E F G H I J K L M N O P Q R S T U V W X Y Z) do (
        if not defined FOUND_IDF if exist "%%V:\" (
            call :scan_root "%%V:\esp"
            call :scan_root "%%V:\Espressif"
            call :scan_root "%%V:\Espressif\frameworks"
            call :scan_root "%%V:\idf"
            call :scan_root "%%V:\"
        )
    )
)

if not defined FOUND_IDF goto missing_idf

echo [OK] ESP-IDF %WANT_MAJOR%.%WANT_MINOR%: %FOUND_IDF%
echo.

:: ---------------------------------------------------------------------
:: Kích hoạt môi trường ESP-IDF
:: ---------------------------------------------------------------------
if defined EIM_SCRIPT (
    call :activate_eim
) else (
    set "IDF_PATH=%FOUND_IDF%"
    call "%FOUND_IDF%\export.bat"
)
cd /d "%~dp0"
where idf.py >nul 2>nul
if errorlevel 1 goto activation_failed

:idf_ready
echo.
call idf.py --version
if errorlevel 1 goto activation_failed

:: ---------------------------------------------------------------------
:: Tự nhận diện target: đã là esp32s3 thì bỏ qua, chưa thì set-target
:: ---------------------------------------------------------------------
call :detect_target
if /i "%CUR_TARGET%"=="%TARGET%" (
    echo.
    echo [OK] Thu muc da set-target %TARGET% - bo qua buoc set-target.
) else (
    echo.
    if defined CUR_TARGET (echo [INFO] Target hien tai: %CUR_TARGET% - can doi sang %TARGET%.) else (echo [INFO] Chua set-target.)
    call :do_set_target
)

:: ---------------------------------------------------------------------
:: Menu
:: ---------------------------------------------------------------------
:menu
call :detect_target
echo.
echo =====================================================================
echo    ESP-IDF %WANT_MAJOR%.%WANT_MINOR%  ^|  Target: %CUR_TARGET%
echo =====================================================================
echo    [1] menuconfig        (cau hinh)
echo    [2] build             (bien dich)
echo    [3] set-target %TARGET%  (lam lai - XOA sdkconfig va build)
echo    [0] Thoat
echo =====================================================================
choice /c 1230 /n /m "Chon [1/2/3/0]: "
if errorlevel 4 goto end
if errorlevel 3 goto opt_set_target
if errorlevel 2 goto opt_build
if errorlevel 1 goto opt_menuconfig
goto menu

:opt_menuconfig
echo.
echo [menuconfig] Luu bang phim S, thoat bang Q / Esc
call idf.py menuconfig
if errorlevel 1 (echo [ERROR] menuconfig that bai.) else (echo [OK] Da dong menuconfig.)
goto menu

:opt_build
echo.
echo [build] idf.py build
call idf.py build
if errorlevel 1 (
    echo.
    echo [ERROR] Build that bai. Xem log o tren.
) else (
    echo.
    echo [OK] Build thanh cong. Firmware nam trong thu muc build\
    echo      Nap firmware: idf.py -p COMx flash monitor
)
goto menu

:opt_set_target
echo.
echo [CANH BAO] set-target se XOA sdkconfig va thu muc build (mat cau hinh menuconfig).
choice /c YN /n /m "Tiep tuc? [Y/N]: "
if errorlevel 2 goto menu
call :do_set_target
goto menu

:end
endlocal
exit /b 0

:: =====================================================================
:: Subroutines
:: =====================================================================

:: detect_target : CUR_TARGET = target trong sdkconfig (rỗng nếu chưa set-target
:: hoặc thư mục build đang ở target khác)
:detect_target
set "CUR_TARGET="
set "_BUILD_TARGET="
if exist "sdkconfig" (
    for /f "tokens=2 delims==" %%T in ('findstr /b /c:"CONFIG_IDF_TARGET=" "sdkconfig"') do set "CUR_TARGET=%%~T"
)
if exist "build\CMakeCache.txt" (
    for /f "tokens=2 delims==" %%T in ('findstr /b /c:"IDF_TARGET:STRING=" "build\CMakeCache.txt"') do set "_BUILD_TARGET=%%~T"
)
if defined _BUILD_TARGET if /i not "%_BUILD_TARGET%"=="%CUR_TARGET%" set "CUR_TARGET=%_BUILD_TARGET%"
exit /b 0

:do_set_target
echo.
echo [set-target] idf.py set-target %TARGET%
call idf.py set-target %TARGET%
if errorlevel 1 (echo [ERROR] set-target that bai. Xem log o tren.) else (echo [OK] Da set-target %TARGET%.)
exit /b 0

:: check_idf <dir> : đặt FOUND_IDF nếu <dir> là ESP-IDF đúng phiên bản 6.1
:check_idf
if defined FOUND_IDF exit /b 0
set "_D=%~1"
if not defined _D exit /b 0
if "%_D:~-1%"=="\" set "_D=%_D:~0,-1%"
if not exist "%_D%\tools\idf.py" exit /b 0
if not exist "%_D%\tools\cmake\version.cmake" exit /b 0
set "_MAJ="
set "_MIN="
for /f "tokens=3 delims=() " %%A in ('findstr /b /c:"set(IDF_VERSION_MAJOR " "%_D%\tools\cmake\version.cmake"') do set "_MAJ=%%A"
for /f "tokens=3 delims=() " %%A in ('findstr /b /c:"set(IDF_VERSION_MINOR " "%_D%\tools\cmake\version.cmake"') do set "_MIN=%%A"
if "%_MAJ%"=="%WANT_MAJOR%" if "%_MIN%"=="%WANT_MINOR%" set "FOUND_IDF=%_D%"
if not defined FOUND_IDF echo [SKIP] %_D% ^(ESP-IDF %_MAJ%.%_MIN%^)
exit /b 0

:: scan_root <dir> : kiểm tra <dir>, <dir>\esp-idf, <dir>\*, <dir>\*\esp-idf
:: (chỉ nhận bản có export.bat - bản cài kiểu cũ)
:scan_root
if defined FOUND_IDF exit /b 0
if not exist "%~1\" exit /b 0
call :check_legacy "%~1"
call :check_legacy "%~1\esp-idf"
for /d %%S in ("%~1\*") do (
    if not defined FOUND_IDF call :check_legacy "%%~fS"
    if not defined FOUND_IDF call :check_legacy "%%~fS\esp-idf"
)
exit /b 0

:check_legacy
if exist "%~1\export.bat" call :check_idf "%~1"
exit /b 0

:: scan_eim : đọc eim_idf.json của ESP-IDF Installation Manager (nếu có)
:scan_eim
set "_EIM_JSON="
if defined IDF_TOOLS_PATH if exist "%IDF_TOOLS_PATH%\eim_idf.json" set "_EIM_JSON=%IDF_TOOLS_PATH%\eim_idf.json"
if not defined _EIM_JSON if exist "%USERPROFILE%\.espressif\tools\eim_idf.json" set "_EIM_JSON=%USERPROFILE%\.espressif\tools\eim_idf.json"
if not defined _EIM_JSON (
    for %%V in (C D E F G H I J K L M N O P Q R S T U V W X Y Z) do (
        if not defined _EIM_JSON if exist "%%V:\Espressif\tools\eim_idf.json" set "_EIM_JSON=%%V:\Espressif\tools\eim_idf.json"
    )
)
if not defined _EIM_JSON exit /b 0
echo [INFO] Doc danh sach cai dat EIM: %_EIM_JSON%
for /f "usebackq tokens=1,2 delims=|" %%A in (`powershell -NoProfile -ExecutionPolicy Bypass -Command "$j = Get-Content -Raw -LiteralPath '%_EIM_JSON%' | ConvertFrom-Json; foreach ($i in $j.idfInstalled) { '{0}|{1}' -f $i.path, $i.activationScript }"`) do (
    if not defined FOUND_IDF (
        call :check_idf "%%~A"
        if defined FOUND_IDF if exist "%%~B" set "EIM_SCRIPT=%%~B"
    )
)
exit /b 0

:: activate_eim : nạp biến môi trường từ script kích hoạt của EIM (cờ -e)
:activate_eim
echo [INFO] Kich hoat moi truong qua EIM: %EIM_SCRIPT%
set "_OLD_PATH=%PATH%"
set "_EIM_PATH="
for /f "usebackq tokens=1,* delims==" %%A in (`powershell -NoProfile -ExecutionPolicy Bypass -File "%EIM_SCRIPT%" -e`) do (
    if /i "%%A"=="PATH" (
        set "_EIM_PATH=%%B"
    ) else if /i not "%%A"=="SYSTEM_PATH" (
        set "%%A=%%B"
    )
)
if not defined _EIM_PATH exit /b 1
set "PATH=%_EIM_PATH%;%_OLD_PATH%"
if defined IDF_PYTHON_ENV_PATH set "VIRTUAL_ENV=%IDF_PYTHON_ENV_PATH%"
exit /b 0

:: =====================================================================
:: Errors
:: =====================================================================
:missing_idf
echo.
echo [ERROR] Khong tim thay ESP-IDF %WANT_MAJOR%.%WANT_MINOR% tren may nay.
echo         - Cai ESP-IDF %WANT_MAJOR%.%WANT_MINOR% bang ESP-IDF Installation Manager (EIM), hoac
echo         - Mo terminal ESP-IDF %WANT_MAJOR%.%WANT_MINOR% roi chay file nay tu do.
pause
exit /b 2

:activation_failed
echo.
echo [ERROR] Kich hoat moi truong ESP-IDF that bai: %FOUND_IDF%
echo         Hay sua/cai lai ESP-IDF %WANT_MAJOR%.%WANT_MINOR% bang EIM (hoac chay install.bat) roi thu lai.
pause
exit /b 2
