/**
 * @file ui_ulp.c
 * @brief UI src.
 * @version 0.1
 * @date 2021-03-07
 * 
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 * 
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "ui_ulp.h"

/* Log defination */
#define TAG ((const char *)"ui_ulp")
#define UI_LOG_TRACE(...)	ESP_LOGD(TAG, ##__VA_ARGS__)

/* Color value defination */
#define COLOR_BG    lv_color_make(238, 241, 245)
#define COLOR_DEEP  lv_color_make(242, 173, 71)
#define COLOR_LIGHT lv_color_make(253, 200, 0)
#define COLOR_THEME lv_color_make(252, 199, 0)

lv_obj_t *slider_r = NULL;
lv_obj_t *slider_g = NULL;
lv_obj_t *slider_b = NULL;
lv_obj_t *btn_sleep = NULL;

static void btn_deep_sleep_event_cb(lv_obj_t *obj, lv_event_t event)
{
    static ext_io_t io_level_sleep = BSP_EXT_IO_SLEEP_LEVEL();

    if (LV_EVENT_CLICKED == event) {

    const int ext_wakeup_pin = 0;
    const uint64_t ext_wakeup_pin_mask = 1ULL << ext_wakeup_pin;
    UI_LOG_TRACE("Enabling EXT1 wakeup on pins GPIO, GPIO%d\n", ext_wakeup_pin);
    esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_mask, ESP_EXT1_WAKEUP_ALL_LOW);
    
    tca9554_write_output_pins(io_level_sleep.val);

        vTaskDelay(10);
        esp_deep_sleep_start();
    }
}

static void slider_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_VALUE_CHANGED == event) {
        ws2812_set_rgb(0,
            lv_slider_get_value(slider_r),
            lv_slider_get_value(slider_g),
            lv_slider_get_value(slider_b));
        ws2812_refresh();
    }
}

void ui_ulp_init(void)
{
    lv_port_sem_take();

    slider_r = lv_slider_create(lv_scr_act(), NULL);
    lv_slider_set_range(slider_r, 0, 255);

    slider_g = lv_slider_create(lv_scr_act(), slider_r);
    slider_b = lv_slider_create(lv_scr_act(), slider_r);

    lv_obj_align(slider_r, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(slider_g, NULL, LV_ALIGN_CENTER, 0, 60);
    lv_obj_align(slider_b, NULL, LV_ALIGN_CENTER, 0, 120);
    lv_obj_set_event_cb(slider_r, slider_event_cb);
    lv_obj_set_event_cb(slider_g, slider_event_cb);
    lv_obj_set_event_cb(slider_b, slider_event_cb);

    btn_sleep = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_value_str(btn_sleep, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Sleep");
    lv_obj_align(btn_sleep, NULL, LV_ALIGN_CENTER, 0, -100);
    lv_obj_set_event_cb(btn_sleep, btn_deep_sleep_event_cb);
    
    gpio_num_t gpio_num = GPIO_NUM_0;
    rtc_gpio_init(gpio_num);
    rtc_gpio_set_direction(gpio_num, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_dis(gpio_num);
    rtc_gpio_pullup_en(gpio_num);
    rtc_gpio_hold_en(gpio_num);
    
    lv_port_sem_give();
}
