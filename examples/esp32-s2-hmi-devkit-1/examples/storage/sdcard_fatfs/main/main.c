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

#include "bsp_i2c.h"
#include "bsp_lcd.h"
#include "bsp_sdcard.h"
#include "fs_hal.h"
#include "ft5x06.h"

#include "lvgl_port.h"

static char *global_path = NULL;
static char *global_sub_path = NULL;

static lv_obj_t *file_list = NULL;
static lv_obj_t *label_path = NULL;
static lv_obj_t *btn_back = NULL;

LV_IMG_DECLARE(icon_file)
LV_IMG_DECLARE(icon_music)
LV_IMG_DECLARE(icon_pic)

static void ui_update_list(char *path);
static void ui_update_path(void);

static void btn_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {

        fs_dir_t file_dir = NULL;
        struct dirent *file = NULL;
        int file_index = lv_list_get_btn_index(file_list, obj);

        fs_open_dir(&file_dir, global_path);
        file = readdir(file_dir);

        for (int i = 0; i < file_index; i++) {
            file = readdir(file_dir);
        }
        if (DT_DIR != file->d_type) {
            fs_close_dir(&file_dir);
            return;
        }

        fs_close_dir(&file_dir);
        strcat(global_path, "/");
        strcat(global_path, file->d_name);
        ui_update_list(global_path);
        ui_update_path();
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
        ui_update_path();
    }
}

static void ui_main(void)
{
    lv_port_sem_take();

    size_t file_count = 0;
    global_path = heap_caps_malloc(256, MALLOC_CAP_SPIRAM);
    global_sub_path = heap_caps_malloc(256, MALLOC_CAP_SPIRAM);
    strcpy(global_path, MOUNT_POINT);

    btn_back = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_value_str(btn_back, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Back");
    lv_obj_set_style_local_value_font(btn_back, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_align(btn_back, NULL, LV_ALIGN_IN_TOP_LEFT, 25, 5);
    lv_obj_set_event_cb(btn_back, btn_back_event_cb);

    label_path = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_size(label_path, 650, 50);
    lv_obj_set_style_local_text_font(label_path, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_label_set_text_static(label_path, global_path);
    lv_obj_align(label_path, btn_back, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

    fs_get_dir_file_count(global_path, &file_count);
    file_list = lv_list_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_border_width(file_list, LV_LIST_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_radius(file_list, LV_LIST_PART_BG, LV_STATE_DEFAULT, 15);
    lv_obj_set_click(file_list, true);
    lv_obj_set_size(file_list, 750, 400);
    lv_obj_align(file_list, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -25);

    lv_obj_set_style_local_text_font(file_list, LV_LIST_PART_BG, LV_STATE_DEFAULT, &font_en_24);

    ui_update_list(global_path);

    lv_port_sem_give();
}

static void ui_update_path(void)
{
    lv_label_set_text_static(label_path, global_path);
}

static void ui_update_list(char *path)
{
    char *file_name = heap_caps_malloc(255 + 1, MALLOC_CAP_SPIRAM);

    while (lv_list_get_size(file_list)) {
        lv_list_remove(file_list, 0);
    }

    size_t file_count = 0;
    fs_get_dir_file_count(path, &file_count);

    fs_dir_t file_dir = NULL;
    fs_open_dir(&file_dir, path);
    if (NULL == file_dir) {
        free(file_name);
        return ;
    }

    struct dirent *file = NULL;

    for (size_t i = 0; i < file_count; i++) {
        file = readdir(file_dir);
        if (NULL == file) {
            fs_close_dir(&file_dir);
            free(file_name);
            return;
        }
        lv_obj_set_event_cb(lv_list_add_btn(file_list, &icon_file, file->d_name), btn_event_cb);
    }

    fs_close_dir(&file_dir);

    free(file_name);
}

void app_main(void)
{
    /* Init I2C bus for sensor communication */
    ESP_ERROR_CHECK(bsp_i2c_init(I2C_NUM_0, 400000));

    /* Init LCD and touch panel */
    ESP_ERROR_CHECK(bsp_lcd_init());
    ESP_ERROR_CHECK(ft5x06_init());
    
    /* Initialize LVGL */
    ESP_ERROR_CHECK(lvgl_init(LVGL_SCR_SIZE / 8, LV_BUF_ALLOC_INTERNAL));

    /* Init SD card and file system hal API */
    ESP_ERROR_CHECK(bsp_sdcard_init());
    ESP_ERROR_CHECK(fs_hal_init());

    /* Init main UI */
    ui_main();
}
