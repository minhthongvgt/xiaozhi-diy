@echo off
setlocal EnableExtensions
chcp 65001 >nul
title Xiaozhi - ESP32-P4 menuconfig / build / flash (ESP-IDF 6.1)
cd /d "%~dp0"

set "TARGET=esp32p4"
set "WANT_MAJOR=6"
set "WANT_MINOR=1"
set "BUILD_DIR=build_p4"
set "BOARD_FILE=build_p4\board.txt"

set "PYTHONUTF8=1"
set "PYTHONIOENCODING=utf-8"
set "MSYSTEM="

echo =====================================================================
echo    XIAOZHI - BUILD %TARGET% VOI ESP-IDF %WANT_MAJOR%.%WANT_MINOR%
echo    Thu muc build rieng: %BUILD_DIR% ^(khong dung cau hinh ESP32-S3^)
echo =====================================================================
echo.

set "FOUND_IDF="
set "EIM_SCRIPT="

if defined IDF_PATH (
    call :check_idf "%IDF_PATH%"
    if defined FOUND_IDF (
        where idf.py >nul 2>nul
        if not errorlevel 1 goto idf_ready
    )
)

if not defined FOUND_IDF call :scan_eim

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

call :load_board
if not defined BOARD_ID call :choose_board
if not exist "%BUILD_DIR%\sdkconfig" call :do_set_target

:menu
call :load_board
echo.
echo =====================================================================
echo    ESP-IDF %WANT_MAJOR%.%WANT_MINOR%  ^|  Target: %TARGET%  ^|  Bo: %BOARD_NAME%
echo    Chip ESP32-P4 v1.x. Chip v3.x: sua REV trong sdkconfig.defaults.esp32p4
echo =====================================================================
echo    [1] menuconfig        (cau hinh)
echo    [2] build             (bien dich)
echo    [3] flash + monitor   (nap va xem log)
echo    [4] doi bo mach       (XOA cau hinh trong %BUILD_DIR%)
echo    [0] Thoat
echo =====================================================================
choice /c 12340 /n /m "Chon [1/2/3/4/0]: "
if errorlevel 5 goto end
if errorlevel 4 goto opt_board
if errorlevel 3 goto opt_flash
if errorlevel 2 goto opt_build
if errorlevel 1 goto opt_menuconfig
goto menu

:opt_menuconfig
echo.
echo [menuconfig] Luu bang phim S, thoat bang Q / Esc
call :idf menuconfig
if errorlevel 1 (echo [ERROR] menuconfig that bai.) else (echo [OK] Da dong menuconfig.)
goto menu

:opt_build
echo.
echo [build] %BOARD_NAME%
call :idf build
if errorlevel 1 (
    echo.
    echo [ERROR] Build that bai. Xem log o tren.
) else (
    echo.
    echo [OK] Build thanh cong. Firmware nam trong thu muc %BUILD_DIR%)
goto menu

:opt_flash
echo.
set "PORT="
set /p "PORT=Cong COM cua bo ESP32-P4 (vd COM5): "
if not defined PORT goto menu
echo [flash] %PORT%  -  thoat monitor bang Ctrl+]
call :idf -p %PORT% flash monitor
goto menu

:opt_board
echo.
echo [CANH BAO] Doi bo mach se XOA sdkconfig va thu muc %BUILD_DIR%.
choice /c YN /n /m "Tiep tuc? [Y/N]: "
if errorlevel 2 goto menu
call :choose_board
call :do_set_target
goto menu

:end
endlocal
exit /b 0

:idf
set "SDKCONFIG_DEFAULTS=sdkconfig.defaults;main/boards/esp32-p4/sdkconfig.%BOARD_ID%"
call idf.py -B "%BUILD_DIR%" -D "SDKCONFIG=%BUILD_DIR%/sdkconfig" %*
set "_RC=%errorlevel%"
set "SDKCONFIG_DEFAULTS="
exit /b %_RC%

:choose_board
echo.
echo    Chon bo mach ESP32-P4:
echo    [1] Guition JC-ESP32P4-M3-Dev        (khong man hinh, camera OV5647)
echo    [2] Espressif ESP32-P4-Function-EV-Board
echo    [3] Waveshare ESP32-P4-Module-DEV-KIT
choice /c 123 /n /m "Chon [1/2/3]: "
if errorlevel 3 (set "BOARD_ID=waveshare-dev-kit") else if errorlevel 2 (set "BOARD_ID=function-ev") else (set "BOARD_ID=guition-m3-dev")
call :board_name
exit /b 0

:load_board
if exist "%BOARD_FILE%" set /p BOARD_ID=<"%BOARD_FILE%"
if exist "%BUILD_DIR%\sdkconfig" (
    findstr /b /c:"CONFIG_BOARD_TYPE_GUITION_ESP32_P4_M3_DEV=y" "%BUILD_DIR%\sdkconfig" >nul && set "BOARD_ID=guition-m3-dev"
    findstr /b /c:"CONFIG_BOARD_TYPE_ESP32_P4_FUNCTION_EV_BOARD=y" "%BUILD_DIR%\sdkconfig" >nul && set "BOARD_ID=function-ev"
    findstr /b /c:"CONFIG_BOARD_TYPE_WAVESHARE_ESP32_P4_DEV_KIT=y" "%BUILD_DIR%\sdkconfig" >nul && set "BOARD_ID=waveshare-dev-kit"
)
call :board_name
exit /b 0

:board_name
set "BOARD_NAME=%BOARD_ID%"
if /i "%BOARD_ID%"=="guition-m3-dev" set "BOARD_NAME=Guition JC-ESP32P4-M3-Dev"
if /i "%BOARD_ID%"=="function-ev" set "BOARD_NAME=Espressif ESP32-P4-Function-EV-Board"
if /i "%BOARD_ID%"=="waveshare-dev-kit" set "BOARD_NAME=Waveshare ESP32-P4-Module-DEV-KIT"
exit /b 0

:do_set_target
echo.
echo [set-target] %TARGET%  -  %BOARD_NAME%
if exist "%BOARD_FILE%" del /q "%BOARD_FILE%"
call :idf set-target %TARGET%
if errorlevel 1 (
    echo [ERROR] set-target that bai. Xem log o tren.
    exit /b 1
)
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
>"%BOARD_FILE%" echo %BOARD_ID%
echo [OK] Da set-target %TARGET%.
exit /b 0

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
