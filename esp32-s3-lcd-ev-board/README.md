# ESP32-S3-LCD-EV-Board Development Board

## User Guide

* ESP32-S3-LCD-EV-Board - [English](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html) / [中文](https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html)

## Examples

The following examples are developed under the ESP-IDF **release/v5.1** branch:

* [86-Box Demo](./examples/86box_demo/)
* [86-Box Smart Panel](./examples/86box_smart_panel/)
* [LVGL Demos](./examples/lvgl_demos/)
* [Smart Panel](./examples/smart_panel/)
* [USB Keyboard](./examples/usb_keyboard/)
* [USB Camera](./examples/usb_camera_lcd/)
* [USB File System](./examples/usb_msc_file_sys/)

## Released Bin

* [Factory Bin](./factory/bin/ESP32-S3-LCD-EV-Board_fac_v0_3_0.bin) for ESP32-S3-LCD-EV-Board (Subboard2 480x480)
* [Factory Bin](./factory/bin/ESP32-S3-LCD-EV-Board-2_fac_v0_5_0.bin) for ESP32-S3-LCD-EV-Board-2 (Subboard3 800x480)

## PSRAM 120M DDR

The PSRAM 120M DDR feature is intended to achieve the best performance of RGB LCD. It is only available with ESP-IDF **release/v5.1** and above version. It can be used by enabling the `IDF_EXPERIMENTAL_FEATURES`, `SPIRAM_SPEED_120M`, `SPIRAM_MODE_OCT` options. please see [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/flash_psram_config.html#all-supported-modes-and-speeds) for more details.

**Important Notes:**

  1. The `ESP32-S3-WROOM-1-N16R16V` module does not support this feature for now. If enabled, there may be an issue of the chip freezing upon power-up and then resetting.
  2. The PSRAM 120 MHz DDR is an experimental feature and it has temperature risks as below:
    * Cannot guarantee normal functioning with a temperature higher than 65 degrees Celsius.
    * Temperature changes can also cause the crash of accessing to PSRAM/Flash, see [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/flash_psram_config.html#all-supported-modes-and-speeds) for more details.
