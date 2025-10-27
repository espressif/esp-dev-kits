/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <esp_rmaker_standard_params.h>
#include "via_keymap.h"
#include "tinyusb_hid.h"
#include "app_nvs.h"
#include "light_manager/light_manager.h"
#include "esp_timer.h"
#include "esp_rmaker_utils.h"
#include "esp_rmaker_mqtt.h"
#include "esp_rmaker_core.h"
#include "esp_log.h"
#include "dual_button.h"
#include "app_utils.h"
#include "btn_progress.h"
#include "bsp/esp-bsp.h"
#include "ble_hid.h"
#include "app_rainmaker.h"

#include "soc/soc_caps.h"

#if SOC_USB_SERIAL_JTAG_SUPPORTED
#include "soc/usb_serial_jtag_reg.h"
#include "hal/usb_serial_jtag_ll.h"
#endif
#include "esp_private/usb_phy.h"

const char *TAG = "esp_dualkey";

static const char *NVS_KEY = "esp_dualkey";
static const char *VIA_KEYMAP_KEY = "via_keymap";

// Storage parameter structure
typedef struct {
    btn_report_type_t report_type;
    uint8_t action_layer;
    app_rainmaker_params_t rmaker_params;
} storage_param_t;

typedef struct {
    keyboard_btn_handle_t kbd_handle;
    storage_param_t storage_param;
    TimerHandle_t ws2812_sleep_timer;
    TimerHandle_t connection_timer;
    button_handle_t btn;
    nvs_handle_t nvs_handle;
} esp_dualkey_context_t;

static esp_dualkey_context_t esp_dualkey_ctx = {
    .kbd_handle = NULL,
    .storage_param = {
        .action_layer = 0,
        .report_type = TINYUSB_HID_REPORT,
        .rmaker_params = APP_RAINMAKER_DEFAULT_PARAMS()
    },
    .ws2812_sleep_timer = NULL,
    .connection_timer = NULL,
    .btn = NULL,
};

led_config_t g_led_config = {{
        // Key Matrix to LED Index
        {0, 1}
    }, {
        // LED Index to Physical Position
        {0, 0}, {32, 0}
    }, {
        // LED Index to Flag
        4, 4
    }
};

// Set default values for storage parameters.
static void set_default_storage_config(storage_param_t *storage_param)
{
    storage_param->report_type = TINYUSB_HID_REPORT;
    storage_param->action_layer = 0;
    storage_param->rmaker_params = (app_rainmaker_params_t)APP_RAINMAKER_DEFAULT_PARAMS();
    ESP_LOGI(TAG, "Set default storage config");
}

// Save storage parameters to NVS.
static esp_err_t save_storage_config(storage_param_t *storage_param)
{
    esp_err_t ret = app_nvs_store(esp_dualkey_ctx.nvs_handle, NVS_KEY, storage_param, sizeof(storage_param_t));
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Storage config saved successfully");
    } else {
        ESP_LOGE(TAG, "Failed to save storage config: %s", esp_err_to_name(ret));
    }
    return ret;
}

// Load storage parameters from NVS.
static esp_err_t load_storage_config(storage_param_t *storage_param)
{
    bool exist = false;

    esp_err_t ret = app_nvs_restore(esp_dualkey_ctx.nvs_handle, NVS_KEY, storage_param, sizeof(storage_param_t), &exist);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Storage config loaded successfully");
    } else {
        ESP_LOGE(TAG, "Failed to load storage config: %s", esp_err_to_name(ret));
    }
    return ret;
}

static esp_err_t via_keymap_store(const void *data, size_t length)
{
    return app_nvs_store(esp_dualkey_ctx.nvs_handle, VIA_KEYMAP_KEY, data, length);
}

static esp_err_t via_keymap_restore(void *data, size_t length)
{
    bool exist = false;
    return app_nvs_restore(esp_dualkey_ctx.nvs_handle, VIA_KEYMAP_KEY, data, length, &exist);

}

static void rainmaker_report(keyboard_btn_report_t kbd_report)
{
    // Process key release events
    for (int i = 0; i < kbd_report.key_release_num; i++) {
        uint8_t input_index = kbd_report.key_release_data[i].input_index;
        ESP_LOGI(TAG, "rainmaker_report: key_release_data[%d] = %d", i, input_index);

        switch (input_index) {
        case 0:
            app_rainmaker_set_switch_l(SWITCH_TOGGLE);
            break;
        case 1:
            app_rainmaker_set_switch_r(SWITCH_TOGGLE);
            break;
        default:
            ESP_LOGW(TAG, "Unknown key release input_index: %d", input_index);
            break;
        }
    }
    // Process key press events
    for (int i = 0; i < kbd_report.key_pressed_num; i++) {
        uint8_t input_index = kbd_report.key_data[i].input_index;
        ESP_LOGI(TAG, "rainmaker_report: key_pressed_data[%d] = %d", i, input_index);
    }
}

static void keyboard_cb(keyboard_btn_handle_t kbd_handle, keyboard_btn_report_t kbd_report, void *user_data)
{
    esp_dualkey_context_t *ctx = (esp_dualkey_context_t *)user_data;

    if (ctx->storage_param.report_type == BLE_HID_REPORT) {
        if (light_manager_is_suspended()) {
            ESP_LOGI(TAG, "keyboard_cb: light_manager_set_enabled");
            light_manager_set_enabled(true);
            light_manager_set_suspended(false);
            light_manager_resume_task();
        }
    }
    if (ctx->storage_param.report_type == RAINMAKER_REPORT) {
        rainmaker_report(kbd_report);
    } else {
        btn_progress(kbd_report);
    }
    /*!< Lighting with key pressed */
    if (kbd_report.key_change_num > 0) {
        for (int i = 1; i <= kbd_report.key_change_num; i++) {
            light_manager_process_key_event(kbd_report.key_data[kbd_report.key_pressed_num - i].output_index, kbd_report.key_data[kbd_report.key_pressed_num - i].input_index, true);
        }
    }
    if (esp_dualkey_ctx.ws2812_sleep_timer != NULL) {
        xTimerReset(esp_dualkey_ctx.ws2812_sleep_timer, 0);
    }
}

static void ws2812_sleep_timer_cb(TimerHandle_t xTimer)
{
    if (!light_manager_is_suspended()) {
        ESP_LOGI(TAG, "light_manager set suspended");
        light_manager_set_suspended(true);
        light_manager_set_enabled(false);
        light_manager_suspend_task();
    }
}

static void connection_monitor_timer_callback(TimerHandle_t xTimer)
{
    static bool is_connected = false;

    bool current_connected = false;

    if (esp_dualkey_ctx.storage_param.report_type == BLE_HID_REPORT) {
        current_connected = ble_hid_is_connected();
    } else if (esp_dualkey_ctx.storage_param.report_type == RAINMAKER_REPORT) {
        current_connected = esp_rmaker_is_mqtt_connected();
    }

    if (current_connected != is_connected) {
        is_connected = current_connected;
        ESP_LOGI(TAG, "Connection status changed: %s", is_connected ? "Connected" : "Disconnected");

        if (is_connected) {
            light_manager_set_mode(esp_dualkey_ctx.storage_param.rmaker_params.light_mode_type);
        } else {
            light_manager_set_mode(5);
        }
    }
}

static void battery_monitor_callback(bool is_charging, float battery_voltage)
{
    ESP_LOGI(TAG, "Battery voltage: %f, is_charging: %d", battery_voltage, is_charging);

    uint8_t battery_level = BATTERY_VOLTAGE_TO_PERCENTAGE(battery_voltage);

    // Update battery level based on current report type
    if (esp_dualkey_ctx.storage_param.report_type == BLE_HID_REPORT) {
        ble_hid_battery_update(battery_level);
    } else if (esp_dualkey_ctx.storage_param.report_type == RAINMAKER_REPORT) {
        app_rainmaker_update_battery_level(battery_level);
    }
}

static void switch_monitor_callback(bsp_switch_state_t switch_state)
{
    const btn_report_type_t report_type_mapping[3] = {TINYUSB_HID_REPORT, BLE_HID_REPORT, RAINMAKER_REPORT};

    // Monitor switch state and handle report type changes
    if (esp_dualkey_ctx.storage_param.report_type != report_type_mapping[switch_state]) {
        // Save new configuration
        esp_dualkey_ctx.storage_param.report_type = report_type_mapping[switch_state];
        esp_err_t ret = save_storage_config(&esp_dualkey_ctx.storage_param);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to save new report type: %s", esp_err_to_name(ret));
            return;
        }

        // Update button progress report type
        btn_progress_set_report_type(esp_dualkey_ctx.storage_param.report_type);
        ESP_LOGI(TAG, "report switch changed, reboot the device");
        vTaskDelay(10 / portTICK_PERIOD_MS);
        esp_restart();

    }
}

static void reconfig_usb_serial_jtag(void)
{
    SET_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_PAD_PULL_OVERRIDE);
    CLEAR_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_DP_PULLUP);
    SET_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_DP_PULLDOWN);
    vTaskDelay(pdMS_TO_TICKS(10));
#if USB_SERIAL_JTAG_LL_EXT_PHY_SUPPORTED
    usb_serial_jtag_ll_phy_enable_external(false);  // Use internal PHY.
    usb_serial_jtag_ll_phy_enable_pad(true);        // Enable USB PHY pads.
#else // USB_SERIAL_JTAG_LL_EXT_PHY_SUPPORTED
    usb_serial_jtag_ll_phy_set_defaults();          // External PHY not supported. Set default values.
#endif // USB_WRAP_LL_EXT_PHY_SUPPORTED
    CLEAR_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_DP_PULLDOWN);
    SET_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_DP_PULLUP);
    CLEAR_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_PAD_PULL_OVERRIDE);
}

static void vbus_monitor_callback(bool is_charging)
{
    if (esp_dualkey_ctx.storage_param.report_type == TINYUSB_HID_REPORT) {
        return;
    }

    if (is_charging) {
        bsp_light_sleep_enable(false);
        reconfig_usb_serial_jtag();
    } else {
        bsp_light_sleep_enable(true);
    }
}

static void button_event_cb(void *arg, void *usr_data)
{
    esp_dualkey_context_t *ctx = (esp_dualkey_context_t *)usr_data;
    button_handle_t btn = (button_handle_t)arg;
    button_event_t event = iot_button_get_event(btn);
    uint32_t ticks = iot_button_get_ticks_time(btn);
    ESP_LOGI(TAG, "button_event_cb: event = %d, ticks = %ld", event, ticks);

    // Handle button press down event.
    if (event == BUTTON_PRESS_DOWN) {
        ESP_LOGI(TAG, "Button pressed down - immediate response");

    } else if (event == BUTTON_LONG_PRESS_UP) {
        if (ticks >= 9000) {
            if (ctx->storage_param.report_type == RAINMAKER_REPORT) {
                // Reset RainMaker to factory settings.
                set_default_storage_config(&ctx->storage_param);
                save_storage_config(&ctx->storage_param);

                // Reset RGB matrix to default mode.
                light_manager_set_suspended(false);

                app_nvs_close(esp_dualkey_ctx.nvs_handle);

                // Erase factory partition.
                const esp_partition_t *factory_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, "fctry");
                esp_partition_erase_range(factory_partition, 0, factory_partition->size);

                ESP_LOGI(TAG, "Start factory reset");
                esp_rmaker_factory_reset(0, 0);

                // Stop button monitoring.
                iot_button_stop();
            }
        }
    }
}

static esp_err_t rainmaker_write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
                                    const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    if (ctx) {
        ESP_LOGI(TAG, "Received write request via : %s", esp_rmaker_device_cb_src_to_str(ctx->src));
    }
    const char *device_name = esp_rmaker_device_get_name(device);
    const char *param_name = esp_rmaker_param_get_name(param);
    app_rainmaker_params_t *rmaker_params = &esp_dualkey_ctx.storage_param.rmaker_params;
    if (strcmp(param_name, ESP_RMAKER_DEF_POWER_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %s for %s - %s",
                 val.val.b ? "true" : "false", device_name, param_name);
        rmaker_params->power = val.val.b;
        light_manager_set_suspended(!val.val.b);
    } else if (strcmp(param_name, ESP_RMAKER_DEF_BRIGHTNESS_NAME) == 0) {
        rmaker_params->brightness = val.val.i;
        light_manager_set_brightness(val.val.i);
        ESP_LOGI(TAG, "Received value = %d for %s - %s",
                 val.val.i, device_name, param_name);
    } else if (strcmp(param_name, ESP_RMAKER_DEF_HUE_NAME) == 0) {
        rmaker_params->hue = val.val.i * 255 / 360;
        light_manager_set_hue(rmaker_params->hue);
        ESP_LOGI(TAG, "Received value = %d for %s - %s",
                 val.val.i, device_name, param_name);
    } else if (strcmp(param_name, ESP_RMAKER_DEF_SATURATION_NAME) == 0) {
        rmaker_params->saturation = val.val.i;
        light_manager_set_saturation(val.val.i);
        ESP_LOGI(TAG, "Received value = %d for %s - %s",
                 val.val.i, device_name, param_name);
    } else if (strcmp(param_name, ESP_RMAKER_DEF_SWITCH_L_NAME) == 0) {
        rmaker_params->switch_l = val.val.b;
        ESP_LOGI(TAG, "Received value = %s for %s - %s",
                 val.val.b ? "true" : "false", device_name, param_name);
    } else if (strcmp(param_name, ESP_RMAKER_DEF_SWITCH_R_NAME) == 0) {
        rmaker_params->switch_r = val.val.b;
        ESP_LOGI(TAG, "Received value = %s for %s - %s",
                 val.val.b ? "true" : "false", device_name, param_name);
    } else if (strcmp(param_name, ESP_RMAKER_DEF_ACTION_NAME) == 0) {
        rmaker_params->action_type = val.val.i;
        int8_t i = 0;
        int size = sizeof(keyboard_action_list) / sizeof(keyboard_action_list[0]);
        for (i = 0; i < size; i++) {
            if (strcmp(val.val.s, keyboard_action_list[i]) == 0) {
                btn_progress_set_layer(i);
                esp_dualkey_ctx.storage_param.action_layer = i;
                break;
            }
        }
        ESP_LOGI(TAG, "Received value = %s for %s - %s mode = %d", val.val.s, device_name, param_name, i);
    } else if (strcmp(param_name, ESP_RMAKER_DEF_LIGHT_MODE_NAME) == 0) {
        int8_t i = 0;
        int size = sizeof(light_mode_list) / sizeof(light_mode_list[0]);
        for (i = 0; i < size; i++) {
            if (strcmp(val.val.s, light_mode_list[i]) == 0) {
                rmaker_params->light_mode_type = i;
                light_manager_set_mode(esp_dualkey_ctx.storage_param.rmaker_params.light_mode_type);
                ESP_LOGI(TAG, "Received value = %s for %s - %s mode = %d", val.val.s, device_name, param_name, i);
                break;
            }
        }
    } else if (strcmp(param_name, ESP_RMAKER_DEF_SPEED_NAME) == 0) {
        rmaker_params->light_speed = val.val.i;
        light_manager_set_speed(val.val.i);
        ESP_LOGI(TAG, "Received value = %d for %s - %s speed = %d", val.val.i, device_name, param_name, val.val.i);
    } else {
        /* Silently ignoring invalid params */
        return ESP_OK;
    }
    esp_rmaker_param_update_and_report(param, val);
    save_storage_config(&esp_dualkey_ctx.storage_param);
    return ESP_OK;
}

static void dual_button_init(esp_dualkey_context_t *ctx)
{
    /*!< Init reset/reconfig event. */
    const button_config_t btn_cfg = {
        .long_press_time = 4000,
    };
    button_dual_config_t dual_cfg = {
        .gpio_num = KBD_INPUT_IOS,
        .active_level = KBD_ATTIVE_LEVEL,
        .skip_gpio_init = true,
    };
    iot_button_new_dual_button_device(&btn_cfg, &dual_cfg, &ctx->btn);
    iot_button_register_cb(ctx->btn, BUTTON_PRESS_DOWN, NULL, button_event_cb, ctx);
    iot_button_register_cb(ctx->btn, BUTTON_LONG_PRESS_START, NULL, button_event_cb, ctx);
    iot_button_register_cb(ctx->btn, BUTTON_LONG_PRESS_HOLD, NULL, button_event_cb, ctx);
    iot_button_register_cb(ctx->btn, BUTTON_LONG_PRESS_UP, NULL, button_event_cb, ctx);
    iot_button_register_cb(ctx->btn, BUTTON_DOUBLE_CLICK, NULL, button_event_cb, ctx);
    // Register long press start with custom press time.
    button_event_args_t btn_event_cfg = {
        .long_press.press_time = 7000,
    };
    iot_button_register_cb(ctx->btn, BUTTON_LONG_PRESS_START, &btn_event_cfg, button_event_cb, ctx);
}

esp_err_t esp_dualkey_init(void)
{
    esp_err_t ret = ESP_OK;
    // Initialize NVS manager.
    ret = app_nvs_open(&esp_dualkey_ctx.nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS manager: %s", esp_err_to_name(ret));
        return ret;
    }
    bsp_pm_lock_init();
    /*!< Initialize and load system config. */
    ret = load_storage_config(&esp_dualkey_ctx.storage_param);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Using default storage config due to load error");
    }
    ESP_LOGI(TAG, "storage_param.report_type = %d", esp_dualkey_ctx.storage_param.report_type);
    /*!< Monitor adc switch. */
    switch_monitor_cfg_t switch_cfg = {
        .monitor_interval_ms = 1000,
        .monitor_cb = switch_monitor_callback,
    };
    bsp_adc_switch_monitor_init(&switch_cfg);

    battery_monitor_cfg_t battery_cfg = {
        .monitor_interval_ms = 60000,
        .monitor_cb = battery_monitor_callback,
    };
    bsp_adc_battery_monitor_init(&battery_cfg);

    vbus_monitor_cfg_t vbus_cfg = {
        .monitor_interval_ms = 1000,
        .monitor_cb = vbus_monitor_callback,
    };
    bsp_adc_vbus_monitor_init(&vbus_cfg);

    /*!< Create connection monitoring timer (1 second interval). */
    esp_dualkey_ctx.connection_timer = xTimerCreate("connection_monitor",
                                       pdMS_TO_TICKS(1000), // 1 second
                                       pdTRUE,              // Auto-reload
                                       &esp_dualkey_ctx,    // Timer ID
                                       connection_monitor_timer_callback);
    if (esp_dualkey_ctx.connection_timer == NULL) {
        ESP_LOGE(TAG, "Failed to create connection monitoring timer");
        return ESP_ERR_NO_MEM;
    }

    // Start the connection monitoring timer.
    if (xTimerStart(esp_dualkey_ctx.connection_timer, 0) != pdPASS) {
        ESP_LOGE(TAG, "Failed to start connection monitoring timer");
        return ESP_FAIL;
    }

    /*!< Initialize light manager. */
    light_manager_config_t light_config = LIGHT_MANAGER_DEFAULT_CONFIG();
    esp_err_t light_ret = light_manager_init(&light_config);
    if (light_ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize light manager: %s", esp_err_to_name(light_ret));
        return ret;
    }

    /*!< Init keyboard key monitor. */
    bsp_keyboard_init(&esp_dualkey_ctx.kbd_handle, NULL);
    keyboard_btn_cb_config_t cb_cfg = {
        .event = KBD_EVENT_PRESSED,
        .callback = keyboard_cb,
        .user_data = &esp_dualkey_ctx,
    };
    keyboard_button_register_cb(esp_dualkey_ctx.kbd_handle, cb_cfg, NULL);
    btn_progress_set_report_type(esp_dualkey_ctx.storage_param.report_type);
    btn_progress_set_layer(esp_dualkey_ctx.storage_param.action_layer);

    dual_button_init(&esp_dualkey_ctx);
    // Set the default lighting effect to 5.
    light_manager_set_mode(5);

    bsp_deep_sleep_config_t config = {
        .sleep_time_ms = CONFIG_DEEP_SLEEP_TIMEOUT_S * 1000,
        .wakeup_pin_1 = 0,
        .wakeup_pin_2 = 17,
    };

    via_keymap_config_t via_keymap_config = {
        .store_fn = via_keymap_store,
        .restore_fn = via_keymap_restore,
    };

    /*!< Init report. */
    switch (esp_dualkey_ctx.storage_param.report_type) {
    case TINYUSB_HID_REPORT:
        via_keymap_init(&via_keymap_config);
        tinyusb_hid_init();
        light_manager_set_mode(esp_dualkey_ctx.storage_param.rmaker_params.light_mode_type);
        break;
    case BLE_HID_REPORT:
        via_keymap_init(&via_keymap_config);
        ble_hid_init();
        bsp_deep_sleep_init(&config);
        bsp_deep_sleep_enable(true);
        light_manager_set_mode(esp_dualkey_ctx.storage_param.rmaker_params.light_mode_type);
        esp_dualkey_ctx.ws2812_sleep_timer = xTimerCreate("ws2812_sleep_timer", pdMS_TO_TICKS(CONFIG_WS2812_SLEEP_TIMEOUT_S * 1000), pdTRUE, NULL, ws2812_sleep_timer_cb);
        if (esp_dualkey_ctx.ws2812_sleep_timer == NULL) {
            ESP_LOGE(TAG, "Failed to create sleep timer");
            return ESP_ERR_NO_MEM;
        }
        xTimerStart(esp_dualkey_ctx.ws2812_sleep_timer, 0);
        break;
    case RAINMAKER_REPORT:
        esp_dualkey_ctx.storage_param.rmaker_params.action_type = esp_dualkey_ctx.storage_param.action_layer;
        esp_dualkey_ctx.storage_param.rmaker_params.write_cb = rainmaker_write_cb;
        esp_dualkey_ctx.storage_param.rmaker_params.read_cb = NULL;
        app_rainmaker_init(&esp_dualkey_ctx.storage_param.rmaker_params);
        light_manager_set_suspended(!esp_dualkey_ctx.storage_param.rmaker_params.power);
        break;
    default:
        break;
    }
    ESP_LOGI(TAG, "RGB_MATRIX_EFFECT_MAX = %d", RGB_MATRIX_EFFECT_MAX);

    return ESP_OK;
}
