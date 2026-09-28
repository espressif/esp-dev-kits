# ESP-SensairShuttle Development Board

The [Factory Demo](./examples/factory_demo/) is the maintained out-of-box firmware
for this board. Updates here cover bug fixes, hardware support, and ESP-IDF
compatibility.

## User Guide

* ESP-SensairShuttle - [English](https://espressif.craft.me/JUsLZQOVMBoNdU) / [中文](https://espressif.craft.me/3f3bg8V3wS7wnF)

## Factory Firmware

Build-verified with ESP-IDF **v5.5.4** and **v6.0.1** for `esp32c5`. Follow the
Factory Demo [English](./examples/factory_demo/README.md) or
[中文](./examples/factory_demo/README_CN.md) README to build and flash, or to
restore the factory image after trying other firmware.

[ESP-Launchpad](https://esp-sensairshuttle-launchpad.pages.dev/) can flash a
matching prebuilt image when one is listed.

## Other Applications

* [gesture_recognition](./examples/gesture_recognition/): on-device TFLite Micro
  demo using the onboard BMI270. It recognizes counterclockwise circle and V
  gestures, and covers data collection, training, and deploy.
* [magedc_ix](./examples/magedc_ix/): magnetic-bead EDC demo using the onboard
  BMM350. LED strip, speaker, and a local web UI for mini-games.
* [xiaozhi](https://github.com/78/xiaozhi-esp32/tree/main/main/boards/espressif/esp-sensairshuttle)

## Support

Report Factory Demo and board-support issues on
[esp-dev-kits](https://github.com/espressif/esp-dev-kits/issues).
