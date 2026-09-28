/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>

/** 0=关闭 Wi-Fi/AP/网页门户（降温）；1=启用 Wi-Fi 网页遥控 */
#ifndef ENABLE_WIFI
#define ENABLE_WIFI 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

void wifi_stream_init(void);
bool wifi_stream_connect(const char *ssid, const char *password);
void wifi_stream_send_text(const char *text);
bool wifi_stream_get_sta_ip(char *buf, size_t len);
bool wifi_stream_http_active(void);
/** WiFi 正在连接/重试时返回 true，主循环应暂缓进入深睡 */
bool wifi_stream_inhibits_sleep(void);
/** 进入深睡前调用：断开 HTTP/SSE 与 WiFi */
void wifi_stream_prepare_sleep(void);
bool wifi_stream_is_connected(void);
/** 9 点校准期间置 true：禁止 Auto Light Sleep，保证采样节奏稳定；校准结束置 false 恢复省电 */
void wifi_stream_set_calibrating(bool busy);

#ifdef __cplusplus
}
#endif
