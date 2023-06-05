/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "esp_log.h"
#include "sys_check.h"
#include "lvgl.h"
#include "bsp/esp-bsp.h"
#include "ui_font_declare.h"

static const char *TAG = "sys_check";

#define LOG_TRACE(...)  ESP_LOGI(TAG, ##__VA_ARGS__)
#define CHECK(a, str, ret_val) do { \
        if (!(a)) { \
            err_show(str); \
            ESP_LOGE(TAG,"%s(%d): %s", __FUNCTION__, __LINE__, str); \
            return (ret_val); \
        } else { \
            LOG_TRACE(str); \
        } \
    } while(0)

static lv_obj_t *msg_box = NULL;

/**
 * @brief Show error message on msgbox. Create one if not created.
 *
 * @param text Text to show on msgbox. lv_msgbox will create a copy of text.
 */
static void err_show(const char *text)
{
    bsp_display_lock(0);
    if (NULL == msg_box) {
        msg_box = lv_msgbox_create(lv_scr_act(), NULL);
        lv_obj_set_style_local_text_font(msg_box, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, &font_en_24);
        lv_obj_set_style_local_border_width(msg_box, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, 0);

    }
    lv_msgbox_set_text(msg_box, text);
    lv_obj_align(msg_box, NULL, LV_ALIGN_CENTER, 0, 30);
    bsp_display_unlock();

    ESP_LOGW(TAG, "err_show:%s", text);
}

esp_err_t sys_check(void)
{
    return ESP_OK;
}

esp_err_t img_file_check(const char *lv_img_path)
{
    lv_fs_file_t file;
    lv_fs_res_t ret = lv_fs_open(&file, lv_img_path, LV_FS_MODE_RD);

    if (LV_FS_RES_OK == ret) {
        lv_fs_close(&file);
        return ESP_OK;
    }

    err_show("Check for image resources!");

    return ESP_FAIL;
}
