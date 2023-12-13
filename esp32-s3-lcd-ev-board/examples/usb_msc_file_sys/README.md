# USB File System

This example demonstrates how to use USB HOST on ESP32-S2/S3 to read and write files on a USB flash drive. You can use the development board `ESP32-S3-LCD-EV-Board` or `ESP32-S3-LCD-EV-Board-2` to connect to a USB flash drive and display its contents on the screen.

Currently supported file formats for preview are:
* JPG
* PNG
* BMP
* GIF
* MP3 (requires an external speaker)

## How to Use the Example

Please first read the [User Guide](https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html#esp32-s3-lcd-ev-board) of the ESP32-S3-LCD-EV-Board to learn about its software and hardware information.

* idf_version: >= release/v5.1

### Hardware Requirements

* An `ESP32-S3-LCD-EV-Board (480x480)` or `ESP32-S3-LCD-EV-Board-2 (800x480)` development board.
* A USB flash drive with a capacity less than 32GB.
* A USB Type-C cable for power supply and programming (please connect to the UART port instead of the USB port).

### Hardware Connection

| USB_DP | USB_DM |
| ------ | ------ |
| GPIO20 | GPIO19 |

**Note:** If the msc is connected to the USB port, diode D1 needs to be short-circuited. Please check the [Power Supply over USB](https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html#power-supply-over-usb) section in the board's schematic for more details.

### Build and Flash

1. The project configure PSRAM with 80M Octal by default. **Only for boards with ESP32-S3-WROOM-1-N16R8 can enable PSRAM 120M DDR(Octal) feature by the following commands**, see [here](../../README.md#psram-120m-ddr) for more details.
    ```
    rm -rf build sdkconfig sdkconfig.old
    idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.test.psram_120m_ddr" reconfigure
    ```
2. Run `idf.py -p PORT flash monitor` to build, flash, and monitor the project. **Note that it must be connected to the UART port instead of the USB port.**

(To exit the serial monitor, type `Ctrl-]`.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

The following animation is a demonstration of the development board running the example.

![usb_msc_file_sys](https://dl.espressif.com/AE/esp-dev-kits/s3-lcd-ev-board_examples_usb_msc_file_sys_5.gif)

## Troubleshooting

* Program build failure
    * Error message with `error: static assertion failed: "FLASH and PSRAM Mode configuration are not supported"`: Please check [documentation](https://docs.espressif.com/projects/esp-idf/en/release-v5.1/esp32s3/api-guides/flash_psram_config.html#all-supported-modes-and-speeds) to make sure the flash and PSRAM mode configuration is correct.
* Program upload failure
    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.
    * Error message with `A fatal error occurred: Could not open /dev/ttyACM0, the port doesn't exist`: Please first make sure the development board is connected, then make the board into "Download Boot" by following steps:
        1. Keep pressing the "BOOT(SW2)" button.
        2. Short press the "RST(SW1)" button.
        3. Release the "BOOT(SW2)" button.
        4. Upload the program and reset.
* Program runtime failure
    * Abnormal display on the sub-board2 screen (480x480), backlight is on but there is no image displayed: If the log level is configured as "Debug" or lower, please also increase the baud rate of log output as well (e.g., 2000000).
    * Warning message with `W (xxx) lcd_panel.io.3wire_spi: Delete but keep CS line inactive`: This is a normal message, please ignore it.
    * Get stuck in the boot process: Only for boards with `ESP32-S3-WROOM-1-N16R8` can enable PSRAM 120M DDR(Octal) feature. Please set the PSRAM configuration to 80M DDR(Octal) in the menuconfig when using boards with `ESP32-S3-WROOM-1-N16R16V`.

## Technical Support and Feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum.
* For feature requests or bug reports, create a [GitHub issue](https://github.com/espressif/esp-dev-kits/issues).

We will get back to you as soon as possible.