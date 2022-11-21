# RGB LCD + USB Camera Demo

An example used to show video which captured be USB camera.

## How to use example

### Hardware Required

* An ESP32-S3-LCD-EV-Board development board with 800x480 LCD
* A CDC USB camera
* An USB Type-C cable for Power supply and programming

### Software Required

* IDF with Octal PSRAM 120M patch (See [here](../factory/README.md#idf-patch))

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project. **Note that it must be powered with UART port instead of USB port.**

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Troubleshooting

* LCD screen drift
  * Slow down the PCLK frequency
  * Adjust other timing parameters like PCLK clock edge (by `pclk_active_neg`), sync porches like VBP (by `vsync_back_porch`) according to your LCD spec
  * Enable `CONFIG_SPIRAM_FETCH_INSTRUCTIONS` and `CONFIG_SPIRAM_RODATA`, which can saves some bandwidth of SPI0 from being consumed by ICache.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-dev-kits/issues)

We will get back to you as soon as possible.
