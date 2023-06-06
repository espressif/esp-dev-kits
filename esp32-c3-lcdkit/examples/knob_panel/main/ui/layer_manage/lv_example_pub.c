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
#include "lv_example_pub.h"

static const char *TAG = "LVGL_PUB";

static lv_group_t *group;

void ui_add_obj_to_encoder_group(lv_obj_t *obj)
{
    lv_group_add_obj(group, obj);
}

void ui_remove_all_objs_from_encoder_group(void)
{
    lv_group_remove_all_objs(group);
}

void ui_obj_to_encoder_init(void)
{
    group = lv_group_create();
    lv_group_set_default(group);
    lv_indev_t *indev = lv_indev_get_next(NULL);
    if (LV_INDEV_TYPE_ENCODER == lv_indev_get_type(indev)) {
        ESP_LOGI(TAG, "add group for encoder");
        lv_indev_set_group(indev, group);
        lv_group_focus_freeze(group, false);
    }
}
