/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#pragma once

#include <stdbool.h>
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LV_EVENT_EXIT_CLOCK,
    LV_EVENT_I_AM_HERE,
    LV_EVENT_I_AM_LEAVING,
    LV_EVENT_LIGHT_RGB_SET,
    LV_EVENT_LIGHT_ON,
    LV_EVENT_LIGHT_OFF,

    LV_EVENT_AC_SET_ON,
    LV_EVENT_AC_SET_OFF,
    LV_EVENT_AC_TEMP_ADD,
    LV_EVENT_AC_TEMP_DEC,
} LV_EVENT_TYPE;

typedef struct {
    LV_EVENT_TYPE event;
    void *event_data;
} lv_event_info_t;

bool sr_echo_is_playing(void);

void sr_handler_task(void *pvParam);

extern esp_err_t app_lvgl_create_queue();

extern esp_err_t app_lvgl_send_event(lv_event_info_t *event);

extern esp_err_t app_lvgl_get_event(lv_event_info_t *result, TickType_t xTicksToWait);

#ifdef __cplusplus
}
#endif
