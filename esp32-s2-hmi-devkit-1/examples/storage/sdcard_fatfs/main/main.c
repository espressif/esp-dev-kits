/**
 * @file main.c
 * @brief SD card file viewer example
 * @version 0.1
 * @date 2021-03-04
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

#include "bsp_ext_io.h"
#include "bsp_i2c.h"
#include "bsp_lcd.h"
#include "bsp_sdcard.h"
#include "fs_hal.h"
#include "ft5x06.h"
#include "tca9554.h"

#include "lvgl_port.h"

static char *global_path = NULL;
static char *global_sub_path = NULL;

static lv_obj_t *file_list = NULL;

static ext_io_t io_config = BSP_EXT_IO_DEFAULT_CONFIG();
static ext_io_t io_level = BSP_EXT_IO_DEFAULT_LEVEL();

LV_IMG_DECLARE(icon_file)
LV_IMG_DECLARE(icon_music)
LV_IMG_DECLARE(icon_pic)

void ui_update_list(char *path);

static void btn_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        fs_get_dir_file_name(global_path, lv_list_get_btn_index(file_list, obj), global_sub_path);
        strcat(global_path, "/");
        strcat(global_path, global_sub_path);
        ui_update_list(global_path);
    }
}

void btn_back_event_cb(lv_obj_t *obj, lv_event_t event)
{
    static size_t str_len = 0;
    static size_t slash_count = 0;
    static size_t slash_index = 0;

    if (LV_EVENT_CLICKED == event) {
        slash_count = 0;
        str_len = strlen(global_path);

        for (size_t i = 0; i < str_len; i++) {
            if ('/' == global_path[i]) {
                slash_count++;
                slash_index = i;
            }
        }

        if (slash_count > 1) {
            global_path[slash_index] = '\0';
        }

        ui_update_list(global_path);
    }
}

static void ui_main(void)
{
    lv_port_sem_take();

    global_path = heap_caps_malloc(256, MALLOC_CAP_SPIRAM);
    global_sub_path = heap_caps_malloc(256, MALLOC_CAP_SPIRAM);

    strcpy(global_path, MOUNT_POINT);

    char *file_name = heap_caps_malloc(255 + 1, MALLOC_CAP_SPIRAM);
    size_t file_count = 0;

    lv_obj_t *btn_back = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_value_str(btn_back, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Back");
    lv_obj_align(btn_back, NULL, LV_ALIGN_IN_TOP_LEFT, 25, 5);
    lv_obj_set_event_cb(btn_back, btn_back_event_cb);

    fs_get_dir_file_count(global_path, &file_count);
    file_list = lv_list_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_border_width(file_list, LV_LIST_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_radius(file_list, LV_LIST_PART_BG, LV_STATE_DEFAULT, 15);
    lv_obj_set_click(file_list, true);
    lv_obj_set_size(file_list, 750, 400);

    lv_obj_set_style_local_text_font(file_list, LV_LIST_PART_BG, LV_STATE_DEFAULT, &font_en_24);
    for (size_t i = 0; i < file_count; i++) {
        fs_get_dir_file_name(global_path, i, file_name);

        if ((0 == strcmp(file_name, "Pic")) || (0 == strcmp(file_name, "UI"))) {
            lv_obj_set_event_cb(lv_list_add_btn(file_list, &icon_pic, file_name), btn_event_cb);
        } else if (0 == strcmp(file_name, "Music")) {
            lv_obj_set_event_cb(lv_list_add_btn(file_list, &icon_music, file_name), btn_event_cb);
        } else {
            lv_obj_set_event_cb(lv_list_add_btn(file_list, &icon_file, file_name), btn_event_cb);
        }
    }
    lv_obj_align(file_list, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -25);
    free(file_name);

    lv_port_sem_give();
}

void ui_update_list(char *path)
{
    char *file_name = heap_caps_malloc(255 + 1, MALLOC_CAP_SPIRAM);

    while (lv_list_get_size(file_list)) {
        lv_list_remove(file_list, 0);
    }

    size_t file_count = 0;
    fs_get_dir_file_count(path, &file_count);

    for (size_t i = 0; i < file_count; i++) {
        fs_get_dir_file_name(path, i, file_name);
        lv_obj_set_event_cb(lv_list_add_btn(file_list, &icon_file, file_name), btn_event_cb);
    }

    free(file_name);
}

void app_main(void)
{
    /* Init I2C bus for sensor communication */
    ESP_ERROR_CHECK(bsp_i2c_init(I2C_NUM_0, 400000));

    /* Turn on sensor power supply */
    ESP_ERROR_CHECK(tca9554_init());
    ESP_ERROR_CHECK(tca9554_set_configuration(io_config.val));
    ESP_ERROR_CHECK(tca9554_write_output_pins(io_level.val));

    /* Init LCD and touch panel */
    ESP_ERROR_CHECK(bsp_lcd_init());
    ESP_ERROR_CHECK(ft5x06_init());
    
    /* Initialize LVGL */
    ESP_ERROR_CHECK(lvgl_init(800 * 480 / 8, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));

    /* Init SD card and file system hal API */
    ESP_ERROR_CHECK(bsp_sdcard_init());
    ESP_ERROR_CHECK(fs_hal_init());

    /* Init main UI */
    ui_main();
}
