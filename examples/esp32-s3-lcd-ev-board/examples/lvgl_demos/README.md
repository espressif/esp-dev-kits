# LVGL Demos Example

An example used to run LVGL's demos. LVGL's internal demos include music player, widgets, stress and benchmark. What's more, the printer and tuner are generated by [Squareline](https://squareline.io/).

This example also shows three methods to avoid tearing effect. It uses two or more frame buffers based on LVGL **buffering modes**. For more information about this, please refer to LVGL [documents](https://docs.lvgl.io/master/porting/display.html?#buffering-modes).

## How to use example

Please first read the [User Guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html#esp32-s3-lcd-ev-board) of the ESP32-S3-LCD-EV-Board to learn about its software and hardware information.

The example can be directly flashed to the board by [ESP Launchpad](https://espressif.github.io/esp-launchpad/?flashConfigURL=https://dl.espressif.com/AE/esp-dev-kits/config.toml).

<a href="https://espressif.github.io/esp-launchpad/?flashConfigURL=https://dl.espressif.com/AE/esp-dev-kits/config.toml&app=lvgl_demos">
    <img alt="Try it with ESP Launchpad" src="https://espressif.github.io/esp-launchpad/assets/try_with_launchpad.png" width="200" height="56">
</a>

### ESP-IDF Required

* The ESP-IDF v5.0.1 or later is required to use this example. For using the branch of ESP-IDF, the latest branch `release/v5.1` is recommended. For using the tag of ESP-IDF, the tag `v5.1.2` or later is recommended.
* Please follow the [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/index.html) to set up the development environment.

### Hardware Required

* An ESP32-S3-LCD-EV-Board development board with subboard3 (800x480) or subboard2 (480x480)

### Configurations

Run `idf.py menuconfig` and go to `Board Support Package`.

### Build and Flash

1. The project configure PSRAM with 80M Octal by default. **Only for boards with ESP32-S3-WROOM-1-N16R8 can enable PSRAM 120M DDR(Octal) feature by the following commands**, see [here](../../README.md#psram-120m-ddr) for more details.
    ```
    rm -rf build sdkconfig sdkconfig.old
    idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.psram_120m_ddr" reconfigure
    ```
2. Run `idf.py -p PORT build flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

The following animations show the example running on different development boards.

![lvgl_demos_480_480](https://dl.espressif.com/AE/esp-dev-kits/s3-lcd-ev-board_examples_lvgl_demos_480_480_2.gif)

![lvgl_demos_800_480](https://dl.espressif.com/AE/esp-dev-kits/s3-lcd-ev-board_examples_lvgl_demos_800_480.gif)

## Performance Test

### Test Environment

|          Item          |                       Value                       |
| :--------------------: | :-----------------------------------------------: |
|      Module Type       |              ESP32-S3-WROOM-1-N16R8               |
| Configuration of PSRAM |                    Octal, 120M                    |
| Configuration of Flash |                     QIO, 120M                     |
|     Version of IDF     |                      v5.1.2                       |
|    Version of LVGL     |                      v8.3.11                      |
|   Test Demo of LVGL    |                   Music player                    |
|  Basic Configurations  | sdkconfig.defaults, sdkconfig.defaults.psram_120m_ddr |

### Description of Buffering Mode

| Buffering Mode |                    Description                    |          Special Configurations           |
| :------------: | :-----------------------------------------------: | :---------------------------------------: |
|     Mode1      | One buffer with 100-line heights in internal SRAM |                     *                     |
|     Mode2      |   One buffer with frame-size in internal PSRAM    |        sdkconfig.test.psram_buffer        |
|     Mode3      | Full-refresh with double frame-size PSRAM buffers | sdkconfig.test.full_refresh_double_buffer |
|     Mode4      | Direct-mode with double frame-size PSRAM buffers  |        sdkconfig.test.direct_mode         |
|     Mode5      | Full-refresh with triple frame-size PSRAM buffers | sdkconfig.test.full_refresh_triple_buffer |

**Notes:**

1. To test the above modes, run the following commands to configure project (take `Mode4` as an example):
```
rm -rf build sdkconfig sdkconfig.old
idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.psram_120m_ddr;sdkconfig.test.direct_mode" reconfigure
```
2. `Mode 3-5` enable the [RGB Bounce Buffer](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/lcd.html#bounce-buffer-with-single-psram-frame-buffer) feature.

### Average FPS with 480x480

| Buffering Mode | Average FPS |
| :------------: | :---------: |
|     Mode1      |     29      |
|     Mode2      |     25      |
|     Mode3      |     24      |
|     Mode4      |     26      |
|     Mode5      |     27      |

### Average FPS with 800x480

| Buffering Mode | Average FPS |
| :------------: | :---------: |
|     Mode1      |     26      |
|     Mode2      |     21      |
|     Mode3      |     18      |
|     Mode4      |     22     |
|     Mode5      |     22      |

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
    * Get stuck in the boot process: The latest version (v1.5) of the ESP32-S3-LCD-EV-Board, equipped with the `ESP32-S3-WROOM-1-N16R16V` module, does not currently support the PSRAM 120M DDR(Octal) feature.
    * Warning message with `W (xxx) lcd_panel.io.3wire_spi: Delete but keep CS line inactive`: This is a normal message, please ignore it.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-dev-kits/issues)

We will get back to you as soon as possible.