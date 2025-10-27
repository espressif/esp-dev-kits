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
#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_idf_version.h>
#include <esp_rmaker_utils.h>
#include <app_network.h>
#include <app_wifi_internal.h>
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 1, 0)
// Features supported in 4.1+
#define ESP_NETIF_SUPPORTED
#endif

#ifdef ESP_NETIF_SUPPORTED
#include <esp_netif.h>
#else
#include <tcpip_adapter.h>
#endif

#if RMAKER_USING_NETWORK_PROV
#include <network_provisioning/manager.h>
#ifdef CONFIG_APP_NETWORK_PROV_TRANSPORT_BLE
#include <network_provisioning/scheme_ble.h>
#else /* CONFIG_APP_NETWORK_PROV_TRANSPORT_SOFTAP */
#include <network_provisioning/scheme_softap.h>
#endif /* CONFIG_APP_NETWORK_PROV_TRANSPORT_BLE */
#else
#include <wifi_provisioning/manager.h>
#ifdef CONFIG_APP_NETWORK_PROV_TRANSPORT_BLE
#include <wifi_provisioning/scheme_ble.h>
#else /* CONFIG_APP_NETWORK_PROV_TRANSPORT_SOFTAP */
#include <wifi_provisioning/scheme_softap.h>
#endif /* CONFIG_APP_NETWORK_PROV_TRANSPORT_BLE */

#endif

#include <app_wifi_internal.h>
#include <app_network.h>

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 3)
#define APP_PROV_STOP_ON_CREDS_MISMATCH
#elif (CONFIG_APP_NETWORK_PROV_MAX_RETRY_CNT > 0)
#warning "Provisioning window stop on max credentials failures, needs IDF version >= 5.1.3"
#endif

#ifdef CONFIG_ESP_RMAKER_NETWORK_OVER_WIFI
static const char *TAG = "app_wifi";
/* Event handler for catching system events */
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
#ifdef CONFIG_APP_NETWORK_RESET_PROV_ON_FAILURE
    static int retries = 0;
#endif

#if RMAKER_USING_NETWORK_PROV
    if (event_base == NETWORK_PROV_EVENT) {
#else
    if (event_base == WIFI_PROV_EVENT) {
#endif
        switch (event_id) {
#if RMAKER_USING_NETWORK_PROV
        case NETWORK_PROV_START:
#else
        case WIFI_PROV_START:
#endif
            ESP_LOGI(TAG, "Provisioning started");
            break;
#if RMAKER_USING_NETWORK_PROV
        case NETWORK_PROV_WIFI_CRED_RECV: {
#else
        case WIFI_PROV_CRED_RECV: {
#endif
            wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            ESP_LOGI(TAG, "Received Wi-Fi credentials"
                     "\n\tSSID     : %s\n\tPassword : %s",
                     (const char *) wifi_sta_cfg->ssid,
                     (const char *) wifi_sta_cfg->password);
            break;
        }
#if RMAKER_USING_NETWORK_PROV
        case NETWORK_PROV_WIFI_CRED_FAIL: {
            network_prov_wifi_sta_fail_reason_t *reason = (network_prov_wifi_sta_fail_reason_t *)event_data;
#else
        case WIFI_PROV_CRED_FAIL: {
            wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
#endif
            ESP_LOGE(TAG, "Provisioning failed!\n\tReason : %s"
                     "\n\tPlease reset to factory and retry provisioning",
#if RMAKER_USING_NETWORK_PROV
                     (*reason == NETWORK_PROV_WIFI_STA_AUTH_ERROR) ?
#else
                     (*reason == WIFI_PROV_STA_AUTH_ERROR) ?
#endif
                     "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
#ifdef CONFIG_APP_NETWORK_RESET_PROV_ON_FAILURE
            retries++;
            if (retries >= CONFIG_APP_NETWORK_PROV_MAX_RETRY_CNT) {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 1)
                ESP_LOGI(TAG, "Failed to connect with provisioned AP, resetting provisioned credentials");
#if RMAKER_USING_NETWORK_PROV
                network_prov_mgr_reset_wifi_sm_state_on_failure();
#else
                wifi_prov_mgr_reset_sm_state_on_failure();
#endif // RMAKER_USING_NETWORK_PROV
                esp_event_post(APP_NETWORK_EVENT, APP_NETWORK_EVENT_PROV_RESTART, NULL, 0, portMAX_DELAY);
#else
                ESP_LOGW(TAG, "Failed to connect with provisioned AP, please reset to provisioning manually");
#endif // ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 1)
                retries = 0;
            }
#endif // CONFIG_APP_NETWORK_RESET_PROV_ON_FAILURE
            break;
        }
#if RMAKER_USING_NETWORK_PROV
        case NETWORK_PROV_WIFI_CRED_SUCCESS:
#else
        case WIFI_PROV_CRED_SUCCESS:
#endif
            ESP_LOGI(TAG, "Provisioning successful");
#ifdef CONFIG_APP_NETWORK_RESET_PROV_ON_FAILURE
            retries = 0;
#endif
            break;
        default:
            break;
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
        esp_wifi_connect();
    }
}

static void wifi_init_sta()
{
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}
#endif // CONFIG_ESP_RMAKER_NETWORK_OVER_WIFI

esp_err_t wifi_init(void)
{
#ifdef CONFIG_ESP_RMAKER_NETWORK_OVER_WIFI
    /* Initialize TCP/IP */
#ifdef ESP_NETIF_SUPPORTED
    esp_netif_init();
#else
    tcpip_adapter_init();
#endif
    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
#if RMAKER_USING_NETWORK_PROV
    ESP_ERROR_CHECK(esp_event_handler_register(NETWORK_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
#else
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
#endif
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    /* Initialize Wi-Fi including netif with default config */
#ifdef ESP_NETIF_SUPPORTED
    esp_netif_create_default_wifi_sta();
#endif
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    return ESP_OK;
#else /* CONFIG_ESP_RMAKER_NETWORK_OVER_THREAD */
    return ESP_ERR_NOT_SUPPORTED;
#endif /* CONFIG_ESP_RMAKER_NETWORK_OVER_WIFI */
}

esp_err_t wifi_start(const char *pop, const char *service_name, const char *service_key, uint8_t *mfg_data,
                     size_t mfg_data_len, bool *provisioned)
{
#ifdef CONFIG_ESP_RMAKER_NETWORK_OVER_WIFI
#if RMAKER_USING_NETWORK_PROV
    /* Configuration for the provisioning manager */
    network_prov_mgr_config_t config = {
        /* What is the Provisioning Scheme that we want ?
         * network_prov_scheme_softap or network_prov_scheme_ble */
#ifdef CONFIG_APP_NETWORK_PROV_TRANSPORT_BLE
        .scheme = network_prov_scheme_ble,
#else /* CONFIG_APP_NETWORK_PROV_TRANSPORT_SOFTAP */
        .scheme = network_prov_scheme_softap,
#endif /* CONFIG_APP_NETWORK_PROV_TRANSPORT_BLE */

        /* Any default scheme specific event handler that you would
         * like to choose. Since our example application requires
         * neither BT nor BLE, we can choose to release the associated
         * memory once provisioning is complete, or not needed
         * (in case when device is already provisioned). Choosing
         * appropriate scheme specific event handler allows the manager
         * to take care of this automatically. This can be set to
         * NETWORK_PROV_EVENT_HANDLER_NONE when using network_prov_scheme_softap*/
#ifdef CONFIG_APP_NETWORK_PROV_TRANSPORT_BLE
        .scheme_event_handler = NETWORK_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
#else /* CONFIG_APP_NETWORK_PROV_TRANSPORT_SOFTAP */
        .scheme_event_handler = NETWORK_PROV_EVENT_HANDLER_NONE,
#endif /* CONFIG_APP_NETWORK_PROV_TRANSPORT_BLE */
    };

    /* Initialize provisioning manager with the
     * configuration parameters set above */
    ESP_ERROR_CHECK(network_prov_mgr_init(config));
#else // RMAKER_USING_NETWORK_PROV
    /* Configuration for the provisioning manager */
    wifi_prov_mgr_config_t config = {
        /* What is the Provisioning Scheme that we want ?
         * wifi_prov_scheme_softap or wifi_prov_scheme_ble */
#ifdef CONFIG_APP_NETWORK_PROV_TRANSPORT_BLE
        .scheme = wifi_prov_scheme_ble,
#else /* CONFIG_APP_NETWORK_PROV_TRANSPORT_SOFTAP */
        .scheme = wifi_prov_scheme_softap,
#endif /* CONFIG_APP_NETWORK_PROV_TRANSPORT_BLE */

        /* Any default scheme specific event handler that you would
         * like to choose. Since our example application requires
         * neither BT nor BLE, we can choose to release the associated
         * memory once provisioning is complete, or not needed
         * (in case when device is already provisioned). Choosing
         * appropriate scheme specific event handler allows the manager
         * to take care of this automatically. This can be set to
         * WIFI_PROV_EVENT_HANDLER_NONE when using wifi_prov_scheme_softap*/
#ifdef CONFIG_APP_NETWORK_PROV_TRANSPORT_BLE
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
#else /* CONFIG_APP_NETWORK_PROV_TRANSPORT_SOFTAP */
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE,
#endif /* CONFIG_APP_NETWORK_PROV_TRANSPORT_BLE */
    };

    /* Initialize provisioning manager with the
     * configuration parameters set above */
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));
#endif // RMAKER_USING_NETWORK_PROV
    /* Let's find out if the device is provisioned */
#if RMAKER_USING_NETWORK_PROV
    network_prov_mgr_is_wifi_provisioned(provisioned);
#else
    wifi_prov_mgr_is_provisioned(provisioned);
#endif
    /* If device is not yet provisioned start provisioning service */
    if (!(*provisioned)) {
        ESP_LOGI(TAG, "Starting provisioning");
#ifdef ESP_NETIF_SUPPORTED
#if CONFIG_ESP_WIFI_SOFTAP_SUPPORT
        esp_netif_create_default_wifi_ap();
#endif
#endif
        /* What is the security level that we want (0 or 1):
         *      - NETWORK_PROV_SECURITY_0/WIFI_PROV_SECURITY_0 is simply plain text communication.
         *      - NETWORK_PROV_SECURITY_1/WIFI_PROV_SECURITY_1 is secure communication which consists of secure handshake
         *          using X25519 key exchange and proof of possession (pop) and AES-CTR
         *          for encryption/decryption of messages.
         */
#if RMAKER_USING_NETWORK_PROV
        network_prov_security_t security = NETWORK_PROV_SECURITY_1;
#else
        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
#endif

#ifdef CONFIG_APP_NETWORK_PROV_TRANSPORT_BLE
        /* This step is only useful when scheme is wifi_prov_scheme_ble. This will
         * set a custom 128 bit UUID which will be included in the BLE advertisement
         * and will correspond to the primary GATT service that provides provisioning
         * endpoints as GATT characteristics. Each GATT characteristic will be
         * formed using the primary service UUID as base, with different auto assigned
         * 12th and 13th bytes (assume counting starts from 0th byte). The client side
         * applications must identify the endpoints by reading the User Characteristic
         * Description descriptor (0x2901) for each characteristic, which contains the
         * endpoint name of the characteristic */
        uint8_t custom_service_uuid[] = {
            /* This is a random uuid. This can be modified if you want to change the BLE uuid. */
            /* 12th and 13th bit will be replaced by internal bits. */
            0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
            0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02,
        };
#if RMAKER_USING_NETWORK_PROV
        esp_err_t err = network_prov_scheme_ble_set_service_uuid(custom_service_uuid);
#else
        esp_err_t err = wifi_prov_scheme_ble_set_service_uuid(custom_service_uuid);
#endif
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "wifi_prov_scheme_ble_set_service_uuid failed %d", err);
            return err;
        }
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 2, 0)
        if (mfg_data) {
#if RMAKER_USING_NETWORK_PROV
            err = network_prov_scheme_ble_set_mfg_data(mfg_data, mfg_data_len);
#else
            err = wifi_prov_scheme_ble_set_mfg_data(mfg_data, mfg_data_len);
#endif
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to set mfg data, err=0x%x", err);
                return err;
            }
        }
#endif
#endif /* CONFIG_APP_NETWORK_PROV_TRANSPORT_BLE */

        /* Start provisioning service */
#if RMAKER_USING_NETWORK_PROV
        ESP_ERROR_CHECK(network_prov_mgr_start_provisioning(security, pop, service_name, service_key));
#else
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, pop, service_name, service_key));
#endif
    } else {
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");
        /* We don't need the manager as device is already provisioned,
         * so let's release it's resources */
#if RMAKER_USING_NETWORK_PROV
        network_prov_mgr_deinit();
#else
        wifi_prov_mgr_deinit();
#endif

        /* Start Wi-Fi station */
        wifi_init_sta();
    }
    return ESP_OK;
#else /* CONFIG_ESP_RMAKER_NETWORK_OVER_THREAD */
    return ESP_ERR_NOT_SUPPORTED;
#endif /* CONFIG_ESP_RMAKER_NETWORK_OVER_WIFI */
}
