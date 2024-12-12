# BSP: ESP32-S3-LCD-EV-Board

[![Component Registry](https://components.espressif.com/components/espressif/esp32_s3_lcd_ev_board/badge.svg)](https://components.espressif.com/components/espressif/esp32_s3_lcd_ev_board)

* [User Guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html)

<div align=center><img src="https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/_images/ESP32-S3-LCD-EV-Board_480x480.png" width=600/></div>

<div align=center><img src="https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/_images/ESP32-S3-LCD-EV-Board_800x480.png" width=600/></div>

ESP32-S3-LCD-EV-Board is a development board for evaluating and verifying ESP32-S3 screen interactive applications. It has the functions of touch screen interaction and voice interaction. The development board has the following characteristics:

* Onboard ESP32-S3-WROOM-1 module, with built-in 16 MB Flash + 8/16 MB PSRAM
* Onboard audio codec + audio amplifier
* Onboard dual microphone pickup
* USB type-C interface download and debugging
* It can be used with different screen sub boards, and supports `RGB`, `8080`, `SPI`, `I2C` interface screens, as below:

|         Board Name         | Screen Size (inch) | Resolution | LCD Driver IC (Interface) | Touch Driver IC |                                                                            Schematic                                                                            | Support |
| -------------------------- | ------------------ | ---------- | ------------------------- | --------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------- |
| ESP32-S3-LCD-EV-Board-SUB1 | 0.9                | 128 x 64   | SSD1315 (I2C)             | *               | [link](https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/_static/esp32-s3-lcd-ev-board/schematics/SCH_ESP32-S3-LCD-Ev-Board-SUB1_V1.0_20220617.pdf) | Not yet |
|                            | 2.4                | 320 x 240  | ST7789V (SPI)             | XTP2046         |                                                                                                                                                                 | Not yet |
| ESP32-S3-LCD-EV-Board-SUB2 | 3.5                | 480 x 320  | ST7796S (8080)            | GT911           | [link](https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/_static/esp32-s3-lcd-ev-board/schematics/SCH_ESP32-S3-LCD-EV-Board-SUB2_V1.2_20230509.pdf) | Not yet |
|                            | 3.95               | 480 x 480  | GC9503CV (RGB)            | FT5x06          |                                                                                                                                                                 | Yes     |
| ESP32-S3-LCD-EV-Board-SUB3 | 4.3                | 800 x 480  | ST7262E43 (RGB)           | GT1151          | [link](https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/_static/esp32-s3-lcd-ev-board/schematics/SCH_ESP32-S3-LCD-EV-Board-SUB3_V1.1_20230315.pdf) | Yes     |

Here are some useful configurations in menuconfig that can be customed by user:

* `BSP_LCD_RGB_BUFFER_NUMS`: Configure the number of frame buffers. The anti-tearing function can be activated only when set to a value greater than one.
* `BSP_LCD_RGB_REFRESH_MODE`: Choose the refresh mode for the RGB LCD.
    * `BSP_LCD_RGB_REFRESH_AUTO`: Use the most common method to refresh the LCD.
    * `BSP_LCD_RGB_REFRESH_MANUALLY`: Refresh the LCD within a dedicated task. This can help manage PSRAM bandwidth.
    * `BSP_LCD_RGB_BOUNCE_BUFFER_MODE`: Enabling bounce buffer mode can lead to a higher PCLK frequency at the expense of increased CPU consumption. **This mode is particularly useful when dealing with [screen drift](https://docs.espressif.com/projects/esp-faq/en/latest/software-framework/peripherals/lcd.html#why-do-i-get-drift-overall-drift-of-the-display-when-esp32-s3-is-driving-an-rgb-lcd-screen), especially in scenarios involving Wi-Fi usage or writing to Flash memory.** This feature should be used in conjunction with `ESP32S3_DATA_CACHE_LINE_64B` configuration. For more detailed information, refer to the [documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/lcd.html#bounce-buffer-with-single-psram-frame-buffer).
* `BSP_DISPLAY_LVGL_BUF_CAPS`: Select the memory type for the LVGL buffer. Internal memory offers better performance.
* `BSP_DISPLAY_LVGL_BUF_HEIGHT`: Set the height of the LVGL buffer, with its width aligning with the LCD's width. The default value is 100, decreasing it can lower memory consumption.
* `BSP_DISPLAY_LVGL_AVOID_TEAR`: Avoid tearing effect by using multiple buffers. This requires setting `BSP_LCD_RGB_BUFFER_NUMS` to a value greater than 1.
    * `BSP_DISPLAY_LVGL_MODE`:
        * `BSP_DISPLAY_LVGL_FULL_REFRESH`: Use LVGL full-refresh mode. Set `BSP_LCD_RGB_BUFFER_NUMS` to `3` will get higher FPS when enable `BSP_DISPLAY_LVGL_ROTATION_NONE`.
        * `BSP_DISPLAY_LVGL_DIRECT_MODE`: Use LVGL's direct mode.
    * `BSP_DISPLAY_LVGL_ROTATION`: Rotate the screen clockwise. This requires setting `BSP_LCD_RGB_BUFFER_NUMS` to `3`. **The rotation is software-based and will substantially reduce FPS if enabled.**
        * `BSP_DISPLAY_LVGL_ROTATION_NONE`: No rotation.
        * `BSP_DISPLAY_LVGL_ROTATION_90`: 90-degree rotation.
        * `BSP_DISPLAY_LVGL_ROTATION_180`: 180-degree rotation.
        * `BSP_DISPLAY_LVGL_ROTATION_270`: 270-degree rotation.

Based on the above configurations, there are three different anti-tearing modes can be used:

* RGB double-buffer + LVGL full-refresh mode:
    * Set `BSP_LCD_RGB_BUFFER_NUMS` to `2`
    * Enable `BSP_DISPLAY_LVGL_AVOID_TEAR` and `BSP_DISPLAY_LVGL_FULL_REFRESH`
* RGB double-buffer + LVGL direct-mode:
    * Set `BSP_LCD_RGB_BUFFER_NUMS` to `2`
    * Enable `BSP_DISPLAY_LVGL_AVOID_TEAR` and `BSP_DISPLAY_LVGL_DIRECT_MODE`
* RGB triple-buffer + LVGL full-refresh mode:
    * Set `BSP_LCD_RGB_BUFFER_NUMS` to `3`
    * Enable `BSP_DISPLAY_LVGL_AVOID_TEAR` and `BSP_DISPLAY_LVGL_FULL_REFRESH`
