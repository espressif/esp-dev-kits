/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include "esp_log.h"
#include "esp_check.h"
#include "keyboard_button.h"
#include "led_strip.h"
#include "rgb_matrix_drivers.h"
#include "rgb_matrix.h"
#include "esp_adc/adc_oneshot.h"
#include "bsp/esp_dualkey.h"
#include "esp_pm.h"
#include "hal/sar_ctrl_ll.h"
#include "esp_sleep.h"

static const char *TAG = "esp_dualkey";

typedef struct {
    int32_t sleep_time_ms;
    int32_t wakeup_pin_1;
    int32_t wakeup_pin_2;
    TimerHandle_t timer_handle;
} bsp_deep_sleep_t;

static led_strip_handle_t s_led_strip = NULL;
static bool s_led_enable = false;
static TimerHandle_t s_battery_monitor_timer = NULL;
static TimerHandle_t s_vbus_monitor_timer = NULL;
static battery_monitor_cb_t s_battery_monitor_cb = NULL;
static vbus_monitor_cb_t s_vbus_monitor_cb = NULL;
static TimerHandle_t s_switch_monitor_timer = NULL;
static switch_monitor_cb_t s_switch_monitor_cb = NULL;
static bool s_is_charging = false;
static float s_battery_voltage = 0;
static bool s_is_vbus_conn = false;
static bsp_switch_state_t s_switch_state = BSP_SWITCH_STATE_MIDDLE;

static bsp_deep_sleep_t *bsp_deep_sleep = NULL;

static esp_pm_lock_handle_t pm_lock;

esp_err_t bsp_keyboard_init(keyboard_btn_handle_t *kbd_handle, keyboard_btn_config_t *ex_cfg)
{
    ESP_RETURN_ON_FALSE(kbd_handle != NULL, ESP_ERR_INVALID_ARG, TAG, "kbd_handle is NULL");

    if (ex_cfg != NULL) {
        return keyboard_button_create(ex_cfg, kbd_handle);
    } else {
        keyboard_btn_config_t cfg = {
            .output_gpios = NULL,
            .output_gpio_num = 0,
            .input_gpios = (int[])KBD_INPUT_IOS,
            .input_gpio_num = KBD_COL_NUM,
            .active_level = KBD_ATTIVE_LEVEL,
            .debounce_ticks = 3,
            .ticks_interval = KBD_TICKS_INTERVAL_US,
            .enable_power_save = true,
        };
        return keyboard_button_create(&cfg, kbd_handle);
    }
}

esp_err_t bsp_ws2812_init(led_strip_handle_t *led_strip)
{
    if (s_led_strip) {
        if (led_strip) {
            *led_strip = s_led_strip;
        }
        return ESP_OK;
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << KBD_WS2812_POWER_IO,
                             .mode = GPIO_MODE_OUTPUT_OD,
                             .pull_down_en = 0,
                             .pull_up_en = 0,
    };
    gpio_config(&io_conf);

    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = KBD_WS2812_GPIO, // The GPIO that connected to the LED strip's data line
        .max_leds = LIGHTMAP_NUM, // The number of LEDs in the strip,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
        .led_model = LED_MODEL_WS2812, // LED strip model
        .flags.invert_out = false, // whether to invert the output signal (useful when your hardware has a level inverter)
    };

    // LED strip backend configuration: SPI
    led_strip_spi_config_t spi_config = {
        .clk_src = SPI_CLK_SRC_XTAL, // different clock source can lead to different power consumption
        .flags.with_dma = true,         // Using DMA can improve performance and help drive more LEDs
        .spi_bus = SPI2_HOST,           // SPI bus ID
    };

    // LED Strip object handle
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &s_led_strip));

    if (led_strip) {
        *led_strip = s_led_strip;
    }
    return ESP_OK;
}

esp_err_t bsp_ws2812_enable(bool enable)
{
    if (!enable) {
        gpio_hold_dis(KBD_WS2812_POWER_IO);
    }
    gpio_set_level(KBD_WS2812_POWER_IO, !enable);
    /*!< Make output stable in light sleep */
    if (enable) {
        gpio_hold_en(KBD_WS2812_POWER_IO);
    }
    s_led_enable = enable;
    return ESP_OK;
}

esp_err_t bsp_ws2812_clear(void)
{
    return led_strip_clear(s_led_strip);
}

bool bsp_ws2812_is_enable(void)
{
    return s_led_enable;
}

esp_err_t bsp_rgb_matrix_init(void)
{
    if (!s_led_strip) {
        bsp_ws2812_init(NULL);
    }
    rgb_matrix_driver_init(s_led_strip, LIGHTMAP_NUM);
    rgb_matrix_init();
    return ESP_OK;
}

static adc_oneshot_unit_handle_t adc1_handle;

esp_err_t bsp_get_adc_handle(adc_oneshot_unit_handle_t *handle)
{
    if (handle) {
        *handle = adc1_handle;
        return ESP_OK;
    }
    return ESP_FAIL;
}

static void switch_monitor_timer_callback(TimerHandle_t xTimer)
{
    bsp_get_switch_state(&s_switch_state);
    if (s_switch_monitor_cb) {
        s_switch_monitor_cb(s_switch_state);
    }
}

esp_err_t bsp_adc_switch_monitor_init(switch_monitor_cfg_t *cfg)
{
    if (!adc1_handle) {
        adc_oneshot_unit_init_cfg_t init_config = {
            .unit_id = ADC_UNIT_1,
        };
        ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));
    }

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    adc_oneshot_config_channel(adc1_handle, KBD_ADC_SWITCH_BLE_CHAN, &config);
    adc_oneshot_config_channel(adc1_handle, KBD_ADC_SWITCH_RAINMAKER_CHAN, &config);
    if (cfg->monitor_interval_ms > 0) {
        s_switch_monitor_timer = xTimerCreate("switch_monitor", pdMS_TO_TICKS(cfg->monitor_interval_ms), pdTRUE, cfg, switch_monitor_timer_callback);
        if (s_switch_monitor_timer == NULL) {
            return ESP_ERR_NO_MEM;
        }
        xTimerStart(s_switch_monitor_timer, 0);
        s_switch_monitor_cb = cfg->monitor_cb;
    }
    return ESP_OK;
}

static void vbus_monitor_timer_callback(TimerHandle_t xTimer)
{
    bool vbus_is_conn = false;
    bsp_get_vbus_state(&vbus_is_conn);
    if (s_is_vbus_conn != vbus_is_conn) {
        s_is_vbus_conn = vbus_is_conn;
        ESP_LOGI(TAG, "Vbus state: %d", s_is_vbus_conn);
        if (s_vbus_monitor_cb) {
            s_vbus_monitor_cb(s_is_vbus_conn);
        }
    }
}

esp_err_t bsp_adc_vbus_monitor_init(vbus_monitor_cfg_t *cfg)
{
    ESP_RETURN_ON_FALSE(cfg != NULL, ESP_ERR_INVALID_ARG, TAG, "cfg is NULL");

    if (!adc1_handle) {
        adc_oneshot_unit_init_cfg_t init_config = {
            .unit_id = ADC_UNIT_1,
        };
        ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));
    }

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(adc1_handle, KBD_ADC_VBUS_MONITOR_CHAN, &config);

    if (cfg->monitor_interval_ms > 0) {
        s_vbus_monitor_timer = xTimerCreate("vbus_monitor", pdMS_TO_TICKS(cfg->monitor_interval_ms), pdTRUE, cfg, vbus_monitor_timer_callback);
        if (s_vbus_monitor_timer == NULL) {
            return ESP_ERR_NO_MEM;
        }
        xTimerStart(s_vbus_monitor_timer, 0);
        s_vbus_monitor_cb = cfg->monitor_cb;
    }
    return ESP_OK;
}

static void battery_monitor_timer_callback(TimerHandle_t xTimer)
{
    bsp_get_battery_state(&s_is_charging, &s_battery_voltage);
    ESP_LOGI(TAG, "Battery voltage: %f, is_charging: %d", s_battery_voltage, s_is_charging);
    if (s_battery_monitor_cb) {
        s_battery_monitor_cb(s_is_charging, s_battery_voltage);
    }
}

esp_err_t bsp_adc_battery_monitor_init(battery_monitor_cfg_t *cfg)
{
    ESP_RETURN_ON_FALSE(cfg != NULL, ESP_ERR_INVALID_ARG, TAG, "cfg is NULL");

    if (!adc1_handle) {
        adc_oneshot_unit_init_cfg_t init_config = {
            .unit_id = ADC_UNIT_1,
        };
        ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));
    }

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    adc_oneshot_config_channel(adc1_handle, KBD_ADC_BATTERY_MONITOR_CHAN, &config);
    adc_oneshot_config_channel(adc1_handle, KBD_ADC_CHARGE_MONITOR_CHAN, &config);

    if (cfg->monitor_interval_ms > 0) {
        s_battery_monitor_timer = xTimerCreate("battery_monitor", pdMS_TO_TICKS(cfg->monitor_interval_ms), pdTRUE, cfg, battery_monitor_timer_callback);
        if (s_battery_monitor_timer == NULL) {
            return ESP_ERR_NO_MEM;
        }
        xTimerStart(s_battery_monitor_timer, 0);
        s_battery_monitor_cb = cfg->monitor_cb;
    }

    return ESP_OK;
}

esp_err_t bsp_get_battery_state(bool *is_charging, float *voltage)
{
    int adc_value[2];
    bsp_pm_lock_acquire();
    sar_ctrl_ll_set_power_mode_from_pwdet(SAR_CTRL_LL_POWER_ON);
    adc_oneshot_read(adc1_handle, KBD_ADC_BATTERY_MONITOR_CHAN, &adc_value[0]);
    adc_oneshot_read(adc1_handle, KBD_ADC_CHARGE_MONITOR_CHAN, &adc_value[1]);
    bsp_pm_lock_release();
    int charge = adc_value[1] / 4095.0f * 3300.0f;
    ESP_LOGD(TAG, "BATTERY[0]: %d, CHARGE[1]: %d", adc_value[0], charge);
    if (charge < (ADC_CHARGING_THRESHOLD + 200)) {
        *is_charging = true;
    } else {
        *is_charging = false;
    }
    *voltage = (adc_value[0] / 4095.0f) * 3.3f;
    *voltage = *voltage * ADC_BATTERY_VOLTAGE_RATIO;
    if (*voltage < ADC_BATTERY_VOLTAGE_MIN) {
        *voltage = ADC_BATTERY_VOLTAGE_MIN;
    }
    if (*voltage > ADC_BATTERY_VOLTAGE_MAX) {
        *voltage = ADC_BATTERY_VOLTAGE_MAX;
    }

    return ESP_OK;
}

esp_err_t bsp_get_switch_state(bsp_switch_state_t *state)
{
    int adc_value[2];
    bsp_pm_lock_acquire();
    sar_ctrl_ll_set_power_mode_from_pwdet(SAR_CTRL_LL_POWER_ON);
    adc_oneshot_read(adc1_handle, KBD_ADC_SWITCH_BLE_CHAN, &adc_value[0]);
    adc_oneshot_read(adc1_handle, KBD_ADC_SWITCH_RAINMAKER_CHAN, &adc_value[1]);
    bsp_pm_lock_release();
    ESP_LOGD(TAG, "adc_value[0]: %d, adc_value[1]: %d", adc_value[0], adc_value[1]);
    if (adc_value[0] > ADC_SWITCH_THRESHOLD) {
        *state = BSP_SWITCH_STATE_LEFT;
    } else if (adc_value[1] > ADC_SWITCH_THRESHOLD) {
        *state = BSP_SWITCH_STATE_RIGHT;
    } else {
        *state = BSP_SWITCH_STATE_MIDDLE;
    }
    return ESP_OK;
}

esp_err_t bsp_get_vbus_state(bool *is_charging)
{
    int adc_value;
    bsp_pm_lock_acquire();
    sar_ctrl_ll_set_power_mode_from_pwdet(SAR_CTRL_LL_POWER_ON);
    adc_oneshot_read(adc1_handle, KBD_ADC_VBUS_MONITOR_CHAN, &adc_value);
    bsp_pm_lock_release();
    if (adc_value > ADC_SWITCH_THRESHOLD) {
        *is_charging = true;
    } else {
        *is_charging = false;
    }
    return ESP_OK;
}

esp_err_t bsp_output_io_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << UART1_TX_IO | 1ULL << UART1_RX_IO | 1ULL << UART2_TX_IO | 1ULL << UART2_RX_IO,
                             .mode = GPIO_MODE_OUTPUT,
                             .pull_up_en = GPIO_PULLUP_DISABLE,
                             .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    return gpio_config(&io_conf);
}

esp_err_t bsp_output_io_set(uint8_t io, bool value)
{
    return gpio_set_level(io, value);
}

void bsp_pm_lock_init(void)
{
    esp_err_t ret = esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "bsp_pm_lock", &pm_lock);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create PM lock: %s", esp_err_to_name(ret));
    }
}

void bsp_pm_lock_release(void)
{
    if (pm_lock == NULL) {
        ESP_LOGE(TAG, "PM lock not initialized");
        return;
    }
    esp_pm_lock_release(pm_lock);
}

void bsp_pm_lock_acquire(void)
{
    if (pm_lock == NULL) {
        ESP_LOGE(TAG, "PM lock not initialized");
        return;
    }
    esp_pm_lock_acquire(pm_lock);
}

static void deep_sleep_register_ext1_wakeup(void)
{
    const int ext_wakeup_pin_1 = bsp_deep_sleep->wakeup_pin_1;
    const int ext_wakeup_pin_2 = bsp_deep_sleep->wakeup_pin_2;
    const uint64_t ext_wakeup_pin_1_mask = 1ULL << ext_wakeup_pin_1;
    const uint64_t ext_wakeup_pin_2_mask = 1ULL << ext_wakeup_pin_2;
    printf("Enabling EXT1 wakeup on pins GPIO%d, GPIO%d\n", ext_wakeup_pin_1, ext_wakeup_pin_2);

#if SOC_PM_SUPPORT_EXT1_WAKEUP_MODE_PER_PIN
    ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup_io(ext_wakeup_pin_1_mask, ESP_EXT1_WAKEUP_ANY_LOW));
    ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup_io(ext_wakeup_pin_2_mask, ESP_EXT1_WAKEUP_ANY_LOW));
#else
    ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup_io(ext_wakeup_pin_1_mask | ext_wakeup_pin_2_mask, ESP_EXT1_WAKEUP_ANY_LOW));
#endif
}

static void deep_sleep_timer_cb(TimerHandle_t xTimer)
{
    bsp_light_sleep_enable(false);
    deep_sleep_register_ext1_wakeup();
    ESP_LOGI(TAG, "Enter deep sleep");
    esp_deep_sleep_start();
}

esp_err_t bsp_light_sleep_enable(bool enable)
{
#if CONFIG_PM_ENABLE
    // Configure dynamic frequency scaling:
    // maximum and minimum frequencies are set in sdkconfig,
    // automatic light sleep is enabled if tickless idle support is enabled.
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 160,
        .min_freq_mhz = 40,
#if CONFIG_FREERTOS_USE_TICKLESS_IDLE
        .light_sleep_enable = enable
#endif
    };
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
    ESP_LOGI(TAG, "Dynamic frequency scaling enabled");

#endif // CONFIG_PM_ENABLE
    return ESP_OK;
}

esp_err_t bsp_deep_sleep_init(bsp_deep_sleep_config_t *config)
{
    if (!bsp_deep_sleep) {
        bsp_deep_sleep = (bsp_deep_sleep_t *)calloc(1, sizeof(bsp_deep_sleep_t));
        if (!bsp_deep_sleep) {
            ESP_LOGE(TAG, "Failed to allocate memory for bsp_deep_sleep");
            return ESP_FAIL;
        }
    }

    bsp_deep_sleep->sleep_time_ms = config->sleep_time_ms;
    bsp_deep_sleep->wakeup_pin_1 = config->wakeup_pin_1;
    bsp_deep_sleep->wakeup_pin_2 = config->wakeup_pin_2;

    if (bsp_deep_sleep->timer_handle) {
        ESP_LOGE(TAG, "Deep sleep timer already initialized");
        return ESP_FAIL;
    }
    bsp_deep_sleep->timer_handle = xTimerCreate("deep_sleep_timer", pdMS_TO_TICKS(config->sleep_time_ms), pdTRUE, NULL, deep_sleep_timer_cb);
    if (!bsp_deep_sleep->timer_handle) {
        ESP_LOGE(TAG, "Failed to create deep sleep timer");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t bsp_deep_sleep_enable(bool enable)
{
    if (bsp_deep_sleep == NULL) {
        ESP_LOGE(TAG, "Deep sleep not initialized");
        return ESP_FAIL;
    }
    if (!enable) {
        if (bsp_deep_sleep->timer_handle) {
            xTimerStop(bsp_deep_sleep->timer_handle, 0);
            xTimerDelete(bsp_deep_sleep->timer_handle, 0);
            bsp_deep_sleep->timer_handle = NULL;
            return ESP_OK;
        } else {
            ESP_LOGE(TAG, "Deep sleep timer not initialized");
            return ESP_FAIL;
        }
    } else {
        if (!bsp_deep_sleep->timer_handle) {
            ESP_LOGE(TAG, "Deep sleep timer not initialized");
            return ESP_FAIL;
        }
        if (xTimerStart(bsp_deep_sleep->timer_handle, 0) != pdPASS) {
            ESP_LOGE(TAG, "Failed to start deep sleep timer");
            return ESP_FAIL;
        }
    }
    return ESP_OK;
}

esp_err_t bsp_deep_sleep_reset(void)
{
    if (bsp_deep_sleep == NULL) {
        ESP_LOGE(TAG, "Deep sleep not initialized");
        return ESP_FAIL;
    }

    if (!bsp_deep_sleep->timer_handle) {
        ESP_LOGE(TAG, "Deep sleep timer not initialized");
        return ESP_FAIL;
    }

    xTimerReset(bsp_deep_sleep->timer_handle, 0);
    return ESP_OK;
}
