// Copyright 2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "app_httpd.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "decode_url.h"
#include "chinese_tts.h"

extern const unsigned char index_setting_page_html_gz_start[] asm("_binary_settingPage_html_gz_start");
extern const unsigned char index_setting_page_html_gz_end[]   asm("_binary_settingPage_html_gz_end");

extern const unsigned char index_setting_succ_html_gz_start[] asm("_binary_SaveResponseSucc_html_gz_start");
extern const unsigned char index_setting_succ_html_gz_end[]   asm("_binary_SaveResponseSucc_html_gz_end");

extern const unsigned char index_setting_error_html_gz_start[] asm("_binary_SaveResponseError_html_gz_start");
extern const unsigned char index_setting_error_html_gz_end[]   asm("_binary_SaveResponseError_html_gz_end");

static const char *TAG = "ap_config";

httpd_handle_t camera_httpd = NULL;

#define REC_MAX 512

static esp_err_t setting_handler(httpd_req_t *req)
{
    char  *buf;
    size_t buf_len;
    char sentence[REC_MAX] = {0,};
    size_t index_setting_succ_html_gz_len = index_setting_succ_html_gz_end - index_setting_succ_html_gz_start;
    size_t index_setting_error_html_gz_len = index_setting_error_html_gz_end - index_setting_error_html_gz_start;
    size_t index_setting_page_html_gz_len = index_setting_page_html_gz_end - index_setting_page_html_gz_start;
    esp_tts_voice_t *voice = &esp_tts_voice_xiaole;
    esp_tts_handle_t *tts_handle = esp_tts_create(voice);

    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");

    buf_len = httpd_req_get_url_query_len(req) + 1;

    if (buf_len > 1) {
        buf = (char *)malloc(buf_len);

        if (!buf) {
            httpd_resp_send(req, (const char *)index_setting_error_html_gz_start, index_setting_error_html_gz_len);
            return ESP_FAIL;
        }

        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            if (httpd_query_key_value(buf, "sentence", sentence, sizeof(sentence)) == ESP_OK) {
            } else {
                free(buf);
                httpd_resp_send(req, (const char *)index_setting_error_html_gz_start, index_setting_error_html_gz_len);
                return ESP_FAIL;
            }

        } else {
            free(buf);
            httpd_resp_send(req, (const char *)index_setting_error_html_gz_start, index_setting_error_html_gz_len);
            return ESP_FAIL;
        }

        free(buf);
    } else {
        httpd_resp_send(req, (const char *)index_setting_error_html_gz_start, index_setting_error_html_gz_len);
        return ESP_FAIL;
    }

    url_decode(sentence);
    ESP_LOGI(TAG, "sentence = %s", sentence);
    tts_output_chinese(tts_handle, sentence);
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");

    return ESP_OK;
}

static esp_err_t index_handler(httpd_req_t *req)
{
    size_t index_setting_page_html_gz_len = index_setting_page_html_gz_end - index_setting_page_html_gz_start;

    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");

    httpd_resp_send(req, (const char *)index_setting_page_html_gz_start, index_setting_page_html_gz_len);
    return ESP_OK;
}

void app_httpd_main()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t index_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = index_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t setting_uri = {
        .uri       = "/setting",
        .method    = HTTP_GET,
        .handler   = setting_handler,
        .user_ctx  = NULL
    };

    ESP_LOGI(TAG, "Starting web server on port: '%d'", config.server_port);

    if (httpd_start(&camera_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(camera_httpd, &index_uri);
        httpd_register_uri_handler(camera_httpd, &setting_uri);
    }
}
