/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "light_manager.h"
#include "bsp/esp-bsp.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <string.h>
#include "rgb_matrix.h"

static const char *TAG = "light_manager";

typedef struct light_manager_ctx_t {
    led_strip_handle_t led_strip;
    light_config_t config;
    TaskHandle_t light_task_handle;
    light_event_callback_t event_callback;
    void *event_user_data;
    bool initialized;
} light_manager_ctx_t;

static light_manager_ctx_t g_light_manager_ctx = {0};

static void light_task(void *pvParameters);
static void notify_event(light_event_type_t type, void *data);
static esp_err_t apply_hsv_changes(void);

esp_err_t light_manager_init(const light_manager_config_t *config)
{
    if (g_light_manager_ctx.initialized) {
        ESP_LOGW(TAG, "Light manager already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (!config) {
        ESP_LOGE(TAG, "Invalid configuration");
        return ESP_ERR_INVALID_ARG;
    }

    // Initialize context with default values
    memset(&g_light_manager_ctx, 0, sizeof(light_manager_ctx_t));

    // Set configuration
    g_light_manager_ctx.config.mode = config->default_mode;
    g_light_manager_ctx.config.brightness = config->default_brightness;
    g_light_manager_ctx.config.hue = config->default_hue;
    g_light_manager_ctx.config.saturation = config->default_saturation;
    g_light_manager_ctx.config.speed = config->default_speed;
    g_light_manager_ctx.config.enabled = config->default_enabled;
    g_light_manager_ctx.config.suspended = false;

    // Set event callback
    g_light_manager_ctx.event_callback = config->event_callback;
    g_light_manager_ctx.event_user_data = config->event_user_data;

    // Initialize BSP components
    esp_err_t ret = bsp_rgb_matrix_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize RGB matrix: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize WS2812
    ret = bsp_ws2812_init(&g_light_manager_ctx.led_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize WS2812: %s", esp_err_to_name(ret));
        return ret;
    }

    if (g_light_manager_ctx.led_strip) {
        led_strip_clear(g_light_manager_ctx.led_strip);
    }

    // Enable WS2812
    bsp_ws2812_enable(g_light_manager_ctx.config.enabled);

    // Set initial RGB matrix mode
    rgb_matrix_mode(g_light_manager_ctx.config.mode + 1);

    // Create light task
    ret = xTaskCreate(light_task, "light_task", config->task_stack_size,
                      &g_light_manager_ctx, config->task_priority,
                      &g_light_manager_ctx.light_task_handle);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create light task");
        return ESP_ERR_NO_MEM;
    }

    g_light_manager_ctx.initialized = true;
    ESP_LOGI(TAG, "Light manager initialized successfully");

    // Set initial HSV values
    ret = apply_hsv_changes();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to apply initial HSV values: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

esp_err_t light_manager_deinit(void)
{
    if (!g_light_manager_ctx.initialized) {
        ESP_LOGW(TAG, "Light manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Delete light task
    if (g_light_manager_ctx.light_task_handle) {
        vTaskDelete(g_light_manager_ctx.light_task_handle);
        g_light_manager_ctx.light_task_handle = NULL;
    }

    // Clear LEDs
    if (g_light_manager_ctx.led_strip) {
        led_strip_clear(g_light_manager_ctx.led_strip);
    }

    // Disable WS2812
    bsp_ws2812_enable(false);

    // Reset context
    memset(&g_light_manager_ctx, 0, sizeof(light_manager_ctx_t));

    ESP_LOGI(TAG, "Light manager deinitialized");
    return ESP_OK;
}

light_manager_handle_t light_manager_get_handle(void)
{
    return g_light_manager_ctx.initialized ? (light_manager_handle_t)&g_light_manager_ctx : NULL;
}

esp_err_t light_manager_set_mode(uint8_t mode)
{
    if (!g_light_manager_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (mode >= RGB_MATRIX_EFFECT_MAX) {
        ESP_LOGE(TAG, "Invalid mode: %u", mode);
        return ESP_ERR_INVALID_ARG;
    }

    if (g_light_manager_ctx.config.mode != mode) {
        g_light_manager_ctx.config.mode = mode;
        rgb_matrix_mode(mode + 1);
        notify_event(LIGHT_EVENT_MODE_CHANGED, &mode);
        ESP_LOGI(TAG, "Light mode changed to: %u", mode);
    }

    return ESP_OK;
}

uint8_t light_manager_get_mode(void)
{
    return g_light_manager_ctx.initialized ? g_light_manager_ctx.config.mode : 0;
}

esp_err_t light_manager_set_brightness(uint8_t brightness)
{
    if (!g_light_manager_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (brightness > 100) {
        ESP_LOGE(TAG, "Invalid brightness: %u", brightness);
        return ESP_ERR_INVALID_ARG;
    }

    if (g_light_manager_ctx.config.brightness != brightness) {
        g_light_manager_ctx.config.brightness = brightness;
        esp_err_t ret = apply_hsv_changes();
        if (ret == ESP_OK) {
            notify_event(LIGHT_EVENT_BRIGHTNESS_CHANGED, &brightness);
            ESP_LOGI(TAG, "Brightness changed to: %u", brightness);
        }
        return ret;
    }

    return ESP_OK;
}

uint8_t light_manager_get_brightness(void)
{
    return g_light_manager_ctx.initialized ? g_light_manager_ctx.config.brightness : 0;
}

esp_err_t light_manager_set_hue(uint8_t hue)
{
    if (!g_light_manager_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (g_light_manager_ctx.config.hue != hue) {
        g_light_manager_ctx.config.hue = hue;
        esp_err_t ret = apply_hsv_changes();
        if (ret == ESP_OK) {
            notify_event(LIGHT_EVENT_HUE_CHANGED, &hue);
            ESP_LOGI(TAG, "Hue changed to: %u", hue);
        }
        return ret;
    }

    return ESP_OK;
}

uint8_t light_manager_get_hue(void)
{
    return g_light_manager_ctx.initialized ? g_light_manager_ctx.config.hue : 0;
}

esp_err_t light_manager_set_saturation(uint8_t saturation)
{
    if (!g_light_manager_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (saturation > 100) {
        ESP_LOGE(TAG, "Invalid saturation: %u", saturation);
        return ESP_ERR_INVALID_ARG;
    }

    if (g_light_manager_ctx.config.saturation != saturation) {
        g_light_manager_ctx.config.saturation = saturation;
        esp_err_t ret = apply_hsv_changes();
        if (ret == ESP_OK) {
            notify_event(LIGHT_EVENT_SATURATION_CHANGED, &saturation);
            ESP_LOGI(TAG, "Saturation changed to: %u", saturation);
        }
        return ret;
    }

    return ESP_OK;
}

uint8_t light_manager_get_saturation(void)
{
    return g_light_manager_ctx.initialized ? g_light_manager_ctx.config.saturation : 0;
}

esp_err_t light_manager_set_speed(uint8_t speed)
{
    if (!g_light_manager_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (speed > 100) {
        ESP_LOGE(TAG, "Invalid speed: %u", speed);
        return ESP_ERR_INVALID_ARG;
    }

    if (g_light_manager_ctx.config.speed != speed) {
        g_light_manager_ctx.config.speed = speed;
        rgb_matrix_set_speed(speed);
        notify_event(LIGHT_EVENT_SPEED_CHANGED, &speed);
        ESP_LOGI(TAG, "Speed changed to: %u", speed);
    }

    return ESP_OK;
}

uint8_t light_manager_get_speed(void)
{
    return g_light_manager_ctx.initialized ? g_light_manager_ctx.config.speed : 0;
}

esp_err_t light_manager_set_enabled(bool enabled)
{
    if (!g_light_manager_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (g_light_manager_ctx.config.enabled != enabled) {
        g_light_manager_ctx.config.enabled = enabled;
        bsp_ws2812_enable(enabled);
        notify_event(LIGHT_EVENT_ENABLED_CHANGED, &enabled);
        ESP_LOGI(TAG, "Light effects %s", enabled ? "enabled" : "disabled");
    }

    return ESP_OK;
}

bool light_manager_is_enabled(void)
{
    return g_light_manager_ctx.initialized ? g_light_manager_ctx.config.enabled : false;
}

esp_err_t light_manager_set_suspended(bool suspended)
{
    if (!g_light_manager_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (g_light_manager_ctx.config.suspended != suspended) {
        g_light_manager_ctx.config.suspended = suspended;
        rgb_matrix_set_suspend_state(suspended);
        notify_event(LIGHT_EVENT_SUSPENDED_CHANGED, &suspended);
        ESP_LOGI(TAG, "Light effects %s", suspended ? "suspended" : "resumed");
    }

    return ESP_OK;
}

bool light_manager_is_suspended(void)
{
    return g_light_manager_ctx.initialized ? g_light_manager_ctx.config.suspended : false;
}

esp_err_t light_manager_set_hsv(uint8_t h, uint8_t s, uint8_t v)
{
    if (!g_light_manager_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    g_light_manager_ctx.config.hue = h;
    g_light_manager_ctx.config.saturation = (s * 100) / 255;
    g_light_manager_ctx.config.brightness = (v * 100) / 255;

    rgb_matrix_sethsv(h, s, v);

    ESP_LOGI(TAG, "HSV set to: h=%u, s=%u, v=%u", h, s, v);
    return ESP_OK;
}

esp_err_t light_manager_get_config(light_config_t *config)
{
    if (!g_light_manager_ctx.initialized || !config) {
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(config, &g_light_manager_ctx.config, sizeof(light_config_t));
    return ESP_OK;
}

esp_err_t light_manager_set_config(const light_config_t *config)
{
    if (!g_light_manager_ctx.initialized || !config) {
        return ESP_ERR_INVALID_ARG;
    }

    // Validate configuration
    if (config->mode >= RGB_MATRIX_EFFECT_MAX ||
            config->brightness > 100 ||
            config->saturation > 100 ||
            config->speed > 100) {
        ESP_LOGE(TAG, "Invalid configuration values");
        return ESP_ERR_INVALID_ARG;
    }

    // Apply configuration
    esp_err_t ret = light_manager_set_mode(config->mode);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = light_manager_set_brightness(config->brightness);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = light_manager_set_hue(config->hue);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = light_manager_set_saturation(config->saturation);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = light_manager_set_speed(config->speed);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = light_manager_set_enabled(config->enabled);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = light_manager_set_suspended(config->suspended);
    if (ret != ESP_OK) {
        return ret;
    }

    ESP_LOGI(TAG, "Light configuration applied successfully");
    return ESP_OK;
}

esp_err_t light_manager_reset_config(void)
{
    if (!g_light_manager_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    light_manager_config_t default_config = LIGHT_MANAGER_DEFAULT_CONFIG();

    g_light_manager_ctx.config.mode = default_config.default_mode;
    g_light_manager_ctx.config.brightness = default_config.default_brightness;
    g_light_manager_ctx.config.hue = default_config.default_hue;
    g_light_manager_ctx.config.saturation = default_config.default_saturation;
    g_light_manager_ctx.config.speed = default_config.default_speed;
    g_light_manager_ctx.config.enabled = default_config.default_enabled;
    g_light_manager_ctx.config.suspended = false;

    // Apply default configuration
    rgb_matrix_mode(g_light_manager_ctx.config.mode + 1);
    rgb_matrix_set_speed(g_light_manager_ctx.config.speed);
    bsp_ws2812_enable(g_light_manager_ctx.config.enabled);

    esp_err_t ret = apply_hsv_changes();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Light configuration reset to defaults");
    }

    return ret;
}

esp_err_t light_manager_resume_task(void)
{
    if (!g_light_manager_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (g_light_manager_ctx.light_task_handle &&
            eTaskGetState(g_light_manager_ctx.light_task_handle) == eSuspended) {
        vTaskResume(g_light_manager_ctx.light_task_handle);
        ESP_LOGI(TAG, "Light task resumed");
    }

    return ESP_OK;
}

esp_err_t light_manager_suspend_task(void)
{
    if (!g_light_manager_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (g_light_manager_ctx.light_task_handle &&
            eTaskGetState(g_light_manager_ctx.light_task_handle) != eSuspended) {
        vTaskSuspend(g_light_manager_ctx.light_task_handle);
        ESP_LOGI(TAG, "Light task suspended");
    }

    return ESP_OK;
}

esp_err_t light_manager_process_key_event(uint8_t output_index, uint8_t input_index, bool pressed)
{
    if (!g_light_manager_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (g_light_manager_ctx.config.enabled && !g_light_manager_ctx.config.suspended) {
        process_rgb_matrix(output_index, input_index, pressed);
    }

    return ESP_OK;
}

esp_err_t light_manager_clear_leds(void)
{
    if (!g_light_manager_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (g_light_manager_ctx.led_strip) {
        led_strip_clear(g_light_manager_ctx.led_strip);
        led_strip_refresh(g_light_manager_ctx.led_strip);
    } else {
        bsp_ws2812_clear();
    }

    return ESP_OK;
}

static void light_task(void *pvParameters)
{
    light_manager_ctx_t *ctx = (light_manager_ctx_t *)pvParameters;

    ESP_LOGI(TAG, "Light task started");

    while (1) {
        if (bsp_ws2812_is_enable() && !ctx->config.suspended) {
            rgb_matrix_task();
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

static esp_err_t apply_hsv_changes(void)
{
    if (!g_light_manager_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    // Convert HSL to HSV and apply
    uint8_t h = g_light_manager_ctx.config.hue;
    uint8_t s = (g_light_manager_ctx.config.saturation * 255) / 100;
    uint8_t v = (g_light_manager_ctx.config.brightness * 255) / 100;

    rgb_matrix_sethsv(h, s, v);

    return ESP_OK;
}

static void notify_event(light_event_type_t type, void *data)
{
    if (!g_light_manager_ctx.event_callback) {
        return;
    }

    if (!data) {
        ESP_LOGW(TAG, "Event data is NULL for event type: %s", light_manager_get_event_type_name(type));
        return;
    }

    light_event_t event = {
        .type = type,
    };

    // Safely extract data based on event type
    switch (type) {
    case LIGHT_EVENT_MODE_CHANGED:
        event.data.mode = *(uint8_t *)data;
        break;
    case LIGHT_EVENT_BRIGHTNESS_CHANGED:
        event.data.brightness = *(uint8_t *)data;
        break;
    case LIGHT_EVENT_HUE_CHANGED:
        event.data.hue = *(uint8_t *)data;
        break;
    case LIGHT_EVENT_SATURATION_CHANGED:
        event.data.saturation = *(uint8_t *)data;
        break;
    case LIGHT_EVENT_SPEED_CHANGED:
        event.data.speed = *(uint8_t *)data;
        break;
    case LIGHT_EVENT_ENABLED_CHANGED:
        event.data.enabled = *(bool *)data;
        break;
    case LIGHT_EVENT_SUSPENDED_CHANGED:
        event.data.suspended = *(bool *)data;
        break;
    default:
        ESP_LOGW(TAG, "Unknown event type: %d", type);
        return;
    }

    // Log event notification for debugging
    ESP_LOGD(TAG, "Notifying event: %s", light_manager_get_event_type_name(type));

    // Call the event callback with proper parameters
    g_light_manager_ctx.event_callback((light_manager_handle_t)&g_light_manager_ctx, &event, g_light_manager_ctx.event_user_data);
}

const char *light_manager_get_event_type_name(light_event_type_t event_type)
{
    switch (event_type) {
    case LIGHT_EVENT_MODE_CHANGED:
        return "MODE_CHANGED";
    case LIGHT_EVENT_BRIGHTNESS_CHANGED:
        return "BRIGHTNESS_CHANGED";
    case LIGHT_EVENT_HUE_CHANGED:
        return "HUE_CHANGED";
    case LIGHT_EVENT_SATURATION_CHANGED:
        return "SATURATION_CHANGED";
    case LIGHT_EVENT_SPEED_CHANGED:
        return "SPEED_CHANGED";
    case LIGHT_EVENT_ENABLED_CHANGED:
        return "ENABLED_CHANGED";
    case LIGHT_EVENT_SUSPENDED_CHANGED:
        return "SUSPENDED_CHANGED";
    default:
        return "UNKNOWN";
    }
}
