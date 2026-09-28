/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

void web_portal_init(void);

/** 在 HTTP 服务启动后注册运行页路由 */
void web_portal_register_handlers(httpd_handle_t server);

/** 推送实时磁数据与判区结果到 HTTP SSE 客户端 */
void web_portal_push_mag(float dx, float dy, float dz, int zone_id, float confidence, float r_xy, float theta_deg);

void web_portal_set_action_snapshot(int action_zone_id, float action_confidence);

void web_portal_push_zone_change(int zone_id, float confidence);

/** 处理上位机控制消息（串口 JSON 或 HTTP POST body） */
esp_err_t web_portal_handle_control_text(const char *text);

#ifdef __cplusplus
}
#endif
