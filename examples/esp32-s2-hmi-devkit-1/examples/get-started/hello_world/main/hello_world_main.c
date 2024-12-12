/**
 * @file hello_world_main.c
 * @brief Hello World Example
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

#include <stdio.h>
#include "sdkconfig.h"

#include "esp_heap_caps.h"
#include "esp_spi_flash.h"
#include "esp_system.h"

#include "lvgl_port.h"

static void btn_click_cb(lv_obj_t *obj, lv_event_t event)
{
    static lv_obj_t *label = NULL;
    static size_t count = 0;

    if (LV_EVENT_CLICKED == event) {
        if (NULL == label) {
            label = lv_label_create(lv_scr_act(), NULL);
            lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
            lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_48);
            lv_label_set_text(label, "Hello world!");
            lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 20);
        } else {
            lv_label_set_text_fmt(label,
                "Hello world!\n"
                "Again! %zu", ++count);
            lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 20);
        }
    } 
}

static void ui_hello_world(void)
{
    lv_port_sem_take();

    lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_size(btn, 150, 50);
    lv_obj_set_event_cb(btn, btn_click_cb);
    lv_obj_align(btn, NULL, LV_ALIGN_CENTER, 0, -100);
    lv_obj_set_style_local_value_str(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Say Hello");
    lv_obj_set_style_local_value_font(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);

    lv_port_sem_give();
}

void app_main(void)
{
    /* Initialize I2C bus for touch IC */
    ESP_ERROR_CHECK(bsp_i2c_init(I2C_NUM_0, 400000));

    /* Initialize LCD */
    ESP_ERROR_CHECK(bsp_lcd_init());

    /* LCD touch IC init */
    ESP_ERROR_CHECK(ft5x06_init());

    /* Initialize LVGL */
    ESP_ERROR_CHECK(lvgl_init(LVGL_SCR_SIZE / 8, LV_BUF_ALLOC_INTERNAL));

    /* Start UI */
    ui_hello_world();
}
