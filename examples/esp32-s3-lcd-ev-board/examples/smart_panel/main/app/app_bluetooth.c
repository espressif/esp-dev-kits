/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

#include "app_bluetooth.h"
#include "settings.h"

#define TAG "app_bluetooth"

/* Global scan result (similar to WiFi scan_result) */
bt_scan_result_t bt_scan_result = {
    .device_count = 0,
    .scan_state = BT_SCAN_IDLE,
};

/* Static variables */
static bt_connect_status_t bt_connect_status = BT_STATUS_DISCONNECTED;
static uint8_t connected_addr[6] = {0};
static bool bt_enabled = false;
static bool is_scanning = false;
static esp_bd_addr_t peer_bda = {0};

/* Thread safety */
static SemaphoreHandle_t bt_mutex = NULL;
static QueueHandle_t bt_event_queue = NULL;

/* Bluetooth task */
static TaskHandle_t bt_task_handle = NULL;

/* Helper function to convert BD address to string */
static char *bda2str(esp_bd_addr_t bda, char *str, size_t size)
{
    if (bda == NULL || str == NULL || size < 18) {
        return NULL;
    }
    snprintf(str, size, "%02x:%02x:%02x:%02x:%02x:%02x",
             bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
    return str;
}

/* Helper to get device name from EIR data */
static bool get_name_from_eir(uint8_t *eir, char *bdname, uint8_t *bdname_len)
{
    uint8_t *rmt_bdname = NULL;
    uint8_t rmt_bdname_len = 0;

    if (!eir) {
        return false;
    }

    rmt_bdname = esp_bt_gap_resolve_eir_data(eir, ESP_BT_EIR_TYPE_CMPL_LOCAL_NAME, &rmt_bdname_len);
    if (!rmt_bdname) {
        rmt_bdname = esp_bt_gap_resolve_eir_data(eir, ESP_BT_EIR_TYPE_SHORT_LOCAL_NAME, &rmt_bdname_len);
    }

    if (rmt_bdname) {
        if (rmt_bdname_len > ESP_BT_GAP_MAX_BDNAME_LEN) {
            rmt_bdname_len = ESP_BT_GAP_MAX_BDNAME_LEN;
        }

        if (bdname) {
            memcpy(bdname, rmt_bdname, rmt_bdname_len);
            bdname[rmt_bdname_len] = '\0';
        }
        if (bdname_len) {
            *bdname_len = rmt_bdname_len;
        }
        return true;
    }

    return false;
}

/* Lock/Unlock functions (similar to WiFi) */
bool app_bluetooth_lock(uint32_t timeout_ms)
{
    if (bt_mutex == NULL) {
        return false;
    }
    
    TickType_t ticks = (timeout_ms == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTake(bt_mutex, ticks) == pdTRUE;
}

void app_bluetooth_unlock(void)
{
    if (bt_mutex != NULL) {
        xSemaphoreGive(bt_mutex);
    }
}

/* Get/Set functions */
bt_scan_result_t app_bluetooth_get_scan_result(void)
{
    bt_scan_result_t result;
    app_bluetooth_lock(0);
    memcpy(&result, &bt_scan_result, sizeof(bt_scan_result_t));
    app_bluetooth_unlock();
    return result;
}

void app_bluetooth_set_scan_state(bt_scan_state_t state)
{
    app_bluetooth_lock(0);
    bt_scan_result.scan_state = state;
    app_bluetooth_unlock();
}

bt_connect_status_t app_bluetooth_get_connect_status(void)
{
    return bt_connect_status;
}

bt_connect_status_t bluetooth_connected_already(void)
{
    return bt_connect_status;
}

/* Send event to Bluetooth task */
void app_bluetooth_send_event(bt_event_t event)
{
    if (bt_event_queue != NULL) {
        xQueueSend(bt_event_queue, &event, 0);
    }
}

/* GAP callback function */
static void bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    char bda_str[18] = {0};

    switch (event) {
        case ESP_BT_GAP_DISC_RES_EVT: {
            // Device discovered
            app_bluetooth_lock(0);

            if (bt_scan_result.device_count >= 20) {
                app_bluetooth_unlock();
                break;
            }

            esp_bd_addr_t bda;
            memcpy(bda, param->disc_res.bda, 6);

            // Extract properties
            int8_t rssi = 0;
            uint32_t cod = 0;
            uint8_t *eir = NULL;
            uint8_t eir_len = 0;

            for (int i = 0; i < param->disc_res.num_prop; i++) {
                esp_bt_gap_dev_prop_t *p = &param->disc_res.prop[i];
                switch (p->type) {
                    case ESP_BT_GAP_DEV_PROP_COD:
                        cod = *(uint32_t *)p->val;
                        break;
                    case ESP_BT_GAP_DEV_PROP_RSSI:
                        rssi = *(int8_t *)p->val;
                        break;
                    case ESP_BT_GAP_DEV_PROP_EIR:
                        eir = (uint8_t *)p->val;
                        eir_len = p->len;
                        break;
                    default:
                        break;
                }
            }

            // Check for duplicates
            bool found = false;
            for (int i = 0; i < bt_scan_result.device_count; i++) {
                if (memcmp(bt_scan_result.devices[i].addr, bda, 6) == 0) {
                    // Update RSSI if stronger
                    if (rssi > bt_scan_result.devices[i].rssi) {
                        bt_scan_result.devices[i].rssi = rssi;
                    }
                    found = true;
                    break;
                }
            }

            if (!found) {
                // Add device
                memcpy(bt_scan_result.devices[bt_scan_result.device_count].addr, bda, 6);
                bt_scan_result.devices[bt_scan_result.device_count].rssi = rssi;

                // Optionally check COD for audio devices
                bool is_audio_device = ((cod & 0x1FFC00) >> 10) & (1 << 3); // Rendering bit
                (void)is_audio_device; // Currently unused

                // Get device name
                uint8_t name_len = 0;
                if (get_name_from_eir(eir, (char *)bt_scan_result.devices[bt_scan_result.device_count].name, &name_len)) {
                    // Name retrieved successfully
                } else {
                    // Fallback: use MAC address as name
                    snprintf(bt_scan_result.devices[bt_scan_result.device_count].name,
                             sizeof(bt_scan_result.devices[0].name),
                             "BT_%02X%02X%02X",
                             bda[3], bda[4], bda[5]);
                }

                bt_scan_result.device_count++;

                if (bt_scan_result.scan_state == BT_SCAN_BUSY) {
                    bt_scan_result.scan_state = BT_SCAN_RENEW;
                }
            }

            app_bluetooth_unlock();
            break;
        }

        case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
            if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED) {
                ESP_LOGI(TAG, "Device discovery stopped, found %d devices", bt_scan_result.device_count);
                app_bluetooth_lock(0);
                if (bt_scan_result.scan_state == BT_SCAN_BUSY) {
                    bt_scan_result.scan_state = BT_SCAN_RENEW;
                }
                is_scanning = false;
                app_bluetooth_unlock();
            } else if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STARTED) {
                ESP_LOGI(TAG, "Device discovery started");
            }
            break;

        case ESP_BT_GAP_AUTH_CMPL_EVT:
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Authentication success: %s", param->auth_cmpl.device_name);
                bda2str(param->auth_cmpl.bda, bda_str, sizeof(bda_str));
                ESP_LOGI(TAG, "Address: %s", bda_str);
            } else {
                ESP_LOGE(TAG, "Authentication failed, status: %d", param->auth_cmpl.stat);
                bt_connect_status = BT_STATUS_CONNECT_FAILED;
            }
            break;

        case ESP_BT_GAP_PIN_REQ_EVT:
            ESP_LOGI(TAG, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
            if (param->pin_req.min_16_digit) {
                ESP_LOGI(TAG, "Input pin code: 0000 0000 0000 0000");
                esp_bt_pin_code_t pin_code = {0};
                esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
            } else {
                ESP_LOGI(TAG, "Input pin code: 1234");
                esp_bt_pin_code_t pin_code;
                pin_code[0] = '1';
                pin_code[1] = '2';
                pin_code[2] = '3';
                pin_code[3] = '4';
                esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
            }
            break;

        default:
            break;
    }
}


/* A2DP callback function */
static void bt_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param)
{
    char bda_str[18] = {0};

    switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT: {
        uint8_t *bda = param->conn_stat.remote_bda;
        bda2str(bda, bda_str, sizeof(bda_str));
        
        if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
            ESP_LOGI(TAG, "A2DP connected to [%s]", bda_str);
            bt_connect_status = BT_STATUS_CONNECTED;
            memcpy(connected_addr, bda, 6);
            
            // Save to settings
            sys_param_t *sys_set = settings_get_parameter();
            memcpy(sys_set->bt_addr, bda, 6);
            settings_write_parameter_to_nvs();
            
        } else if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
            ESP_LOGI(TAG, "A2DP disconnected from [%s]", bda_str);
            bt_connect_status = BT_STATUS_DISCONNECTED;
        } else if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTING) {
            ESP_LOGI(TAG, "A2DP connecting to [%s]", bda_str);
            bt_connect_status = BT_STATUS_CONNECTING;
        }
        break;
    }
    
    case ESP_A2D_AUDIO_STATE_EVT:
        if (param->audio_stat.state == ESP_A2D_AUDIO_STATE_STARTED) {
            ESP_LOGI(TAG, "A2DP audio started");
        } else if (param->audio_stat.state == ESP_A2D_AUDIO_STATE_STOPPED || 
                   param->audio_stat.state == ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND) {
            ESP_LOGI(TAG, "A2DP audio stopped/suspended");
        }
        break;
        
    case ESP_A2D_AUDIO_CFG_EVT:
        ESP_LOGI(TAG, "A2DP audio stream configuration, codec type %d", param->audio_cfg.mcc.type);
        break;
        
    default:
        ESP_LOGW(TAG, "Unhandled A2DP event: %d", event);
        break;
    }
}

/* Bluetooth event processing task (similar to WiFi network_event_task) */
static void bluetooth_event_task(void *arg)
{
    bt_event_t event;
    
    while (1) {
        if (xQueueReceive(bt_event_queue, &event, portMAX_DELAY) == pdTRUE) {
            switch (event) {
            case BT_EVENT_SCAN:
                ESP_LOGI(TAG, "BT_EVENT_SCAN received");
                app_bluetooth_stop_scan();
                vTaskDelay(pdMS_TO_TICKS(100));
                app_bluetooth_set_scan_state(BT_SCAN_UPDATE);
                app_bluetooth_start_scan();
                break;
                
            case BT_EVENT_RECONNECT:
                ESP_LOGI(TAG, "BT_EVENT_RECONNECT received");
                app_bluetooth_disconnect();
                vTaskDelay(pdMS_TO_TICKS(500));
                
                sys_param_t *sys_set = settings_get_parameter();
                bool has_saved_addr = false;
                for (int i = 0; i < 6; i++) {
                    if (sys_set->bt_addr[i] != 0) {
                        has_saved_addr = true;
                        break;
                    }
                }
                
                if (has_saved_addr) {
                    app_bluetooth_connect(sys_set->bt_addr);
                }
                break;
                
            case BT_EVENT_DISCONNECT:
                ESP_LOGI(TAG, "BT_EVENT_DISCONNECT received");
                app_bluetooth_disconnect();
                break;
                
            default:
                break;
            }
        }
    }
}

esp_err_t app_bluetooth_init(void)
{
    esp_err_t ret;

    // Create mutex for thread safety
    bt_mutex = xSemaphoreCreateMutex();
    if (bt_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create Bluetooth mutex");
        return ESP_FAIL;
    }

    // Create event queue
    bt_event_queue = xQueueCreate(10, sizeof(bt_event_t));
    if (bt_event_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create Bluetooth event queue");
        return ESP_FAIL;
    }

    // Initialize Bluetooth controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth controller init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth controller enable failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize Bluedroid
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluedroid init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluedroid enable failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Register GAP callback
    ret = esp_bt_gap_register_callback(bt_gap_cb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GAP register callback failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize A2DP source
    ret = esp_a2d_register_callback(bt_a2d_cb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "A2DP register callback failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_a2d_source_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "A2DP source init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Set discoverable and connectable mode
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

    // Set device name
    esp_bt_gap_set_device_name("MyDevice");

    // Create Bluetooth event task
    BaseType_t task_ret = xTaskCreate(bluetooth_event_task, "bt_event_task", 
                                      4096, NULL, 5, &bt_task_handle);
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create Bluetooth event task");
        return ESP_FAIL;
    }

    bt_enabled = true;
    bt_connect_status = BT_STATUS_DISCONNECTED;
    
    ESP_LOGI(TAG, "Bluetooth A2DP source initialized successfully");
    return ESP_OK;
}

esp_err_t app_bluetooth_start_scan(void)
{
    if (!bt_enabled) {
        ESP_LOGE(TAG, "Bluetooth not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Clear previous scan results
    app_bluetooth_lock(0);
    bt_scan_result.device_count = 0;
    memset(bt_scan_result.devices, 0, sizeof(bt_scan_result.devices));
    bt_scan_result.scan_state = BT_SCAN_BUSY;
    app_bluetooth_unlock();

    // Start device discovery
    esp_err_t ret = esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Start discovery failed: %s", esp_err_to_name(ret));
        app_bluetooth_set_scan_state(BT_SCAN_IDLE);
        return ret;
    }

    is_scanning = true;
    ESP_LOGI(TAG, "Bluetooth scan started");
    return ESP_OK;
}

esp_err_t app_bluetooth_stop_scan(void)
{
    if (!is_scanning) {
        return ESP_OK;
    }

    esp_err_t ret = esp_bt_gap_cancel_discovery();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Cancel discovery failed: %s", esp_err_to_name(ret));
    }

    is_scanning = false;
    ESP_LOGI(TAG, "Bluetooth scan stopped");
    return ESP_OK;
}

esp_err_t app_bluetooth_connect(const uint8_t *addr)
{
    if (!bt_enabled) {
        ESP_LOGE(TAG, "Bluetooth not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (addr == NULL) {
        ESP_LOGE(TAG, "Invalid device address");
        return ESP_ERR_INVALID_ARG;
    }

    // Stop scanning before connecting
    app_bluetooth_stop_scan();

    // Store the target address
    memcpy(peer_bda, addr, 6);
    memcpy(connected_addr, addr, 6);

    char bda_str[18];
    bda2str(peer_bda, bda_str, sizeof(bda_str));
    ESP_LOGI(TAG, "Connecting to device: %s", bda_str);

    bt_connect_status = BT_STATUS_CONNECTING;

    // Connect to the device
    esp_err_t ret = esp_a2d_source_connect(peer_bda);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "A2DP connect failed: %s", esp_err_to_name(ret));
        bt_connect_status = BT_STATUS_CONNECT_FAILED;
        return ret;
    }

    return ESP_OK;
}

esp_err_t app_bluetooth_disconnect(void)
{
    if (!bt_enabled) {
        ESP_LOGE(TAG, "Bluetooth not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (bt_connect_status != BT_STATUS_CONNECTED && bt_connect_status != BT_STATUS_CONNECTING) {
        return ESP_OK;
    }

    esp_err_t ret = esp_a2d_source_disconnect(peer_bda);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "A2DP disconnect failed: %s", esp_err_to_name(ret));
    }

    bt_connect_status = BT_STATUS_DISCONNECTED;
    ESP_LOGI(TAG, "Disconnected from device");

    return ESP_OK;
}