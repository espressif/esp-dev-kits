/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "esp_err.h"
#include "esp_log.h"

#include "app_network.h"

#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_params.h>
#include <esp_rmaker_standard_devices.h>
#include <esp_rmaker_standard_types.h>
#include <esp_rmaker_schedule.h>
#include <esp_rmaker_scenes.h>

#include <app_wifi.h>
#include <app_insights.h>
#include <string.h>
#include "app_rainmaker.h"
#include "bsp/esp_dualkey.h"

static const char *TAG = "rainmaker";

int keyboard_action_type = 0;

esp_rmaker_device_t *switch_device;
esp_rmaker_param_t *switch_l_param;
esp_rmaker_param_t *switch_r_param;
esp_rmaker_param_t *battery_level_param;

static esp_rmaker_param_t *create_action_mode_param(const char *param_name, const char *val)
{
    esp_rmaker_param_t *param = esp_rmaker_param_create(param_name, ESP_RMAKER_DEF_MODE_NAME,
                                esp_rmaker_str(val), PROP_FLAG_READ | PROP_FLAG_WRITE);
    if (param) {
        esp_rmaker_param_add_ui_type(param, ESP_RMAKER_UI_DROPDOWN);
        esp_rmaker_param_add_valid_str_list(param, keyboard_action_list, sizeof(keyboard_action_list) / sizeof(keyboard_action_list[0]));
    }
    return param;
}

static esp_rmaker_param_t *create_light_mode_param(const char *param_name, const char *val)
{
    esp_rmaker_param_t *param = esp_rmaker_param_create(param_name, ESP_RMAKER_DEF_MODE_NAME,
                                esp_rmaker_str(val), PROP_FLAG_READ | PROP_FLAG_WRITE);
    if (param) {
        esp_rmaker_param_add_ui_type(param, ESP_RMAKER_UI_DROPDOWN);
        esp_rmaker_param_add_valid_str_list(param, light_mode_list, sizeof(light_mode_list) / sizeof(light_mode_list[0]));
    }
    return param;
}

static esp_rmaker_param_t *create_battery_level_param(const char *param_name, int val)
{
    esp_rmaker_param_t *param = esp_rmaker_param_create(param_name, ESP_RMAKER_DEF_BATTERY_LEVEL_NAME,
                                esp_rmaker_int(val), PROP_FLAG_READ);
    return param;
}

esp_rmaker_param_t *esp_rmaker_light_speed_param_create(const char *param_name, int val)
{
    esp_rmaker_param_t *param = esp_rmaker_param_create(param_name, ESP_RMAKER_PARAM_SPEED,
                                esp_rmaker_int(val), PROP_FLAG_READ | PROP_FLAG_WRITE);
    if (param) {
        esp_rmaker_param_add_ui_type(param, ESP_RMAKER_UI_SLIDER);
        esp_rmaker_param_add_bounds(param, esp_rmaker_int(0), esp_rmaker_int(100), esp_rmaker_int(1));
    }
    return param;
}

esp_err_t app_rainmaker_update_battery_level(uint8_t level)
{
    esp_rmaker_param_update_and_report(battery_level_param, esp_rmaker_int(level));
    return ESP_OK;
}

esp_err_t app_rainmaker_set_switch_l(switch_state_t state)
{
    // Apply new state based on switch_state_t
    bool new_state = false;
    switch (state) {
    case SWITCH_ON:
        new_state = true;
        break;
    case SWITCH_OFF:
        new_state = false;
        break;
    case SWITCH_TOGGLE:
        bool current_state = esp_rmaker_param_get_val(switch_l_param)->val.b;
        new_state = !current_state;
        break;
    default:
        return ESP_ERR_INVALID_ARG;
    }
    bsp_output_io_set(UART1_TX_IO, new_state);
    bsp_output_io_set(UART1_RX_IO, new_state);

    esp_rmaker_param_update_and_report(switch_l_param, esp_rmaker_bool(new_state));
    return ESP_OK;
}

esp_err_t app_rainmaker_set_switch_r(switch_state_t state)
{
    // Apply new state based on switch_state_t
    bool new_state = false;
    switch (state) {
    case SWITCH_ON:
        new_state = true;
        break;
    case SWITCH_OFF:
        new_state = false;
        break;
    case SWITCH_TOGGLE:
        bool current_state = esp_rmaker_param_get_val(switch_r_param)->val.b;
        new_state = !current_state;
        break;
    default:
        return ESP_ERR_INVALID_ARG;
    }
    bsp_output_io_set(UART2_TX_IO, new_state);
    bsp_output_io_set(UART2_RX_IO, new_state);
    esp_rmaker_param_update_and_report(switch_r_param, esp_rmaker_bool(new_state));
    return ESP_OK;
}

esp_err_t app_rainmaker_init(app_rainmaker_params_t *params)
{
    app_network_init();
    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = false,
    };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, "ESP RainMaker Device", "Switch");
    if (!node) {
        ESP_LOGE(TAG, "Could not initialise node. Aborting!!!");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        abort();
    }

    /* Create a device and add the relevant parameters to it */
    switch_device = esp_rmaker_switch_device_create(params->device_name, NULL, params->power);
    esp_rmaker_device_add_cb(switch_device, params->write_cb, params->read_cb);

    switch_l_param = esp_rmaker_power_param_create(ESP_RMAKER_DEF_SWITCH_L_NAME, params->switch_l);
    esp_rmaker_device_add_param(switch_device, switch_l_param);
    switch_r_param = esp_rmaker_power_param_create(ESP_RMAKER_DEF_SWITCH_R_NAME, params->switch_r);
    esp_rmaker_device_add_param(switch_device, switch_r_param);

    esp_rmaker_device_add_param(switch_device, create_action_mode_param(ESP_RMAKER_DEF_ACTION_NAME, keyboard_action_list[params->action_type]));
    esp_rmaker_device_add_param(switch_device, create_light_mode_param(ESP_RMAKER_DEF_LIGHT_MODE_NAME, light_mode_list[params->light_mode_type]));

    esp_rmaker_device_add_param(switch_device, esp_rmaker_hue_param_create(ESP_RMAKER_DEF_HUE_NAME, params->hue));
    esp_rmaker_device_add_param(switch_device, esp_rmaker_saturation_param_create(ESP_RMAKER_DEF_SATURATION_NAME, params->saturation));
    esp_rmaker_device_add_param(switch_device, esp_rmaker_brightness_param_create(ESP_RMAKER_DEF_BRIGHTNESS_NAME, params->brightness));
    esp_rmaker_device_add_param(switch_device, esp_rmaker_light_speed_param_create(ESP_RMAKER_DEF_SPEED_NAME, params->light_speed));

    battery_level_param = create_battery_level_param(ESP_RMAKER_DEF_BATTERY_LEVEL_NAME, 50);
    esp_rmaker_device_add_param(switch_device, battery_level_param);

    esp_rmaker_node_add_device(node, switch_device);

    /* Enable OTA */
    esp_rmaker_ota_enable_default();

    /* Enable timezone service which will be require for setting appropriate timezone
    * from the phone apps for scheduling to work correctly.
    * For more information on the various ways of setting timezone, please check
    * https://rainmaker.espressif.com/docs/time-service.html.
    */
    esp_rmaker_timezone_service_enable();

    /* Enable scheduling. */
    esp_rmaker_schedule_enable();

    /* Enable Scenes */
    esp_rmaker_scenes_enable();

    /* Enable Insights. Requires CONFIG_ESP_INSIGHTS_ENABLED=y */
    app_insights_enable();

    /* Start the ESP RainMaker Agent */
    esp_rmaker_start();

    esp_err_t err = app_network_set_custom_mfg_data(MGF_DATA_DEVICE_TYPE_SWITCH, MFG_DATA_DEVICE_SUBTYPE_SWITCH);
    /* Start the Wi-Fi.
    * If the node is provisioned, it will start connection attempts,
    * else, it will start Wi-Fi provisioning. The function will return
    * after a connection has been successfully established
    */
    err = app_network_start(POP_TYPE_NONE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not start Wifi. Aborting!!!");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        abort();
    }
    return ESP_OK;
}
