# ESP32-C3-LCD-EV-BOARD Development Board

The demo is based on the RISC-V core ESP32-C3 SoC to drive a SPI interface round LCD. The software GUI graphical interface is developed based on IDF v5.0 and LVGL v8.3. The LCD is 1.28 inch, resolution 240 x240, driver IC is GC9A01.
ESP32-C3 supports I2C and SPI interface with small package, safety, stability, low power consumption and cost advantage, and is applicable for small screen display applications, such as washing machine, body scale, electric brushes, etc.

## How to use example

For more information on structure and contents of ESP-IDF projects, please refer to Section [Build System](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html) of the ESP-IDF Programming Guide.

## Troubleshooting

* Program upload failure

    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-idf/issues)

We will get back to you as soon as possible.
