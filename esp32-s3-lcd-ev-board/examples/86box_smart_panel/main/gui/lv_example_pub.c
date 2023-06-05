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
sys_param_t *sys_set;

button_style_t g_btn_styles;
lv_style_t style_main, style_knob, style_indicator;
lv_style_t style_set_main, style_set_indicator, style_set_pressed_color;

uint32_t sys_clock_getSecond()
{
    uint32_t sec = (lv_tick_get() + 1234567) / 1000;
    return sec;
}

esp_err_t app_lvgl_send_event(lv_event_info_t *event)
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

    static const lv_style_prop_t props[] = {LV_STYLE_BG_COLOR, 0};
    static lv_style_transition_dsc_t transition_dsc;
    lv_style_transition_dsc_init(&transition_dsc, props, lv_anim_path_linear, 300, 0, NULL);

    lv_style_init(&style_set_main);
    lv_style_set_bg_opa(&style_set_main, LV_OPA_COVER);
    lv_style_set_bg_color(&style_set_main, lv_color_hex(COLOUR_GREY_8F));
    lv_style_set_radius(&style_set_main, LV_RADIUS_CIRCLE);
    lv_style_set_pad_ver(&style_set_main, -2); /*Makes the indicator larger*/

    lv_style_init(&style_set_indicator);//click
    lv_style_set_bg_opa(&style_set_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&style_set_indicator, lv_color_hex(COLOUR_WHITE));
    lv_style_set_radius(&style_set_indicator, LV_RADIUS_CIRCLE);
    lv_style_set_transition(&style_set_indicator, &transition_dsc);

    lv_style_init(&style_set_pressed_color);//slider
    lv_style_set_bg_color(&style_set_pressed_color, lv_color_hex(COLOUR_WHITE));


    /*Init the style for the default state*/
    lv_style_init(&g_btn_styles.style);
    lv_style_set_radius(&g_btn_styles.style, 5);
    lv_style_set_bg_color(&g_btn_styles.style, lv_color_make(255, 255, 255));

    lv_style_set_border_opa(&g_btn_styles.style, LV_OPA_30);
    lv_style_set_border_width(&g_btn_styles.style, 2);
    lv_style_set_border_color(&g_btn_styles.style, lv_palette_main(LV_PALETTE_GREY));

    lv_style_set_shadow_width(&g_btn_styles.style, 7);
    lv_style_set_shadow_color(&g_btn_styles.style, lv_color_make(0, 0, 0));
    lv_style_set_shadow_ofs_x(&g_btn_styles.style, 0);
    lv_style_set_shadow_ofs_y(&g_btn_styles.style, 0);

    lv_style_init(&g_btn_styles.style_pr);
    lv_style_set_border_opa(&g_btn_styles.style_pr, LV_OPA_40);
    lv_style_set_border_width(&g_btn_styles.style_pr, 2);
    lv_style_set_border_color(&g_btn_styles.style_pr, lv_palette_main(LV_PALETTE_GREY));

    lv_style_init(&g_btn_styles.style_focus);
    lv_style_set_outline_color(&g_btn_styles.style_focus, lv_color_make(255, 0, 0));

    lv_style_init(&g_btn_styles.style_focus_no_outline);
    lv_style_set_outline_width(&g_btn_styles.style_focus_no_outline, 0);

    app_lvgl_create_queue();

    sys_set = settings_get_parameter();
    // sys_set->need_hint = true;

}
