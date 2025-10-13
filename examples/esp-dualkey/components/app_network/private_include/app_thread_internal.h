/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/*
 *    This example code is in the Public Domain (or CC0 licensed, at your option.)
 *
 *       Unless required by applicable law or agreed to in writing, this
 *          software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *             CONDITIONS OF ANY KIND, either express or implied.
 *             */
#pragma once
#include <esp_err.h>
#include <esp_event.h>
#include <esp_openthread_types.h>
#include <esp_idf_version.h>

#ifdef __cplusplus
extern "C" {
#endif

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 0) && defined(CONFIG_ESP_RMAKER_USING_NETWORK_PROV)
#define RMAKER_USING_NETWORK_PROV 1
#else
#define RMAKER_USING_NETWORK_PROV 0
#endif

#if !RMAKER_USING_NETWORK_PROV
#error "Please use IDF v5.1+ and enable ESP_RMAKER_USING_NETWORK_PROV for Thread devices"
#endif

#if SOC_IEEE802154_SUPPORTED
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG() \
    {                                         \
        .radio_mode = RADIO_MODE_NATIVE,      \
    }

#else
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()          \
    {                                                  \
        .radio_mode = RADIO_MODE_UART_RCP,             \
        .radio_uart_config = {                         \
            .port = 1,                                 \
            .uart_config = {                           \
                .baud_rate = 460800,                   \
                .data_bits = UART_DATA_8_BITS,         \
                .parity = UART_PARITY_DISABLE,         \
                .stop_bits = UART_STOP_BITS_1,         \
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, \
                .rx_flow_ctrl_thresh = 0,              \
                .source_clk = UART_SCLK_DEFAULT,       \
            },                                         \
            .rx_pin = 4,                               \
            .tx_pin = 5,                               \
        },                                             \
    }
#endif

#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()               \
    {                                                      \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE, \
    }

#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG() \
    {                                        \
        .storage_partition_name = "nvs",     \
        .netif_queue_size = 10,              \
        .task_queue_size = 10,               \
    }

esp_err_t thread_init();

esp_err_t thread_start(const char *pop, const char *service_name, const char *service_key, uint8_t *mfg_data,
                       size_t mfg_data_len, bool *provisioned);

#ifdef __cplusplus
}
#endif
