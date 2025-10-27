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
#include <sdkconfig.h>
#include <esp_event.h>
#include <esp_idf_version.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <esp_rmaker_utils.h>
#include <app_network.h>

#ifdef CONFIG_ESP_RMAKER_NETWORK_OVER_THREAD
#include <app_thread_internal.h>
#include <esp_vfs_eventfd.h>
#include <esp_openthread.h>
#include <esp_openthread_cli.h>
#include <esp_openthread_lock.h>
#include <esp_openthread_netif_glue.h>
#include <esp_openthread_types.h>

#include <openthread/cli.h>
#include <openthread/instance.h>
#include <openthread/logging.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>

#include <network_provisioning/manager.h>
#include <network_provisioning/scheme_ble.h>

static const char *TAG = "app_thread";
/* Event handler for catching system events */
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
#ifdef CONFIG_APP_NETWORK_RESET_PROV_ON_FAILURE
    static int retries = 0;
#endif
    if (event_base == NETWORK_PROV_EVENT) {
        switch (event_id) {
        case NETWORK_PROV_START:
            ESP_LOGI(TAG, "Provisioning started");
            break;
        case NETWORK_PROV_THREAD_DATASET_RECV: {
            break;
        }
        case NETWORK_PROV_THREAD_DATASET_FAIL: {
#ifdef CONFIG_APP_NETWORK_RESET_PROV_ON_FAILURE
            retries++;
            if (retries >= CONFIG_APP_NETWORK_PROV_MAX_RETRY_CNT) {
                ESP_LOGI(TAG, "Failed to connect with provisioned network, resetting provisioned dataset");
                network_prov_mgr_reset_thread_sm_state_on_failure();
                esp_event_post(APP_NETWORK_EVENT, APP_NETWORK_EVENT_PROV_RESTART, NULL, 0, portMAX_DELAY);
                retries = 0;
            }
#endif
            break;
        }
        case NETWORK_PROV_THREAD_DATASET_SUCCESS:
            ESP_LOGI(TAG, "Provisioning successful");
#ifdef CONFIG_APP_NETWORK_RESET_PROV_ON_FAILURE
            retries = 0;
#endif
            break;
        default:
            break;
        }
    }
}

static esp_netif_t *init_openthread_netif(const esp_openthread_platform_config_t *config)
{
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_OPENTHREAD();
    esp_netif_t *netif = esp_netif_new(&cfg);
    assert(netif != NULL);
    ESP_ERROR_CHECK(esp_netif_attach(netif, esp_openthread_netif_glue_init(config)));

    return netif;
}

static void ot_task_worker(void *aContext)
{
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    /* Initialize the OpenThread stack */
    ESP_ERROR_CHECK(esp_openthread_init(&config));
#if CONFIG_OPENTHREAD_LOG_LEVEL_DYNAMIC
    /* The OpenThread log level directly matches ESP log level */
    (void)otLoggingSetLevel(CONFIG_LOG_DEFAULT_LEVEL);
#endif
    esp_netif_t *openthread_netif = init_openthread_netif(&config);
    /* Initialize the esp_netif bindings */
    esp_netif_set_default_netif(openthread_netif);

    /* Run the main loop */
    esp_openthread_launch_mainloop();
    /* Clean up */
    esp_netif_destroy(openthread_netif);
    esp_openthread_netif_glue_deinit();

    esp_vfs_eventfd_unregister();
    vTaskDelete(NULL);
}
#endif /* CONFIG_ESP_RMAKER_NETWORK_OVER_THREAD */

esp_err_t thread_init()
{
#ifdef CONFIG_ESP_RMAKER_NETWORK_OVER_THREAD
    /* Initialize TCP/IP */
    esp_netif_init();

    /* Register our event handler for OpenThread and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_register(NETWORK_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    esp_vfs_eventfd_config_t eventfd_config = {
        .max_fds = 3,
    };
    ESP_ERROR_CHECK(esp_vfs_eventfd_register(&eventfd_config));
    xTaskCreate(ot_task_worker, "ot_task", 6144, xTaskGetCurrentTaskHandle(), 5, NULL);
    return ESP_OK;
#else
    return ESP_ERR_NOT_SUPPORTED;
#endif /* CONFIG_ESP_RMAKER_NETWORK_OVER_THREAD */
}

esp_err_t thread_start(const char *pop, const char *service_name, const char *service_key, uint8_t *mfg_data,
                       size_t mfg_data_len, bool *provisioned)
{
#ifdef CONFIG_ESP_RMAKER_NETWORK_OVER_THREAD
    /* Configuration for the provisioning manager */
    network_prov_mgr_config_t config = {
        .scheme = network_prov_scheme_ble,

        /* Any default scheme specific event handler that you would
         * like to choose. Since our example application requires
         * neither BT nor BLE, we can choose to release the associated
         * memory once provisioning is complete, or not needed
         * (in case when device is already provisioned). Choosing
         * appropriate scheme specific event handler allows the manager
         * to take care of this automatically.*/
        .scheme_event_handler = NETWORK_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
    };

    /* Initialize provisioning manager with the
     * configuration parameters set above */
    ESP_ERROR_CHECK(network_prov_mgr_init(config));

    /* Let's find out if the device is provisioned */
    ESP_ERROR_CHECK(network_prov_mgr_is_thread_provisioned(provisioned));
    /* If device is not yet provisioned start provisioning service */
    if (!(*provisioned)) {
        ESP_LOGI(TAG, "Starting provisioning");

        /* What is the security level that we want (0 or 1):
         *      - NETWORK_PROV_SECURITY_0 is simply plain text communication.
         *      - NETWORK_PROV_SECURITY_1 is secure communication which consists of secure handshake
         *          using X25519 key exchange and proof of possession (pop) and AES-CTR
         *          for encryption/decryption of messages.
         */
        network_prov_security_t security = NETWORK_PROV_SECURITY_1;

        /* This step is only useful when scheme is network_prov_scheme_ble. This will
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
        esp_err_t err = network_prov_scheme_ble_set_service_uuid(custom_service_uuid);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "thread_prov_scheme_ble_set_service_uuid failed %d", err);
            return err;
        }

        if (mfg_data) {
            err = network_prov_scheme_ble_set_mfg_data(mfg_data, mfg_data_len);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to set mfg data, err=0x%x", err);
                return err;
            }
        }

        /* Start provisioning service */
        ESP_ERROR_CHECK(network_prov_mgr_start_provisioning(security, pop, service_name, service_key));
    } else {
        ESP_LOGI(TAG, "Already provisioned, enabling netif and starting Thread");
        /* We don't need the manager as device is already provisioned,
         * so let's release it's resources */
        network_prov_mgr_deinit();

        esp_openthread_lock_acquire(portMAX_DELAY);
        otInstance *instance = esp_openthread_get_instance();
        (void)otIp6SetEnabled(instance, true);
        (void)otThreadSetEnabled(instance, true);
        esp_openthread_lock_release();
    }
    return ESP_OK;
#else
    return ESP_ERR_NOT_SUPPORTED;
#endif /* CONFIG_ESP_RMAKER_NETWORK_OVER_THREAD */
}
