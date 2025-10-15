/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LIGHT_MANAGER_H
#define LIGHT_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "led_strip.h"
#include "rgb_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

// Light effect configuration structure
typedef struct {
    uint8_t mode;           // Current light effect mode
    uint8_t brightness;     // Brightness level (0-100)
    uint8_t hue;           // Hue value (0-255)
    uint8_t saturation;    // Saturation value (0-255)
    uint8_t speed;         // Animation speed (0-100)
    bool enabled;          // Light effect enabled/disabled
    bool suspended;        // Light effect suspended state
} light_config_t;

// Light effect event types
typedef enum {
    LIGHT_EVENT_MODE_CHANGED,
    LIGHT_EVENT_BRIGHTNESS_CHANGED,
    LIGHT_EVENT_HUE_CHANGED,
    LIGHT_EVENT_SATURATION_CHANGED,
    LIGHT_EVENT_SPEED_CHANGED,
    LIGHT_EVENT_ENABLED_CHANGED,
    LIGHT_EVENT_SUSPENDED_CHANGED,
} light_event_type_t;

// Light effect event structure
typedef struct {
    light_event_type_t type;
    union {
        uint8_t mode;
        uint8_t brightness;
        uint8_t hue;
        uint8_t saturation;
        uint8_t speed;
        bool enabled;
        bool suspended;
    } data;
} light_event_t;

// Light manager context structure
typedef struct light_manager_ctx_t *light_manager_handle_t;

// Light effect event callback
typedef void (*light_event_callback_t)(light_manager_handle_t ctx, const light_event_t *event, void *user_data);

// Light manager configuration
typedef struct {
    uint8_t default_mode;
    uint8_t default_brightness;
    uint8_t default_hue;
    uint8_t default_saturation;
    uint8_t default_speed;
    bool default_enabled;
    uint32_t task_stack_size;
    uint8_t task_priority;
    light_event_callback_t event_callback;
    void *event_user_data;
} light_manager_config_t;

// Default configuration
#define LIGHT_MANAGER_DEFAULT_CONFIG() { \
    .default_mode = 5, \
    .default_brightness = 100, \
    .default_hue = 0, \
    .default_saturation = 100, \
    .default_speed = 100, \
    .default_enabled = true, \
    .task_stack_size = 4096, \
    .task_priority = 5, \
    .event_callback = NULL, \
    .event_user_data = NULL, \
}

// Function declarations

/**
 * @brief Initialize light manager
 * @param config Configuration for light manager
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t light_manager_init(const light_manager_config_t *config);

/**
 * @brief Deinitialize light manager
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t light_manager_deinit(void);

/**
 * @brief Get light manager context
 * @return Pointer to light manager context, NULL if not initialized
 */
light_manager_handle_t light_manager_get_handle(void);

/**
 * @brief Set light effect mode
 * @param mode Light effect mode (0 to RGB_MATRIX_EFFECT_MAX-1)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t light_manager_set_mode(uint8_t mode);

/**
 * @brief Get current light effect mode
 * @return Current light effect mode
 */
uint8_t light_manager_get_mode(void);

/**
 * @brief Set brightness level
 * @param brightness Brightness level (0-100)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t light_manager_set_brightness(uint8_t brightness);

/**
 * @brief Get current brightness level
 * @return Current brightness level (0-100)
 */
uint8_t light_manager_get_brightness(void);

/**
 * @brief Set hue value
 * @param hue Hue value (0-255)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t light_manager_set_hue(uint8_t hue);

/**
 * @brief Get current hue value
 * @return Current hue value (0-255)
 */
uint8_t light_manager_get_hue(void);

/**
 * @brief Set saturation value
 * @param saturation Saturation value (0-255)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t light_manager_set_saturation(uint8_t saturation);

/**
 * @brief Get current saturation value
 * @return Current saturation value (0-255)
 */
uint8_t light_manager_get_saturation(void);

/**
 * @brief Set animation speed
 * @param speed Animation speed (0-100)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t light_manager_set_speed(uint8_t speed);

/**
 * @brief Get current animation speed
 * @return Current animation speed (0-100)
 */
uint8_t light_manager_get_speed(void);

/**
 * @brief Enable or disable light effects
 * @param enabled True to enable, false to disable
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t light_manager_set_enabled(bool enabled);

/**
 * @brief Check if light effects are enabled
 * @return True if enabled, false otherwise
 */
bool light_manager_is_enabled(void);

/**
 * @brief Set suspended state
 * @param suspended True to suspend, false to resume
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t light_manager_set_suspended(bool suspended);

/**
 * @brief Check if light effects are suspended
 * @return True if suspended, false otherwise
 */
bool light_manager_is_suspended(void);

/**
 * @brief Set HSV color values
 * @param h Hue value (0-255)
 * @param s Saturation value (0-255)
 * @param v Value/brightness (0-255)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t light_manager_set_hsv(uint8_t h, uint8_t s, uint8_t v);

/**
 * @brief Get current light configuration
 * @param config Pointer to store current configuration
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t light_manager_get_config(light_config_t *config);

/**
 * @brief Set light configuration
 * @param config New configuration
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t light_manager_set_config(const light_config_t *config);

/**
 * @brief Reset light configuration to defaults
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t light_manager_reset_config(void);

/**
 * @brief Resume light task if suspended
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t light_manager_resume_task(void);

/**
 * @brief Suspend light task
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t light_manager_suspend_task(void);

/**
 * @brief Process RGB matrix for key press effects
 * @param output_index Output index
 * @param input_index Input index
 * @param pressed True if key pressed, false if released
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t light_manager_process_key_event(uint8_t output_index, uint8_t input_index, bool pressed);

/**
 * @brief Clear all LEDs
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t light_manager_clear_leds(void);

/**
 * @brief Get event type name as string
 * @param event_type Event type
 * @return String representation of event type
 */
const char *light_manager_get_event_type_name(light_event_type_t event_type);

#ifdef __cplusplus
}
#endif

#endif // LIGHT_MANAGER_H
