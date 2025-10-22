/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <string.h>
#include "dual_button.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_err.h"
#include "button_types.h"
#include "iot_button.h"
#include "driver/gpio.h"
#include "button_interface.h"

static const char *TAG = "dual_button";

typedef struct {
    button_driver_t base;          /**< button driver */
    int32_t gpio_num[2];           /**< num of gpio */
    uint8_t active_level;          /**< gpio level when press down */
} button_dual_obj;

static uint8_t button_dual_get_key_level(button_driver_t *button_driver)
{
    button_dual_obj *btn = __containerof(button_driver, button_dual_obj, base);
    int level[2] = {0};
    level[0] = gpio_get_level(btn->gpio_num[0]);
    level[1] = gpio_get_level(btn->gpio_num[1]);
    if (level[0] == btn->active_level && level[1] == btn->active_level) {
        return 1;
    }
    return 0;
}

esp_err_t iot_button_new_dual_button_device(const button_config_t *button_config, const button_dual_config_t *dual_btn_cfg, button_handle_t *ret_button)
{
    button_dual_obj *obj = NULL;
    esp_err_t ret = ESP_OK;
    ESP_GOTO_ON_FALSE(button_config && dual_btn_cfg && ret_button, ESP_ERR_INVALID_ARG, err, TAG, "Invalid argument");

    obj = (button_dual_obj *)calloc(1, sizeof(button_dual_obj));
    ESP_GOTO_ON_FALSE(obj, ESP_ERR_NO_MEM, err, TAG, "No memory for gpio button");
    memcpy(&obj->gpio_num, &dual_btn_cfg->gpio_num, sizeof(obj->gpio_num));
    obj->active_level = dual_btn_cfg->active_level;

    // Initialize GPIO if not skipped
    if (!dual_btn_cfg->skip_gpio_init) {
        gpio_config_t gpio_conf = {
            .intr_type = GPIO_INTR_DISABLE,
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = (1ULL << dual_btn_cfg->gpio_num[0]) | (1ULL << dual_btn_cfg->gpio_num[1]),
        };

        if (dual_btn_cfg->active_level) {
            gpio_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
            gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        } else {
            gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        }

        ret = gpio_config(&gpio_conf);
        ESP_GOTO_ON_FALSE(ret == ESP_OK, ESP_FAIL, err, TAG, "GPIO configuration failed");

    } else {
        ESP_LOGI(TAG, "Dual button GPIO initialization skipped");
    }

    obj->base.get_key_level = button_dual_get_key_level;

    ret = iot_button_create(button_config, &obj->base, ret_button);
    ESP_GOTO_ON_FALSE(ret == ESP_OK, ESP_FAIL, err, TAG, "Create button failed");
    return ESP_OK;
err:
    if (obj) {
        free(obj);
    }
    return ret;
}
