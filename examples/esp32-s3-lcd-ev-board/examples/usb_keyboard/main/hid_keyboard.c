/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tinyusb.h"
#include "sdkconfig.h"
#include "lvgl.h"
#include "class/hid/hid.h"
#include "class/hid/hid_device.h"
#include "bsp/esp-bsp.h"

static const char *TAG = "example_usb_keyboard";

enum {
    REPORT_ID_KEYBOARD = 1,
    REPORT_ID_MOUSE,
    REPORT_ID_CONSUMER_CONTROL,
    REPORT_ID_GAMEPAD,
    REPORT_ID_COUNT
};

typedef struct {
    uint8_t hidv;
    char *txt;
} key_value_t;

static const key_value_t key_value[] = {
    {HID_KEY_NONE, NULL},
    {HID_KEY_A, "A" },
    {HID_KEY_B, "B" },
    {HID_KEY_C, "C" },
    {HID_KEY_D, "D" },
    {HID_KEY_E, "E" },
    {HID_KEY_F, "F" },
    {HID_KEY_G, "G" },
    {HID_KEY_H, "H" },
    {HID_KEY_I, "I" },
    {HID_KEY_J, "J" },
    {HID_KEY_K, "K" },
    {HID_KEY_L, "L" },
    {HID_KEY_M, "M" },
    {HID_KEY_N, "N" },
    {HID_KEY_O, "O" },
    {HID_KEY_P, "P" },
    {HID_KEY_Q, "Q" },
    {HID_KEY_R, "R" },
    {HID_KEY_S, "S" },
    {HID_KEY_T, "T" },
    {HID_KEY_U, "U" },
    {HID_KEY_V, "V" },
    {HID_KEY_W, "W" },
    {HID_KEY_X, "X" },
    {HID_KEY_Y, "Y" },
    {HID_KEY_Z, "Z" },
    {HID_KEY_1, "1" },
    {HID_KEY_2, "2" },
    {HID_KEY_3, "3" },
    {HID_KEY_4, "4" },
    {HID_KEY_5, "5" },
    {HID_KEY_6, "6" },
    {HID_KEY_7, "7" },
    {HID_KEY_8, "8" },
    {HID_KEY_9, "9" },
    {HID_KEY_0, "0" },
    {HID_KEY_ENTER, LV_SYMBOL_NEW_LINE},
    {HID_KEY_ESCAPE, "ESC"},
    {HID_KEY_BACKSPACE, LV_SYMBOL_BACKSPACE},
    {HID_KEY_TAB, "Tab"},
    {HID_KEY_SPACE, " "},
    {HID_KEY_MINUS, "-"},
    {HID_KEY_EQUAL, "="},
    {HID_KEY_BRACKET_LEFT, "["},
    {HID_KEY_BRACKET_RIGHT, "]"},
    {HID_KEY_BACKSLASH, "\\"},
    {HID_KEY_EUROPE_1, NULL},
    {HID_KEY_SEMICOLON, ":"},
    {HID_KEY_APOSTROPHE, "\""},
    {HID_KEY_GRAVE, "~"},
    {HID_KEY_COMMA, ","},
    {HID_KEY_PERIOD, "."},
    {HID_KEY_SLASH, "/"},
    {HID_KEY_CAPS_LOCK, NULL},
    {HID_KEY_F1,  "F1"},
    {HID_KEY_F2,  "F2"},
    {HID_KEY_F3,  "F3"},
    {HID_KEY_F4,  "F4"},
    {HID_KEY_F5,  "F5"},
    {HID_KEY_F6,  "F6"},
    {HID_KEY_F7,  "F7"},
    {HID_KEY_F8,  "F8"},
    {HID_KEY_F9,  "F9"},
    {HID_KEY_F10, "F10"},
    {HID_KEY_F11, "F11"},
    {HID_KEY_F12, "F12"},
    {HID_KEY_PRINT_SCREEN, NULL},
    {HID_KEY_SCROLL_LOCK, NULL},
    {HID_KEY_PAUSE, NULL},
    {HID_KEY_INSERT, NULL},
    {HID_KEY_HOME, NULL},
    {HID_KEY_PAGE_UP, NULL},
    {HID_KEY_DELETE, "Del"},
    {HID_KEY_END, NULL},
    {HID_KEY_PAGE_DOWN, NULL},
    {HID_KEY_ARROW_RIGHT, LV_SYMBOL_RIGHT},
    {HID_KEY_ARROW_LEFT, LV_SYMBOL_LEFT},
    {HID_KEY_ARROW_DOWN, LV_SYMBOL_DOWN},
    {HID_KEY_ARROW_UP, LV_SYMBOL_UP},
    {HID_KEY_NUM_LOCK, NULL},
    {HID_KEY_KEYPAD_DIVIDE, NULL},
    {HID_KEY_KEYPAD_MULTIPLY, NULL},
    {HID_KEY_KEYPAD_SUBTRACT, NULL},
    {HID_KEY_KEYPAD_ADD, NULL},
    {HID_KEY_KEYPAD_ENTER, NULL},
    {HID_KEY_KEYPAD_1, "1"},
    {HID_KEY_KEYPAD_2, "2"},
    {HID_KEY_KEYPAD_3, "3"},
    {HID_KEY_KEYPAD_4, "4"},
    {HID_KEY_KEYPAD_5, "5"},
    {HID_KEY_KEYPAD_6, "6"},
    {HID_KEY_KEYPAD_7, "7"},
    {HID_KEY_KEYPAD_8, "8"},
    {HID_KEY_KEYPAD_9, "9"},
    {HID_KEY_KEYPAD_0, "0"},
    {HID_KEY_KEYPAD_DECIMAL, NULL},
    {HID_KEY_EUROPE_2, NULL},
    {HID_KEY_APPLICATION, NULL},
    {HID_KEY_POWER, NULL},
    {HID_KEY_KEYPAD_EQUAL, NULL},
    {HID_KEY_F13, NULL},
    {HID_KEY_F14, NULL},
    {HID_KEY_F15, NULL},
    {HID_KEY_F16, NULL},
    {HID_KEY_F17, NULL},
    {HID_KEY_F18, NULL},
    {HID_KEY_F19, NULL},
    {HID_KEY_F20, NULL},
    {HID_KEY_F21, NULL},
    {HID_KEY_F22, NULL},
    {HID_KEY_F23, NULL},
    {HID_KEY_F24, NULL},
    {HID_KEY_EXECUTE, NULL},
    {HID_KEY_HELP, NULL},
    {HID_KEY_MENU, NULL},
    {HID_KEY_SELECT, NULL},
    {HID_KEY_STOP, NULL},
    {HID_KEY_AGAIN, NULL},
    {HID_KEY_UNDO, NULL},
    {HID_KEY_CUT, NULL},
    {HID_KEY_COPY, NULL},
    {HID_KEY_PASTE, NULL},
    {HID_KEY_FIND, NULL},
    {HID_KEY_MUTE, NULL},
    {HID_KEY_VOLUME_UP, NULL},
    {HID_KEY_VOLUME_DOWN, NULL},
    {HID_KEY_LOCKING_CAPS_LOCK, NULL},
    {HID_KEY_LOCKING_NUM_LOCK, NULL},
    {HID_KEY_LOCKING_SCROLL_LOCK, NULL},
    {HID_KEY_KEYPAD_COMMA, NULL},
    {HID_KEY_KEYPAD_EQUAL_SIGN, NULL},
    {HID_KEY_KANJI1, NULL},
    {HID_KEY_KANJI2, NULL},
    {HID_KEY_KANJI3, NULL},
    {HID_KEY_KANJI4, NULL},
    {HID_KEY_KANJI5, NULL},
    {HID_KEY_KANJI6, NULL},
    {HID_KEY_KANJI7, NULL},
    {HID_KEY_KANJI8, NULL},
    {HID_KEY_KANJI9, NULL},
    {HID_KEY_LANG1, NULL},
    {HID_KEY_LANG2, NULL},
    {HID_KEY_LANG3, NULL},
    {HID_KEY_LANG4, NULL},
    {HID_KEY_LANG5, NULL},
    {HID_KEY_LANG6, NULL},
    {HID_KEY_LANG7, NULL},
    {HID_KEY_LANG8, NULL},
    {HID_KEY_LANG9, NULL},
    {HID_KEY_ALTERNATE_ERASE, NULL},
    {HID_KEY_SYSREQ_ATTENTION, NULL},
    {HID_KEY_CANCEL, NULL},
    {HID_KEY_CLEAR, NULL},
    {HID_KEY_PRIOR, NULL},
    {HID_KEY_RETURN, NULL},
    {HID_KEY_SEPARATOR, NULL},
    {HID_KEY_OUT, NULL},
    {HID_KEY_OPER, NULL},
    {HID_KEY_CLEAR_AGAIN, NULL},
    {HID_KEY_CRSEL_PROPS, NULL},
    {HID_KEY_EXSEL, NULL},
    // RESERVED                        0xA5-DF
    {HID_KEY_CONTROL_LEFT, "Ctrl"},
    {HID_KEY_SHIFT_LEFT, "Shift"},
    {HID_KEY_ALT_LEFT, "Alt"},
    {HID_KEY_GUI_LEFT, LV_SYMBOL_DRIVE},
    {HID_KEY_CONTROL_RIGHT, NULL},
    {HID_KEY_SHIFT_RIGHT, NULL},
    {HID_KEY_ALT_RIGHT, NULL},
    {HID_KEY_GUI_RIGHT, NULL},
};

static lv_obj_t *kb;

#define LV_KB_BTN(width) LV_BTNMATRIX_CTRL_POPOVER | width

static const char * default_kb_map_custom_lc[] = {
    "ESC", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "Del", "\n",
    "~", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", LV_SYMBOL_BACKSPACE, "\n",
    "Tab", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "\\", "\n",
    "ABC", "a", "s", "d", "f", "g", "h", "j", "k", "l", ":", "\"", LV_SYMBOL_NEW_LINE, "\n",
    "Shift", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", LV_SYMBOL_UP, "Shift", "\n",
    "Ctrl", LV_SYMBOL_DRIVE, "Alt", " ", "Alt", "Ctrl", LV_SYMBOL_LEFT, LV_SYMBOL_DOWN, LV_SYMBOL_RIGHT, ""
};

static const char * default_kb_map_custom_uc[] = {
    "ESC", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "Del", "\n",
    "~", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", LV_SYMBOL_BACKSPACE, "\n",
    "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "\\", "\n",
    "abc", "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\"", LV_SYMBOL_NEW_LINE, "\n",
    "Shift", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", LV_SYMBOL_UP, "Shift", "\n",
    "Ctrl", LV_SYMBOL_DRIVE, "Alt", " ", "Alt", "Ctrl", LV_SYMBOL_LEFT, LV_SYMBOL_DOWN, LV_SYMBOL_RIGHT, ""
};

static const lv_btnmatrix_ctrl_t default_kb_ctrl_map_custom[] = {

    LV_KEYBOARD_CTRL_BTN_FLAGS | 3, LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_BTNMATRIX_CTRL_CHECKED | 5,
    LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_BTNMATRIX_CTRL_CHECKED | 5,
    LV_KB_BTN(3), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1),
    LV_KB_BTN(3), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_BTNMATRIX_CTRL_CHECKED | 5,
    LV_KB_BTN(3), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1),
    LV_KB_BTN(3), LV_KEYBOARD_CTRL_BTN_FLAGS | 2, LV_KB_BTN(1), LV_KB_BTN(6), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1),
};

static uint8_t keycode[6] = { 0 };

static void ta_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (LV_EVENT_PRESSED == code) {
        lv_obj_t *obj = lv_event_get_target(e);
        uint16_t btn_id   = lv_btnmatrix_get_selected_btn(obj);
        if (btn_id == LV_BTNMATRIX_BTN_NONE) {
            return;
        }

        const char *txt = lv_btnmatrix_get_btn_text(obj, btn_id);
        if (txt == NULL) {
            return;
        }
        printf("t=%s\n", txt);
        if (strcmp(txt, "abc") == 0) {
            keycode[0] = HID_KEY_CAPS_LOCK;
        } else if (strcmp(txt, "ABC") == 0) {
            keycode[0] = HID_KEY_CAPS_LOCK;
        } else if (strcmp(txt, "Shift") == 0) {
            if (HID_KEY_SHIFT_LEFT != keycode[1]) {
                keycode[1] = HID_KEY_SHIFT_LEFT;
                lv_btnmatrix_set_btn_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_CHECKED);
            } else {
                lv_btnmatrix_clear_btn_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_CHECKED);
                keycode[1] = 0;
            }
        } else if (strcmp(txt, "Ctrl") == 0) {
            if (HID_KEY_CONTROL_LEFT != keycode[2]) {
                keycode[2] = HID_KEY_CONTROL_LEFT;
                lv_btnmatrix_set_btn_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_CHECKED);
            } else {
                lv_btnmatrix_clear_btn_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_CHECKED);
                keycode[2] = 0;
            }
        } else if (strcmp(txt, "Alt") == 0) {
            if (HID_KEY_ALT_LEFT != keycode[3]) {
                lv_btnmatrix_set_btn_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_CHECKED);
                keycode[3] = HID_KEY_ALT_LEFT;
            } else {
                lv_btnmatrix_clear_btn_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_CHECKED);
                keycode[3] = 0;
            }
        } else {
            char c[8] = {0};
            bool matched = 0;
            strncpy(c, txt, sizeof(c) - 1);
            if (c[0] >= 'a' && c[0] <= 'z') {
                c[0] -= 32; // to upper
            }

            for (size_t i = 0; i < sizeof(key_value) / sizeof(key_value_t); i++) {
                if (key_value[i].txt && strcmp(c, key_value[i].txt) == 0) {
                    keycode[0] = key_value[i].hidv;
                    matched = 1;
                    break;
                }
            }
            if (!matched) {
                keycode[0] = 0;
            }
        }
    } else {
        keycode[0] = 0;
    }

    // skip if hid is not ready yet
    if (tud_hid_ready() ) {
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
    }

}

static uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD( HID_REPORT_ID(REPORT_ID_KEYBOARD         )),
    TUD_HID_REPORT_DESC_MOUSE   ( HID_REPORT_ID(REPORT_ID_MOUSE            )),
    TUD_HID_REPORT_DESC_CONSUMER( HID_REPORT_ID(REPORT_ID_CONSUMER_CONTROL )),
    TUD_HID_REPORT_DESC_GAMEPAD ( HID_REPORT_ID(REPORT_ID_GAMEPAD          ))
};

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void) instance;
    return desc_hid_report;
}

void usb_keyboard_init(void)
{
    ESP_LOGI(TAG, "USB initialization");

    // Setting of descriptor. You can use descriptor_tinyusb and
    // descriptor_str_tinyusb as a reference
    tusb_desc_device_t my_descriptor = {
        .bLength            = sizeof(tusb_desc_device_t),
        .bDescriptorType    = TUSB_DESC_DEVICE,
        .bcdUSB             = 0x0200, // USB version. 0x0200 means version 2.0
        .bDeviceClass       = 0x00,
        .bDeviceSubClass    = 0x00,
        .bDeviceProtocol    = 0x00,
        .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

        .idVendor           = 0x303A,
        .idProduct          = 0x3000,
        .bcdDevice          = 0x0100,

        .iManufacturer      = 0x01,
        .iProduct           = 0x02,
        .iSerialNumber      = 0x03,

        .bNumConfigurations = 0x01
    };

    char const *my_string_descriptor[] = {
        // array of pointer to string descriptors
        (const char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
        "I",                  // 1: Manufacturer
        "My Custom Device",   // 2: Product
        "012-345",            // 3: Serials, should use chip ID
    };

    enum {
        ITF_NUM_HID,
        ITF_NUM_TOTAL
    };
#define  CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

#define EPNUM_HID   0x81
    uint8_t desc_configuration[] = {
        // Config number, interface count, string index, total length, attribute, power in mA
        TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

        // Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
        TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 5)
    };

    tinyusb_config_t tusb_cfg = {
        .descriptor = &my_descriptor,
        .configuration_descriptor = desc_configuration,
        .string_descriptor = my_string_descriptor,
        .external_phy = false // In the most cases you need to use a `false` value
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    ESP_LOGI(TAG, "USB initialization DONE");

    bsp_display_lock(0);
    kb = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(kb, LV_HOR_RES, LV_VER_RES);
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_USER_2, default_kb_map_custom_uc, default_kb_ctrl_map_custom);
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_USER_1, default_kb_map_custom_lc, default_kb_ctrl_map_custom);
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_USER_1);
    lv_obj_remove_event_cb(kb, lv_keyboard_def_event_cb);
    lv_obj_add_event_cb(kb, ta_event_cb, LV_EVENT_PRESSED, kb);
    lv_obj_add_event_cb(kb, ta_event_cb, LV_EVENT_RELEASED, kb);
    bsp_display_unlock();

    while (1) {
        // Remote wakeup
        if ( tud_suspended() && 1 ) {
            // Wake up host if we are in suspend mode
            // and REMOTE_WAKEUP feature is enabled by host
            tud_remote_wakeup();
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    ESP_LOGI(TAG, "%s", __FUNCTION__);
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    // TODO not Implemented
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
    (void) instance;

    if (report_type == HID_REPORT_TYPE_OUTPUT) {
        // Set keyboard LED e.g Capslock, Numlock etc...
        if (report_id == REPORT_ID_KEYBOARD) {
            // bufsize should be (at least) 1
            if ( bufsize < 1 ) {
                return;
            }

            uint8_t const kbd_leds = buffer[0];

            if (kbd_leds & KEYBOARD_LED_CAPSLOCK) {
                // Capslock On: disable blink, turn led on
                ESP_LOGI(TAG, "%s", "Capslock On");
                lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_USER_2);
                lv_btnmatrix_set_btn_ctrl(kb, 42, LV_BTNMATRIX_CTRL_CHECKED);
            } else {
                // Caplocks Off: back to normal blink
                ESP_LOGI(TAG, "%s", "Caplocks Off");
                lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_USER_1);
                lv_btnmatrix_clear_btn_ctrl(kb, 42, LV_BTNMATRIX_CTRL_CHECKED);
            }
        }
    }
}

