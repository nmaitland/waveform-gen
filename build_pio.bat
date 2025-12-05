@echo off
REM PlatformIO Build Script for Waveform Generator

echo === PlatformIO Build Script ===
echo.

REM Check if Python is installed
echo Checking Python installation...
python --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Python not found! Please install Python 3.x
    exit /b 1
)
python --version
echo.

REM Create virtual environment
echo Creating virtual environment...
if not exist ".venv" (
    python -m venv .venv
    if exist ".venv" (
        echo Virtual environment created successfully
    ) else (
        echo ERROR: Failed to create virtual environment
        exit /b 1
    )
) else (
    echo Virtual environment already exists
)
echo.

REM Install PlatformIO
echo Installing PlatformIO...
call .venv\Scripts\pip.exe install -r requirements.txt
if errorlevel 1 (
    echo ERROR: Failed to install PlatformIO
    exit /b 1
)
echo PlatformIO installed successfully
echo.

REM Run PlatformIO build
echo Building project with PlatformIO...
call .venv\Scripts\platformio.exe run
if errorlevel 1 (
    echo ERROR: Build failed!
    exit /b 1
)
echo.
echo Build completed successfully!
echo.

REM Check for hex file
set "hexFile=.pio\build\nanoatmega328\firmware.hex"
if exist "%hexFile%" (
    echo Hex file generated:
    echo   Path: %cd%\%hexFile%
    for %%A in ("%hexFile%") do echo   Size: %%~zA bytes
) else (
    echo WARNING: Hex file not found at expected location
    echo Searching for hex files...
    for /r ".pio" %%F in (*.hex) do echo   Found: %%F
)
echo.

echo === Build Complete ===
