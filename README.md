# Waveform Generator with AD9833

A microcontroller-based waveform generator using the AD9833 DDS chip, featuring a rotary encoder interface and LCD display. Built with PlatformIO for Arduino-compatible boards. The project includes both embedded firmware and Wokwi simulation support.

## Features

- **Waveform Types**: Sine, Square, and Triangle waves
- **Frequency Range**: 1Hz to 12.5MHz
- **Adjustable Step Sizes**: 1, 10, 100, 1K, 10K, 100K, 1M
- **Interactive Interface**:
  - Rotary encoder for frequency adjustment with acceleration
  - Push buttons for waveform selection and step size adjustment
  - 16x2 LCD display showing frequency and settings

## Hardware Requirements (Embedded Version)

- Arduino Nano (ATmega328) or compatible board
- AD9833 DDS Module
- 16x2 I2C LCD Display
- Rotary Encoder with push button
- 2x Push buttons
- Connecting wires

## PlatformIO Configuration

This project is configured for Arduino Nano (ATmega328). To change the target board, modify `platformio.ini`:
```ini
[env:nanoatmega328]
platform = atmelavr
board = nanoatmega328  ; Change this for different board
framework = arduino
```

## Pin Connections

| Component | Arduino Pin |
|-----------|-------------|
| Rotary Encoder CLK | D2 |
| Rotary Encoder DT | D3 |
| Waveform Button | D9 |
| Scale/Step Button | D4 |
| AD9833 FSYNC (SS) | D10 |
| AD9833 SCLK | D13 (SCK) |
| AD9833 SDATA | D11 (MOSI) |
| I2C SDA | A4 (SDA) |
| I2C SCL | A5 (SCL) |

## Wokwi Simulation

The project includes a Wokwi simulation that mimics the hardware behavior:

- Located in the `/wokwi` directory
- Simulates the AD9833 chip behavior
- Includes a virtual oscilloscope for waveform visualization
- Uses the same codebase with conditional compilation for simulation

### Running the Simulation

1. Open the project in Wokwi
2. The simulation will start automatically
3. Use the virtual rotary encoder and buttons to control the waveform generator

## Dependencies

- [RotaryEncoder](https://github.com/mathertel/RotaryEncoder)
- [Bounce2](https://github.com/thomasfredericks/Bounce2)
- [AD9833](https://github.com/billwilliams1952/AD9833-Library)
- [LiquidCrystal_I2C](https://github.com/johnrickman/LiquidCrystal_I2C)
- [Arduino_DebugUtils](https://github.com/arduino-libraries/Arduino_DebugUtils)

## Building and Flashing

### Using Make (Recommended)
```bash
# Install PlatformIO and build
make install
make build

# Upload to device
make upload

# Build and see hex file location
make hex
```

### Manual Build
```bash
# Create virtual environment and install PlatformIO
python -m venv .venv
.venv\Scripts\pip install -r requirements.txt

# Build project
.venv\Scripts\platformio run

# Upload to device
.venv\Scripts\platformio run --target upload
```

### Windows Batch File Alternative
```cmd
# Run batch build script
build_pio.bat
```

## Testing

This project includes a comprehensive native test suite that verifies core calculation logic without requiring hardware.

### Running Tests Locally (Windows)

Using Docker (recommended - no gcc installation needed):
```cmd
# Requires Docker Desktop to be running
test_pio.bat
```

After running tests, a code coverage report is generated:
- **Coverage summary**: Displayed in terminal output
- **HTML report**: `coverage-report/index.html` (open in browser)

### Running Tests in CI/CD

The GitLab CI pipeline automatically runs tests on every push:
- **Native tests**: 13 unit tests covering frequency calculations and utilities
- **Compilation tests**: Verifies firmware compiles for Arduino Nano
- **Test reporting**: Results displayed in merge requests with pass/fail details
- **Code coverage**: Coverage percentage shown in MRs with detailed HTML reports

### Test Coverage

The test suite verifies:
- Frequency scale detection (Hz/KHz/MHz)
- Frequency clamping to valid ranges (1Hz - 12.5MHz)
- Step value cycling (1→10→100→...→1M→1)
- Frequency divisor calculations

## Usage

1. Power on the device
2. The LCD will display the current frequency and waveform
3. Rotate the encoder to adjust frequency
4. Press the waveform button to cycle through available waveforms
5. Press the scale button to change the frequency step size

## Project Structure

```
waveform-gen/
├── src/                      # Main source code
│   └── fx-gen.cpp            # Main firmware
├── include/                  # Header files
│   └── waveform_utils.h      # Testable utility functions
├── test/                     # Test suite
│   └── test_native/          # Native unit tests
│       └── test_waveform_utils.cpp
├── wokwi/                    # Wokwi simulation files
│   ├── ad9833.chip.c         # AD9833 simulation
│   ├── sketch.ino            # Simulation entry point
│   └── diagram.json          # Wokwi circuit diagram
├── platformio.ini            # PlatformIO configuration
├── .gitlab-ci.yml            # CI/CD pipeline configuration
├── docker-compose.yml        # Docker test environment
├── build_pio.bat             # Windows build script
├── test_pio.bat              # Windows test script (Docker)
└── requirements.txt          # Python dependencies
```

## CI/CD Pipeline

This project uses GitLab CI/CD for automated testing and deployment:

### Pipeline Stages

1. **Test Stage**
   - Native unit tests (13 tests covering core logic)
   - Firmware compilation for Arduino Nano
   - Memory usage reporting

2. **Build Stage** (tags only)
   - Release artifact generation (.hex and .elf files)
   - Version tagging

3. **Deploy Stage**
   - GitLab Pages deployment with downloadable firmware
   - Test results visualization in merge requests

### Viewing Test Results

Test results are automatically displayed in merge requests:
- **Test Summary**: Widget shows pass/fail counts
- **Test Details**: Available in "Tests" tab with execution times
- **Code Coverage**: Coverage percentage badge in MR overview
- **Coverage Report**: Download HTML coverage report from pipeline artifacts

## Contributing

Contributions are welcome! To contribute:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes and ensure tests pass
4. Commit your changes (`git commit -m 'Add amazing feature'`)
5. Push to the branch (`git push origin feature/amazing-feature`)
6. Open a merge request

### Development Setup

1. Install dependencies: `pip install -r requirements.txt`
2. Run tests locally: `test_pio.bat` (Windows with Docker)
3. Build firmware: `build_pio.bat` or `make build`
4. Ensure CI pipeline passes before submitting MR

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

This means you are free to use, modify, and distribute this software, even commercially, as long as you include the original copyright notice.
