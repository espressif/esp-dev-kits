#ifndef ESPNOW_LITE_H
#define ESPNOW_LITE_H

#include <assert.h>
#include <string.h>

#include "esp_crc.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_now.h"
#include "esp_random.h"
#include "esp_wifi.h"

#define CHANNELMAX 14

#define ESPNOW_PAYLOADMAX 250
#define ESPNOW_PAYLOADLEN 4

#define ESPNOW_HEADLEN 3
#define ESPNOW_SENDLEN (ESPNOW_PAYLOADLEN + ESPNOW_HEADLEN)

#define ESPNOW_BROADCAST 1
#define ESPNOW_UNICAST 0

typedef enum {
    ESPNOW_SEND_CB,
    ESPNOW_RECV_CB,
} espnow_event_id_t;
typedef struct {
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    esp_now_send_status_t status;
} espnow_event_send_cb_t;

typedef struct {
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    int data_len;
    uint8_t *data;
} espnow_event_recv_cb_t;
typedef union {
    espnow_event_send_cb_t send_cb;
    espnow_event_recv_cb_t recv_cb;
} espnow_event_info_t;

/* When ESPNOW sending or receiving callback function is called, post event to ESPNOW task. */
typedef struct {
    espnow_event_id_t id;
    espnow_event_info_t info;
} espnow_event_t;

/* User defined field of ESPNOW data */
typedef struct {
    uint8_t casttype; // Broadcast or unicast ESPNOW data.
    uint16_t crc; // CRC16 value of ESPNOW data.
    uint8_t payload[0]; // Real payload of ESPNOW data.

} __attribute__((packed)) espnow_data_t;

/* User defined field of ESPNOW data */
typedef struct {
    uint8_t casttype; // Broadcast or unicast ESPNOW data.
    uint16_t loadlen;
    uint8_t mac_addr[ESP_NOW_ETH_ALEN]; // MAC address of destination device.
    uint8_t payload[ESPNOW_PAYLOADMAX]; // Real payload of ESPNOW data.

} espnow_receive_data_t;

/* Parameters of sending ESPNOW data. */
typedef struct {
    uint8_t casttype; // ESPNOW cast data type.
    int len; // Length of ESPNOW data to be sent, unit: byte.
    uint8_t *buffer; // Buffer pointing to ESPNOW data.
    uint8_t dest_mac[ESP_NOW_ETH_ALEN]; // MAC address of destination device.
} espnow_send_param_t;

typedef enum {
    ESPNOW_ACK = 0,
    ESPNOW_BIND,
    ESPNOW_UNBIND,
    ESPNOW_BINDACK,
    BUTTON1_SINGLE_CLICK,
    BUTTON1_DOUBLE_CLICK,
    BUTTON1_LONG_PRESS,
    BUTTON2_SINGLE_CLICK,
    BUTTON2_DOUBLE_CLICK,
    BUTTON2_LONG_PRESS,
    BUTTON3_SINGLE_CLICK,
    BUTTON3_DOUBLE_CLICK,
    BUTTON3_LONG_PRESS,

} espnow_payload_t;
typedef enum {
    WAIT_BIND_DEVICE,
    BIND_DEVICE,
    WAIT_INFO,
    SEND_INFO,
} espnow_task_evt_t;
void espnow_wifi_init();
esp_err_t espnow_init(void);
esp_err_t espnow_send(const uint8_t *dest_mac, uint8_t *payload, uint16_t payloadlen);
int16_t NVS_read_peer_list_size(void);
esp_err_t NVS_read_peer_list(esp_now_peer_info_t *peer, int16_t peer_number);
#endif
