# ESP32-S3-LCD-EV-Board Development Board

## User Guide

* ESP32-S3-LCD-EV-Board - [English](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html) / [中文](https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html)

## Examples

The following examples are developed under the ESP-IDF **release/v5.1** branch and can be directly flashed to the board by [ESP Launchpad](https://espressif.github.io/esp-launchpad/?flashConfigURL=https://dl.espressif.com/AE/esp-dev-kits/config.toml).

* [86-Box Demo](./examples/86box_demo/)
* [86-Box Smart Panel](./examples/86box_smart_panel/)
* [LVGL Demos](./examples/lvgl_demos/)
* [Smart Panel](./examples/smart_panel/)
* [USB Keyboard](./examples/usb_keyboard/)
* [USB Camera](./examples/usb_camera_lcd/)
* [USB File System](./examples/usb_msc_file_sys/)

## Factory Bin

* [Factory Bin](https://dl.espressif.com/AE/esp-dev-kits/86box_demo-esp32-s3-lcd-ev-board-esp32s3-v5.1.bin) for ESP32-S3-LCD-EV-Board (Subboard2 480x480), programmed with the [86-Box Smart Panel](./examples/86box_smart_panel/) example.
* [Factory Bin](https://dl.espressif.com/AE/esp-dev-kits/smart_panel-esp32-s3-lcd-ev-board-esp32s3-v5.1.bin) for ESP32-S3-LCD-EV-Board-2 (Subboard3 800x480), programmed with the [Smart Panel](./examples/smart_panel/) example.

## PSRAM 120M DDR

The PSRAM 120M DDR feature is intended to achieve the best performance of RGB LCD. To enable this feature, please refer to [ESP-FAQ - LCD](https://docs.espressif.com/projects/esp-faq/en/latest/software-framework/peripherals/lcd.html#how-can-i-enable-psram-120m-octal-ddr-on-esp32-s3r8) for more details.
**Important Note**: The latest version (v1.5) of the ESP32-S3-LCD-EV-Board, equipped with the `ESP32-S3-WROOM-1-N16R16V` module, does not currently support this feature. If enabled, there may be an issue of the chip freezing upon power-up and then resetting.
