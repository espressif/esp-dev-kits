/* SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "app_serial.h"
#include "app_mode.h"
#include "app_espnow.h"
#include "app_helper.h"
#include "ctrl.h"
#include "driver/uart.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "espnow.h"
#include "espnow_storage.h"

static const char* TAG = "app_espnow";
static RingbufHandle_t sendbuf = NULL;
static usb_bridge_mode_t *q_usb_bridge_mode = NULL;

work_mode_t work_mode = MODE_WIRED;

espnow_ctrl_bind_info_t bind_info = {0}; 

static void _wifi_init()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void espnow_bind_task(void *pvParameters)
{
    while (timer_is_active()) {
        espnow_ctrl_initiator_bind(ESPNOW_ATTRIBUTE_KEY_1, true);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    vTaskDelete(NULL);
}

static void espnow_sender_task(void *pvParameters)
{
    while (1) {
        size_t ringbuf_received;
        uint8_t *buf = (uint8_t *) xRingbufferReceiveUpTo(sendbuf, &ringbuf_received, pdMS_TO_TICKS(100),
                       ESPNOW_DATA_LEN);

        if (buf) {
            uint8_t int_buf[ESPNOW_DATA_LEN];
            memcpy(int_buf, buf, ringbuf_received);
            vRingbufferReturnItem(sendbuf, (void *) buf);
            app_espnow_send(ESPNOW_DATA_TYPE_DEBUG_LOG, int_buf, ringbuf_received, NULL, portMAX_DELAY);
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
    }
    vTaskDelete(NULL);
}

esp_err_t mode_wireless_host_debug_process(uint8_t *data, size_t size)
{
    if (xRingbufferSend(sendbuf, data, size, pdMS_TO_TICKS(10)) != pdTRUE) {
        ESP_LOGW(TAG, "Cannot write to ringbuffer (free %d of %d)!", xRingbufferGetCurFreeSize(sendbuf), USB_SEND_RINGBUFFER_SIZE);
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t mode_wireless_slave_debug_process(uint8_t *data, size_t size)
{
    const int transferred = uart_write_bytes(SLAVE_UART_NUM, data, size);
    if (transferred != size) {
        ESP_LOGW(TAG, "uart_write_bytes transferred %d bytes only!", transferred);
        return ESP_FAIL;
    }
    return ESP_OK;
}

static esp_err_t _espnow_debug_recv_process(uint8_t *src_addr, void *data, size_t size, wifi_pkt_rx_ctrl_t *rx_ctrl)
{
    ESP_PARAM_CHECK(src_addr);
    ESP_PARAM_CHECK(data);
    ESP_PARAM_CHECK(size);
    ESP_PARAM_CHECK(rx_ctrl);
    uint8_t *recv_data = (uint8_t *)data;

    if (memcmp(src_addr, q_usb_bridge_mode->bind_mac ,6) != 0) {
        ESP_LOGD(TAG,"Received data from an unknown sender!");
        return ESP_OK;
    }

    if (q_usb_bridge_mode->espnow_debug_process) {
        q_usb_bridge_mode->espnow_debug_process(recv_data, size);
    }

    return ESP_OK;
}

static esp_err_t _espnow_debug_command_recv_process(uint8_t *src_addr, void *data, size_t size, wifi_pkt_rx_ctrl_t *rx_ctrl)
{
    ESP_PARAM_CHECK(src_addr);
    ESP_PARAM_CHECK(data);
    ESP_PARAM_CHECK(size);
    ESP_PARAM_CHECK(rx_ctrl);
    espnow_debug_command_frame_t *recv_data = (espnow_debug_command_frame_t *)data;

    if (memcmp(src_addr, q_usb_bridge_mode->bind_mac ,6) != 0) {
        ESP_LOGD(TAG,"Received data from an unknown sender!");
        return ESP_OK;
    }

    if (recv_data->tud_cdc_action == LINE_CODING) {
        if (q_usb_bridge_mode->set_baudrate) {
            q_usb_bridge_mode->set_baudrate(recv_data->baudrate);
        }
    } else if (recv_data->tud_cdc_action == LINE_STATE) {
        bool dtr = recv_data->state[0];
        bool rts = recv_data->state[1];
        if (q_usb_bridge_mode->set_boot_reset) {
            q_usb_bridge_mode->set_boot_reset(dtr, rts);
        }
    }

    return ESP_OK;
}

esp_err_t app_espnow_send(espnow_data_type_t type, const void *data, size_t size, const espnow_frame_head_t *frame_config, TickType_t wait_ticks)
{
    if (!q_usb_bridge_mode->if_bind) {
        return ESP_FAIL;
    }
    return espnow_send(type, q_usb_bridge_mode->bind_mac, data, size, frame_config, wait_ticks);
}

bool app_espnow_check_if_bind(void)
{
    size_t size = 0;
    ctrl_responder_get_bindlist(NULL, &size);
    if (size == 1) {
        return true;
    }
    return false;
}

esp_err_t app_espnow_get_bind_mac(uint8_t *mac)
{
    if (app_espnow_check_if_bind())
    {
        espnow_ctrl_bind_info_t list;
        size_t size = 1;
        ctrl_responder_get_bindlist(&list, &size);
        memcpy(mac, list.mac, 6);
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t app_espnow_remove_bind(void)
{
    size_t size = 0;
    ctrl_responder_get_bindlist(NULL, &size);
    if (size > 1) {
        ESP_LOGE(TAG,"size can't larger than 1");
        return ESP_FAIL;
    } else if (size == 0) {
        ESP_LOGI(TAG,"No bind list!");
        return ESP_FAIL;
    } else {
        espnow_ctrl_bind_info_t list;
        ctrl_responder_get_bindlist(&list, &size);
        ESP_LOGI(TAG,"Remove bind list, uuid: "MACSTR", initiator_type: %d", MAC2STR(list.mac), list.initiator_attribute);
        ctrl_responder_remove_bindlist(&list);
        espnow_ctrl_initiator_bind(ESPNOW_ATTRIBUTE_KEY_1, false);
    }
    return ESP_OK;
}

static void app_espnow_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    if (base != ESP_EVENT_ESPNOW) {
        return;
    }

    switch (id) {
    case ESP_EVENT_ESPNOW_CTRL_BIND: {
        espnow_ctrl_bind_info_t *info = (espnow_ctrl_bind_info_t *)event_data;
        ESP_LOGI(TAG, "bind, uuid: " MACSTR ", initiator_type: %d", MAC2STR(info->mac), info->initiator_attribute);
        q_usb_bridge_mode->if_bind = true;
        q_usb_bridge_mode->led_update_mode(BLINK_WIRELESS_DISCONNECTED, false);
        q_usb_bridge_mode->led_update_mode(BLINK_WIRELESS_CONNECTING, false);
        q_usb_bridge_mode->led_update_mode(BLINK_MODE, true);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        timer_stop();
        memcpy(q_usb_bridge_mode->bind_mac, info->mac, 6);
        break;
    }

    case ESP_EVENT_ESPNOW_CTRL_UNBIND: {
        espnow_ctrl_bind_info_t *info = (espnow_ctrl_bind_info_t *)event_data;
        ESP_LOGI(TAG, "unbind, uuid: "MACSTR", initiator_type: %d", MAC2STR(info->mac), info->initiator_attribute);
        q_usb_bridge_mode->if_bind = false;
        memset(q_usb_bridge_mode->bind_mac, 0, 6);
        ERROR_CHECK_RETURN_VOID(ESP_OK == q_usb_bridge_mode->led_update_mode(BLINK_MODE, false), "led_indicator_update_mode failed");
        ERROR_CHECK_RETURN_VOID(ESP_OK == q_usb_bridge_mode->led_update_mode(BLINK_WIRELESS_DISCONNECTED, true), "led_indicator_update_mode failed");
        break;
    }

    default:
        break;
    }
}

void app_espnow_init(void)
{
    _wifi_init();
    espnow_config_t espnow_config = ESPNOW_INIT_CONFIG_DEFAULT();
    espnow_config.qsize = 128;
    espnow_init(&espnow_config);
    q_usb_bridge_mode = usb_bridge_get_handle();
    ctrl_responder_init();
    ERROR_CHECK_RETURN_VOID(NULL != q_usb_bridge_mode, "q_usb_bridge_mode can't be NULL");
    if (app_espnow_check_if_bind()) {
        q_usb_bridge_mode->if_bind = true;
        q_usb_bridge_mode->led_update_mode(BLINK_WIRELESS_DISCONNECTED, false);
        q_usb_bridge_mode->led_update_mode(BLINK_MODE, true);
        ERROR_CHECK_RETURN_VOID(ESP_OK == app_espnow_get_bind_mac(q_usb_bridge_mode->bind_mac), "app_espnow_get_bind_mac failed");
        ESP_LOGI(TAG,"Already bind MAC addr: "MACSTR"", MAC2STR(q_usb_bridge_mode->bind_mac));
    }
    esp_event_handler_register(ESP_EVENT_ESPNOW, ESP_EVENT_ANY_ID, app_espnow_event_handler, NULL);
}

esp_err_t mode_wireless_espnow_bind_begin(uint32_t wait_ms, int8_t rssi)
{
    ctrl_responder_bind_window_set(wait_ms, rssi);
    xTaskCreate(espnow_bind_task, "espnow_bind_task", 3 * 1024, NULL, 5, NULL);
    return ESP_OK;
}

esp_err_t mode_wireless_host_espnow_create(RingbufHandle_t ringBuf)
{
    app_espnow_init();
    ERROR_CHECK(NULL != ringBuf, "ringBuf can't be NULL", ESP_FAIL);
    sendbuf = ringBuf;
    ERROR_CHECK(ESP_OK == espnow_set_config_for_data_type(ESPNOW_DATA_TYPE_DEBUG_LOG, true, _espnow_debug_recv_process), "espnow_set_config_for_data_type failed", ESP_FAIL);
    return ESP_OK;
}

esp_err_t mode_wireless_slave_espnow_create(RingbufHandle_t ringBuf)
{
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    app_espnow_init();
    ERROR_CHECK(NULL != ringBuf, "ringBuf can't be NULL", ESP_FAIL);
    sendbuf = ringBuf;
    xTaskCreate(espnow_sender_task, "espnow_sender_task", 3 * 1024, NULL, 5, NULL);
    ERROR_CHECK(ESP_OK == espnow_set_config_for_data_type(ESPNOW_DATA_TYPE_DEBUG_LOG, true, _espnow_debug_recv_process), "espnow_set_config_for_data_type failed", ESP_FAIL);
    ERROR_CHECK(ESP_OK == espnow_set_config_for_data_type(ESPNOW_DATA_TYPE_DEBUG_COMMAND, true, _espnow_debug_command_recv_process), "espnow_set_config_for_data_type failed", ESP_FAIL);
    return ESP_OK;
}
