# ESP-DUALKEY Smart Button Controller Example

## Overview

This project demonstrates the smart button controller functionality based on the ESP-DUALKEY development board. The device integrates smart home control, Bluetooth keyboard, USB keyboard and other functions, and can switch between different working modes through physical switches.

## Hardware Requirements

- ESP-DUALKEY development board

## IDF Version

- esp-idf release/v5.5

## Features

### 1. Smart Home Controller Mode (Rainmaker)
- Supports Rainmaker cloud platform integration
- Can function as smart switch and smart bulb
- Supports scene automation and remote control
- Supports adjusting button backlight blinking mode brightness, hue, and saturation

### 2. Bluetooth Keyboard Mode
- Supports Bluetooth HID device functionality
- Uses the same key layout as USB HID
- Supports low power consumption

### 3. USB Keyboard Mode
- Supports USB HID device functionality
- Plug and play, no pairing required
- Supports arbitrary key configuration using web interface (Via)
- Pre-configured with four key functions

## Usage Instructions

### Mode Switching
Switch working modes through physical switch:
- **Left position**: Bluetooth keyboard mode
- **Right position**: Rainmaker mode  
- **OFF position**: USB keyboard mode

### Rainmaker Mode Configuration
1. Switch to the right to enter Rainmaker mode
2. Two LEDs alternately blinking indicates waiting for provisioning/connection
3. Through Rainmaker APP, select add device, choose BLE (no QR code)
4. Set device prefix starting with "M_Key" for provisioning
5. After successful provisioning, button lighting enters preset state (default is heatmap mode)

#### Controllable Parameters
- **Brightness**: Controls device's own lighting brightness
- **Hue**: Controls lighting hue
- **Saturation**: Controls lighting saturation
- **Switch L/R**: Left/right button switch states
- **Keyboard Layer**: Select USB/BLE HID key functions
- **Light Mode**: Select button lighting effects

### Bluetooth Keyboard Mode Configuration
1. Switch to the left to enter Bluetooth keyboard mode
2. Two LEDs alternately blinking indicates waiting for connection
3. Connect to "ESP_DualKey" device via Bluetooth on phone/computer
4. Key functions are set to volume control by default
5. After successful connection, button lighting enters preset state (default is heatmap mode)

### USB Keyboard Mode
1. Switch to OFF position
2. Connect to computer via USB
3. Device is automatically recognized as USB keyboard

## Build and Flash

### Environment Setup
1. Install ESP-IDF development environment
2. Set Rainmaker path:
   ```bash
   export RMAKER_PATH=/path/to/rainmaker
   ```

### Build Steps
1. Enter ESP-IDF directory and source environment:
   ```bash
   cd esp-idf
   . ./export.sh
   ```

2. Enter example directory:
   ```bash
   cd esp-dualkey/examples/factory_demo
   ```

3. Build and flash:
   ```bash
   idf.py flash monitor
   ```

## Project Structure

```
factory_demo/
├── main/                           # Source code
│   ├── main.c                      # Main program entry
│   ├── esp_dualkey.c               # ESP DualKey main implementation
│   ├── esp_dualkey.h               # ESP DualKey header file
│   ├── CMakeLists.txt              # Main CMake configuration
│   ├── idf_component.yml           # Component dependencies
│   ├── Kconfig.projbuild           # Project configuration options
│   ├── dual_button/                # Dual button handling
│   │   ├── dual_button.c
│   │   └── dual_button.h
│   ├── btn_progress/               # Button processing
│   │   ├── btn_progress.c
│   │   ├── btn_progress.h
│   │   └── media_control_config.h
│   ├── app_rainmaker/              # Rainmaker cloud integration
│   │   ├── app_rainmaker.c
│   │   └── app_rainmaker.h
│   ├── app_nvs/                    # NVS storage management
│   │   ├── app_nvs.c
│   │   └── app_nvs.h
│   ├── app_utils/                  # Application utilities
│   │   ├── app_utils.c
│   │   └── app_utils.h
│   ├── ble_hid/                    # Bluetooth HID functionality
│   │   ├── ble_hid.c
│   │   ├── ble_hid.h
│   │   ├── esp_hid_gap.c
│   │   └── esp_hid_gap.h
│   ├── usb_hid/                    # USB HID functionality
│   │   ├── tinyusb_hid.c
│   │   ├── tinyusb_hid.h
│   │   ├── tusb_config.h
│   │   ├── usb_descriptors.c
│   │   └── usb_descriptors.h
│   ├── light_manager/              # LED lighting control
│   │   ├── light_manager.c
│   │   └── light_manager.h
│   └── via_keymap/                 # VIA keymap configuration
│       ├── via_keymap.c
│       └── via_keymap.h
├── json/                           # JSON configuration files
│   └── ESP_DualKey_VIA_Definition.json
├── CMakeLists.txt                  # Root CMake configuration
├── sdkconfig.defaults              # Default SDK configuration
├── partitions.csv                  # Partition table configuration
└── README.md                       # Project documentation
```

## Configuration Options

The project supports the following configuration options (configured through `idf.py menuconfig`):

- **CONFIG_LIGHT_SLEEP_TIMEOUT_MS**: Enter light sleep after CONFIG_LIGHT_SLEEP_TIMEOUT_MS of time without pressing any key
- **CONFIG_DEEP_SLEEP_TIMEOUT_S**：Enter deep sleep after CONFIG_DEEP_SLEEP_TIMEOUT_S of time without pressing any key

## License

This project is open source under the Apache 2.0 License.

## Notes

Power Save function is enabled by default, so the device can be powered on directly

## Related Documentation

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32s3/)
- [Rainmaker Cloud Platform Documentation](https://rainmaker.espressif.com/)
