# Smart Panel Example

A factory demo designed for smart control panel (800x480) which has offline voice recognition function.

## How to use example

Please first read the [User Guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html#esp32-s3-lcd-ev-board) of the ESP32-S3-LCD-EV-Board to learn about its software and hardware information.

### Hardware Required

* An ESP32-S3-LCD-EV-Board development board with subboard3 (800x480)
* An USB Type-C cable for Power supply and programming
* A Speaker

### Configure

Run `idf.py menuconfig`, then modify the below optional configurations:

1. QWeather Configuration
   * `QWEATHER_REQUEST_KEY`: To enable the function of realtime weather, please fill it with your own key. (see https://console.qweather.com/#/apps)
2. wifi Configuration
   * `ESP_WIFI_SSID`: The name of default Wi-Fi AP.
   * `ESP_WIFI_PASSWORD`: The password of default Wi-Fi AP.

### Build and Flash

1. The project configure PSRAM with 80M DDR(Octal) by default. **Only for boards with ESP32-S3-WROOM-1-N16R8 can enable PSRAM 120M DDR(Octal) feature**, see [here](../../README.md#psram-120m-ddr) for more details.
2. Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Troubleshooting

* Program build failure
    * Error message with `error: static assertion failed: "FLASH and PSRAM Mode configuration are not supported"`: Please make sure ESP-IDF support `PSRAM Octal 120M` feature.
    * Error message with `CMake Error at xxx (message): ERROR: Some components (espressif/esp-sr) in the "managed_components"`: Please remove the ".component_hash" file by running `rm -rf managed_components/espressif__esp-sr/.component_hash`.
* Program runtime failure
    * Abnormal display on the sub-board2 screen (480x480), backlight is on but there is no image displayed: If the log level is configured as "Debug" or lower, please also increase the baud rate of log output as well (e.g., 2000000).
    * Warning message with `W (xxx) lcd_panel.io.3wire_spi: Delete but keep CS line inactive`: This is a normal message, please ignore it.
    * Get stuck in the boot process: Only for boards with `ESP32-S3-WROOM-1-N16R8` can enable PSRAM 120M DDR(Octal) feature. Please set the PSRAM configuration to 80M DDR(Octal) in the menuconfig when using boards with `ESP32-S3-WROOM-1-N16R16V`.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-dev-kits/issues)

We will get back to you as soon as possible.
