/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "esp_system.h"
#ifdef ESP_IDF_VERSION
#include "esp_log.h"
#endif
#include "lvgl.h"
#include "ui.h"
#include "ui_menu.h"
#include <math.h>
#include "lv_example_pub.h"

static const char *TAG = "ui";
static lv_group_t *group;

esp_err_t ui_init(void)
{
    bsp_display_lock(0);
    group = lv_group_create();
    lv_group_set_default(group);
    lv_indev_t *indev = lv_indev_get_next(NULL);
    if (LV_INDEV_TYPE_ENCODER == lv_indev_get_type(indev)) {
        LV_LOG_USER("add group for encoder");
        lv_indev_set_group(indev, group);
        lv_group_focus_freeze(group, false);
    }

    lv_home_create();
    bsp_display_unlock();
    return ESP_OK;
}

void ui_add_obj_to_encoder_group(lv_obj_t *obj)
{
    lv_group_add_obj(group, obj);
}

void ui_remove_all_objs_from_encoder_group(void)
{
    lv_group_remove_all_objs(group);
}

uint32_t ui_get_num_offset(uint32_t num, int32_t max, int32_t offset)
{
    if (num >= max) {
        LV_LOG_USER("[ERROR] num should less than max");
        return num;
    }

    uint32_t i;
    if (offset >= 0) {
        i = (num + offset) % max;
    } else {
        offset = max + (offset % max);
        i = (num + offset) % max;
    }

    return i;
}