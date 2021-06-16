[[中文]](./README_cn.md)
# ESP32-S2-HMI-DevKit-1 

This document provides information on ESP32-S2-HMI-DevKit-1.

## Overview

ESP32-S2-HMI-DevKit-1 is an HMI development board designed based on the ESP32-S2. The following figure describes its key on-board resources:

![ESP32-S2-HMI-DevKit-0 开发板功能框图](docs/_static/board_func.jpg)

The ESP32-S2-HMI-DevKit-1 board is embedded with a ESP32-S2-WROVER module, with 4 MB flash and 2 MB PSRAM included. It has a 4.3-inch TFT-LCD which uses 16-bit 8080 parallel port with 480×800 resolution, and 256-level hardware DC backlight adjustment circuit (V1.1 does not support this function yet). On top of that, this board is equipped with a capacitive touch panel, which can use I2C interface for communication.

This board has rich components including IMU with 3-axis accelerometer and 3-axis gyroscope, ambient light sensor, temperature and humidity sensor, IO expander, I2C ADC, programmable RGB LEDs, microphone, audio PA, Micro-SD card connector and etc.

It provides multiple extended interfaces for customized development, including 5 V and 3.3 V power supply interface, programming/UART interface, I2C interface, SPI interface, USB interface (supports USB OTG), TWAI interface (compatible with CAN 2.0) and etc.

It is also equipped with a 1950 mAh single-core lithium battery, with a charge IC for power supplement.

## What You Need 

- 1 x PC loaded with Windows, mac OS or Linux (Linux operating system is recommended)
- 1 x ESP32-S2-HMI-DevKit-1
- 1 x USB-C cable (it is recommended to prepare two USB-C cables if you want to evaluate MCU's USB functions)
- 1 x Speaker (8 Ohm, 2 W)
- 1 x Micro-SD card (some examples may have large storage needs)

## Get Started

Before powering up the ESP32-S2-HMI-DevKit-1, please make sure that the board has been received in good condition with no obvious signs of damage.

### Initial Setup

To facilitate your quick evaluation of all examples, please follow these steps to set up the board:

2. Insert Micro-SD card into the connector. Please make sure all the important data is backed up, as the Micro-SD card may be formatted if its partition format is not FAT.
3. If you need to evaluate the audio playback function, please connect the speaker pad near the USB port on the bottom of the board to the supplied speaker, or to another speaker with similar size (8 Ohm, 2 W).

### Build Projects

First, please make sure you have configured the ESP-IDF development environment correctly. To ensure this, please enter `idf.py --version` in your terminal and if the output is similar to `ESP-IDF v4.2-dev-2084-g98d5b5dfd`, it means you have installed ESP-IDF correctly. For detailed information about installation and configuration, please refer to [ESP-IDF Get Started](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/get-started/index.html).

After configuration completed, you can switch back to the `esp-dev-kits/esp32-s2-hmi-devkit-1` directory. All code examples are placed under the examples directory, you can build projects by running `idf.py build`.

### Project Options

You can configure project options by entering `idf.py menuconfig` in each example directory.

Please make sure to correctly configure the following options in menuconfig:

- `(Top) > HMI Board Config > HMI board`: Select board version. By default, please select `ESP32-S2-HMI-DevKit-V2`;
- `(Top) > HMI Board Config > Audio HAL`: Select audio output interface, whether to use PWM or DAC;
- `(Top) > HMI Board Config > LCD Drivers`: Select display IC type for LCD. By default, ESP32-S2-HMI-DevKit-1 uses RM68120 as its display IC;
- In `(Top) > Component config > ESP32S2-specific`, please go to the `Support for external, SPI-connected RAM` option:
  - Go to `SPI RAM config > Set RAM clock speed`, and set the PSRAM clock as `80 MHz clock speed`;
- `(Top) -> Component config -> FreeRTOS`: set `Tick rate (Hz)` as 1000.

In each example folder, we have provided a default configuration file named `sdkconfig.defaults`, with above options configured correctly.

### IDF Version Dependencies

The `examples/storage/usb_msc` example needs to be built in IDF v4.3, while other examples can be built in IDF v4.2 and later versions.

<font color="red">**Notice:** Due to strict regulation on battery export, for deliveries outside of China mainland, we are shipping ESP32-S2-HMI-DevKit-1 without the battery. As a substitute, you can use the iPhone 5 replacement battery (the connector type is non-standard). The battery capacity is not critical.
