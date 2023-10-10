# ChangeLog

## v1.0.3 - 2023-02-13

### Features

* Configurations:
    * Support to set number of frame buffers for RGB
    * Support to use bounce-buffer for RGB
    * Support to change the minimum delay time of LVGL task
    * Support to change the stack size of LVGL task
    * Support to enable anti-tearing function simply by using multiple buffers
* RGB Anti-tearing:
    * Optimize direct mode
    * Use triple-buffer in full-refresh mode

## v2.0.0 - 2023-08-07

### Features

* Configurations:
    * Add support for screen rotation using triple buffers when enabling RGB anti-tearing.
    * Add support for configuring SPIFFS.
* Implementations:
    * Use `esp_lcd_panel_io_additions` and `esp_lcd_gc9503` components to drive the LCD of sub_board2.
    * Use `esp_codec_dev` component to handle audio chips.
    * Use `button` component to handle button.
    * Implement automatic detection of the LCD sub-board type
* APIs:
    * Add new APIs for display in `bsp/display.h`
    * Add new APIs for touch in `bsp/touch.h`
    * Add new APIs for spiffs, audio in `bsp/esp32_s3_lcd_ev_board.h`
    * Add new APIs for ADC in `bsp/esp32_s3_lcd_ev_board.h`
