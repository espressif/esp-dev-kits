/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#pragma once
#include <esp_err.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_SCAN_LIST_SIZE  20

typedef struct {
    uint8_t scan_done;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count;
} scan_info_t;

typedef enum {
    NET_EVENT_NONE = 0,
    NET_EVENT_RECONNECT,
    NET_EVENT_SCAN,
    NET_EVENT_NTP,
    NET_EVENT_MAX,
} net_event_t;

extern scan_info_t scan_info_result;

esp_err_t send_network_event(net_event_t event);
bool wifi_connected_already(void);
esp_err_t app_wifi_get_wifi_ssid(char *ssid, size_t len);

void app_network_start(void);

#ifdef __cplusplus
}
#endif
