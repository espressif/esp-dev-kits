# ESP32-S2-HMI-DevKit-1 Instructions for Use

This document describes how to use the ESP32-S2-HMI-DevKit-1 development board.
## Overview

The board is an ESP32-S2-based HMI board. On-board resources are shown in the following image.

![ESP32-S2-HMI-DevKit-0 Board function block diagram](docs/_static/board_func.jpg)

The board uses the ESP32-S2-WROVER module, which has 4MB flash and 2MB PSRAM built in. The board features a 4.3-inch TFT-LCD with a 16-bit 8080 and a resolution of 480×800 with a 256-stage hardware DC backlight adjustment circuit. It is equipped with a capacitive touch panel that communicates using the I2C interface.

The board is equipped with inertial sensors with three-axis accelerometers and three-axis gyroscopes, ambient light sensors, temperature and humidity sensors, IO extenders, programmable RGB LED lights, dual-wire, audio amplifiers, SD card interfaces and more.

The board provides a number of extended interfaces for secondary development, including 5 V and 3.3 V power interfaces, program download/UART interfaces, I2C interfaces, SPI interfaces, USB interfaces (USB OTG support), TWAI (compatible WITH CAN 2.0) interfaces and more.

The board is equipped with a single-cell lithium-ion battery of 1950 mAh and a charging IC to charge the battery.

> The engineering version of the board has the following issues that may or may not cause some functions to function properly:
>
>- The entire board has not been designed for low power for the time being, causing the remaining ICs and some resistors of the chip in Deep Sleep mode to still consume current (tested between 28 and 40 mA). Therefore, the battery is not connected to the motherboard when shipped by default to prevent the battery from over-letting over. This issue has been issued in the official version.
>- The light sensor used in the engineering version causes the I2C bus to be clamped to about 1.2 V, which in turn prevents the bus from communicating properly. The device has been removed by default shipping and the issue will occur in the official version.
>- IO46 must be low when entering download mode, but the input used by IO46 as an IO extender is high when no is running, which may prevent downloading. When the device is powered on, it is configured by default as a weak pull-up input, and the falling edge of each channel is sent to IO46 at a low level as a processing condition and remains low until the device's IO input register is read. In this way, the IO46 can be pulled down by touching the screen by touching the IC's in-in-one IO extension IO extender, thus successfully entering download mode. This issue has been issued in the official version.
>- The infrared diode and digital LED (WS2812) do not have a switching circuit set up at this time. As a result, leDs may flash or light up during the operation of the infrared diode, and infrared diodes may also emit an outside line red during operation of the LED. This issue has been issued in the official version.
>- When the hardware is reset, the screen content may not be updated immediately. Because the display IC of the screen comes with a memory, if the display IC does not reset at the same time as the MCU, it retains the contents of the previous memory until it is reset by the new overwritten data or hardware. This issue has been issued in the official version.

## Hardware preparation

- 1 x PCs with Windows, Mac OS, or Linux (Linux recommended)
- 1 x ESP32-S2-HMI-DevKit-1
- 1 x USB-C cable (2 USB-C cables recommended if you want to evaluate the USB function of the MCU at the same time)
- 1 x speaker (8 Ohm, 2 W)
- 1 x Micro-SD card (some examples may require mass storage)

### Get started

Before powering on, make sure that the ESP32-S2-HMI-DevKit-1 development board is intact.
Initial settings

To make it easier for you to quickly evaluate all routines, follow these steps to set up the board:

1. Insert the Micro-SD card into the card slot. Make sure that your important material is backed up and that the Micro-SD may be formatted if the partition is not in FAT format.
2. If you need to evaluate the playback audio function, connect the speaker pad near the USB port below the board to the included speakers, or to other speakers of similar size (8 Ohm, 2 W).

> The engineering version of the speaker is not currently welded to the motherboard and requires you to weld it yourself.

### Compilation project

First, make sure that you have completed the IDF environment configuration correctly. To ensure this, enter `idf --version`in the terminal, which represents a successful installation if the output is similar to `ESP-IDF v4.2-dev-2084-g98d5b5dfd` For detailed installation and configuration instructions, refer to the [Quick Start documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/get-started/index.html).

When the configuration is complete, switch to the directory where the warehouse is located. Assuming that the path of the document is `~/esp/esp-dev-kits/esp-hmi-devkits/` enter the following command at the terminal to configure the environmental variables in the engineering directory:
```bash
cd ~/esp/esp-dev-kits/esp-hmi-devkits
set HMI_PATH=$(pwd)
```
Once the environment is configured, you can switch to the various routine folders in the examples directory and start compiling the program.
### Engineering options

You can enter the `idf.py menuconfig` sample directory.

In menuconfig, make sure that the following options are configured correctly:

- `(Top) > HMI Board Config > HMI board:` version selection, `ESP32-S2-HMI-DevKit-V2`
- `(Top) > HMI Board Config > Audio HAL:` output interface selection, using PWM or DAC;
- `(Top) > HMI Board Config > LCD Drivers:` display IC model selection, ESP32-S2-HMI-DevKit-1 IC display RM68120;
- `(Top) > Component config > ESP32S2-specific:` go to `Support for external, SPI-connected RAM` Options:
   - In `SPI RAM config > Set RAM clock speed:` set the clock of PSRAM to `80 MHz clock speed`;
- `(Top) -> Component config -> FreeRTOS Tick rate (Hz):` set to `1000`.

In each example, we have provided a default configuration file called `sdkconfig.defaults` and the above configuration of the options is complete.
