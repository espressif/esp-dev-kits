/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __APP_RAINMAKER_H__
#define __APP_RAINMAKER_H__

#include "esp_rmaker_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ESP_RMAKER_DEF_SWITCH_L_NAME "Switch_L"
#define ESP_RMAKER_DEF_SWITCH_R_NAME "Switch_R"
#define ESP_RMAKER_DEF_BATTERY_LEVEL_NAME "Battery Level"
#define ESP_RMAKER_DEF_ACTION_NAME "Keyboard Layer"
#define ESP_RMAKER_DEF_LIGHT_MODE_NAME "Light Mode"
#define ESP_RMAKER_DEF_MODE_NAME     "Mode"
static const char *keyboard_action_list[] = {"1.VOLUME UP/DOWN", "2.PAGE UP/DOWN", "3.CTRL + C/V", "4.PREVIOUS/NEXT"};
static const char *light_mode_list[] = {"1.Constant", "2.Breathing", "3.Blink", "4.Heatmap"};
/**
 * @brief RainMaker device initialization parameters structure
 */
typedef struct {
    esp_rmaker_device_write_cb_t write_cb;           // Write callback function
    esp_rmaker_device_read_cb_t read_cb;           // Read callback function
    bool power;                       // power state
    uint8_t brightness;               // brightness value
    uint8_t hue;                      // hue value
    uint8_t saturation;               // saturation value
    uint8_t light_speed;                    // speed value
    bool switch_l;                    // left switch state
    bool switch_r;                    // right switch state
    uint8_t action_type;              // action type index
    uint8_t light_mode_type;          // light mode type index
    uint8_t battery_level;            // battery level
    const char *device_name;                  // Device name
    const char *device_type;                  // Device type
    const char *node_name;                    // Node name
} app_rainmaker_params_t;

/**
 * @brief Default RainMaker initialization parameters
 */
#define APP_RAINMAKER_DEFAULT_PARAMS() { \
    .write_cb = NULL, \
    .power = true, \
    .brightness = 100, \
    .hue = 120, \
    .saturation = 100, \
    .light_speed = 100, \
    .switch_l = false, \
    .switch_r = false, \
    .action_type = 0, \
    .light_mode_type = 3, \
    .battery_level = 50, \
    .device_name = "ESP_DualKey", \
    .device_type = "Keyboard", \
    .node_name = "ESP_DualKey" \
}

typedef enum {
    SWITCH_OFF = 0,
    SWITCH_ON,
    SWITCH_TOGGLE,
} switch_state_t;

esp_err_t app_rainmaker_set_switch_l(switch_state_t state);
esp_err_t app_rainmaker_set_switch_r(switch_state_t state);
esp_err_t app_rainmaker_update_battery_level(uint8_t level);
esp_err_t app_rainmaker_init(app_rainmaker_params_t *params);

#ifdef __cplusplus
}
#endif

#endif
