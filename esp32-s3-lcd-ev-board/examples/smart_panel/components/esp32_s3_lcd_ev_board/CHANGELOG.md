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

## v3.0.0 - 2023-12-02

### Bugfix

* Use `on_bounce_frame_finish` instead of `on_vsync` when enabling RGB bounce buffer mode.
* Fix some issues when enabling anti-tearing and rotation

### Features

* Configurations:
    * Support to set the pinned core for LVGL task
* Implementations:
    * Support ESP32-S3-WROOM-1-N16R16V module
    * Add warning for compiling and running when using `ESP-IDF` version `<5.1.2`

### Dependencies

* Update the version of `ESP-IDF` to `>5.0.1`
* Use `esp_lcd_gc9503` version `^1` when using `ESP-IDF` version `<5.1.2`
* Use `esp_lcd_gc9503` version `^3` when using `ESP-IDF` version `>=5.1.2`
