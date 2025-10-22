/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "keyboard_button.h"
#include "led_strip.h"
#include "esp_adc/adc_oneshot.h"

#define ADC_SWITCH_THRESHOLD        2000
#define ADC_CHARGER_FULL_THRESHOLD  2200
#define ADC_CHARGING_THRESHOLD      1650
#define ADC_CHARGER_NO_THRESHOLD    3300
#define ADC_SWITCH_THRESHOLD        2000
#define ADC_BATTERY_VOLTAGE_RATIO   1.51f
#define ADC_BATTERY_VOLTAGE_MIN     3.0f
#define ADC_BATTERY_VOLTAGE_MAX     4.2f

/*!< Keyboard active level */
#define KBD_ATTIVE_LEVEL      0
#define KBD_TICKS_INTERVAL_US 1000

/*!< Keyboard matrix GPIO */
#define KBD_COL_NUM  2

#define KBD_COL_IO_0 17
#define KBD_COL_IO_1 0

#define KBD_INPUT_IOS  {KBD_COL_IO_0, KBD_COL_IO_1}

/*!< WS2812 power control GPIO */
#define LIGHTMAP_NUM             2
#define KBD_WS2812_POWER_IO    (40)
#define KBD_WS2812_GPIO        (21)

#define UART1_TX_IO             (47)
#define UART1_RX_IO             (48)
#define UART2_TX_IO             (6)
#define UART2_RX_IO             (5)

/*!< Mode switch monitor GPIO */
#define KBD_ADC_SWITCH_BLE_IO           (8)
#define KBD_ADC_SWITCH_BLE_CHAN         (ADC_CHANNEL_7)
#define KBD_ADC_SWITCH_RAINMAKER_IO     (7)
#define KBD_ADC_SWITCH_RAINMAKER_CHAN   (ADC_CHANNEL_6)

/*!< Vbus monitor GPIO */
#define KBD_ADC_VBUS_MONITOR_IO      (2)
#define KBD_ADC_VBUS_MONITOR_CHAN    (ADC_CHANNEL_1)

/*!< Battery monitor GPIO */
#define KBD_ADC_BATTERY_MONITOR_IO      (10)
#define KBD_ADC_BATTERY_MONITOR_CHAN    (ADC_CHANNEL_9)

/*  NO Charge: 3.3v.
 *  Charging: 1.65v.
 *  Full: 2.2v.
 */
#define KBD_ADC_CHARGE_MONITOR_IO       (9)
#define KBD_ADC_CHARGE_MONITOR_CHAN     (ADC_CHANNEL_8)

// Convert battery voltage to percentage (0% at ADC_BATTERY_VOLTAGE_MIN, 100% at ADC_BATTERY_VOLTAGE_MAX)
#define BATTERY_VOLTAGE_TO_PERCENTAGE(battery_voltage) \
    (uint8_t)((((battery_voltage) - ADC_BATTERY_VOLTAGE_MIN) / (ADC_BATTERY_VOLTAGE_MAX - ADC_BATTERY_VOLTAGE_MIN)) * 100.0f)

typedef enum {
    BSP_SWITCH_STATE_MIDDLE = 0,
    BSP_SWITCH_STATE_LEFT,
    BSP_SWITCH_STATE_RIGHT,
} bsp_switch_state_t;

/**
 * @brief Callback type for charge/discharge state changes
 *
 * @param is_charging true if charging, false if discharging
 */
typedef void (*battery_monitor_cb_t)(bool is_charging, float voltage);

/**
 * @brief Battery monitor configuration
 *
 * @param monitor_interval_ms Monitor interval in milliseconds 0 for disable callback
 * @param monitor_cb Callback function for battery state changes
 */
typedef struct {
    uint32_t monitor_interval_ms;
    battery_monitor_cb_t monitor_cb;
} battery_monitor_cfg_t;

/**
 * @brief Callback type for vbus state changes
 *
 * @param is_charging true if charging, false if discharging
 */
typedef void (*vbus_monitor_cb_t)(bool is_charging);

/**
 * @brief Vbus monitor configuration
 *
 * @param monitor_interval_ms Monitor interval in milliseconds 0 for disable callback
 * @param monitor_cb Callback function for vbus state changes
 */
typedef struct {
    uint32_t monitor_interval_ms;
    vbus_monitor_cb_t monitor_cb;
} vbus_monitor_cfg_t;

/**
 * @brief Callback type for switch state changes
 *
 * @param state Switch state
 */
typedef void (*switch_monitor_cb_t)(bsp_switch_state_t state);
/**
 * @brief Switch monitor configuration
 *
 * @param monitor_interval_ms Monitor interval in milliseconds 0 for disable callback
 * @param monitor_cb Callback function for switch state changes
 */
typedef struct {
    uint32_t monitor_interval_ms;
    switch_monitor_cb_t monitor_cb;
} switch_monitor_cfg_t;

typedef struct {
    int32_t sleep_time_ms;
    int32_t wakeup_pin_1;
    int32_t wakeup_pin_2;
} bsp_deep_sleep_config_t;

esp_err_t bsp_keyboard_init(keyboard_btn_handle_t *kbd_handle, keyboard_btn_config_t *ex_cfg);

esp_err_t bsp_ws2812_init(led_strip_handle_t *led_strip);

esp_err_t bsp_ws2812_enable(bool enable);

esp_err_t bsp_ws2812_clear(void);

bool bsp_ws2812_is_enable(void);

esp_err_t bsp_lamp_array_init(uint32_t bind);

esp_err_t bsp_rgb_matrix_init(void);

esp_err_t bsp_get_adc_handle(adc_oneshot_unit_handle_t *handle);

esp_err_t bsp_adc_switch_monitor_init(switch_monitor_cfg_t *cfg);

esp_err_t bsp_adc_battery_monitor_init(battery_monitor_cfg_t *cfg);

esp_err_t bsp_get_battery_state(bool *is_charging, float *voltage);

esp_err_t bsp_get_switch_state(bsp_switch_state_t *state);

esp_err_t bsp_get_vbus_state(bool *is_charging);

esp_err_t bsp_adc_vbus_monitor_init(vbus_monitor_cfg_t *cfg);

esp_err_t bsp_output_io_init(void);

esp_err_t bsp_output_io_set(uint8_t io, bool value);

void bsp_pm_lock_init(void);

void bsp_pm_lock_release(void);

void bsp_pm_lock_acquire(void);

esp_err_t bsp_light_sleep_enable(bool enable);

esp_err_t bsp_deep_sleep_init(bsp_deep_sleep_config_t *config);

esp_err_t bsp_deep_sleep_enable(bool enable);

esp_err_t bsp_deep_sleep_reset(void);

#ifdef __cplusplus
}
#endif
