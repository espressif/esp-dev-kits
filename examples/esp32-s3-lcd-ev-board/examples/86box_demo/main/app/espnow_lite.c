#include "espnow_lite.h"
#include "nvs.h"
#include "nvs_flash.h"
static const char *TAG = "espnow_lite";
#define STORAGE_NAMESPACE "storage"
#define ESPNOW_MAXDELAY 512
static QueueHandle_t s_espnow_queue;
#define ESPNOW_QUEUE_SIZE 6
#define ESPNOW_RECEIVE_QUEUE_SIZE 2
espnow_send_param_t *send_param;

#define WAITBIND_MAXDELAY 5000
#define SEND_MAXDELAY 5
#define WAITCALL_MAXDELAY 5

#define CONFIG_ESPNOW_PMK   "pmk1234567890123"

#define RESEND_COUNTMAX (3 * CHANNELMAX)
#define ISBROADCAST(addr) (((addr)[0] & (addr)[1] & (addr)[2] & (addr)[3] & (addr)[4] & (addr)[5]) == 0xFF)
static uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF,
                                                   0xFF, 0xFF, 0xFF
                                                 };

TaskHandle_t espnowTask = NULL;

extern TaskHandle_t buttonTask;
/* ESPNOW sending or receiving callback function is called in WiFi task.
 * Users should not do lengthy operations from this task. Instead, post
 * necessary data to a queue and handle it from a lower priority task. */
void espnow_send_cb(uint8_t *mac_addr, esp_now_send_status_t status)
{
    espnow_event_t evt;
    espnow_event_send_cb_t *send_cb = &evt.info.send_cb;

    if (mac_addr == NULL) {
        ESP_LOGE(TAG, "Send cb arg error");
        return;
    }

    evt.id = ESPNOW_SEND_CB;
    memcpy(send_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    send_cb->status = status;
    if (xQueueSend(s_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Send send queue fail");
    }
}

static void espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len)
{
    static uint8_t swicth_status;

    ESP_LOGE(TAG, "espnow_recv_cb 111 " MACSTR "", MAC2STR(mac_addr));
    swicth_status++;

    if (swicth_status % 2) {
        //bsp_led_set_rgb(0, 255, 0, 0);
    } else {
        //bsp_led_set_rgb(0, 0, 0, 0);
    }
    // espnow_event_t evt;
    // espnow_event_recv_cb_t* recv_cb = &evt.info.recv_cb;
    // if (mac_addr == NULL || data == NULL || len <= 0) {
    //     ESP_LOGE(TAG, "Receive cb arg error");
    //     return;
    // }
    // if (data[0] != ESPNOW_BROADCAST && data[0] != ESPNOW_UNICAST) {
    //     ESP_LOGE(TAG, "Receive cb data error");
    //     return;
    // }

    // evt.id = ESPNOW_RECV_CB;
    // memcpy(recv_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    // recv_cb->data = malloc(len);
    // if (recv_cb->data == NULL) {
    //     ESP_LOGE(TAG, "Malloc receive data fail");
    //     return;
    // }
    // memcpy(recv_cb->data, data, len);
    // recv_cb->data_len = len;
    // if (xQueueSend(s_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
    //     ESP_LOGW(TAG, "Send receive queue fail");
    //     free(recv_cb->data);
    // }
}

int16_t NVS_read_peer_list_size(void)
{
    size_t required_size = 0; // value will default to 0, if not set yet in NVS
    nvs_handle_t my_handle;
    esp_err_t err;
    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        return -1;
    }
    err = nvs_get_blob(my_handle, "peer_list", NULL, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        return -1;
    }
    // Close
    nvs_close(my_handle);
    return required_size / sizeof(esp_now_peer_info_t);
}

esp_err_t NVS_read_peer_list(esp_now_peer_info_t *peer, int16_t peer_number)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    size_t size = sizeof(esp_now_peer_info_t) * peer_number;
    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_get_blob(my_handle, "peer_list", peer, &size);
    if (err != ESP_OK) {
        return err;
    }
    // Close
    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t NVS_save_peer_list(esp_now_peer_info_t *peer, int16_t peer_number)
{
    size_t required_size = peer_number * sizeof(esp_now_peer_info_t);
    nvs_handle_t my_handle;
    esp_err_t err;
    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        return err;
    }
    err = nvs_set_blob(my_handle, "peer_list", peer, required_size);
    if (err != ESP_OK) {
        return err;
    }

    // Commit
    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        return err;
    }

    // Close
    nvs_close(my_handle);
    return ESP_OK;
}

/* Prepare ESPNOW data to be sent. */
void espnow_data_prepare(espnow_send_param_t *send_param,
                         uint8_t *payload,
                         uint16_t loadlen)
{
    espnow_data_t *buf = (espnow_data_t *)send_param->buffer;
    assert(send_param->len >= sizeof(espnow_data_t));
    buf->casttype = send_param->casttype;
    buf->crc = 0;
    memcpy(buf->payload, payload, loadlen);
    buf->crc = esp_crc16_le(UINT16_MAX, (uint8_t const *)buf, send_param->len);
}
/* Parse received ESPNOW data. */
esp_err_t espnow_data_parse(uint8_t *data,
                            uint16_t data_len,
                            espnow_payload_t *res)
{
    if (data_len != ESPNOW_SENDLEN) {
        ESP_LOGE(TAG, "data len error");
        return ESP_FAIL;
    }
    espnow_data_t *buf = (espnow_data_t *)data;
    uint16_t crc, crc_cal = 0;
    crc = buf->crc;
    buf->crc = 0;
    crc_cal = esp_crc16_le(UINT16_MAX, (uint8_t const *)data, data_len);
    if (crc == crc_cal) {
        memcpy(res, buf->payload, data_len - ESPNOW_HEADLEN);
        free(data);
        return ESP_OK;
    }
    free(data);
    return ESP_FAIL;
}

esp_err_t espnow_send(const uint8_t *dest_mac, uint8_t *payload, uint16_t payloadlen)
{
    memcpy(send_param->dest_mac, dest_mac, ESP_NOW_ETH_ALEN);
    send_param->casttype = ISBROADCAST(send_param->dest_mac);
    send_param->len = ESPNOW_HEADLEN + payloadlen;
    if (send_param->len > ESPNOW_PAYLOADMAX) {
        ESP_LOGE(TAG, "Too large payloadlen");
        return ESP_FAIL;
    }

    send_param->buffer = malloc(send_param->len);
    if (send_param->buffer == NULL) {
        ESP_LOGE(TAG, "Malloc send buffer fail");
        return ESP_FAIL;
    }
    espnow_data_prepare(send_param, payload, payloadlen);
    if (esp_now_send(send_param->dest_mac, send_param->buffer,
                     send_param->len)
            != ESP_OK) {
        ESP_LOGE(TAG, "Send error");
    }
    free(send_param->buffer);
    return ESP_OK;
}
static void espnow_task(void *pvParameter)
{
    espnow_event_t evt;
    esp_err_t rst;
    uint8_t current_channel1 = 0;
    wifi_second_chan_t current_channel2;
    espnow_task_evt_t task_evt = WAIT_INFO;
    uint8_t resend_count = 0;
    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
    espnow_payload_t *send_data = malloc(sizeof(espnow_payload_t));
    espnow_payload_t *receive_data = malloc(sizeof(espnow_payload_t));
    espnow_event_recv_cb_t *recv_cb;
    if (peer == NULL) {
        ESP_LOGE(TAG, "Malloc peer information fail");
    }
    for (;;) {
        switch (task_evt) {
        case SEND_INFO: {
            memset(peer, 0, sizeof(esp_now_peer_info_t));
            rst = NVS_read_peer_list(peer, 1);
            if (rst == ESP_OK) {
                ESP_LOGI(TAG, "read  addr: " MACSTR " ,channel %d ,ifidx %d ,encrypt %d", MAC2STR(peer->peer_addr), peer->channel, peer->ifidx, peer->encrypt);
                *send_data = BUTTON1_SINGLE_CLICK;
                espnow_send(peer->peer_addr, (uint8_t *)send_data, sizeof(espnow_payload_t));
                ESP_LOGI(TAG, "EVT Send");
                if (xQueueReceive(s_espnow_queue, &evt, WAITCALL_MAXDELAY) == pdTRUE) {
                    recv_cb = &evt.info.recv_cb;
                    rst = espnow_data_parse(recv_cb->data, recv_cb->data_len, receive_data);
                    if (rst == ESP_OK && *receive_data == ESPNOW_ACK) {
                        ESP_LOGI(TAG, "EVT Send succeed!");
                        task_evt = WAIT_INFO;
                    }
                } else {
                    ESP_LOGW(TAG, "receive ack timeout");
                    esp_wifi_get_channel(&current_channel1, &current_channel2);
                    current_channel1 >= CHANNELMAX ? (current_channel1 = 1) : (current_channel1 += 1);
                    esp_wifi_set_channel(current_channel1, current_channel2);
                    resend_count++;
                }
                if (resend_count >= RESEND_COUNTMAX) {
                    ESP_LOGE(TAG, "resend count out");
                    resend_count = 0;
                    task_evt = WAIT_INFO;
                }
            } else {
                ESP_LOGI(TAG, "NVS_read_peer_list err,no: %d", rst);
            }

            break;
        }
        case BIND_DEVICE: {

            peer->channel = 0;
            peer->ifidx = ESP_IF_WIFI_STA;
            peer->encrypt = false;
            memcpy(peer->peer_addr, broadcast_mac, ESP_NOW_ETH_ALEN);
            *send_data = ESPNOW_BIND;

            espnow_send(broadcast_mac, (uint8_t *)send_data, sizeof(espnow_payload_t));
            ESP_LOGI(TAG, "BIND_DEVICE Send");
            if (xQueueReceive(s_espnow_queue, &evt, WAITCALL_MAXDELAY) == pdTRUE) {
                recv_cb = &evt.info.recv_cb;
                rst = espnow_data_parse(recv_cb->data, recv_cb->data_len, receive_data);
                if (rst == ESP_OK && *receive_data == ESPNOW_BINDACK) {
                    ESP_LOGI(TAG, "BIND_DEVICE succeed!");
                    if (esp_now_is_peer_exist(recv_cb->mac_addr)) {
                        ESP_LOGI(TAG, "addr already in list");
                        task_evt = WAIT_INFO;
                    } else {
                        memset(peer, 0, sizeof(esp_now_peer_info_t));
                        peer->channel = 0;
                        peer->ifidx = ESP_IF_WIFI_STA;
                        peer->encrypt = false;
                        memcpy(peer->peer_addr, recv_cb->mac_addr, ESP_NOW_ETH_ALEN);
                        ESP_ERROR_CHECK(esp_now_add_peer(peer));
                        NVS_save_peer_list(peer, 1);
                        ESP_LOGI(TAG, "add addr in list");
                    }
                    *send_data = ESPNOW_BINDACK;
                    espnow_send(broadcast_mac, (uint8_t *)send_data, sizeof(espnow_payload_t));

                    resend_count = 0;
                    task_evt = WAIT_INFO;
                } else {
                    ESP_LOGE(TAG, "ack err");
                    current_channel1 >= CHANNELMAX ? (current_channel1 = 1) : (current_channel1 += 1);
                    esp_wifi_set_channel(current_channel1, current_channel2);
                    resend_count++;
                }
            } else {
                ESP_LOGW(TAG, "receive ack timeout");
                esp_wifi_get_channel(&current_channel1, &current_channel2);
                current_channel1 >= CHANNELMAX ? (current_channel1 = 1) : (current_channel1 += 1);
                esp_wifi_set_channel(current_channel1, current_channel2);
                resend_count++;
            }
            if (resend_count >= RESEND_COUNTMAX) {
                ESP_LOGE(TAG, "resend count out");
                resend_count = 0;
                task_evt = WAIT_INFO;
            }
            break;
        }
        case WAIT_BIND_DEVICE: {
            ESP_LOGI(TAG, "WaitBind device");

            if (xQueueReceive(s_espnow_queue, &evt, WAITBIND_MAXDELAY) == pdTRUE) {
                recv_cb = &evt.info.recv_cb;
                rst = espnow_data_parse(recv_cb->data, recv_cb->data_len, receive_data);
                if ((rst == ESP_OK) && (*receive_data == ESPNOW_BIND)) {
                    ESP_LOGI(TAG, "received bind info.");
                    *send_data = ESPNOW_BINDACK;
                    espnow_send(broadcast_mac, (uint8_t *)send_data, sizeof(espnow_payload_t));
                    if (xQueueReceive(s_espnow_queue, &evt, WAITCALL_MAXDELAY) == pdTRUE) {
                        recv_cb = &evt.info.recv_cb;
                        rst = espnow_data_parse(recv_cb->data, recv_cb->data_len, receive_data);
                        if (rst == ESP_OK && *receive_data == ESPNOW_BINDACK) {

                            if (esp_now_is_peer_exist(recv_cb->mac_addr)) {
                                ESP_LOGI(TAG, "addr already in list");
                                ESP_ERROR_CHECK(esp_now_mod_peer(peer));
                                task_evt = WAIT_INFO;
                            } else {
                                ESP_LOGI(TAG, "WAIT_BIND_DEVICE succeed");
                                memset(peer, 0, sizeof(esp_now_peer_info_t));
                                peer->channel = 0;
                                peer->ifidx = ESP_IF_WIFI_STA;
                                peer->encrypt = false;
                                memcpy(peer->peer_addr, recv_cb->mac_addr, ESP_NOW_ETH_ALEN);
                                ESP_ERROR_CHECK(esp_now_add_peer(peer));
                                NVS_save_peer_list(peer, 1);
                                ESP_LOGI(TAG, "before addr: " MACSTR " ,channel %d ,ifidx %d ,encrypt %d", MAC2STR(peer->peer_addr), peer->channel, peer->ifidx, peer->encrypt);
                                free(peer);
                                task_evt = WAIT_INFO;
                            }
                        }
                    } else {
                        ESP_LOGE(TAG, "WAIT_BIND_DEVICE ack err,rst %d,receive_data,%d ,%d", rst, *receive_data, ESPNOW_BIND);
                    }
                }
            } else {
                ESP_LOGI(TAG, "WaitBind device timeout");
                task_evt = WAIT_INFO;
            }
            break;
        }
        case WAIT_INFO: {
            ESP_LOGI(TAG, "WAIT_INFO");
            if (xQueueReceive(s_espnow_queue, &evt, WAITBIND_MAXDELAY) == pdTRUE) {
                recv_cb = &evt.info.recv_cb;
                rst = espnow_data_parse(recv_cb->data, recv_cb->data_len, receive_data);
                if (rst == ESP_OK) {
                    ESP_LOGE(TAG, "received data from " MACSTR " ,data: %d ", MAC2STR(recv_cb->mac_addr), *receive_data);
                    if (*receive_data != ESPNOW_ACK) {
                        *send_data = ESPNOW_ACK;
                        espnow_send(recv_cb->mac_addr, (uint8_t *)send_data, sizeof(espnow_payload_t));
                        resend_count = 0;
                    }
                }
            }
        }
        default:
            break;
        }
    }
}

/* WiFi should start before using ESPNOW */
void espnow_wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK(esp_wifi_set_protocol(
                        ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR));
#endif
}

esp_err_t espnow_init(void)
{

    s_espnow_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(espnow_event_t));
    if (s_espnow_queue == NULL) {
        ESP_LOGE(TAG, "Create mutex fail");
        return ESP_FAIL;
    }
    send_param = malloc(sizeof(espnow_send_param_t));
    memset(send_param, 0, sizeof(espnow_send_param_t));
    if (send_param == NULL) {
        ESP_LOGE(TAG, "Malloc send parameter fail");
        // there are some thing need to do
    }
    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK(esp_now_init());
    // ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_send_cb));
    ESP_ERROR_CHECK(esp_now_register_recv_cb((esp_now_recv_cb_t)espnow_recv_cb));
#if CONFIG_ESP_WIFI_STA_DISCONNECTED_PM_ENABLE
    ESP_ERROR_CHECK(esp_now_set_wake_window(65535));
#endif
    /* Set primary master key. */
    ESP_ERROR_CHECK(esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK));
    /* Add broadcast peer information to peer list. */
    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) {
        ESP_LOGE(TAG, "Malloc peer information fail");
        esp_now_deinit();
        return ESP_FAIL;
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = 0;
    peer->ifidx = ESP_IF_WIFI_STA;
    peer->encrypt = false;
    memcpy(peer->peer_addr, broadcast_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK(esp_now_add_peer(peer));
    free(peer);

    // int16_t peer_size = NVS_read_peer_list_size();
    // ESP_LOGI(TAG, "peer_size %d", peer_size);
    // if (peer_size > 0) {
    //     esp_now_peer_info_t* peer = malloc(sizeof(esp_now_peer_info_t) * peer_size);
    //     NVS_read_peer_list(peer, 1);
    //     ESP_LOGI(TAG, "addr: " MACSTR " ,channel %d ,ifidx %d ,encrypt %d", MAC2STR(peer->peer_addr), peer->channel, peer->ifidx, peer->encrypt);
    //     ESP_ERROR_CHECK(esp_now_add_peer(peer));
    //     free(peer);
    // }

    /* Set espnow_rate */
    // esp_wifi_config_espnow_rate(ESP_IF_WIFI_STA,WIFI_PHY_RATE_48M);

    /* Initialize sending parameters. */
    send_param->casttype = ESPNOW_BROADCAST;
    memcpy(send_param->dest_mac, broadcast_mac, ESP_NOW_ETH_ALEN);
    //   xTaskCreate(espnow_task, "espnow_task", 2048, NULL, 4, &espnowTask);

    return ESP_OK;
}
