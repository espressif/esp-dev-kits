
/**
 * @file main.c
 * @brief Blink WS2812 RGB LED.
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
#include <string.h>
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_heap_caps.h"
#include "esp_log.h"

#include "ws2812.h"
#include "bsp_i2c.h"
#include "lvgl_port.h"
#include "ft5x06.h"
#include "bsp_ext_io.h"

#define LED_INDEX   (0)

static const lv_color_t led_color_list[] = {
    LV_COLOR_RED,       LV_COLOR_BLACK,
    LV_COLOR_GREEN,     LV_COLOR_BLACK,
    LV_COLOR_BLUE,      LV_COLOR_BLACK,
    LV_COLOR_YELLOW,    LV_COLOR_BLACK,
    LV_COLOR_MAGENTA,   LV_COLOR_BLACK,
    LV_COLOR_CYAN,      LV_COLOR_BLACK,
    LV_COLOR_WHITE,     LV_COLOR_BLACK,
};

static ext_io_t io_config = BSP_EXT_IO_DEFAULT_CONFIG();
static ext_io_t io_level = BSP_EXT_IO_DEFAULT_LEVEL();

void app_main(void)
{
    ESP_ERROR_CHECK(bsp_i2c_init(I2C_NUM_0, 400000));

    /* Initialize LCD */
    ESP_ERROR_CHECK(bsp_lcd_init());

    /* LCD touch IC init */
    ESP_ERROR_CHECK(ft5x06_init());

    /* Initialize LVGL */
    ESP_ERROR_CHECK(lvgl_init(LVGL_SCR_SIZE / 8, LV_BUF_ALLOC_INTERNAL));

    ESP_ERROR_CHECK(ws2812_init());
    
    /* UI */
    lv_port_sem_take();
    lv_obj_t *obj_led = lv_led_create(lv_scr_act(), NULL);
    lv_obj_set_size(obj_led, 100, 100);
    lv_obj_align(obj_led, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_led_on(obj_led);
    lv_port_sem_give();

    while (1) {
        for (int i = 0; i < sizeof led_color_list / sizeof led_color_list[0]; i++) {
            /* The color format of WS2812 if RGB888, However, LVGL is configured as RGB565. */
            ws2812_set_rgb(LED_INDEX,
                led_color_list[i].ch.red << 3,
                led_color_list[i].ch.green << 2,
                led_color_list[i].ch.blue << 3);
            ws2812_refresh();

            /* Switch LED widget state */
            if (LV_COLOR_BLACK.full == led_color_list[i].full) {
                lv_led_off(obj_led);
            } else {
                lv_led_on(obj_led);
            }

            /* Change color of LED widget */
            lv_obj_set_style_local_bg_color(obj_led, LV_LED_PART_MAIN, LV_STATE_DEFAULT, led_color_list[i]);
            lv_obj_set_style_local_shadow_color(obj_led, LV_LED_PART_MAIN, LV_STATE_DEFAULT, led_color_list[i]);

            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}
