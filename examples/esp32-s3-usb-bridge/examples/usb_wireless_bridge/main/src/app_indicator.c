/* SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_log.h"
#include "app_mode.h"
#include "app_indicator.h"
#include "led_indicator.h"
#include "ws2812_led.h"

#define WS2812_CHANNEL_ON  100
#define WS2812_CHANNEL_OFF (!WS2812_CHANNEL_ON)

static uint16_t q_hue = 0; 
static led_indicator_handle_t led_indicator_handle = NULL;

/***************************************************** LED Indicator *********************************************************************/
static const blink_step_t blink_eub_abort[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 100},
    {LED_BLINK_HOLD, LED_STATE_OFF, 100},
    {LED_BLINK_HOLD, LED_STATE_ON, 100},
    {LED_BLINK_HOLD, LED_STATE_OFF, 100},
    {LED_BLINK_HOLD, LED_STATE_ON, 100},
    {LED_BLINK_HOLD, LED_STATE_OFF, 100},
    {LED_BLINK_STOP, 0, 0},
};

static const blink_step_t blink_mode[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 1000},
    {LED_BLINK_LOOP, 0, 0},
};

static const blink_step_t blink_wireless_connecting[] = {
    {LED_BLINK_HOLD, LED_STATE_OFF, 0},
    {LED_BLINK_BREATHE, LED_STATE_ON, 500},
    {LED_BLINK_BREATHE, LED_STATE_OFF, 500},
    {LED_BLINK_LOOP, 0, 0},
};

static const blink_step_t blink_wireless_disconnected[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_LOOP, 0, 0},
};

static blink_step_t const *led_indicator_mode_lst[] = {
    [BLINK_EUB_ABORT] = blink_eub_abort,
    [BLINK_MODE] = blink_mode,
    [BLINK_WIRELESS_CONNECTING] = blink_wireless_connecting,
    [BLINK_WIRELESS_DISCONNECTED] = blink_wireless_disconnected,
    [BLINK_NUM] = NULL,
};

static esp_err_t led_indicator_custom_init(void *param)
{
    return ws2812_led_init();
}

static esp_err_t led_indicator_custom_deinit(void *param)
{
    return ws2812_led_clear();
}

static esp_err_t led_indicator_custom_set_on_off(void *hardware_data, bool on_off)
{
    uint8_t brightness = on_off?WS2812_CHANNEL_ON:WS2812_CHANNEL_OFF;
    return ws2812_led_set_hsv(q_hue, 100, brightness);
}

static esp_err_t led_indicator_custom_set_brightness(void *hardware_data, uint32_t brightness)
{
    brightness = brightness * 100 / 255;
    return ws2812_led_set_hsv(q_hue, 100, brightness);
}

void app_indicator_set_hue(uint16_t hue)
{
    q_hue = hue; 
}

uint16_t app_indicator_get_hue(void)
{
    return q_hue; 
}

esp_err_t app_indicator_update_mode(uint16_t hue, const led_indicator_mode_type mode_type, const bool if_start)
{
    q_hue = hue;
    if (if_start) {
        return led_indicator_start(led_indicator_handle, mode_type);
    } else {
        return led_indicator_stop(led_indicator_handle, mode_type);
    }
}

esp_err_t app_indicator_init(uint16_t hue)
{
    q_hue = hue; 
    led_indicator_custom_config_t led_indicator_custom_config = {
        .is_active_level_high = 1,
        .duty_resolution = 8,
        .hal_indicator_init = led_indicator_custom_init,
        .hal_indicator_deinit = led_indicator_custom_deinit,
        .hal_indicator_set_on_off = led_indicator_custom_set_on_off,
        .hal_indicator_set_brightness = led_indicator_custom_set_brightness,
        .hardware_data = NULL,
    };

    led_indicator_config_t config = {
        .led_indicator_custom_config = &led_indicator_custom_config,
        .mode = LED_CUSTOM_MODE,
        .blink_lists = led_indicator_mode_lst,
        .blink_list_num = BLINK_NUM,
    };

    led_indicator_handle = led_indicator_create(&config);
    return led_indicator_handle != NULL ? ESP_OK:ESP_FAIL;
}