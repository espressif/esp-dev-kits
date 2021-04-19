#include "https_request.h"

static const char *TAG = "https_request";
#define LOG_TRACE(...)	ESP_LOGD(TAG, ##__VA_ARGS__)
#define RSP_OUTPUT      (0)

esp_err_t https_request(https_request_t *req)
{
    int ret_val = 0, msg_length = 0, total_length = 0;
    size_t bytes_written = 0;

    esp_tls_cfg_t esp_tls_cfg = {
        .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms = 30000,
    };

    struct esp_tls *tls = esp_tls_conn_http_new(req->url, &esp_tls_cfg);

    if(NULL != tls) {
        LOG_TRACE("Connection established");
    } else {
        ESP_LOGE(TAG, "Connection failed");
        esp_tls_conn_delete(tls);
        return ESP_FAIL;
    }
    
    do {
        ret_val = esp_tls_conn_write(
            tls, req->req + bytes_written, strlen(req->req) - bytes_written);
        if (ret_val >= 0) {
            LOG_TRACE("%d bytes written", ret_val);
            bytes_written += ret_val;
        } else if (ret_val != ESP_TLS_ERR_SSL_WANT_READ  && ret_val != ESP_TLS_ERR_SSL_WANT_WRITE) {
            ESP_LOGE(TAG, "esp_tls_conn_write  returned 0x%x", ret_val);
            esp_tls_conn_delete(tls);
            return ESP_FAIL;
        }
    } while(bytes_written < strlen(req->req));

    LOG_TRACE("Reading HTTP response");

    /* Appedix '\0' */
    bzero(req->rsp, req->rsp_size);

    do {
        msg_length = esp_tls_conn_read(tls, (char *)(req->rsp + total_length), req->rsp_size);

        if((ESP_TLS_ERR_SSL_WANT_WRITE == ret_val)  || (ESP_TLS_ERR_SSL_WANT_READ == ret_val)) {
            continue;
        }

        /*!< Rest bytes can't be negative, error occurred */
        if(msg_length < 0) {
            ESP_LOGE(TAG, "esp_tls_conn_read  returned -0x%x", -ret_val);
            break;
        }

        /*!< Rest bytes is 0, response over */
        if(0 == msg_length) {
            LOG_TRACE("connection closed");
            break;
        }

        LOG_TRACE("%d bytes read", msg_length);
        total_length += msg_length;
    } while(1);

#if RSP_OUTPUT
    /* Print response directly to stdout as it is read */
    for(int i = 0; i < total_length; i++) {
        putchar(req->rsp[i]);
    }
    putchar('\n');
#endif

    esp_tls_conn_delete(tls);

    return ESP_OK;
}

esp_err_t https_request_get_msg_body(const char *msg, char **body)
{
    static const char *needle = "\r\n\r\n";

    if ((NULL == msg) || (NULL == body)) {
        return ESP_FAIL;
    }

    *body = strstr(msg, "\r\n\r\n");

    if (NULL == *body) {
        return ESP_FAIL;
    }

    *body += strlen(needle);

    return ESP_OK;
}
