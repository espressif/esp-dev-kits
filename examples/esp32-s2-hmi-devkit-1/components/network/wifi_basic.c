#include "wifi_basic.h"

static const char *TAG = "wifi_basic";

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define EXAMPLE_ESP_MAXIMUM_RETRY   (5)

static uint16_t g_ap_count = 0;
static wifi_ap_record_t *g_ap_info = NULL;
static xSemaphoreHandle wifi_scan_lock = NULL;

/*!< Wi-Fi STA config */
static int s_retry_num = 0;
static wifi_config_t wifi_config;

/*!< Event group for WI-Fi STA */
static EventGroupHandle_t s_wifi_event_group;

/*!< Static task forward declaration */
static void wifi_scan_task(void *pvParam);
static void wifi_connect_task(void *pvParam);

static void wifi_sta_event_handler(void *, esp_event_base_t, int32_t, void *);

esp_err_t wifi_sta_start(void)
{
    /*!< Register Wi-Fi event group */
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());


    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_sta_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_sta_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    if (NULL == wifi_scan_lock) {
        wifi_scan_lock = xSemaphoreCreateBinary();
        xSemaphoreGive(wifi_scan_lock);
    }

    return ESP_OK;
}

esp_err_t wifi_sta_wait_for_connect(TickType_t ticks_to_wait, bool clear_bits)
{

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        clear_bits,
        pdFALSE,
        ticks_to_wait);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap");
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to AP");
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    s_retry_num = 0;

    return ESP_FAIL;
}

esp_err_t wifi_sta_set_ssid(const char *ssid)
{
    strcpy((char *)wifi_config.sta.ssid, ssid);
    return ESP_OK;
}

esp_err_t wifi_sta_set_pswd(const char *pswd)
{
    strcpy((char *)wifi_config.sta.password, pswd);
    return ESP_OK;
}

esp_err_t wifi_sta_connect(void)
{
    xTaskCreate(
		(TaskFunction_t)        wifi_connect_task,
		(const char * const)    "Wi-Fi Connect Task",
		(const uint32_t)        4 * 1024,
		(void * const)          NULL,
		(UBaseType_t)           configMAX_PRIORITIES - 3,
		(TaskHandle_t * const)  NULL);
    return ESP_OK;
}

/*!< ******************************** WIFI SCAN FUNCTION(S) ********************************/

esp_err_t wifi_scan_start(void)
{
    /*!< Check if still in scanning */
    if (pdPASS != xSemaphoreTake(wifi_scan_lock, 0)) {
        ESP_LOGE(TAG, "Other scanning procedure is still running");
        return ESP_FAIL;
    } else {
        BaseType_t task_state = xTaskCreate(
            (TaskFunction_t)        wifi_scan_task,
            (const char * const)    "Wi-Fi Scan Task",
            (const uint32_t)        4 * 1024,
            (void * const)          NULL,
            (UBaseType_t)           configMAX_PRIORITIES - 3,
            (TaskHandle_t * const)  NULL);
        if (pdPASS != task_state) {
            ESP_LOGE(TAG, "Failed create task");
            return ESP_FAIL;
        }
    }
    return ESP_OK;
}

void wifi_scan_get_count(uint16_t *p_ap_count)
{
    *p_ap_count = g_ap_count;
}

void wifi_scan_get_ap_info(size_t index, wifi_ap_record_t **p_info)
{
    *p_info = &(g_ap_info[index]);
}

void wifi_scan_free_memory(void)
{
    free(g_ap_info);
}

/*!< ******************************** TASK(S) ********************************/

esp_err_t wifi_scan_wait_done(TickType_t ticks_to_wait)
{
    if (pdPASS != xSemaphoreTake(wifi_scan_lock, ticks_to_wait)) {
        return ESP_FAIL;
    } else {
        xSemaphoreGive(wifi_scan_lock);
    }
    return ESP_OK;
}

static void wifi_connect_task(void *pvParam)
{
    (void) pvParam;

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());
    vTaskDelete(NULL);
}

static void wifi_scan_task(void *pvParam)
{
    (void) pvParam;
    static uint16_t wifi_scan_num = 20;

    /* allocate memory for Wi-Fi scan reseult */
    if (NULL == g_ap_info) {
        ESP_LOGI(TAG, "AP list buffer not allocated. Try to allocate...");
        g_ap_info = heap_caps_malloc(wifi_scan_num * sizeof(wifi_ap_record_t), MALLOC_CAP_SPIRAM);
    } else {
        ESP_LOGI(TAG, "AP list buffer is allocated. Try to reallocate...");
        free(g_ap_info);
        g_ap_info = heap_caps_malloc(wifi_scan_num * sizeof(wifi_ap_record_t), MALLOC_CAP_SPIRAM);
    }

    memset(g_ap_info, 0, wifi_scan_num * sizeof(wifi_ap_record_t));
    
    /* Start Wi-Fi scan */
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&wifi_scan_num, g_ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&g_ap_count));

    xSemaphoreGive(wifi_scan_lock);

    ESP_LOGI(TAG, "Total APs scanned = %u", g_ap_count);

    vTaskDelete(NULL);
}

/*!< ******************************** EVENT HANDLER(S) ********************************/

static void wifi_sta_event_handler(
    void* arg, esp_event_base_t event_base,
    int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}