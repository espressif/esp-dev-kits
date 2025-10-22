/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "btn_progress.h"
#include "tinyusb_hid.h"
#include "ble_hid.h"
#include "bsp/keycodes.h"
#include "usb_descriptors.h"
#include "rgb_matrix.h"
#include "esp_system.h"
#include "esp_pm.h"
#include "bsp/esp-bsp.h"
#include "bsp/quantum_keycodes.h"
#include "tinyusb_hid.h"
#include "esp_check.h"
#include "media_control_config.h"
#include "iot_button.h"

static const char *TAG = "btn_progress";

static btn_report_type_t s_report_type = TINYUSB_HID_REPORT;
static uint8_t s_action_layer = 1;

typedef struct {
    button_driver_t base;          /**< button driver */
    int32_t gpio_num[2];           /**< num of gpio */
    uint8_t active_level;          /**< gpio level when press down */
} button_dual_obj;

// Media control handling is now managed by media_control_config.h

#define HSV_MAX 18
static uint8_t hsv_index = 0;
static uint8_t hsv_color[HSV_MAX][3] = {
    {132, 102, 255}, // HSV_AZURE
    {170, 255, 255}, // HSV_BLUE
    {64,  255, 255}, // HSV_CHARTREUSE
    {11,  176, 255}, // HSV_CORAL
    {128, 255, 255}, // HSV_CYAN
    {36,  255, 255}, // HSV_GOLD
    {30,  218, 218}, // HSV_GOLDENROD
    {85,  255, 255}, // HSV_GREEN
    {213, 255, 255}, // HSV_MAGENTA
    {21,  255, 255}, // HSV_ORANGE
    {234, 128, 255}, // HSV_PINK
    {191, 255, 255}, // HSV_PURPLE
    {0,   255, 255}, // HSV_RED
    {106, 255, 255}, // HSV_SPRINGGREEN
    {128, 255, 128}, // HSV_TEAL
    {123, 90,  112}, // HSV_TURQUOISE
    {0,   0,   255}, // HSV_WHITE
    {43,  255, 255}, // HSV_YELLOW
};

static void _report(hid_report_t report)
{
    switch (s_report_type) {
    case TINYUSB_HID_REPORT:
        if (report.report_id == REPORT_ID_MOUSE) {
            tinyusb_hid_mouse_report(report);
        } else {
            tinyusb_hid_keyboard_report(report);
        }
        break;
    case BLE_HID_REPORT:
        if (report.report_id == REPORT_ID_MOUSE) {
            ble_hid_mouse_report(report);
        } else {
            ble_hid_keyboard_report(report);
        }
        break;
    default:
        break;
    }
}

void btn_progress(keyboard_btn_report_t kbd_report)
{
    uint8_t mo_action_layer = s_action_layer;
    uint8_t keycode[120] = {0};
    uint8_t keynum = 0;
    uint8_t modify = 0;
    hid_report_t kbd_hid_report = {0};
    hid_report_t consumer_report = {0};
    hid_report_t mouse_report = {0};
    bool if_consumer_report = false;
    bool release_consumer_report = false;
    bool if_mouse_report = false;

    /*!< Report with key pressed */
    for (int i = 0; i < kbd_report.key_pressed_num; i++) {
        uint8_t row = kbd_report.key_data[i].output_index;
        uint8_t col = kbd_report.key_data[i].input_index;
        uint16_t kc = tinyusb_hid_get_via_keycode(mo_action_layer, row, col);
        ESP_LOGI(TAG, "VIA: Get keycode L%d R%d C%d = 0x%04X", mo_action_layer, row, col, kc);
        switch (kc) {
        case QK_MOMENTARY ... QK_MOMENTARY_MAX:
            // Momentary action_layer
            mo_action_layer = QK_MOMENTARY_GET_LAYER(kc);
            printf("Momentary action_layer: %d\n", mo_action_layer);
            break;

        case QK_LCTL ... QK_RGUI:
            // All modifier keys (left and right modifiers)
            // Extract modifier bits and basic keycode
            uint8_t mods = QK_MODS_GET_MODS(kc);
            uint8_t basic_kc = QK_MODS_GET_BASIC_KEYCODE(kc);

            // Set modifier bits in modify variable
            modify |= mods;
            keycode[keynum++] = basic_kc;
            break;

        case HID_KEY_CONTROL_LEFT ... HID_KEY_GUI_RIGHT:
            // Direct HID modifier key
            modify |= 1 << (kc - HID_KEY_CONTROL_LEFT);
            break;

        case KC_MS_WH_UP:
            mouse_report.mouse_report.wheel = 1;  // Scroll up
            if_mouse_report = true;
            break;

        case KC_MS_WH_DOWN:
            mouse_report.mouse_report.wheel = -1; // Scroll down
            if_mouse_report = true;
            break;

        case QK_BACKLIGHT_UP:
            rgb_matrix_increase_val();
            break;

        case QK_BACKLIGHT_DOWN:
            rgb_matrix_decrease_val();
            break;

        case RGB_SPD:
            rgb_matrix_decrease_speed();
            break;

        case RGB_SPI:
            rgb_matrix_increase_speed();
            break;

        case QK_BACKLIGHT_TOGGLE:
            rgb_matrix_toggle();
            if (!rgb_matrix_is_enabled()) {
                bsp_ws2812_clear();
            }
            bsp_ws2812_enable(rgb_matrix_is_enabled());
            break;

        case RGB_MODE_FORWARD: {
            uint16_t index = (rgb_matrix_get_mode() + 1) % RGB_MATRIX_EFFECT_MAX;
            rgb_matrix_mode(index);
            break;
        }

        case RGB_MODE_REVERSE: {
            uint16_t index = rgb_matrix_get_mode() - 1;
            if (index < 1) {
                index = RGB_MATRIX_EFFECT_MAX - 1;
            }
            rgb_matrix_mode(index);
            break;
        }

        case RGB_TOG:
            rgb_matrix_sethsv(hsv_color[hsv_index][0], hsv_color[hsv_index][1], hsv_color[hsv_index][2]);
            hsv_index = (hsv_index + 1) % HSV_MAX;
            break;

        default:
            // Check if this is a media control key
            uint16_t hid_usage;
            if (find_extended_media_key_mapping(kc, &hid_usage)) {
                consumer_report.consumer_report.keycode = hid_usage;
                if_consumer_report = true;
                ESP_LOGI(TAG, "Media key: 0x%04X -> 0x%04X", kc, hid_usage);
                break;
            }
            if (kc != HID_KEY_NONE) {
                keycode[keynum++] = kc;
            }
            break;
        }
    }

    /*!< Find if consumer key release */
    for (int i = 0; i < kbd_report.key_release_num; i++) {
        uint8_t row = kbd_report.key_release_data[i].output_index;
        uint8_t col = kbd_report.key_release_data[i].input_index;
        uint16_t kc = tinyusb_hid_get_via_keycode(mo_action_layer, row, col);

        // Check if released key is a media control key
        if (is_media_control_key(kc)) {
            release_consumer_report = true;
            ESP_LOGD(TAG, "Media key released: 0x%04X", kc);
        }
    }

    if (keynum <= 6) {
        kbd_hid_report.report_id = REPORT_ID_KEYBOARD;
        kbd_hid_report.keyboard_report.modifier = modify;
        for (int i = 0; i < keynum; i++) {
            kbd_hid_report.keyboard_report.keycode[i] = keycode[i];
        }
    } else {
        kbd_hid_report.report_id = REPORT_ID_FULL_KEY_KEYBOARD;
        kbd_hid_report.keyboard_full_key_report.modifier = modify;
        for (int i = 0; i < keynum; i++) {
            // USAGE ID for keyboard starts from 4
            uint8_t key = keycode[i] - 3;
            uint8_t byteIndex = (key - 1) / 8;
            uint8_t bitIndex = (key - 1) % 8;
            kbd_hid_report.keyboard_full_key_report.keycode[byteIndex] |= (1 << bitIndex);
        }
    }

    if (if_consumer_report && !release_consumer_report) {
        consumer_report.report_id = REPORT_ID_CONSUMER;
        _report(consumer_report);
        return;
    } else if (release_consumer_report) {
        consumer_report.report_id = REPORT_ID_CONSUMER;
        consumer_report.consumer_report.keycode = 0;
        _report(consumer_report);
        return;
    }

    if (if_mouse_report) {
        mouse_report.report_id = REPORT_ID_MOUSE;
        _report(mouse_report);
        return;
    }

    _report(kbd_hid_report);
}

void btn_progress_set_report_type(btn_report_type_t type)
{
    s_report_type = type;
}

void btn_progress_set_layer(uint8_t layer)
{
    s_action_layer = layer;
}

void btn_progress_report(hid_report_t report)
{
    _report(report);
}
