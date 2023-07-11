/* SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "app_mode.h"
#include "app_espnow.h"
#include "iot_button.h"
#include "app_helper.h"
#include "app_io.h"
#include "esp_timer.h"
#include "esp_mac.h"
#include "esp_private/periph_ctrl.h"

#define KEY_WORK_MODE    "mode"

static const char *TAG = "bridge_main";
static usb_bridge_mode_t *q_usb_bridge_mode = NULL;
static esp_timer_handle_t oneshot_timer;

void timer_stop(void)
{
    if (esp_timer_is_active(oneshot_timer)) {
        ERROR_CHECK_RETURN_VOID(ESP_OK == esp_timer_stop(oneshot_timer), "esp_timer_stop failed");
    }
}

bool timer_is_active(void)
{
    return esp_timer_is_active(oneshot_timer);
}

static void _oneshot_timer_callback(void* arg)
{
    q_usb_bridge_mode->led_update_mode(BLINK_WIRELESS_CONNECTING, false);
}

static esp_err_t _timer_init(void)
{
    const esp_timer_create_args_t oneshot_timer_args = {
        .callback = &_oneshot_timer_callback,
        /* name is optional, but may help identify the timer when debugging */
        .name = "one-shot"
    };
    ESP_ERROR_CHECK(esp_timer_create(&oneshot_timer_args, &oneshot_timer));
    return ESP_OK;
}

#ifdef CONFIG_BRIDGE_SUPPORT_WIRELESS
static esp_err_t _get_value_from_nvs(char *key, void *value)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("memory", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return err;
    } else {
        err = nvs_get_i32(my_handle, key, value);
        switch (err) {
        case ESP_OK:
            ESP_LOGI(TAG, "%s value is %" PRIu32 "", key, *(int32_t *)value);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGI(TAG, "%s is not initialized yet!", key);
            break;
        default :
            ESP_LOGE(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
        }

        nvs_close(my_handle);
        return err;
    }
}

static esp_err_t _set_value_to_nvs(char *key, int value)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("memory", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return ESP_FAIL;
    } else {
        err = nvs_set_i32(my_handle, key, value);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "NVS set failed %s", esp_err_to_name(err));
        }

        err = nvs_commit(my_handle);
        if ( err != ESP_OK) {
            ESP_LOGE(TAG, "NVS commit failed");
        }

        nvs_close(my_handle);
    }

    return err;
}

static void switch_button_change_mode_cb(void *arg, void *data)
{
    work_mode_t mode = MODE_WIRED;
    _get_value_from_nvs(KEY_WORK_MODE, (void *)&mode);
    mode += 1;
    if (mode >= MODE_MAX) {
        mode = MODE_WIRED;
    }

    ERROR_CHECK_RETURN_VOID(ESP_OK == _set_value_to_nvs(KEY_WORK_MODE, mode), "set mode to nvs failed");
    // TODO: Dynamically modify mode instead of rebooting
    esp_restart();
}

static void switch_button_bind_cb(void *arg, void *data)
{
    ERROR_CHECK_RETURN_VOID(NULL != q_usb_bridge_mode, "q_usb_bridge_mode can't be NULL");

    if (q_usb_bridge_mode->mode == MODE_WIRED) {
        return;
    }

    if (!q_usb_bridge_mode->if_bind) {
        if (!esp_timer_is_active(oneshot_timer)) {
            ERROR_CHECK_RETURN_VOID(ESP_OK == q_usb_bridge_mode->led_update_mode(BLINK_WIRELESS_CONNECTING, true), "led_indicator_update_mode failed");
            ERROR_CHECK_RETURN_VOID(ESP_OK == esp_timer_start_once(oneshot_timer, 10000000), "esp_timer_start_once failed");
            ERROR_CHECK_RETURN_VOID(ESP_OK == q_usb_bridge_mode->espnow_bind_begin(10*1000,-55), "espnow_bind_begin failed");
        }
    } else {
        ERROR_CHECK_RETURN_VOID(ESP_OK == q_usb_bridge_mode->led_update_mode(BLINK_MODE, false), "led_indicator_update_mode failed");
        ERROR_CHECK_RETURN_VOID(ESP_OK == q_usb_bridge_mode->led_update_mode(BLINK_WIRELESS_DISCONNECTED, true), "led_indicator_update_mode failed");
        ERROR_CHECK_RETURN_VOID(ESP_OK == app_espnow_remove_bind(), "app_espnow_remove_bind failed");
        memset(q_usb_bridge_mode->bind_mac, 0, 6);
        q_usb_bridge_mode->if_bind = false;
    }
}

static esp_err_t _switch_button_init(void)
{
    button_config_t button_config = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = 2000,
        .gpio_button_config = {
            .gpio_num = SWITCH_BUTTON,
            .active_level = 0,
        },
    };

    button_handle_t button_handle = iot_button_create(&button_config);
    assert(button_handle != NULL);
    esp_err_t ret = iot_button_register_cb(button_handle, BUTTON_DOUBLE_CLICK, switch_button_change_mode_cb, NULL);
    ret |= iot_button_register_cb(button_handle, BUTTON_LONG_PRESS_START, switch_button_bind_cb, NULL);
    ERROR_CHECK(ESP_OK == ret, "button register callback fail", ret);
    return ESP_OK;
}
#endif

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        /* NVS partition was truncated and needs to be erased */
        ESP_ERROR_CHECK(nvs_flash_erase());
        /* Retry nvs_flash_init */
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    _timer_init();
    work_mode_t mode = MODE_WIRED;
#ifdef CONFIG_BRIDGE_SUPPORT_WIRELESS
    _get_value_from_nvs(KEY_WORK_MODE, (void *)&mode);
    ERROR_CHECK_RETURN_VOID(ESP_OK == _switch_button_init(), "switch button init failed");
#endif
    q_usb_bridge_mode = usb_bridge_mode_create(mode);
    ERROR_CHECK_RETURN_VOID(NULL != q_usb_bridge_mode, "usb_bridge_mode_create failed");
    ERROR_CHECK_RETURN_VOID(ESP_OK == q_usb_bridge_mode->init(), "usb_bridge_mode init failed");
}