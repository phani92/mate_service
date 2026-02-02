@echo off
REM Build and Upload Script for Mate Tracker ESP32-C3 (Windows)

echo ========================================
echo   Mate Tracker ESP32-C3 Build Script
echo ========================================
echo.

REM Check if PlatformIO is available
where pio >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Error: PlatformIO CLI is not installed
    echo Install it with: pip install platformio
    exit /b 1
)

REM Navigate to firmware directory
cd /d "%~dp0"

if "%1"=="build" (
    echo Building firmware...
    pio run
    goto :done
)

if "%1"=="upload" (
    echo Uploading firmware...
    pio run --target upload
    goto :done
)

if "%1"=="uploadfs" (
    echo Uploading LittleFS filesystem...
    pio run --target uploadfs
    goto :done
)

if "%1"=="all" (
    echo Building and uploading everything...
    pio run --target uploadfs
    pio run --target upload
    goto :done
)

if "%1"=="monitor" (
    echo Opening serial monitor...
    pio device monitor
    goto :done
)

if "%1"=="clean" (
    echo Cleaning build files...
    pio run --target clean
    goto :done
)

echo Usage: %0 {build^|upload^|uploadfs^|all^|monitor^|clean}
echo.
echo Commands:
echo   build     - Compile the firmware
echo   upload    - Upload firmware to ESP32-C3
echo   uploadfs  - Upload LittleFS filesystem (web files)
echo   all       - Build and upload both firmware and filesystem
echo   monitor   - Open serial monitor
echo   clean     - Clean build files
echo.
echo Example workflow:
echo   1. Edit src\config.h with your WiFi credentials
echo   2. Run: %0 all
echo   3. Run: %0 monitor
exit /b 1

:done
echo.
echo Done!
