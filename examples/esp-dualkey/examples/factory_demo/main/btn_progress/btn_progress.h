/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "esp_err.h"
#include "keyboard_button.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TINYUSB_HID_REPORT = 0,
    BLE_HID_REPORT,
    RAINMAKER_REPORT,
    BTN_REPORT_TYPE_MAX,
} btn_report_type_t;

typedef struct {
    uint32_t report_id;    // Report identifier
    union {
        struct {
            uint8_t modifier;     // Modifier keys
            uint8_t reserved;     // Reserved byte
            uint8_t keycode[15];  // Keycode
        } keyboard_full_key_report;  // Keyboard full key report
        struct {
            uint8_t modifier;   // Modifier keys
            uint8_t reserved;   // Reserved byte
            uint8_t keycode[6]; // Keycodes
        } keyboard_report;  // Keyboard report
        struct {
            uint16_t keycode;    // Keycode
        } consumer_report;
        struct {
            uint8_t buttons;      // Mouse buttons (bits: 0=L, 1=R, 2=M, 3=Back, 4=Forward)
            int8_t x;            // X axis movement (-127 to 127)
            int8_t y;            // Y axis movement (-127 to 127)
            int8_t wheel;        // Vertical wheel scroll (-127 to 127)
            int8_t pan;          // Horizontal wheel scroll (-127 to 127)
        } mouse_report;        // Mouse report
        struct {
            uint8_t data[32];   // VIA Raw HID data (32 bytes)
        } via_raw_report;      // VIA Raw report for keyboard configuration
    };
} hid_report_t;

/**
 * @brief Button progress function.
 *
 * @param kbd_report Keyboard button report.
 */
void btn_progress(keyboard_btn_report_t kbd_report);

/**
 * @brief Set the report type for button progress.
 *
 * @param type Button report type.
 */
void btn_progress_set_report_type(btn_report_type_t type);

/**
 * @brief Set the layer for button progress.
 *
 * @param layer Layer.
 */
void btn_progress_set_layer(uint8_t layer);

/**
 * @brief Report the button progress.
 *
 * @param report HID report structure containing button data.
 */
void btn_progress_report(hid_report_t report);

#ifdef __cplusplus
}
#endif
