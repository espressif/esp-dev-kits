/* SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "nvs.h"
#include "nvs_flash.h"
#include "inttypes.h"
#include "app_io.h"
#include "app_mode.h"
#include "app_espnow.h"
#include "app_helper.h"
#include "app_serial.h"
#include "app_indicator.h"
#include "app_tusb.h"
#include "app_msc.h"
#include "app_jtag.h"
#include "driver/uart.h"
#include "espnow.h"
#include "esp_log.h"
#include "esp_system.h"

static const char* TAG = "app_mode";
static usb_bridge_mode_t *q_usb_bridge_mode = NULL;

const char* work_mode_str[] = {
    "MODE_WIRED",
    "MODE_WIRELESS_HOST",
    "MODE_WIRELESS_SLAVE",
    "MODE_MAX",
};

/******************************* WIRED MODE ********************************************/
static esp_err_t mode_wired_write_cdc_rx_data(const void *data, size_t size)
{
    const int transferred = uart_write_bytes(SLAVE_UART_NUM, data, size);
    ERROR_CHECK(transferred == size, "uart_write_bytes transferred bytes error", ESP_FAIL);
    return ESP_OK;
}

static esp_err_t mode_wired_set_baudrate(const int baudrate)
{
    bool if_ok = serial_set_baudrate(baudrate);
    ERROR_CHECK( true == if_ok, "serial_set_baudrate failed", ESP_FAIL);
    return ESP_OK;
}

static esp_err_t mode_wired_set_boot_reset(const bool dtr, const bool rts)
{
    serial_boot_reset(dtr, rts);
    return ESP_OK;
}

static esp_err_t mode_wired_led_update_mode(const led_indicator_mode_type mode_type, const bool if_start)
{
    /* Only support blink mode */
    return app_indicator_update_mode(q_usb_bridge_mode->hue, BLINK_MODE, if_start);
}

static esp_err_t mode_wired_init(void)
{
    ERROR_CHECK(NULL != q_usb_bridge_mode, "usb bridge already create", ESP_ERR_INVALID_ARG);
    ERROR_CHECK(ESP_OK == app_indicator_init(q_usb_bridge_mode->hue), "led indicator init failed", ESP_FAIL);
    ERROR_CHECK(ESP_OK == q_usb_bridge_mode->led_update_mode(BLINK_WIRELESS_DISCONNECTED, true), "led indicator update mode failed", ESP_FAIL);
    
    app_tusb_init();
    msc_init();
    RingbufHandle_t ringbuf = NULL;
    q_usb_bridge_mode->serial_init(&ringbuf);
    jtag_init();
    ESP_LOGI(TAG,"Current mode: %s", work_mode_str[q_usb_bridge_mode->mode]);
    return ESP_OK;
}

/******************************* WIRELESS MODE ********************************************/
static esp_err_t mode_wireless_led_update_mode(const led_indicator_mode_type mode_type, const bool if_start)
{
    return app_indicator_update_mode(q_usb_bridge_mode->hue, mode_type, if_start);
}

/******************************* WIRELESS HOST MODE ********************************************/
static esp_err_t mode_wireless_host_write_cdc_rx_data(const void *data, size_t size)
{
    return app_espnow_send(ESPNOW_DATA_TYPE_DEBUG_LOG, data, size, NULL, portMAX_DELAY);
}

static esp_err_t mode_wireless_host_set_baudrate(const int baudrate)
{
    espnow_debug_command_frame_t frame = {
        .tud_cdc_action = LINE_CODING,
        .baudrate = baudrate,
    };
    return app_espnow_send(ESPNOW_DATA_TYPE_DEBUG_COMMAND, &frame, sizeof(espnow_debug_command_frame_t), NULL, portMAX_DELAY);
}

static esp_err_t mode_wireless_host_set_boot_reset(const bool dtr, const bool rts)
{
    espnow_debug_command_frame_t frame = {0};
    frame.tud_cdc_action = LINE_STATE;
    frame.state[0] = dtr;
    frame.state[1] = rts;
    return app_espnow_send(ESPNOW_DATA_TYPE_DEBUG_COMMAND, &frame, sizeof(espnow_debug_command_frame_t), NULL, portMAX_DELAY);
}

static esp_err_t mode_wireless_host_init(void)
{
    ERROR_CHECK(NULL != q_usb_bridge_mode, "usb bridge already create", ESP_ERR_INVALID_ARG);
    ERROR_CHECK(ESP_OK == app_indicator_init(q_usb_bridge_mode->hue), "led indicator init failed", ESP_FAIL);
    ERROR_CHECK(ESP_OK == q_usb_bridge_mode->led_update_mode(BLINK_WIRELESS_DISCONNECTED, true), "led indicator update mode failed", ESP_FAIL);
    app_tusb_init();
    msc_init();
    RingbufHandle_t ringbuf = NULL;
    q_usb_bridge_mode->serial_init(&ringbuf);
    ERROR_CHECK(NULL != ringbuf, "serial init failed", ESP_FAIL);
    q_usb_bridge_mode->espnow_int(ringbuf);
    jtag_init();
    ESP_LOGI(TAG,"Current mode: %s", work_mode_str[q_usb_bridge_mode->mode]);
    return ESP_OK;
}

/******************************* WIRELESS SLAVE MODE ********************************************/
static esp_err_t mode_wireless_slave_set_baudrate(const int baudrate)
{
    bool if_ok = serial_set_baudrate(baudrate);
    ERROR_CHECK(true == if_ok, "serial_set_baudrate failed", ESP_FAIL);
    return ESP_OK;
}

static esp_err_t mode_wireless_slave_set_boot_reset(const bool dtr, const bool rts)
{
    serial_boot_reset(dtr, rts);
    return ESP_OK;
}

static esp_err_t mode_wireless_slave_init(void)
{
    ERROR_CHECK(NULL != q_usb_bridge_mode, "usb bridge already create", ESP_ERR_INVALID_ARG);
    ERROR_CHECK(ESP_OK == app_indicator_init(q_usb_bridge_mode->hue), "led indicator init failed", ESP_FAIL);
    ERROR_CHECK(ESP_OK == q_usb_bridge_mode->led_update_mode(BLINK_WIRELESS_DISCONNECTED, true), "led indicator update mode failed", ESP_FAIL);

    RingbufHandle_t ringbuf = NULL;
    q_usb_bridge_mode->serial_init(&ringbuf);
    ERROR_CHECK(NULL != ringbuf, "serial init failed", ESP_FAIL);
    q_usb_bridge_mode->espnow_int(ringbuf);
    ESP_LOGI(TAG,"Current mode: %s", work_mode_str[q_usb_bridge_mode->mode]);
    return ESP_OK;
}

/******************************* APP MODE API ********************************************/
usb_bridge_mode_t *usb_bridge_get_handle(void)
{
    return q_usb_bridge_mode;
}

usb_bridge_mode_t *usb_bridge_mode_create(work_mode_t mode)
{
    ERROR_CHECK(NULL == q_usb_bridge_mode, "usb bridge already create", NULL);
    usb_bridge_mode_t *usb_bridge_mode = (usb_bridge_mode_t *)calloc(1, sizeof(usb_bridge_mode_t));
    q_usb_bridge_mode = usb_bridge_mode;
    ERROR_CHECK(NULL != usb_bridge_mode, "usb_bridge_mode create failed", NULL);
    usb_bridge_mode->mode = mode;
    switch (mode) {
        case MODE_WIRED:
            usb_bridge_mode->hue = MODE_WIRED_HUE;
            usb_bridge_mode->write_cdc_rx_data = &mode_wired_write_cdc_rx_data;
            usb_bridge_mode->set_baudrate = &mode_wired_set_baudrate;
            usb_bridge_mode->set_boot_reset = &mode_wired_set_boot_reset;
            usb_bridge_mode->led_update_mode = &mode_wired_led_update_mode;
            usb_bridge_mode->init = &mode_wired_init;
            usb_bridge_mode->serial_init = &mode_wired_serial_init;
            usb_bridge_mode->espnow_int = NULL;
            usb_bridge_mode->espnow_bind_begin = NULL;
            usb_bridge_mode->espnow_debug_process = NULL;
            break;
        case MODE_WIRELESS_HOST:
            usb_bridge_mode->hue = MODE_WIRELESS_HOST_HUE;
            usb_bridge_mode->write_cdc_rx_data = &mode_wireless_host_write_cdc_rx_data;
            usb_bridge_mode->set_baudrate = &mode_wireless_host_set_baudrate;
            usb_bridge_mode->set_boot_reset = &mode_wireless_host_set_boot_reset;
            usb_bridge_mode->led_update_mode = &mode_wireless_led_update_mode;
            usb_bridge_mode->init = &mode_wireless_host_init;
            usb_bridge_mode->serial_init = &mode_wireless_host_serial_init;
            usb_bridge_mode->espnow_int = &mode_wireless_host_espnow_create;
            usb_bridge_mode->espnow_bind_begin = &mode_wireless_espnow_bind_begin;
            usb_bridge_mode->espnow_debug_process = &mode_wireless_host_debug_process;
            break;
        case MODE_WIRELESS_SLAVE:
            usb_bridge_mode->hue = MODE_WIRELESS_SLAVE_HUE;
            usb_bridge_mode->write_cdc_rx_data = NULL;
            usb_bridge_mode->set_baudrate = &mode_wireless_slave_set_baudrate;
            usb_bridge_mode->set_boot_reset = &mode_wireless_slave_set_boot_reset;
            usb_bridge_mode->led_update_mode = &mode_wireless_led_update_mode;
            usb_bridge_mode->init = &mode_wireless_slave_init;
            usb_bridge_mode->serial_init = &mode_wireless_slave_serial_init;
            usb_bridge_mode->espnow_int = &mode_wireless_slave_espnow_create;
            usb_bridge_mode->espnow_bind_begin = &mode_wireless_espnow_bind_begin;
            usb_bridge_mode->espnow_debug_process = &mode_wireless_slave_debug_process;
            break;
        default:
            break;
    }
    return usb_bridge_mode;
}