/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

#include "lvgl.h"
#include "lv_example_func.h"
#include "lv_example_pub.h"

static const char *TAG = "LVGL_PUB";

bool is_time_out(time_out_count *tm)
{
    int32_t isTmOut;
    isTmOut = (lv_tick_get() - (tm->time_base + tm->timeOut));
    if ((isTmOut > 0) || (0 == tm->time_base)) {
        tm->time_base = lv_tick_get();
        return true;
    } else {
        return false;
    }
}

bool set_time_out(time_out_count *tm, uint32_t ms)
{
    tm->time_base = lv_tick_get();
    tm->timeOut = ms;
    return true;
}

bool reload_time_out(time_out_count *tm)
{
    tm->time_base = lv_tick_get();
    return true;
}

void lv_home_create(void)
{
    //freetype_font_load_init();
    lv_func_goto_layer(&boot_Layer);
}
