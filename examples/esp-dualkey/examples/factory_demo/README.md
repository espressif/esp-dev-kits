# ESP-DualKey Smart Button Controller Example

This project demonstrates the smart button controller functionality based on the ESP-DualKey development board. The device integrates smart home control, Bluetooth keyboard, USB keyboard and other functions, and can switch between different working modes through physical switches.

## Features

### 1. Smart Home Controller Mode (ESP RainMaker)

- Integrates with the RainMaker cloud platform
- Functions as both a smart switch and smart bulb
- Supports scene automation and remote control
- Allows adjustment of button backlight brightness, hue, saturation, and blinking mode

### 2. Bluetooth Keyboard Mode

- Operates as a Bluetooth HID device
- Uses the same key layout as USB HID
- Designed for low power consumption

### 3. USB Keyboard Mode

- Operates as a USB HID device
- Plug-and-play functionality, no pairing required
- Supports customizable key configurations via the Via web interface
- Preconfigured with four default key functions

## Usage Instructions

### Mode Switching

Use the physical switch to change operating modes:

- **Left position**: Bluetooth Keyboard Mode
- **Right position**: ESP RainMaker Mode
- **OFF position**: USB Keyboard Mode

### RainMaker Mode Configuration

1. Switch to the **right** position to enter RainMaker Mode.
2. Two LEDs blinking alternately indicate the device is waiting for provisioning or connection.
3. In the RainMaker app, select `Add Device`, then choose `BLE (no QR code)`.
4. Set the device name prefix to `M_Key` during provisioning.
5. After successful provisioning, the button lighting will switch to a preset mode (default: **Heatmap Mode**).

#### Controllable Parameters

- **Brightness** – Adjusts button backlight brightness
- **Hue** – Adjusts lighting hue
- **Saturation** – Adjusts color saturation
- **Switch L/R** – Controls left and right button states
- **Keyboard Layer** – Selects between USB and BLE HID key mappings
- **Light Mode** – Changes button lighting effects

### Bluetooth Keyboard Mode Configuration

1. Switch to the **left** position to enter Bluetooth Keyboard Mode.
2. Two LEDs blinking alternately indicate the device is waiting for a connection.
3. On your phone or computer, connect to the Bluetooth device named `ESP_DualKey`.
4. By default, the key functions are assigned to **volume control**.
5. After a successful connection, the button lighting switches to the preset mode (default: **Heatmap Mode**).

### USB Keyboard Mode

1. Switch the mode selector to the `OFF` position.
2. Connect the device to a computer via USB.
3. The computer automatically recognizes it as a USB keyboard.


## Getting Started

### Prerequisites

- ESP-DualKey development board

### ESP-IDF Required

- This example supports ESP-IDF release/v[5.5] and all bugfix versions.

### Get the esp-dev-kits Repository

To start from the examples in esp-dev-kits, clone the repository to the local PC by running the following commands in the terminal:

```
git clone --recursive https://github.com/espressif/esp-dev-kits.git
```


## Configuration Options

The project supports the following configuration options (configured through `idf.py menuconfig`):

- **CONFIG_LIGHT_SLEEP_TIMEOUT_MS**: Enter Light-sleep after CONFIG_LIGHT_SLEEP_TIMEOUT_MS of time without pressing any key
- **CONFIG_DEEP_SLEEP_TIMEOUT_S**：Enter Deep-sleep after CONFIG_DEEP_SLEEP_TIMEOUT_S of time without pressing any key


## How to Use the Example

### Build and Flash the Example

### Environment Setup

1. Install ESP-IDF development environment
2. Set RainMaker path:
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

## License

This project is open-source under the Apache 2.0 License.

## Notes

The Power-Save function is enabled by default, so the device can be powered on directly.

## Related Documentation

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32s3/)
- [RainMaker Cloud Platform Documentation](https://rainmaker.espressif.com/)

## Technical Support and Feedback

Please use the following feedback channels:

- For technical queries, go to the [esp32.com](https://esp32.com/viewforum.php?f=22) forum.
- For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-dev-kits/issues).

We will get back to you as soon as possible.
