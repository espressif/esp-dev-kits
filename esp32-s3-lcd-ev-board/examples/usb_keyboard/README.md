# USB Keyboard Example

An example for using LCD and LVGL to simulate USB keyboard.

## How to use example

Please first read the [User Guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html#esp32-s3-lcd-ev-board) of the ESP32-S3-LCD-EV-Board to learn about its software and hardware information.

### Hardware Required

* An ESP32-S3-LCD-EV-Board development board with subboard3 (800x480) or subboard2 (480x480)
* At least one USB Type-C cable for Power supply, programming and USB communication.

### Configurations

Run `idf.py menuconfig` and go to `Board Support Package`.

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

**Note:** Please use `UART port` to flash and monitor program.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

Run the example, you will see a GUI of USB keyboard. The board includes most common keys like "F1-F12", "0-9", "a-z" and so on. When press key "a", pc will receive key "a" just like from a real keyboard.

## Troubleshooting

* Program build failure
    * Error message with `error: static assertion failed: "FLASH and PSRAM Mode configuration are not supported"`: Please make sure ESP-IDF support `PSRAM Octal 120M` feature.
    * Error message with `error: 'esp_lcd_rgb_panel_config_t' has no member named 'num_fbs'`: Please update the branch (release/v5.0 or master) of ESP-IDF.
* Program upload failure
    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.
    * Error message with `A fatal error occurred: Could not open /dev/ttyACM0, the port doesn't exist`: Please first make sure the development board connected, then make board into "Download Boot" by following steps:
        1. keep press "BOOT(SW2)" button
        2. short press "RST(SW1)" button
        3. release "BOOT(SW2)".
        4. upload program and reset
* Program runtime failure
    * PC can't get key from the development board: Make sure the board connected with USB port.
    * Abnormal display on the sub-board2 screen (480x480), backlight is on but there is no image displayed: If the log level is configured as "Debug" or lower, please also increase the baud rate of log output as well (e.g., 2000000).
    * Warning message with `W (xxx) lcd_panel.io.3wire_spi: Delete but keep CS line inactive`: This is a normal message, please ignore it.
    * Get stuck in the boot process: Only for boards with `ESP32-S3-WROOM-1-N16R8` can enable PSRAM 120M DDR(Octal) feature. Please set the PSRAM configuration to 80M DDR(Octal) in the menuconfig when using boards with `ESP32-S3-WROOM-1-N16R16V`.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-dev-kits/issues)

We will get back to you as soon as possible.
