/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <esp_err.h>
#include <esp_log.h>
#ifdef CONFIG_ESP_INSIGHTS_ENABLED
#include <esp_rmaker_mqtt.h>
#include <esp_insights.h>
#include <string.h>
#include <esp_rmaker_core.h>
#include <esp_rmaker_common_events.h>

#if CONFIG_APP_INSIGHTS_ENABLE_LOG_TYPE_ALL
#define APP_INSIGHTS_LOG_TYPE               ESP_DIAG_LOG_TYPE_ERROR \
                                            | ESP_DIAG_LOG_TYPE_WARNING \
                                            | ESP_DIAG_LOG_TYPE_EVENT
#else
#define APP_INSIGHTS_LOG_TYPE               ESP_DIAG_LOG_TYPE_ERROR
#endif /* CONFIG_APP_INSIGHTS_ENABLE_LOG_TYPE_ALL */

#define INSIGHTS_TOPIC_SUFFIX       "diagnostics/from-node"
#define INSIGHTS_TOPIC_RULE         "insights_message_delivery"

static int app_insights_data_send(void *data, size_t len)
{
    char topic[128];
    int msg_id = -1;
    if (data == NULL) {
        return 0;
    }
    char *node_id = esp_rmaker_get_node_id();
    if (!node_id) {
        return -1;
    }
    if (esp_rmaker_mqtt_is_budget_available() == false) {
        /* the API `esp_rmaker_mqtt_publish` already checks if the budget is available.
            This also raises an error message, which we do not want for esp-insights.
            silently return with error */
        return ESP_FAIL;
    }
    esp_rmaker_create_mqtt_topic(topic, sizeof(topic), INSIGHTS_TOPIC_SUFFIX, INSIGHTS_TOPIC_RULE);
    esp_rmaker_mqtt_publish(topic, data, len, RMAKER_MQTT_QOS1, &msg_id);
    return msg_id;
}

static void rmaker_common_event_handler(void *arg, esp_event_base_t event_base,
                                        int32_t event_id, void *event_data)
{
    if (event_base != RMAKER_COMMON_EVENT) {
        return;
    }
    esp_insights_transport_event_data_t data;
    switch (event_id) {
    case RMAKER_MQTT_EVENT_PUBLISHED:
        memset(&data, 0, sizeof(data));
        data.msg_id = *(int *)event_data;
        esp_event_post(INSIGHTS_EVENT, INSIGHTS_EVENT_TRANSPORT_SEND_SUCCESS, &data, sizeof(data), portMAX_DELAY);
        break;
#ifdef CONFIG_MQTT_REPORT_DELETED_MESSAGES
    case RMAKER_MQTT_EVENT_MSG_DELETED:
        memset(&data, 0, sizeof(data));
        data.msg_id = *(int *)event_data;
        esp_event_post(INSIGHTS_EVENT, INSIGHTS_EVENT_TRANSPORT_SEND_FAILED, &data, sizeof(data), portMAX_DELAY);
        break;
#endif /* CONFIG_MQTT_REPORT_DELETED_MESSAGES */
    default:
        break;
    }
}
#endif /* CONFIG_ESP_INSIGHTS_ENABLED */

#define TAG "app_insights"

esp_err_t app_insights_enable(void)
{
#ifdef CONFIG_ESP_INSIGHTS_ENABLED
#ifndef CONFIG_ESP_INSIGHTS_TRANSPORT_MQTT
    ESP_LOGE(TAG, "Please select the CONFIG_ESP_INSIGHTS_TRANSPORT_MQTT option from menuconfig");
#endif
    /* Initialize the event loop, if not done already. */
    esp_err_t err = esp_event_loop_create_default();
    /* If the default event loop is already initialized, we get ESP_ERR_INVALID_STATE */
    if (err != ESP_OK) {
        if (err == ESP_ERR_INVALID_STATE) {
            ESP_LOGW(TAG, "Event loop creation failed with ESP_ERR_INVALID_STATE. Proceeding since it must have been created elsewhere.");
        } else {
            ESP_LOGE(TAG, "Failed to create default event loop, err = %x", err);
            return err;
        }
    }
#ifdef CONFIG_ESP_RMAKER_SELF_CLAIM
    ESP_LOGW(TAG, "Nodes with Self Claiming may not be accessible for Insights.");
#endif
    char *node_id = esp_rmaker_get_node_id();

    esp_insights_transport_config_t transport = {
        .callbacks.data_send  = app_insights_data_send,
    };
    esp_insights_transport_register(&transport);

    esp_event_handler_register(RMAKER_COMMON_EVENT, ESP_EVENT_ANY_ID, rmaker_common_event_handler, NULL);

    esp_insights_config_t config = {
        .log_type = APP_INSIGHTS_LOG_TYPE,
        .node_id  = node_id,
        .alloc_ext_ram = true,
    };

    esp_insights_enable(&config);

    if (esp_insights_cmd_resp_enable() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enabled insights command response");
    }
#else
    ESP_LOGI(TAG, "Enable CONFIG_ESP_INSIGHTS_ENABLED to get Insights.");
#endif /* ! CONFIG_ESP_INSIGHTS_ENABLED */
    return ESP_OK;
}
