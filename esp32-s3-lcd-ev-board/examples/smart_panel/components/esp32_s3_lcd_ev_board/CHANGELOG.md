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
