# ESP32-S3-LCD-EV-BOARD Development Board

ESP32-S3-LCD-EV-BOARD is a development board for evaluating and verifying esp32s3 screen interactive applications. It has the functions of touch screen interaction and voice interaction. The development board has the following characteristics:

* Onboard ESP32-S3-WROOM-1 module, with built-in 16 MB Flash + 8 MB PSRAM
* Onboard audio codec + audio amplifier
* Onboard dual microphone pickup
* It can be used with different screen sub boards, and supports `RGB`, `8080`, `SPI`, `I2C` interface screens
* USB type-C interface download and debugging

<div align="center">
    <img src="../docs/_static/esp32-s3-lcd-ev-board/esp32-s3-lcd-ev-board.png" width="400px"/>
</div>
<div align="center"> ESP32-S3-LCD-EV-BOARD (click to enlarge) </div>

**The document consists of the following major sections:**

* [Getting Started](https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32s3/esp32-s3-usb-otg/user_guide.html#getting-started): Provides a brief overview of ESP32-S3-LCD-EV-BOARD and necessary hardware and software information.

## Tips

* ESP32-S3 RGB driver only supports 16-bit RGB565 and 8-bit RGB888 interface
* Please use octal PSRAM at least to guarantee the RGB interface usability, otherwise the frame rate will be unbearably slow
* The function of avoiding tearing effect is only aviliable for RGB driver. It can be demonstrated on examples [LVGL Demos](./examples/lvgl_demos/) and [86-Box GUI](./examples/86box_demo/).

## Examples

* [86-Box GUI](./examples/86box_demo/)
* [LVGL Demos](./examples/lvgl_demos/)
* [Smart Panel](./examples/smart-panel/)
* [USB keyboard](./examples/usb_keyboard/)

## Released Bin

* [Factory bin](./examples/factory/factory_bin)

## Related Documents

* [ESP32-S3 Technical Specification](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_cn.pdf) (PDF)
* [ESP32­S3­WROOM­1 & ESP32­S3­WROOM­1U Technical Specifications](https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_cn.pdf) (PDF)
* [ESP32-S3-LCD-EV-BOARD-MB 原理图](../_static/schematics/esp32-s3-lcd-ev-board/SCH_ESP32-S3-LCD_EV_Board_MB_V1.1_20220713.pdf) (PDF)
* [ESP32-S3-LCD-EV-BOARD-MB PCB布局图](../_static/schematics/esp32-s3-lcd-ev-board/PCB_ESP32-S3-LCD_Ev_Board_MB_V1.0_20220610.pdf) (PDF)
* [ESP32-S3-LCD-EV-BOARD-SUB1 原理图](../_static/schematics/esp32-s3-lcd-ev-board/SCH_ESP32-S3-LCD_Ev_Board_SUB1_V1.0_20220617.pdf) (PDF)
* [ESP32-S3-LCD-EV-BOARD-SUB1 PCB布局图](../_static/schematics/esp32-s3-lcd-ev-board/PCB_ESP32-S3-LCD_Ev_Board_SUB1_V1.0_20220617.pdf) (PDF)
* [ESP32-S3-LCD-EV-BOARD-SUB2 原理图](../_static/schematics/esp32-s3-lcd-ev-board/SCH_ESP32-S3-LCD_Ev_Board_SUB2_V1.0_20220615.pdf) (PDF)
* [ESP32-S3-LCD-EV-BOARD-SUB2 PCB布局图](../_static/schematics/esp32-s3-lcd-ev-board/PCB_ESP32-S3-LCD_Ev_Board_SUB2_V1.1_20220708.pdf) (PDF)
* [ESP32-S3-LCD-EV-BOARD-SUB3 原理图](../_static/schematics/esp32-s3-lcd-ev-board/SCH_ESP32-S3-LCD_Ev_Board_SUB3_V1.0_20220617.pdf) (PDF)
* [ESP32-S3-LCD-EV-BOARD-SUB3 PCB布局图](../_static/schematics/esp32-s3-lcd-ev-board/PCB_ESP32-S3-LCD_Ev_Board_SUB3_V1.0_20220617.pdf) (PDF)
* [GC9503CV 规格书](../_static/esp32-s3-lcd-ev-board/datasheets/3.95_480x480_SmartDisplay/GC9503NP_DataSheet_V1.7.pdf) (PDF)
