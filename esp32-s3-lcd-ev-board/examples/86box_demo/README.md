# 86-box Demo Example

A GUI demo designed for control panel (480x480) which is usually used fo wall-mounted 86 type box.

**Note: This example is only applicable to 480 x 480 LCD.**

## How to use example

Please first read the [User Guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html#esp32-s3-lcd-ev-board) of the ESP32-S3-LCD-EV-Board to learn about its software and hardware information.

### Hardware Required

* An ESP32-S3-LCD-EV-Board development board with subboard2 (480x480)
* An USB Type-C cable for Power supply and programming
* A Speaker

### Build and Flash

1. The project configure PSRAM with 80M DDR(Octal) by default. **Only for boards with ESP32-S3-WROOM-1-N16R8 can enable PSRAM 120M DDR(Octal) feature**, see [here](../../README.md#psram-120m-ddr) for more details.
2. Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

### Functions

Run the example, you can achieve a smooth graphic operation experience, for example horizontally swiping between weather report, hot water heater and warm air heater page. Trigger the control centre by swiping down from the bottom of the screen. Open music player to test the audio output function of the board.

## Troubleshooting

* Program build failure
    * Error message with `error: static assertion failed: "FLASH and PSRAM Mode configuration are not supported"`: Please check [documentation](https://docs.espressif.com/projects/esp-idf/en/release-v5.1/esp32s3/api-guides/flash_psram_config.html#all-supported-modes-and-speeds) to make sure the flash and PSRAM mode configuration is correct.
* Program upload failure
    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.
    * Error message with `A fatal error occurred: Could not open /dev/ttyACM0, the port doesn't exist`: Please first make sure the development board connected, then make board into "Download Boot" by following steps:
        1. keep press "BOOT(SW2)" button
        2. short press "RST(SW1)" button
        3. release "BOOT(SW2)".
        4. upload program and reset
* Program runtime failure
    * Abnormal display on the sub-board2 screen (480x480), backlight is on but there is no image displayed: If the log level is configured as "Debug" or lower, please also increase the baud rate of log output as well (e.g., 2000000).
    * Warning message with `W (xxx) lcd_panel.io.3wire_spi: Delete but keep CS line inactive`: This is a normal message, please ignore it.
    * Get stuck in the boot process: Only for boards with `ESP32-S3-WROOM-1-N16R8` can enable PSRAM 120M DDR(Octal) feature. Please set the PSRAM configuration to 80M DDR(Octal) in the menuconfig when using boards with `ESP32-S3-WROOM-1-N16R16V`.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-dev-kits/issues)

We will get back to you as soon as possible.
