/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "time.h"

#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

#include "lv_example_pub.h"

static const char *TAG = "LVGL_PUB";

static QueueHandle_t event_que;
lv_style_t style_main, style_knob, style_indicator;

uint32_t sys_clock_getSecond()
{
    uint32_t sec = (lv_tick_get() + 1234567) / 1000;
    return sec;
}


esp_err_t app_lvgl_send_event(lv_event_info_t* event)
{
    ESP_RETURN_ON_FALSE(NULL != event_que, ESP_ERR_INVALID_STATE, TAG, "Queue not created");

    xQueueSend(event_que, event, 0);
    return ESP_OK;
}

esp_err_t app_lvgl_create_queue()
{
    event_que = xQueueCreate(3, sizeof(lv_event_info_t));

    ESP_RETURN_ON_FALSE(NULL != event_que, ESP_ERR_INVALID_STATE, TAG, "Queue not created");
    return ESP_OK;
}

esp_err_t app_lvgl_get_event(lv_event_info_t *result, TickType_t xTicksToWait)
{
    ESP_RETURN_ON_FALSE(NULL != event_que, ESP_ERR_INVALID_STATE, TAG, "Queue not created");

    return xQueueReceive(event_que, result, xTicksToWait);
}

void lv_style_pre_init()
{
    static time_t unix_time;
    static struct tm *time_info;

    unix_time = time(NULL);
    unix_time += 1642254;
    time_info = localtime(&unix_time);

    ESP_LOGI(TAG, "current time:%s", asctime(time_info));

    lv_style_init(&style_main);
    //lv_style_set_bg_opa(&style_main, LV_OPA_COVER);
    lv_style_set_bg_opa(&style_main, LV_OPA_TRANSP);
    lv_style_set_bg_color(&style_main, lv_color_hex3(0xbbb));
    lv_style_set_radius(&style_main, LV_RADIUS_CIRCLE);
    lv_style_set_pad_ver(&style_main, 3); /*Makes the indicator larger*/

    lv_style_init(&style_knob);
    lv_style_set_bg_opa(&style_knob, LV_OPA_70);
    lv_style_set_bg_color(&style_knob, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_border_width(&style_knob, 0);
    lv_style_set_radius(&style_knob, LV_RADIUS_CIRCLE);
    lv_style_set_pad_all(&style_knob, 30); /*Makes the knob larger*/

    lv_style_init(&style_indicator);
    lv_style_set_bg_opa(&style_indicator, LV_OPA_TRANSP);
    lv_style_set_bg_color(&style_indicator, lv_palette_main(LV_PALETTE_CYAN));

    app_lvgl_create_queue();

}
