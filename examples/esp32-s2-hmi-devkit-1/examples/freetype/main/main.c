/**
 * @file main.c
 * @brief Sensor example
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

#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include "bsp_ext_io.h"
#include "bsp_lcd.h"
#include "bsp_sdcard.h"
#include "ft5x06.h"
#include "lvgl_port.h"
#include "lv_freetype.h"
#include "tca9554.h"

#define FONT_CNT    (1)
#define FONT_SIZE   (48)
#define BASE_PATH   "/spiffs"

static const char *TAG = "main";

#define LOG_TRACE(...)  ESP_LOGD(TAG, ##__VA_ARGS__);

static char fmt_text[512];
static lv_font_t font;
static lv_obj_t *label = NULL;
static const char *font_name = BASE_PATH "/KaiTi.ttf";

/**
 * @brief Print memory usage for debug.
 * 
 */
static void print_sys_info(const char *text)
{
    if (NULL != text) {
        LOG_TRACE("%s", text);
    }

    LOG_TRACE("DRAM : [%6u], PSRAM : [%7u]",
        heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    vTaskList(fmt_text);
    LOG_TRACE("Task List:\n%s", fmt_text);
}

/**
 * @brief Init SPIFFS storage to read font file.
 * 
 * @return esp_err_t Init result.
 */
static esp_err_t init_storage(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = BASE_PATH,
      .partition_label = NULL,
      .max_files = 2,
      .format_if_mount_failed = false
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        return ret;
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    return ESP_OK;
}

/**
 * @brief Callback function of button.
 * 
 * @param obj Pointer to the button.
 * @param event Triggered event.
 */
static void btn_event_cb(lv_obj_t *obj, lv_event_t event)
{
    (void) event;
    if (LV_EVENT_CLICKED == event) {
        lv_label_set_text(label,
            "使用 FreeType 输出中文\n"
            "您可以长按拖拽文本");
    }
}

static void ui_freetype(void)
{
    label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_drag(label, true);
    lv_obj_set_pos(label, 100, 50);
    lv_label_set_text(label, "你好，自由格式 Freetype!");
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font);

    lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_width(btn, 160);
    lv_obj_set_style_local_value_str(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Change Text");
    lv_obj_set_event_cb(btn, btn_event_cb);
    lv_obj_align(btn, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -20);
}

void app_main(void)
{
    print_sys_info("App begin");
    ESP_ERROR_CHECK(bsp_i2c_init(I2C_NUM_0, 400000));
    
    /* Init LCD and touch IC */
    ESP_ERROR_CHECK(bsp_lcd_init());
    ESP_ERROR_CHECK(ft5x06_init());

    /* Init storage with font file */
    ESP_ERROR_CHECK(init_storage());

    /* Disable default LVGL handler task in `lvgl_port.c` */
    lv_port_use_default_handler(false);
    ESP_ERROR_CHECK(lvgl_init(LVGL_SCR_SIZE / 8, LV_BUF_ALLOC_INTERNAL));

    /* Set background color. And manually call lv_task_handler to update background */
    lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_task_handler();

    /* Init freetype and create font with given file */
    print_sys_info("Initializing freetype");
    ESP_ERROR_CHECK(lv_freetype_init(FONT_CNT));
    ESP_ERROR_CHECK(lv_freetype_font_init(&font, font_name, FONT_SIZE));

    /* Initialize UI */
    print_sys_info("Creating UI");
    ui_freetype();

    print_sys_info("Handler start");
    TickType_t tick = xTaskGetTickCount();
    while (1) {
        lv_task_handler();
        vTaskDelay(pdMS_TO_TICKS(1));
        vTaskDelayUntil(&tick, pdMS_TO_TICKS(20));
    }
}
