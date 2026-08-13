# ESP32-P4X-Function-EV-Board Development Board

## User Guide

* ESP32-P4X-Function-EV-Board - [English](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32p4/esp32-p4x-function-ev-board/user_guide.html) / [中文](https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/esp32p4/esp32-p4x-function-ev-board/user_guide.html)

## Examples

The following examples are developed under the ESP-IDF **release/v5.5** branch. The [ESP-Brookesia Phone](./examples/esp_brookesia_phone/) is the factory firmware of the development board.

* [ESP-Brookesia Phone](./examples/esp_brookesia_phone/)
* [LVGL Demos v8](./examples/lvgl_demo_v8/)
* [LVGL Demos v9](./examples/lvgl_demo_v9/)

## Factory Bin

* [Factory Bin](https://dl.espressif.com/AE/esp-dev-kits/p4x_function_board_factory_demo_v16_120.bin) for ESP32-P4X-Function-EV-Board, programmed with the [Factory Demo](./examples/esp_brookesia_phone/) example.

<a href="https://espressif.github.io/esp-launchpad/?flashConfigURL=https://espressif2022.github.io/ESP32-P4-Function-EV-Board/launchpad.toml">
    <img alt="Try it with ESP Launchpad" src="https://dl.espressif.com/AE/esp-dev-kits/new_launchpad.png" width="316" height="100">
</a>

Experience more examples instantly with the ESP-LaunchPad.

**Note:**
* Firmware files with the `p4x_` prefix are for ESP32-P4X boards. Firmware files with the `p4_` prefix are for the original ESP32-P4 boards.
* The recommended IDF version for compiling these examples is **ESP-IDF release/v5.5**. These examples will **not** be updated to support IDF v6.0 or later. If you need to use them with IDF v6.0+, please refer to the components and examples provided in [esp-iot-solution](https://github.com/espressif/esp-iot-solution).
