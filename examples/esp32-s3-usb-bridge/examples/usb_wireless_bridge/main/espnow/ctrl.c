/* SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/param.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "espnow.h"
#include "espnow_storage.h"
#include "espnow_ctrl.h"

#define BIND_LIST_MAX_SIZE  1

typedef struct {
    int8_t rssi;
    uint32_t timestamp;
    size_t size;
    espnow_ctrl_bind_info_t data[BIND_LIST_MAX_SIZE];
} espnow_bindlist_t;

static espnow_bindlist_t g_bindlist = {0};
static const char *TAG = "espnow_ctrl";

static bool _ctrl_responder_is_bindlist(const uint8_t *mac, espnow_attribute_t initiator_attribute)
{
    for (int i = 0; i < g_bindlist.size; ++i) {
        if (!memcmp(g_bindlist.data[i].mac, mac, 6)
                && g_bindlist.data[i].initiator_attribute == initiator_attribute) {
            return true;
        }
    }

    return false;
}

esp_err_t ctrl_responder_get_bindlist(espnow_ctrl_bind_info_t *list, size_t *size)
{
    if (!list) {
        *size = g_bindlist.size;
    } else {
        *size = MIN(g_bindlist.size, *size);
        memcpy(list, g_bindlist.data, sizeof(espnow_ctrl_bind_info_t) * (*size));
    }

    return ESP_OK;
}

esp_err_t ctrl_responder_set_bindlist(const espnow_ctrl_bind_info_t *info)
{
    if (g_bindlist.size >= BIND_LIST_MAX_SIZE) {
        return ESP_FAIL;
    }

    if (!_ctrl_responder_is_bindlist(info->mac, info->initiator_attribute)) {
        memcpy(g_bindlist.data + g_bindlist.size, info, sizeof(espnow_ctrl_bind_info_t));
        g_bindlist.size++;
        espnow_storage_set("bindlist", &g_bindlist, sizeof(g_bindlist));
    }

    return ESP_OK;
}

esp_err_t ctrl_responder_remove_bindlist(const espnow_ctrl_bind_info_t *info)
{
    for (int i = 0; i < g_bindlist.size; ++i) {
        if (!memcmp(g_bindlist.data + i, info, sizeof(espnow_ctrl_bind_info_t))) {
            g_bindlist.size--;

            g_bindlist.data[i].initiator_attribute = 0;
            memset(g_bindlist.data[i].mac, 0, 6);
            if (g_bindlist.size > 0 && i < g_bindlist.size) {
                g_bindlist.data[i] = g_bindlist.data[g_bindlist.size];
                g_bindlist.data[g_bindlist.size].initiator_attribute = 0;
            }

            espnow_storage_set("bindlist", &g_bindlist, sizeof(g_bindlist));
            break;
        }
    }

    return ESP_OK;
}

static esp_err_t _ctrl_responder_bind_process(uint8_t *src_addr, void *data,
        size_t size, wifi_pkt_rx_ctrl_t *rx_ctrl)
{
    ESP_PARAM_CHECK(src_addr);
    ESP_PARAM_CHECK(data);
    ESP_PARAM_CHECK(size);
    ESP_PARAM_CHECK(rx_ctrl);

    espnow_ctrl_data_t *ctrl_data = (espnow_ctrl_data_t *)data;
    if (ctrl_data->responder_value_b) {
        ESP_LOGD(TAG, "bind, esp_log_timestamp: %"PRIu32", timestamp: %"PRIu32", rssi: %d, rssi: %d",
                 esp_log_timestamp(), g_bindlist.timestamp, rx_ctrl->rssi, g_bindlist.rssi);

        if (esp_log_timestamp() < g_bindlist.timestamp && rx_ctrl->rssi > g_bindlist.rssi) {
            ESP_LOGD("control_func", "addr: "MACSTR", initiator_type: %d, initiator_value: %d",
                     MAC2STR(src_addr), ctrl_data->initiator_attribute >> 8, ctrl_data->initiator_attribute & 0xff);

            if (!_ctrl_responder_is_bindlist(src_addr, ctrl_data->initiator_attribute) && g_bindlist.size < BIND_LIST_MAX_SIZE) {
                g_bindlist.data[g_bindlist.size].initiator_attribute = ctrl_data->initiator_attribute;
                memcpy(g_bindlist.data[g_bindlist.size].mac, src_addr, 6);

                esp_event_post(ESP_EVENT_ESPNOW, ESP_EVENT_ESPNOW_CTRL_BIND,
                               g_bindlist.data + g_bindlist.size, sizeof(espnow_ctrl_bind_info_t), 0);

                g_bindlist.size++;
                espnow_storage_set("bindlist", &g_bindlist, sizeof(g_bindlist));
            }
        }
    } else {
        if (_ctrl_responder_is_bindlist(src_addr, ctrl_data->initiator_attribute)) {
            ESP_LOGD(TAG, "unbind, addr: "MACSTR", esp_log_timestamp: %"PRIu32", timestamp: %"PRIu32", rssi: %d, rssi: %d", MAC2STR(src_addr),
                        esp_log_timestamp(), g_bindlist.timestamp, rx_ctrl->rssi, g_bindlist.rssi);

            for (int i = 0; i < g_bindlist.size; ++i) {
                if (!memcmp(g_bindlist.data[i].mac, src_addr, 6)
                        && g_bindlist.data[i].initiator_attribute == ctrl_data->initiator_attribute) {
                    esp_event_post(ESP_EVENT_ESPNOW, ESP_EVENT_ESPNOW_CTRL_UNBIND,
                                    g_bindlist.data + i, sizeof(espnow_ctrl_bind_info_t), 0);

                    g_bindlist.size--;

                    for (int j = i; j < g_bindlist.size; j++) {
                        g_bindlist.data[j].initiator_attribute = g_bindlist.data[j + 1].initiator_attribute;
                        memcpy(g_bindlist.data[j].mac, g_bindlist.data[j + 1].mac, 6);
                    }
                    g_bindlist.data[g_bindlist.size].initiator_attribute = 0;
                    memset(g_bindlist.data[g_bindlist.size].mac, 0, 6);

                    espnow_storage_set("bindlist", &g_bindlist, sizeof(g_bindlist));
                    break;
                }
            }
        }
    }

    return ESP_OK;
}

esp_err_t ctrl_responder_init(void)
{
    espnow_storage_get("bindlist", &g_bindlist, sizeof(g_bindlist));
    g_bindlist.timestamp = 0;
    g_bindlist.rssi      = 0;
    return espnow_set_config_for_data_type(ESPNOW_DATA_TYPE_CONTROL_BIND, true, _ctrl_responder_bind_process);

}

esp_err_t ctrl_responder_bind_window_set(uint32_t wait_ms, int8_t rssi)
{
    g_bindlist.timestamp = esp_log_timestamp() + wait_ms;
    g_bindlist.rssi      = rssi;
    return ESP_OK;
}
