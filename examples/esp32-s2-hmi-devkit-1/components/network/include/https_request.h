#pragma once

#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
// #include "nvs_flash.h"
// #include "protocol_examples_common.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "esp_tls.h"
#include "esp_crt_bundle.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *req;
    char *url;
    char *rsp;
    size_t rsp_size;
} https_request_t;

esp_err_t https_request(https_request_t *req);

esp_err_t https_request_get_msg_body(const char *msg, char **body);

#ifdef __cplusplus
}
#endif
