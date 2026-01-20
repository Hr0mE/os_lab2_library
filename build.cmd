@echo off
setlocal enabledelayedexpansion

REM ================== визуальный профиль ==================
REM основной цвет — зелёный (OK)
color 0A

set ROOT_DIR=%~dp0
set BUILD_DIR=%ROOT_DIR%build

REM ================== фаза инициализации ==================
echo [INIT] Scanning workspace...

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
    color 0E
    echo [INIT] Build environment created
    color 0A
) else (
    echo [INIT] Build environment detected
)

cd /d "%BUILD_DIR%" || (
    color 0C
    echo [ERROR] Unable to enter build directory
    goto fail
)

REM ================== фаза конфигурации ==================
echo [PHASE-1] Calibrating toolchain
echo [PHASE-1] Source matrix: %ROOT_DIR%

cmake ..
if errorlevel 1 (
    color 0C
    echo [ERROR] Toolchain calibration failed
    goto fail
)

echo [PHASE-1] Toolchain ready

REM ================== фаза сборки ==================
echo [PHASE-2] Assembling artifacts

cmake --build . --config Release
if errorlevel 1 (
    color 0C
    echo [ERROR] Assembly process interrupted
    goto fail
)

echo [PHASE-2] Assembly completed

REM ================== сводка ==================
echo.
echo [SUMMARY] Deployment notes
color 08
echo   Artifacts location: build\bin\Release\
echo.
echo   Test sequence:
echo     cd build\bin\Release
echo     test_code.exe --bin-dir ".\"
color 0A
echo.

REM ================== запуск теста ==================
set /p answer="Хотите запустить тестовый скрипт? [Д/н]:  "

if /i "%answer%"=="" set answer=Y

if /i "%answer%"=="Y" (
    echo [RUN] Launching test script...
    build\bin\Release\test_code.exe --bin-dir "build\bin\Release"
    echo.
    pause
) else if /i "%answer%"=="N" (
    echo [INFO] Test run skipped
) else (
    color 0C
    echo [ERROR] Invalid input, aborting
)

goto end

:fail
echo.
color 0C
echo [FATAL] Process aborted
echo.

:end
color 07
endlocal

