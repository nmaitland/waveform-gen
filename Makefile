.PHONY: help build test clean deep-clean run-wokwi venv install upload hex

# Configuration
PIO_BUILD_DIR = .pio/build/nanoatmega328
VENV_DIR = venv

# Python/PlatformIO commands
ifeq ($(OS),Windows_NT)
    PYTHON = python
    VENV_BIN = $(VENV_DIR)\Scripts
    VENV_PYTHON = $(VENV_BIN)\python.exe
    VENV_PIP = $(VENV_BIN)\pip.exe
    VENV_PIO = $(VENV_BIN)\platformio.exe
else
    PYTHON = python3
    VENV_BIN = $(VENV_DIR)/bin
    VENV_PYTHON = $(VENV_BIN)/python
    VENV_PIP = $(VENV_BIN)/pip
    VENV_PIO = $(VENV_BIN)/platformio
endif

# Detect Windows
ifeq ($(OS),Windows_NT)
    MKDIR = if not exist $(subst /,\,$(1)) mkdir $(subst /,\,$(1))
    RMDIR = if exist $(subst /,\,$(1)) rmdir /s /q $(subst /,\,$(1))
    RM = if exist $(subst /,\,$(1)) del /q $(subst /,\,$(1))
    PATHSEP = \
    SHELL = cmd
else
    MKDIR = mkdir -p $(1)
    RMDIR = rm -rf $(1)
    RM = rm -f $(1)
    PATHSEP = /
endif

help:
	@echo "Available targets:"
	@echo "  install         Install PlatformIO in virtual environment"
	@echo "  build           Build the project"
	@echo "  test            Run tests"
	@echo "  hex             Build and show hex file location"
	@echo "  upload          Upload to device"
	@echo "  clean           Clean build artifacts"
	@echo "  deep-clean      Clean everything including virtual environment"
	@echo "  run-wokwi       Start the Wokwi simulation"
	@echo "  win             Windows PowerShell build (alternative)"

# Create Python virtual environment
venv:
	@echo Creating Python virtual environment...
	@if not exist "$(VENV_DIR)" $(PYTHON) -m venv $(VENV_DIR)
	@echo Virtual environment created.

# Install PlatformIO
install: venv
	@echo Installing PlatformIO in virtual environment...
	@$(VENV_PIP) install -r requirements.txt
	@echo PlatformIO installed.

# Build the project
build: install
	@echo Building with PlatformIO...
	@$(VENV_PIO) run

# Run tests
test: install
	@echo Running tests with PlatformIO...
	@$(VENV_PIO) test

# Clean build artifacts
clean:
	@echo Cleaning PlatformIO build...
	@if exist "$(VENV_PIO)" $(VENV_PIO) run --target clean
	@if exist ".pio" rmdir /s /q ".pio"

# Deep clean including virtual environment
deep-clean: clean
	@echo Cleaning virtual environment...
	@if exist "$(VENV_DIR)" rmdir /s /q "$(VENV_DIR)"

# Upload to device
upload: install
	@echo Uploading with PlatformIO...
	@$(VENV_PIO) run --target upload

# Build and show hex file location
hex: build
	@echo.
	@echo Hex file generated at:
	@echo $(PIO_BUILD_DIR)/firmware.hex
	@echo.

# Run the Wokwi simulation
run-wokwi:
	@echo "Note: Wokwi CLI requires Node.js and npm to be installed"
	@wokwi-cli --version || npm install -g @wokwi/wokwi-cli
	@cd wokwi && wokwi-cli start --id .

# Windows PowerShell build (alternative)
win:
	@powershell -ExecutionPolicy Bypass -File build_pio.ps1
