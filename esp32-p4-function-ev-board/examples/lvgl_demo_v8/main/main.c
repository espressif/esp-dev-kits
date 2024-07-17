#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_memory_utils.h"
#include "bsp/esp-bsp.h"
#include "bsp/bsp_board_extra.h"
#include "bsp/display.h"
#include "lvgl.h"
#include "lv_demos.h"

void app_main(void)
{
    bsp_display_start();
    bsp_display_backlight_on();

    bsp_display_lock(0);

    lv_demo_music();
    // lv_demo_benchmark();
    // lv_demo_widgets();

    bsp_display_unlock();
}
