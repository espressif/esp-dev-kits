# ESP-SensairShuttle Factory Demo

[中文版本](./README_CN.md)

## Overview

Maintained Factory Demo for ESP-SensairShuttle in `esp-dev-kits`: compass,
environment, gesture detect, 2048, and factory guide. Updates focus on bug
fixes, hardware support, and ESP-IDF compatibility. Use it to restore the
factory image after trying other firmware.

## Hardware Requirements

### Supported Boards
- **ESP32-SensairShuttle**

### Required Sensors
- **BMI270** - 6-axis IMU (Accelerometer + Gyroscope)
- **BMM350** - 3-axis Magnetometer
- **BME690** - Environmental sensor (Temperature, Humidity, Pressure, Gas)

### I2C Configuration
Default I2C pins (can be configured in board YAML):
- SDA: GPIO 2
- SCL: GPIO 3
- Frequency: 100 kHz

## Software Requirements

### ESP-IDF Version
- Supported ESP-IDF branches: release/v5.5 and release/v6.0. Build-verified on esp32c5 with **v5.5.4** and **v6.0.1**.

### Dependencies
- Local `brookesia_*` components: in-tree Brookesia 0.7 snapshot. Not interchangeable with Registry `espressif/brookesia_system_core` 0.8.
- `esp_board_manager` - Hardware abstraction layer
- `esp_lvgl_port` - LVGL integration
- `espressif/bme690` - Environmental sensor driver
- `espressif/bmi270_sensor` - IMU driver
- `espressif/bmm350` - Magnetometer driver
- Boost Thread Library - Multi-threading support

## Getting Started

Check the board revision and sensor daughterboards in the
[board user guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32c5/esp-sensairshuttle/index.html).

### 1. Enter the example directory

```bash
cd examples/esp-sensairshuttle/examples/factory_demo
```

### 2. Set up ESP-IDF Environment

Follow the [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c5/get-started/index.html).
Use a version listed above. On Linux or macOS:

```bash
. $HOME/esp/esp-idf/export.sh
```

On Windows, use the ESP-IDF terminal for that version.

### 3. Configure Board (Important)

The example uses ESP Board Manager `~0.7.3~1`, the `esp_boards` package `0.6.2`, and the upstream
`esp_boards/esp_sensairshuttle` definition. The overlay in
`boards/esp_sensairshuttle` retains the tested landscape layout
and touch mapping, while sensor applications own their driver initialization.

Run `set-target esp32c5` to select the chip and download the Registry dependencies
before generating the board configuration. Regenerate after changing the board
package or overlay. ESP-IDF 5.5 requires the extension path below;
ESP-IDF 6.0 also discovers component extensions automatically.

#### Linux / macOS

```bash
idf.py set-target esp32c5
export IDF_EXTRA_ACTIONS_PATH="$PWD/managed_components/espressif__esp_board_manager"
idf.py bmgr -c ./boards -b esp_sensairshuttle
```

#### Windows

```bat
idf.py set-target esp32c5
set IDF_EXTRA_ACTIONS_PATH=%CD%\managed_components\espressif__esp_board_manager
idf.py bmgr -c .\boards -b esp_sensairshuttle
```

### 4. Build and Flash

```bash
# Build the project
idf.py build

# Flash to device (replace PORT with your serial port)
idf.py -p PORT flash monitor
```

To exit the serial monitor, type `Ctrl-]`.

## Factory Firmware Instructions

### Operating System Features

- **Open Application**: Tap the application icon

- **Switch Display (to find different applications)**: Swipe left or right on the page

- **Minimize Open Program**: Swipe up on the virtual home bar

- **Open Program Switcher**: Swipe up on the virtual bar and hold

    - In the program switcher page, you can swipe up on thumbnails to close the selected app's background. Or tap the trash icon below the thumbnails to close all currently open applications.

    - In the program switcher page, the top of the screen displays the used SRAM and PSRAM ratio.

## Application Guide

### Environment (Requires BME690 sensor sub-board to be inserted)

Displays current environmental temperature, relative humidity, pressure, and air quality.

- **Relative Humidity Interface**: Displays relative humidity and comfort level emoji (emoji color changes based on comfort level)
  - 30-60%: ^_^ (Comfortable, displayed in green)
  - 20-30% or 60-70%: 0_0 (Normal, displayed in yellow)
  - <20% or >70%: T_T (Uncomfortable, displayed in orange)

- **Pressure Interface**: Displays current pressure and inferred weather (weather text color changes based on pressure)
  - >1018.25 hPa (Standard pressure 1013.25 hPa + 5.0 hPa): Maybe Sunny (Possible sunny, displayed in green)
  - <1008.25 hPa (Standard pressure 1013.25 hPa - 5.0 hPa): Maybe Rainy (Possible rainy, displayed in red)
  - 1008.25-1018.25 hPa: Good (Normal, displayed in gray)

- **Air Quality Interface**: Displays Indoor Air Quality Index (IAQ) and CO2 equivalent concentration (IAQ value and level text color change based on air quality level).
  - **IAQ Display**: 0-500 range mapped to 0-100% display, lower values indicate better air quality. The interface displays English level text (color changes based on level)
    - 0-50: Excellent (Excellent, displayed in green)
    - 51-100: Good (Good, displayed in light green)
    - 101-150: Fair (Normal, displayed in yellow)
    - 151-200: Poor (Poor, displayed in orange)
    - 201-300: Bad (Bad, displayed in red)
    - 301-500: Very Bad (Very bad, displayed in dark red)
  - **CO2 Equivalent Concentration**: Displays estimated CO2 concentration (unit: ppm), value color changes based on current IAQ level (consistent with level text color).
  - **Usage Instructions**:
    - Air quality is implemented through the BSEC algorithm of the BME690 sensor. First use requires 5-30 minutes to collect initial data. IAQ value accuracy improves after 15-30 minutes
    - After running for 4-24 hours, IAQ accuracy reaches maximum and values are most reliable. Keep the application running continuously to maintain calibration state. Keep away from heat sources, strong airflow, and chemicals for best performance

### Gesture Detect (Requires BMI270 & BMM350 sensor sub-boards to be inserted)

Detects the current motion state of the development board and displays the following categories.

- **Roll**: Detects device roll action (angle >90°). When roll is detected, the interface displays the current device's absolute orientation information, such as "X Down [vvv]", "Y Up [<<<]", "Z Down [X]", etc., where arrow symbols (such as [^^^], [vvv], [>>>], [<<<], [X], [O]) indicate roll direction. Supported orientations include: X Up/Down, Y Up/Down, Z Up/Down.
- **Shake**: Detects device shake action. When shake is detected, the interface displays the coordinate axis information where the shake action was detected, such as "Shake heavy on X", "Shake slight on Y/Z", etc. It can display a single coordinate axis (X, Y, Z) or combinations of multiple coordinate axes (such as X/Y, Y/Z, X/Y/Z), and distinguishes between heavy shake and slight shake.

### Compass (Requires BMI270 & BMM350 sensor sub-boards to be inserted)

Implements compass functionality. Keep level and away from magnetic interference sources.

- **First-time Calibration**:
  1. Launch the Compass app from the home screen
  2. Follow the on-screen instructions for calibration
  3. Slowly rotate the device in **all directions** (figure-8 pattern)
  4. Ensure coverage of all spatial orientations. Calibration requires at least 500 samples

- **Recalibration**:
  - **Long press screen**: Long press on the compass interface to recalibrate
  - **Auto-save**: After calibration is complete, hard/soft iron parameters and status are written to NVS and automatically restored after restart

- **Using the Compass**:
  - The pointer will automatically point to magnetic north
  - Tilt compensation ensures accurate display even when the device is not level
  - Real-time heading update frequency is 20 Hz
  - **Data Persistence**: Calibration data remains valid after device restart

- **Troubleshooting**:
  - **Calibration fails**: Ensure away from magnetic interference sources (electronic devices, metal structures, coiled power lines)
  - **Unstable readings**: Recalibrate in an open area
  - **Data loss**: If calibration data is lost, the system will automatically prompt for recalibration

### 2048

2048 game.

### Factory Guide

Displays a QR code for the current `User Guide` web link.


## Project Structure

```
factory_demo/
├── main/                          # Main application
│   ├── main.cpp                  # Main program entry
│   ├── display.cpp               # Display initialization
│   ├── display.hpp               # Display header file
│   ├── idf_component.yml         # Dependency configuration
│   └── CMakeLists.txt            # Build configuration
├── common_components/            # Common components
│   ├── brookesia_app_compass/           # Compass application
│   ├── brookesia_app_gesture_detect/    # Gesture detection application
│   ├── brookesia_app_temperature/       # Temperature & air quality application
│   ├── brookesia_app_game_2048/         # 2048 game application
│   ├── brookesia_app_factory_guide/     # Factory guide application
│   ├── brookesia_system_core/          # In-tree Brookesia 0.7 core
│   └── brookesia_system_phone/         # In-tree Brookesia 0.7 phone system
├── boards/                       # Application-specific BMGR overlays
│   └── esp_sensairshuttle/       # Landscape UI and device pruning
└── CMakeLists.txt                # Project build configuration
```

## Known Limitations

### Compass Related
1. **Magnetic interference**: Performance degrades near strong magnetic fields
2. **Dynamic declination**: Manual magnetic declination correction not yet implemented

### Air Quality Related
3. **IAQ calibration time**: First use requires 5-30 minutes for initial calibration
4. **BSEC state persistence**: State save not yet implemented, requires recalibration after restart
5. **Environmental adaptability**: BSEC algorithm performs best in stable environments

## Troubleshooting

### Sensor Initialization Fails
```
Check I2C connections
Verify sensor addresses (BMI270: 0x68, BMM350: 0x14/0x15, BME690: 0x76)
Ensure pull-up resistors on I2C lines
Check if sensor sub-boards are correctly inserted
```

### IAQ Shows Initial Value or Not Updating
```
Problem: IAQ stays at 50, accuracy stays at 0
Causes:
  1. Sensor just started, stabilizing (wait for a period of time)
  2. Gas sensor data invalid (check heater operation)
  3. BSEC not receiving continuous valid data
  
Solutions:
  1. Wait at least 10 minutes, observe if accuracy changes from 0
  2. Check serial logs for "Gas measurement not valid" warnings
  3. Verify sensor power supply and connections
  4. Increase thread stack size (minimum 16KB)
  5. Check BSEC log output to confirm algorithm is running
```

### IAQ Accuracy Not Improving
```
Problem: IAQ accuracy stuck at 0 or 1 for extended periods
Causes:
  1. Environment too stable, lacking air quality variations
  2. Sensor needs exposure to different air quality conditions to learn
  
Solutions:
  1. Expose device to different environments (outdoor, kitchen, well-ventilated room)
  2. Run continuously for 24+ hours
  3. Ensure sensor can detect air quality changes
```

### Gesture Detect Application Initialization Fails
```
Problem: Application displays "Initialize BMI270 failed. Please insert the BMI270 sub-board."
Causes:
  1. BMI270 sensor sub-board not inserted or connection issue
  2. I2C bus initialization failed
  3. BMI270 sensor configuration failed
  
Solutions:
  1. Check if BMI270 & BMM350 sensor sub-boards are correctly inserted
  2. Check I2C connections (SDA: GPIO 2, SCL: GPIO 3)
  3. Check specific error messages in serial logs
  4. Verify sensor address (BMI270: 0x68)
  5. Ensure pull-up resistors on I2C lines
```

### Program Crash or Stack Overflow
```
Problem: Guru Meditation Error: Stack protection fault
Causes:
  1. Insufficient thread stack space (BSEC requires larger stack)
  2. Too many parameters in log formatting
  
Solutions:
  1. Increase boost::thread stack size to 16KB or larger
  2. Reduce number of parameters in single log output
  3. Avoid formatting too many floating-point numbers in logs
```

### Thread Issues
```
Suggestions:
  1. Enable debug logs (e.g. add ESP_LOGI in close())
  2. Check thread stop messages in serial output
  3. Verify bmm_running_ flag transitions
```


## Contributing

For bug reports, include the board revision, firmware commit, ESP-IDF version,
connected sensors, and serial logs.

## License

Apache License 2.0. See [LICENSE](../../LICENSE).

## Technical Support

- **Forum**: [esp32.com](https://esp32.com/viewforum.php?f=35)
- **GitHub Issues**: [esp-dev-kits/issues](https://github.com/espressif/esp-dev-kits/issues)
- **Documentation**: [ESP-SensairShuttle User Guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32c5/esp-sensairshuttle/index.html)
