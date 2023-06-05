# BSP: ESP32-S3-LCD-EV-BOARD

[![Component Registry](https://components.espressif.com/components/espressif/esp32_s3_lcd_ev_board/badge.svg)](https://components.espressif.com/components/espressif/esp32_s3_lcd_ev_board)

* [User Guide](https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html)

![](https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/_images/esp32-s3-lcd-ev-board-isometric-raw.png)

ESP32-S3-LCD-EV-BOARD is a development board for evaluating and verifying ESP32-S3 screen interactive applications. It has the functions of touch screen interaction and voice interaction. The development board has the following characteristics:

* Onboard ESP32-S3-WROOM-1 module, with built-in 16 MB Flash + 8 MB PSRAM
* Onboard audio codec + audio amplifier
* Onboard dual microphone pickup
* USB type-C interface download and debugging
* It can be used with different screen sub boards, and supports `RGB`, `8080`, `SPI`, `I2C` interface screens, as below:

| Board Name                 | Screen Size (inch) | Resolution | LCD Driver IC (Interface) | Touch Driver IC |                                                                          Schematic                                                                           | Support |
| -------------------------- | ------------------ | ---------- | ------------------------- | --------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ | ------- |
| ESP32-S3-LCD_Ev_Board_SUB1 | 0.9                | 128 x 64   | SSD1315 (I2C)             | *               | [link](https://github.com/espressif/esp-dev-kits/blob/master/docs/_static/esp32-s3-lcd-ev-board/schematics/SCH_ESP32-S3-LCD_Ev_Board_SUB1_V1.0_20220617.pdf) | Not yet |
|                            | 2.4                | 320 x 240  | ST7789V (SPI)             | XTP2046         |                                                                                                                                                              | Not yet |
| ESP32-S3-LCD_Ev_Board_SUB2 | 3.5                | 480 x 320  | ST7796S (8080)            | GT911           | [link](https://github.com/espressif/esp-dev-kits/blob/master/docs/_static/esp32-s3-lcd-ev-board/schematics/SCH_ESP32-S3-LCD_Ev_Board_SUB2_V1.0_20220615.pdf) | Not yet |
|                            | 3.95               | 480 x 480  | GC9503CV (RGB)            | FT5x06          |                                                                                                                                                              | Yes     |
| ESP32-S3-LCD_Ev_Board_SUB3 | 4.3                | 800 x 480  | Unkonw (RGB)              | GT1151          | [link](https://github.com/espressif/esp-dev-kits/blob/master/docs/_static/esp32-s3-lcd-ev-board/schematics/SCH_ESP32-S3-LCD_Ev_Board_SUB2_V1.0_20220615.pdf) | Yes     |

Here are some useful configurations in menuconfig that can be customed by user:

* `BSP_LCD_SUB_BOARD`: Choose a LCD sub-board according to hardware.
* `BSP_LCD_RGB_BUFFER_NUMS`: Set number of frame buffers. Only when it is set to multiple can the anti-tearing function be turned on.
* `BSP_LCD_RGB_REFRESH_MODE`: Select the refresh mode for RGB LCD.
    * `BSP_LCD_RGB_REFRESH_AUTO`: Refresh the LCD in the most common way.
    * `BSP_LCD_RGB_REFRESH_MANUALLY`:Refresh the LCD in a specific task. It can reduce the bandwidth of PSRAM usage.
    * `BSP_LCD_RGB_BOUNCE_BUFFER_MODE`: Enable bounce buffer mode can achieve higher PCLK frequency at the cost of higher CPU consumption. **And it's helpful when the screen drifts (Especially using Wi-Fi or OTA)**. It should be used with `ESP32S3_DATA_CACHE_LINE_64B`.
* `BSP_DISPLAY_LVGL_BUF_CAPS`: Choose the memory type of LVGL buffer. Internal memory is more fast.
* `BSP_DISPLAY_LVGL_BUF_HEIGHT`: Set the height of LVGL buffer, and its width is equal to LCD's width.
* `BSP_DISPLAY_LVGL_AVOID_TEAR`: Avoid tearing effect by using multiple buffers. Need to set `BSP_LCD_RGB_BUFFER_NUMS` more than 1.
    * `BSP_DISPLAY_LVGL_FULL_REFRESH`: Use LVGL full-refresh mode. Set `BSP_LCD_RGB_BUFFER_NUMS` to 3 will get higher FPS.
    * `BSP_DISPLAY_LVGL_DIRECT_MODE`: Use LVGL direct mode.

