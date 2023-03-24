/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_check.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "app_wifi.h"
#include "app_sntp.h"
#include "settings.h"

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
//#define EXAMPLE_ESP_WIFI_SSID      "esp-office-2.4G"
//#define EXAMPLE_ESP_WIFI_PASS      "1qazxsw2"
#define EXAMPLE_ESP_MAXIMUM_RETRY  10

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_FAIL_BIT           BIT1

#define portTICK_RATE_MS        10

static const char *TAG = "wifi station";
static int s_retry_num = 0;
static bool s_reconnect = true;

static bool wifi_connected = false;
static QueueHandle_t audio_event_queue = NULL;

scan_info_t scan_info_result ={
    .scan_done = false,
    .ap_count = 0,
};

bool wifi_connected_already(void)
{
    return wifi_connected;
}

esp_err_t app_wifi_get_wifi_ssid(char *ssid, size_t len)
{
    wifi_config_t wifi_cfg;
    if (esp_wifi_get_config(WIFI_IF_STA, &wifi_cfg) != ESP_OK) {
        return ESP_FAIL;
    }
    strncpy(ssid, (const char *)wifi_cfg.sta.ssid, len);
    return ESP_OK;
}

esp_err_t send_network_event(net_event_t event)
{
    net_event_t eventOut = event;
    BaseType_t ret_val = xQueueSend(audio_event_queue, &eventOut, 0);

    ESP_RETURN_ON_FALSE(pdPASS == ret_val, ESP_ERR_INVALID_STATE,
        TAG, "The last event has not been processed yet");

    return ESP_OK;
}

// static void print_auth_mode(int authmode)
// {
//     switch (authmode) {
//     case WIFI_AUTH_OPEN:
//         ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OPEN");
//         break;
//     case WIFI_AUTH_WEP:
//         ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WEP");
//         break;
//     case WIFI_AUTH_WPA_PSK:
//         ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_PSK");
//         break;
//     case WIFI_AUTH_WPA2_PSK:
//         ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_PSK");
//         break;
//     case WIFI_AUTH_WPA_WPA2_PSK:
//         ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
//         break;
//     case WIFI_AUTH_WPA2_ENTERPRISE:
//         ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_ENTERPRISE");
//         break;
//     case WIFI_AUTH_WPA3_PSK:
//         ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_PSK");
//         break;
//     case WIFI_AUTH_WPA2_WPA3_PSK:
//         ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
//         break;
//     default:
//         ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_UNKNOWN");
//         break;
//     }
// }

// static void print_cipher_type(int pairwise_cipher, int group_cipher)
// {
//     switch (pairwise_cipher) {
//     case WIFI_CIPHER_TYPE_NONE:
//         ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_NONE");
//         break;
//     case WIFI_CIPHER_TYPE_WEP40:
//         ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP40");
//         break;
//     case WIFI_CIPHER_TYPE_WEP104:
//         ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP104");
//         break;
//     case WIFI_CIPHER_TYPE_TKIP:
//         ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP");
//         break;
//     case WIFI_CIPHER_TYPE_CCMP:
//         ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_CCMP");
//         break;
//     case WIFI_CIPHER_TYPE_TKIP_CCMP:
//         ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
//         break;
//     default:
//         ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
//         break;
//     }

//     switch (group_cipher) {
//     case WIFI_CIPHER_TYPE_NONE:
//         ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_NONE");
//         break;
//     case WIFI_CIPHER_TYPE_WEP40:
//         ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP40");
//         break;
//     case WIFI_CIPHER_TYPE_WEP104:
//         ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP104");
//         break;
//     case WIFI_CIPHER_TYPE_TKIP:
//         ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP");
//         break;
//     case WIFI_CIPHER_TYPE_CCMP:
//         ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_CCMP");
//         break;
//     case WIFI_CIPHER_TYPE_TKIP_CCMP:
//         ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
//         break;
//     default:
//         ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
//         break;
//     }
// }


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        wifi_connected = false;
        ESP_LOGI(TAG, "start connect to the AP");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_reconnect && ++s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            ESP_LOGI(TAG, "sta disconnect, retry attempt %d...", s_retry_num);
        }
        else {
            ESP_LOGI(TAG, "sta disconnected");
            send_network_event(NET_EVENT_SCAN);
        }
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        wifi_connected = false;
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        wifi_connected = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_reconnect_sta(void)
{
    int bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, 0, 1, 0);

    wifi_config_t wifi_config = { 0 };

    sys_param_t * sys_param = settings_get_parameter();
    memcpy(wifi_config.sta.ssid, sys_param->ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, sys_param->password, sizeof(wifi_config.sta.password));
    //ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );

   if (bits & WIFI_CONNECTED_BIT) {
        s_reconnect = false;
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_ERROR_CHECK( esp_wifi_disconnect() );
        xEventGroupWaitBits(s_wifi_event_group, WIFI_FAIL_BIT, 0, 1, portTICK_RATE_MS);
    }

    s_reconnect = true;
    s_retry_num = 0;
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    esp_wifi_connect();

    ESP_LOGI(TAG, "wifi_reconnect_sta finished.%d, %s, %d, %s", \
                sys_param->ssid_len, wifi_config.sta.ssid,
                sys_param->password_len, wifi_config.sta.password);

    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, 0, 1, 5000 / portTICK_RATE_MS);
}

static void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	     //.threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
	     //.sae_pwe_h2e = 2,
        },
    };

    sys_param_t * sys_param = settings_get_parameter();
    memcpy(wifi_config.sta.ssid, sys_param->ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, sys_param->password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );
    ESP_LOGI(TAG, "wifi_init_sta finished.%d, %d, %s, %s", \
                sys_param->ssid_len, sys_param->password_len, \
                wifi_config.sta.ssid, wifi_config.sta.password);

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        send_network_event(NET_EVENT_NTP);
        send_network_event(NET_EVENT_SCAN);
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 wifi_config.sta.ssid, wifi_config.sta.password);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 wifi_config.sta.ssid, wifi_config.sta.password);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

/* Initialize Wi-Fi as sta and set scan method */
static void wifi_scan(void)
{
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    scan_info_result.scan_done = false;

    esp_err_t ret = esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(TAG, "Total APs scanned = %u, ret:%d", ap_count, ret);

    for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++) {
        ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
        /*
        ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
        print_auth_mode(ap_info[i].authmode);
        if (ap_info[i].authmode != WIFI_AUTH_WEP) {
            print_cipher_type(ap_info[i].pairwise_cipher, ap_info[i].group_cipher);
        }
        ESP_LOGI(TAG, "Channel \t\t%d\n", ap_info[i].primary);
        */
    }

    if(ap_count && (ESP_OK == ret)){
        scan_info_result.scan_done = true;
        scan_info_result.ap_count = (ap_count < DEFAULT_SCAN_LIST_SIZE) ? ap_count:DEFAULT_SCAN_LIST_SIZE;
        memcpy(&scan_info_result.ap_info[0], &ap_info[0], sizeof(wifi_ap_record_t)*scan_info_result.ap_count);
    }
}

void app_wifi_enable(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
}

static void network_task(void *args)
{
    net_event_t net_event;

    audio_event_queue = xQueueCreate(4, sizeof(net_event_t));
    if (NULL == audio_event_queue) {
        ESP_LOGI(TAG, "audio_event_queue failed");
        vTaskDelete(NULL);
    }

    /* Start the Wi-Fi. */
    app_wifi_enable();

    while(1){

        if (pdPASS == xQueueReceive(audio_event_queue, &net_event, portTICK_RATE_MS/5)) {
            switch(net_event)
            {
                case NET_EVENT_RECONNECT:
                ESP_LOGI(TAG, "event:NET_EVENT_RECONNECT");
                wifi_reconnect_sta();
                break;

                case NET_EVENT_SCAN:
                ESP_LOGI(TAG, "event:NET_EVENT_SCAN");
                wifi_scan();
                break;

                case NET_EVENT_NTP:
                ESP_LOGI(TAG, "event:NET_EVENT_NTP");
                app_sntp_init();
                break;

                default:
                break;
            }
        }
    }
    vTaskDelete(NULL);
}

void app_network_start(void)
{
    BaseType_t ret_val = xTaskCreatePinnedToCore(network_task, "NetWork Task", 6 * 1024, NULL, 1, NULL, 0);
    ESP_ERROR_CHECK_WITHOUT_ABORT((pdPASS == ret_val) ? ESP_OK : ESP_FAIL);
}
