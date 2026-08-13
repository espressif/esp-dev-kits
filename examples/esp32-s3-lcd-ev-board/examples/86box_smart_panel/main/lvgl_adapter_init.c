/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "lvgl_adapter_init.h"

#include "sdkconfig.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_lv_adapter.h"
#include "bsp/display.h"
#include "bsp/touch.h"

static const char *TAG = "lvgl_adapter_init";

lv_display_t *lvgl_adapter_init(const bsp_display_config_t *cfg)
{
    const esp_lv_adapter_rotation_t rotation = ESP_LV_ADAPTER_ROTATE_0;
    const esp_lv_adapter_tear_avoid_mode_t tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_DOUBLE_DIRECT;
    const uint8_t required_fbs = esp_lv_adapter_get_required_frame_buffer_count(
        tear_avoid_mode, rotation);
    const bsp_display_config_t default_cfg = { 0 };
    const bsp_display_config_t *display_cfg = (cfg != NULL) ? cfg : &default_cfg;

    if (required_fbs != CONFIG_BSP_LCD_RGB_BUFFER_NUMS) {
        ESP_LOGE(TAG, "Frame buffer mismatch: required=%u configured=%u", required_fbs, CONFIG_BSP_LCD_RGB_BUFFER_NUMS);
        return NULL;
    }

    esp_lcd_panel_handle_t panel = NULL;
    esp_lcd_panel_io_handle_t panel_io = NULL;
    esp_err_t ret = bsp_display_new(display_cfg, &panel, &panel_io);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "BSP display init failed (%s)", esp_err_to_name(ret));
        return NULL;
    }

    const esp_lv_adapter_config_t adapter_cfg = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    ret = esp_lv_adapter_init(&adapter_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LVGL adapter init failed (%s)", esp_err_to_name(ret));
        return NULL;
    }

    esp_lv_adapter_display_config_t disp_cfg = ESP_LV_ADAPTER_DISPLAY_RGB_DEFAULT_CONFIG(
        panel, panel_io, BSP_LCD_H_RES, BSP_LCD_V_RES, rotation);
    disp_cfg.tear_avoid_mode = tear_avoid_mode;

    lv_display_t *disp = esp_lv_adapter_register_display(&disp_cfg);
    if (disp == NULL) {
        ESP_LOGE(TAG, "Register display failed");
        return NULL;
    }

    esp_lcd_touch_handle_t touch = NULL;
    ret = bsp_touch_new(NULL, &touch);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Touch init failed (%s)", esp_err_to_name(ret));
        return NULL;
    }

    const esp_lv_adapter_touch_config_t touch_cfg = ESP_LV_ADAPTER_TOUCH_DEFAULT_CONFIG(disp, touch);
    if (esp_lv_adapter_register_touch(&touch_cfg) == NULL) {
        ESP_LOGE(TAG, "Register touch failed");
        return NULL;
    }

    ret = esp_lv_adapter_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LVGL adapter start failed (%s)", esp_err_to_name(ret));
        return NULL;
    }

    ESP_LOGI(TAG, "LVGL adapter started: %ux%u, tear_mode=%d", BSP_LCD_H_RES, BSP_LCD_V_RES, tear_avoid_mode);
    return disp;
}
