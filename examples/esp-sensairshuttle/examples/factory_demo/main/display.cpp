/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include "lvgl.h"
#include "esp_board_manager.h"
#include "esp_board_manager_includes.h"
#include "devices/dev_display_lcd/dev_display_lcd.h"
#include "devices/dev_lcd_touch_i2c/dev_lcd_touch_i2c.h"

#include "esp_lvgl_port.h"
#include "esp_lvgl_port_disp.h"
#include "brookesia/system_phone/phone.hpp"
#include "boost/thread.hpp"
#ifdef ESP_UTILS_LOG_TAG
#   undef ESP_UTILS_LOG_TAG
#endif
#define ESP_UTILS_LOG_TAG "Display"
#include "esp_lib_utils.h"
#include "display.hpp"

constexpr int  LVGL_TASK_PRIORITY        = 4;
constexpr int  LVGL_TASK_CORE_ID         = 0;
constexpr int  LVGL_TASK_STACK_SIZE      = 20 * 1024;
constexpr int  LVGL_TASK_MAX_SLEEP_MS    = 500;
constexpr int  LVGL_TASK_TIMER_PERIOD_MS = 5;
constexpr bool LVGL_TASK_STACK_CAPS_EXT  = true;
constexpr int  BRIGHTNESS_MIN            = 10;
constexpr int  BRIGHTNESS_MAX            = 100;
constexpr int  BRIGHTNESS_DEFAULT        = 100;

using namespace esp_brookesia::gui;
using namespace esp_brookesia::systems::phone;

static bool init_devices();
static bool init_lvgl_port();

static dev_display_lcd_handles_t *lcd_handles = nullptr;
static dev_display_lcd_config_t *lcd_cfg = nullptr;
static dev_lcd_touch_i2c_handles_t *touch_handles = nullptr;

static lv_disp_t *lvgl_disp = nullptr;

static bool set_lcd_backlight(int brightness_percent)
{
    ESP_UTILS_LOG_TRACE_GUARD();

#ifdef CONFIG_ESP_BOARD_DEV_LEDC_CTRL_SUPPORT

    periph_ledc_config_t *ledc_config = nullptr;
    periph_ledc_handle_t *ledc_handle = nullptr;
    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_board_manager_get_device_handle("lcd_brightness", (void **)&ledc_handle), false,
        "Get LEDC control device handle failed"
    );
    dev_ledc_ctrl_config_t *dev_ledc_cfg = nullptr;
    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_board_manager_get_device_config("lcd_brightness", (void **)&dev_ledc_cfg), false,
        "Get LEDC control device config failed"
    );
    ESP_UTILS_CHECK_NULL_RETURN(dev_ledc_cfg, false, "LEDC control device config is NULL");
    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_board_manager_get_periph_config(dev_ledc_cfg->ledc_name, (void **)&ledc_config), false,
        "Get LEDC peripheral config failed"
    );

    brightness_percent = std::clamp(brightness_percent, BRIGHTNESS_MIN, BRIGHTNESS_MAX);

    uint32_t duty = (brightness_percent * ((1 << (uint32_t)ledc_config->duty_resolution) - 1)) / 100;
    ESP_UTILS_CHECK_ERROR_RETURN(
        ledc_set_duty(ledc_handle->speed_mode, ledc_handle->channel, duty), false, "Set LEDC duty failed"
    );
    ESP_UTILS_CHECK_ERROR_RETURN(
        ledc_update_duty(ledc_handle->speed_mode, ledc_handle->channel), false, "Update LEDC duty failed"
    );

    return true;
#else
    return true;
#endif
}

bool display_init()
{
    ESP_UTILS_LOG_TRACE_GUARD();

    ESP_UTILS_CHECK_FALSE_RETURN(init_devices(), false, "Initialize devices failed");
    ESP_UTILS_CHECK_FALSE_RETURN(init_lvgl_port(), false, "Initialize LVGL port failed");

    return true;
}

bool display_get_info(display_info_t *info)
{
    ESP_UTILS_LOG_TRACE_GUARD();

    ESP_UTILS_CHECK_NULL_RETURN(info, false, "Invalid info");

    info->width = lcd_cfg->lcd_width;
    info->height = lcd_cfg->lcd_height;

    return true;
}

static bool init_devices()
{
    ESP_UTILS_LOG_TRACE_GUARD();

    void *dev_lcd_handle = nullptr;
    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_board_manager_get_device_handle("display_lcd", &dev_lcd_handle), false, "Get LCD device handle failed"
    );
    ESP_UTILS_CHECK_NULL_RETURN(dev_lcd_handle, false, "LCD device handle is NULL");
    lcd_handles = (dev_display_lcd_handles_t *)dev_lcd_handle;
    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_board_manager_get_device_config("display_lcd", (void **)&lcd_cfg), false, "Get LCD device config failed"
    );

    void *dev_touch_handle = nullptr;
    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_board_manager_get_device_handle("lcd_touch", &dev_touch_handle), false, "Get touch device handle failed"
    );
    ESP_UTILS_CHECK_NULL_RETURN(dev_touch_handle, false, "Touch device handle is NULL");
    touch_handles = (dev_lcd_touch_i2c_handles_t *)dev_touch_handle;

    return true;
}

static bool init_lvgl_port()
{
    ESP_UTILS_LOG_TRACE_GUARD();

    lvgl_port_cfg_t lvgl_port_cfg = {
        .task_priority = LVGL_TASK_PRIORITY,
        .task_stack = LVGL_TASK_STACK_SIZE,
        .task_affinity = LVGL_TASK_CORE_ID,
        .task_max_sleep_ms = LVGL_TASK_MAX_SLEEP_MS,
        .task_stack_caps = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT,
        .timer_period_ms = LVGL_TASK_TIMER_PERIOD_MS,
    };
    ESP_UTILS_CHECK_ERROR_RETURN(lvgl_port_init(&lvgl_port_cfg), false, "Initialize LVGL port failed");

    lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_handles->io_handle,
        .panel_handle = lcd_handles->panel_handle,
        .buffer_size = static_cast<uint32_t>(lcd_cfg->lcd_width * 50),
        .double_buffer = true,
        .hres = lcd_cfg->lcd_width,
        .vres = lcd_cfg->lcd_height,
        .monochrome = false,
        /* Rotation values must be same as used in esp_lcd for initial settings of the screen */
        .rotation = {
            .swap_xy = static_cast<bool>(lcd_cfg->swap_xy),
            .mirror_x = static_cast<bool>(lcd_cfg->mirror_x),
            .mirror_y = static_cast<bool>(lcd_cfg->mirror_y),
        },
        .flags = {
            .buff_dma = true,
            .buff_spiram = false,
            .swap_bytes = true,
            // .default_dummy_draw = 0, // Avoid white screen
        },
    };

    if (strcmp(lcd_cfg->sub_type, "spi") == 0) {
        lvgl_disp = lvgl_port_add_disp(&disp_cfg);
    } else {
        lvgl_port_display_dsi_cfg_t dsi_cfg = {
            .flags = {.avoid_tearing = false},
        };
        lvgl_disp = lvgl_port_add_disp_dsi(&disp_cfg, &dsi_cfg);
    }
    ESP_UTILS_CHECK_NULL_RETURN(lvgl_disp, false, "Add display to LVGL port failed");

    set_lcd_backlight(BRIGHTNESS_DEFAULT);
    lvgl_port_touch_cfg_t touch_cfg = {
        .disp = lvgl_disp,
        .handle = touch_handles->touch_handle,
    };
    ESP_UTILS_CHECK_NULL_RETURN(lvgl_port_add_touch(&touch_cfg), false, "Add touch to LVGL port failed");

    /* Configure LVGL lock and unlock */
    LvLock::registerCallbacks([](int timeout_ms) {
        if (timeout_ms < 0) {
            timeout_ms = 0;
        } else if (timeout_ms == 0) {
            timeout_ms = 1;
        }
        ESP_UTILS_CHECK_FALSE_RETURN(lvgl_port_lock(timeout_ms), false, "Lock failed");

        return true;
    }, []() {
        lvgl_port_unlock();

        return true;
    });

    return true;
}
