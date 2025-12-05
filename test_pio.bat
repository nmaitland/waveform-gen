@echo off
REM PlatformIO Test Script using Docker
REM Runs native tests in a containerized environment with gcc/g++

echo === PlatformIO Test Script (Docker) ===
echo.

REM Check if Docker is running
echo Checking Docker installation...
docker info >nul 2>&1
if errorlevel 1 (
    echo ERROR: Docker is not running or not installed
    echo Please install Docker Desktop and ensure it's running
    exit /b 1
)
echo Docker is running
echo.

REM Run tests in Docker container
echo Running native tests in Docker container...
echo This will install gcc/g++ and run the test suite
echo.
docker-compose run --rm platformio

if errorlevel 1 (
    echo.
    echo ERROR: Tests failed!
    exit /b 1
)

echo.
echo === Tests Complete ===
