/* SPDX-FileCopyrightText: 2022-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_private/usb_phy.h"
#include "tinyusb_hid.h"
#include "usb_descriptors.h"
#include "device/usbd.h"
#include "bsp/esp_dualkey.h"
#include "via_keymap.h"

//--------------------------------------------------------------------+
// VIA Protocol Constants
//--------------------------------------------------------------------+
#define VIA_PROTOCOL_VERSION 0x000C
#define VIA_CMD_GET_PROTOCOL_VERSION 0x01
#define VIA_CMD_GET_KEYBOARD_VALUE 0x02
#define VIA_CMD_SET_KEYBOARD_VALUE 0x03
#define VIA_CMD_DYNAMIC_KEYMAP_GET_KEYCODE 0x04
#define VIA_CMD_DYNAMIC_KEYMAP_SET_KEYCODE 0x05
#define VIA_CMD_DYNAMIC_KEYMAP_RESET 0x06
#define VIA_CMD_CUSTOM_SET_VALUE 0x07
#define VIA_CMD_CUSTOM_GET_VALUE 0x08
#define VIA_CMD_CUSTOM_SAVE 0x09
#define VIA_CMD_EEPROM_RESET 0x0A
#define VIA_CMD_BOOTLOADER_JUMP 0x0B
#define VIA_CMD_DYNAMIC_KEYMAP_MACRO_GET_COUNT 0x0C
#define VIA_CMD_DYNAMIC_KEYMAP_MACRO_GET_BUFFER_SIZE 0x0D
#define VIA_CMD_DYNAMIC_KEYMAP_MACRO_GET_BUFFER 0x0E
#define VIA_CMD_DYNAMIC_KEYMAP_MACRO_SET_BUFFER 0x0F
#define VIA_CMD_DYNAMIC_KEYMAP_MACRO_RESET 0x10
#define VIA_CMD_DYNAMIC_KEYMAP_GET_LAYER_COUNT 0x11
#define VIA_CMD_DYNAMIC_KEYMAP_GET_BUFFER 0x12
#define VIA_CMD_DYNAMIC_KEYMAP_SET_BUFFER 0x13

static const char *TAG = "tinyusb_hid.h";

typedef struct {
    TaskHandle_t task_handle;
    QueueHandle_t hid_queue;
} tinyusb_hid_t;

static tinyusb_hid_t *s_tinyusb_hid = NULL;

//--------------------------------------------------------------------+
// USB PHY config
//--------------------------------------------------------------------+
static void usb_phy_init(void)
{
    usb_phy_handle_t phy_hdl;
    // Configure USB PHY
    usb_phy_config_t phy_conf = {
        .controller = USB_PHY_CTRL_OTG,
        .otg_mode = USB_OTG_MODE_DEVICE,
    };
    phy_conf.target = USB_PHY_TARGET_INT;
    usb_new_phy(&phy_conf, &phy_hdl);
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

static void tusb_device_task(void *arg)
{
    while (1) {
        tud_task();
    }
}

void tinyusb_hid_keyboard_report(hid_report_t report)
{
    static bool use_full_key = false;
    // Remote wakeup
    if (tud_suspended()) {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    } else {
        switch (report.report_id) {
        case REPORT_ID_FULL_KEY_KEYBOARD:
            use_full_key = true;
            break;
        case REPORT_ID_KEYBOARD: {
            if (use_full_key) {
                hid_report_t _report = {0};
                _report.report_id = REPORT_ID_FULL_KEY_KEYBOARD;
                xQueueSend(s_tinyusb_hid->hid_queue, &_report, 0);
                use_full_key = false;
            }
            break;
        }
        default:
            break;
        }

        xQueueSend(s_tinyusb_hid->hid_queue, &report, 0);
    }
}

void tinyusb_hid_mouse_report(hid_report_t report)
{
    // Remote wakeup
    if (tud_suspended()) {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    } else {
        xQueueSend(s_tinyusb_hid->hid_queue, &report, 0);
    }
}

// Function to send VIA Raw HID response
void tinyusb_hid_send_via_response(uint8_t *data, uint16_t len)
{
    if (len > 32) {
        len = 32;    // Limit to 32 bytes
    }

    // Send Input Report to VIA Raw HID interface (interface 1) - NO REPORT ID ( compatible)
    if (tud_hid_n_ready(1)) {
        bool sent = tud_hid_n_report(1, 0, data, len);  // Report ID = 0 (no report ID)
        if (sent) {
            ESP_LOGI(TAG, "VIA Raw HID response sent (no Report ID): %02X %02X %02X %02X...",
                     data[0], data[1], data[2], data[3]);
        } else {
            ESP_LOGE(TAG, "Failed to send VIA Raw HID response");
        }
    } else {
        ESP_LOGW(TAG, "VIA Raw HID interface not ready");
    }
}

// Public function to get keycode from VIA keymap
uint16_t tinyusb_hid_get_via_keycode(uint8_t layer, uint8_t row, uint8_t col)
{
    return via_keymap_get_keycode(layer, row, col);
}

// tinyusb_hid_task function to process the HID reports
static void tinyusb_hid_task(void *arg)
{
    (void) arg;
    hid_report_t report;
    while (1) {
        if (xQueueReceive(s_tinyusb_hid->hid_queue, &report, portMAX_DELAY)) {
            // Remote wakeup
            if (tud_suspended()) {
                // Wake up host if we are in suspend mode
                // and REMOTE_WAKEUP feature is enabled by host
                tud_remote_wakeup();
                xQueueReset(s_tinyusb_hid->hid_queue);
            } else {
                if (report.report_id == REPORT_ID_KEYBOARD) {
                    tud_hid_n_report(0, REPORT_ID_KEYBOARD, &report.keyboard_report, sizeof(report.keyboard_report));
                } else if (report.report_id == REPORT_ID_FULL_KEY_KEYBOARD) {
                    tud_hid_n_report(0, REPORT_ID_FULL_KEY_KEYBOARD, &report.keyboard_full_key_report, sizeof(report.keyboard_full_key_report));
                } else if (report.report_id == REPORT_ID_CONSUMER) {
                    tud_hid_n_report(0, REPORT_ID_CONSUMER, &report.consumer_report, sizeof(report.consumer_report));
                } else if (report.report_id == REPORT_ID_MOUSE) {
                    tud_hid_n_report(0, REPORT_ID_MOUSE, &report.mouse_report, sizeof(report.mouse_report));
                } else if (report.report_id == REPORT_ID_VIA_RAW) {
                    tud_hid_n_report(0, REPORT_ID_VIA_RAW, &report.via_raw_report, sizeof(report.via_raw_report));
                } else {
                    // Unknown report
                    continue;
                }
                // Wait until report is sent
                if (!ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100))) {
                    ESP_LOGW(TAG, "Report not sent");
                }
            }
        }
    }
}

esp_err_t tinyusb_hid_init(void)
{
    if (s_tinyusb_hid) {
        ESP_LOGW(TAG, "tinyusb_hid already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing TinyUSB with dual HID interfaces...");
    ESP_LOGI(TAG, "Interface 0: Standard keyboard (Report IDs 1-3)");
    ESP_LOGI(TAG, "Interface 1: VIA Raw HID (Report ID %d)", REPORT_ID_VIA_RAW);

    esp_err_t ret = ESP_OK;
    s_tinyusb_hid = calloc(1, sizeof(tinyusb_hid_t));
    ESP_RETURN_ON_FALSE(s_tinyusb_hid, ESP_ERR_NO_MEM, TAG, "calloc failed");

    usb_phy_init();
    tud_init(BOARD_TUD_RHPORT);

    s_tinyusb_hid->hid_queue = xQueueCreate(10, sizeof(hid_report_t));   // Adjust queue length and item size as per your requirement
    ESP_GOTO_ON_FALSE(s_tinyusb_hid->hid_queue, ESP_ERR_NO_MEM, fail, TAG, "xQueueCreate failed");
    xTaskCreate(tusb_device_task, "TinyUSB", 4096, NULL, 5, NULL);
    xTaskCreate(tinyusb_hid_task, "tinyusb_hid_task", 4096, NULL, 5, &s_tinyusb_hid->task_handle);
    xTaskNotifyGive(s_tinyusb_hid->task_handle);

    ESP_LOGI(TAG, "TinyUSB HID initialization complete - Ready for VIA communication");
    return ret;
fail:
    free(s_tinyusb_hid);
    s_tinyusb_hid = NULL;
    return ret;
}

/************************************************** TinyUSB callbacks ***********************************************/
// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t itf, uint8_t const *report, uint16_t len)
{
    if (len > 0) {
        ESP_LOGI(TAG, "Report sent: itf=%d, report_id=%d, len=%d", itf, report[0], len);
        if (itf == 1 && report[0] == REPORT_ID_VIA_RAW) {
            ESP_LOGI(TAG, "VIA response sent successfully to interface 1");
        } else if (itf == 0) {
            ESP_LOGI(TAG, "Standard keyboard report sent successfully");
        }
    } else {
        ESP_LOGI(TAG, "Report sent: itf=%d, len=%d", itf, len);
    }

    xTaskNotifyGive(s_tinyusb_hid->task_handle);
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    ESP_LOGI(TAG, "GET_REPORT: itf=%d, report_id=%d, type=%d, reqlen=%d",
             itf, report_id, report_type, reqlen);

    if (itf == 1) {  // VIA Raw HID interface
        // VIA uses Input/Output reports, not Feature reports for communication
        ESP_LOGI(TAG, "VIA GET_REPORT: Interface 1, use Input reports via tud_hid_n_report()");
    } else if (itf == 0) {  // Standard keyboard interface
        ESP_LOGI(TAG, "Standard keyboard GET_REPORT request");
        // Handle standard keyboard reports if needed
    }

    ESP_LOGW(TAG, "GET_REPORT: Unsupported request itf=%d, report_id=%d, type=%d", itf, report_id, report_type);
    return 0;
}

// Forward declaration of VIA handler function
static void handle_via_command(uint8_t const *buffer, uint16_t bufsize);

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
    ESP_LOGI(TAG, "SET_REPORT: itf=%d, report_id=%d, type=%d, size=%d",
             itf, report_id, report_type, bufsize);

    // Log first few bytes for debugging
    if (bufsize > 0) {
        ESP_LOGI(TAG, "SET_REPORT data: %02X %02X %02X %02X...",
                 buffer[0], bufsize > 1 ? buffer[1] : 0,
                 bufsize > 2 ? buffer[2] : 0, bufsize > 3 ? buffer[3] : 0);
    }

    if (itf == 1) {  // VIA Raw HID interface
        if (report_type == HID_REPORT_TYPE_OUTPUT && bufsize >= 32) {
            ESP_LOGI(TAG, "Processing VIA Raw HID output report (32 bytes) -  compatible");
            handle_via_command(buffer, bufsize);
        } else if (report_id == 0 && bufsize >= 32) {  //  compatible - no report ID
            ESP_LOGI(TAG, "Processing VIA Raw HID report without ID -  compatible");
            handle_via_command(buffer, bufsize);
        } else {
            ESP_LOGW(TAG, "VIA interface: invalid report type=%d or size=%d", report_type, bufsize);
        }
    } else if (itf == 0) {  // Standard keyboard interface
        if (bufsize > 0) {
            // Handle keyboard LED states (CAPLOCK, NUMLOCK etc...)
            ESP_LOGI(TAG, "Keyboard LED state: 0x%02X", buffer[0]);
        }
    } else {
        ESP_LOGW(TAG, "Unhandled SET_REPORT: itf=%d, report_id=%d, size=%d", itf, report_id, bufsize);
    }
}

// Invoked when device is mounted
void tud_mount_cb(void)
{
    ESP_LOGI(TAG, "USB Mount - Dual HID interfaces ready:");
    ESP_LOGI(TAG, "  Interface 0: Standard keyboard");
    ESP_LOGI(TAG, "  Interface 1: VIA Raw HID (Usage Page 0xFF60)");
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    ESP_LOGI(TAG, "USB Un-Mount");
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    ESP_LOGI(TAG, "USB Suspend");
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    ESP_LOGI(TAG, "USB Resume");
}

//--------------------------------------------------------------------+
// VIA Protocol Implementation
//--------------------------------------------------------------------+

// Basic VIA command handler
static void handle_via_command(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < 1) {
        return;
    }

    uint8_t response[32] = {0};
    uint8_t command = buffer[0];

    ESP_LOGI(TAG, "VIA Command received: 0x%02X", command);

    switch (command) {
    case VIA_CMD_GET_PROTOCOL_VERSION:
        // Return protocol version
        response[0] = VIA_CMD_GET_PROTOCOL_VERSION;
        response[1] = (VIA_PROTOCOL_VERSION >> 8) & 0xFF;  // High byte
        response[2] = VIA_PROTOCOL_VERSION & 0xFF;         // Low byte
        ESP_LOGI(TAG, "VIA: Get protocol version -> 0x%04X", VIA_PROTOCOL_VERSION);
        break;

    case VIA_CMD_GET_KEYBOARD_VALUE:
        if (bufsize >= 2) {
            uint8_t value_id = buffer[1];
            response[0] = VIA_CMD_GET_KEYBOARD_VALUE;
            response[1] = value_id;

            switch (value_id) {
            case 0x01: // Matrix size
                response[2] = VIA_MATRIX_ROWS;
                response[3] = VIA_MATRIX_COLS;
                ESP_LOGI(TAG, "VIA: Get matrix size -> %dx%d", VIA_MATRIX_ROWS, VIA_MATRIX_COLS);
                break;
            case 0x02: // Layer count
                response[2] = VIA_MAX_LAYERS;
                ESP_LOGI(TAG, "VIA: Get layer count -> %d", VIA_MAX_LAYERS);
                break;
            default:
                response[2] = 0x00; // Default value
                ESP_LOGI(TAG, "VIA: Get keyboard value ID=%d (unknown)", value_id);
                break;
            }
        }
        break;

    case VIA_CMD_SET_KEYBOARD_VALUE:
        if (bufsize >= 3) {
            uint8_t value_id = buffer[1];
            uint8_t value = buffer[2];
            response[0] = VIA_CMD_SET_KEYBOARD_VALUE;
            response[1] = value_id;
            response[2] = value;
            ESP_LOGI(TAG, "VIA: Set keyboard value ID=%d, value=%d", value_id, value);
        }
        break;

    case VIA_CMD_DYNAMIC_KEYMAP_GET_KEYCODE:
        if (bufsize >= 4) {
            uint8_t layer = buffer[1];
            uint8_t row = buffer[2];
            uint8_t col = buffer[3];

            // Get real keycode from keymap
            uint16_t keycode = via_keymap_get_keycode(layer, row, col);

            response[0] = VIA_CMD_DYNAMIC_KEYMAP_GET_KEYCODE;
            response[1] = layer;
            response[2] = row;
            response[3] = col;
            response[4] = (keycode >> 8) & 0xFF; // Keycode high byte
            response[5] = keycode & 0xFF;        // Keycode low byte

            ESP_LOGI(TAG, "VIA: Get keycode L%d R%d C%d -> 0x%04X", layer, row, col, keycode);
        }
        break;

    case VIA_CMD_DYNAMIC_KEYMAP_SET_KEYCODE:
        if (bufsize >= 6) {
            uint8_t layer = buffer[1];
            uint8_t row = buffer[2];
            uint8_t col = buffer[3];
            uint16_t keycode = (buffer[4] << 8) | buffer[5];

            // Set real keycode in keymap and save to NVS
            esp_err_t ret = via_keymap_set_keycode(layer, row, col, keycode);

            response[0] = VIA_CMD_DYNAMIC_KEYMAP_SET_KEYCODE;
            response[1] = layer;
            response[2] = row;
            response[3] = col;
            response[4] = buffer[4];
            response[5] = buffer[5];

            if (ret == ESP_OK) {
                ESP_LOGI(TAG, "VIA: Set keycode L%d R%d C%d = 0x%04X (saved to NVS)",
                         layer, row, col, keycode);
            } else {
                ESP_LOGE(TAG, "VIA: Failed to set keycode L%d R%d C%d = 0x%04X (%s)",
                         layer, row, col, keycode, esp_err_to_name(ret));
            }
        }
        break;

    case VIA_CMD_DYNAMIC_KEYMAP_MACRO_GET_COUNT:
        // Return macro count (0 = no macros supported)
        response[0] = VIA_CMD_DYNAMIC_KEYMAP_MACRO_GET_COUNT;
        response[1] = 0; // Macro count = 0
        ESP_LOGI(TAG, "VIA: Get macro count -> 0 (not supported)");
        break;

    case VIA_CMD_DYNAMIC_KEYMAP_MACRO_GET_BUFFER_SIZE:
        // Return macro buffer size (0 = no macros supported)
        response[0] = VIA_CMD_DYNAMIC_KEYMAP_MACRO_GET_BUFFER_SIZE;
        response[1] = 0; // Buffer size high byte
        response[2] = 0; // Buffer size low byte
        ESP_LOGI(TAG, "VIA: Get macro buffer size -> 0 (not supported)");
        break;

    case VIA_CMD_DYNAMIC_KEYMAP_GET_LAYER_COUNT:
        // Return layer count
        response[0] = VIA_CMD_DYNAMIC_KEYMAP_GET_LAYER_COUNT;
        response[1] = VIA_MAX_LAYERS;
        ESP_LOGI(TAG, "VIA: Get layer count -> %d", VIA_MAX_LAYERS);
        break;

    case VIA_CMD_DYNAMIC_KEYMAP_RESET:
        // Reset keymaps to default
        response[0] = VIA_CMD_DYNAMIC_KEYMAP_RESET;
        via_keymap_reset(); // Reset to default keymaps and save to NVS
        ESP_LOGI(TAG, "VIA: Reset dynamic keymap");
        break;

    case VIA_CMD_DYNAMIC_KEYMAP_GET_BUFFER:
        if (bufsize >= 4) {
            uint16_t offset = (buffer[1] << 8) | buffer[2];
            uint8_t size = buffer[3];

            response[0] = VIA_CMD_DYNAMIC_KEYMAP_GET_BUFFER;
            response[1] = buffer[1]; // offset high
            response[2] = buffer[2]; // offset low
            response[3] = size;      // size

            // Fill keymap data starting from response[4]
            // VIA keymap format: layer-by-layer, row-by-row, col-by-col, 2 bytes per keycode
            for (uint8_t i = 0; i < size && (4 + i) < 32; i++) {
                uint16_t pos = (offset + i) / 2; // Position in keymap (keycode index)
                uint8_t byte_index = (offset + i) % 2; // High or low byte of keycode

                // Calculate layer, row, col from position
                uint8_t layer = pos / (VIA_MATRIX_ROWS * VIA_MATRIX_COLS);
                uint8_t remaining = pos % (VIA_MATRIX_ROWS * VIA_MATRIX_COLS);
                uint8_t row = remaining / VIA_MATRIX_COLS;
                uint8_t col = remaining % VIA_MATRIX_COLS;

                if (layer < VIA_MAX_LAYERS) {
                    uint16_t keycode = via_keymap_get_keycode(layer, row, col);
                    if (byte_index == 0) {
                        response[4 + i] = (keycode >> 8) & 0xFF; // High byte
                    } else {
                        response[4 + i] = keycode & 0xFF;        // Low byte
                    }
                } else {
                    response[4 + i] = 0x00; // Invalid position
                }
            }

            ESP_LOGI(TAG, "VIA: Get keymap buffer offset=%d, size=%d", offset, size);
        }
        break;

    case VIA_CMD_DYNAMIC_KEYMAP_SET_BUFFER:
        if (bufsize >= 4) {
            uint16_t offset = (buffer[1] << 8) | buffer[2];
            uint8_t size = buffer[3];

            response[0] = VIA_CMD_DYNAMIC_KEYMAP_SET_BUFFER;
            response[1] = buffer[1]; // offset high
            response[2] = buffer[2]; // offset low
            response[3] = size;      // size

            // Set keymap data from buffer[4] onwards
            for (uint8_t i = 0; i < size && (4 + i) < bufsize; i++) {
                uint16_t pos = (offset + i) / 2; // Position in keymap (keycode index)
                uint8_t byte_index = (offset + i) % 2; // High or low byte of keycode

                // Calculate layer, row, col from position
                uint8_t layer = pos / (VIA_MATRIX_ROWS * VIA_MATRIX_COLS);
                uint8_t remaining = pos % (VIA_MATRIX_ROWS * VIA_MATRIX_COLS);
                uint8_t row = remaining / VIA_MATRIX_COLS;
                uint8_t col = remaining % VIA_MATRIX_COLS;

                if (layer < VIA_MAX_LAYERS && via_keymap_is_valid_position(layer, row, col)) {
                    static uint16_t temp_keycode = 0;
                    if (byte_index == 0) {
                        temp_keycode = (buffer[4 + i] << 8); // High byte
                    } else {
                        temp_keycode |= buffer[4 + i];       // Low byte
                        via_keymap_set_keycode(layer, row, col, temp_keycode);
                    }
                }
            }

            ESP_LOGI(TAG, "VIA: Set keymap buffer offset=%d, size=%d", offset, size);
        }
        break;

    default:
        ESP_LOGW(TAG, "VIA: Unknown command 0x%02X", command);
        response[0] = command;
        break;
    }

    // Send response back to host
    tinyusb_hid_send_via_response(response, 32);
}
