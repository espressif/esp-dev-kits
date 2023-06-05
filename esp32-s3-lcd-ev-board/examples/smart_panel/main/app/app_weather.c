/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "app_weather.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_http_client.h"

#include "zlib.h"
#include "zutil.h"
#include "inftrees.h"
#include "inflate.h"

static const char *TAG = "app_weather";

#define LOG_TRACE(...)  ESP_LOGI(TAG, ##__VA_ARGS__)
#define CHECK(a, str, ret_val) do { \
        if (!(a)) { \
            ESP_LOGE(TAG,"%s(%d): %s", __FUNCTION__, __LINE__, str); \
            return (ret_val); \
        } \
    } while(0)


#define MAX_HTTP_RECV_BUFFER            (1024)
/**
 * @brief User Agent string
 *
 */
#define _UA_                            "esp32_S3_86_box"
/**
 * @brief key string, get it at https://console.qweather.com/#/apps
 *
 */
#define QWEATHER_REQUEST_KEY_LEN        (32)
#define _KEY_VALUE_                     CONFIG_QWEATHER_REQUEST_KEY
#define _KEY_                           "key=" _KEY_VALUE_

/**
 * @brief HTTPS request param. You can found location code at QWeather API page.
 *      And please keep gzip=n, otherwise you need to unzip the data by your self and use more Flash and RAM.
 *
 */
#define LOCATION_SHANGHAI               "101020100"
#define LOCATION_BEIJING                "101010100"
#define LOCATION_SHENZHEN               "101280601"

#define _OPTION_                        "location=101020100&gzip=n&lang=zh"
#define _OPTION_MULTI                   "location=%s&lang=en"

/**
 * @brief See more at https://dev.qweather.com/docs/api/
 *
 */
#define WEATHER_SERVER      "devapi.qweather.com"
#define WEB_URL_NOW         "https://" WEATHER_SERVER "/v7/weather/now?" _OPTION_MULTI "&" _KEY_
#define WEB_URL_AIR         "https://" WEATHER_SERVER "/v7/air/now?"     _OPTION_ "&" _KEY_

static esp_err_t app_weather_parse_now(char *buffer, location_num_t location);

static air_info_t *air_info = NULL;
static weather_info_t *weather_info[LOCATION_NUM_MAX];

static weather_daily_info_t *weather_daily_info[7] = { [0 ... 6] = NULL };
static weather_hourly_info_t *weather_hourly_info[24] = { [0 ... 23] = NULL };

static int network_gzip_decompress(void *in_buf, size_t in_size, void *out_buf, size_t *out_size, size_t out_buf_size)
{
    int err = 0;
    z_stream d_stream = {0}; /* decompression stream */
    d_stream.zalloc = NULL;
    d_stream.zfree = NULL;
    d_stream.opaque = NULL;
    d_stream.next_in  = in_buf;
    d_stream.avail_in = 0;
    d_stream.next_out = out_buf;

    if ((err = inflateInit2(&d_stream, 47)) != Z_OK) {
        return err;
    }
    while (d_stream.total_out < out_buf_size - 1 && d_stream.total_in < in_size) {
        d_stream.avail_in = d_stream.avail_out = 1;
        if ((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) {
            break;
        }
        if (err != Z_OK) {
            return err;
        }
    }

    if ((err = inflateEnd(&d_stream)) != Z_OK) {
        return err;
    }

    *out_size = d_stream.total_out;
    ((char *)out_buf)[*out_size] = '\0';

    return Z_OK;
}

esp_err_t response_handler(esp_http_client_event_t *evt)
{
    static char *data = NULL;
    static int data_len = 0;

    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        printf("HTTP_EVENT_ERROR\n");
        break;

    case HTTP_EVENT_ON_CONNECTED:
        data_len = 0;
        printf("HTTP_EVENT_ON_CONNECTED\n");
        break;

    case HTTP_EVENT_HEADER_SENT:
        printf("HTTP_EVENT_HEADER_SENT\n");
        break;

    case HTTP_EVENT_ON_HEADER:
        if (evt->data_len) {
            printf("HTTP_EVENT_ON_HEADER\n");
            printf("%.*s", evt->data_len, (char *)evt->user_data);
        }
        break;

    case HTTP_EVENT_ON_DATA:
        printf("HTTP_EVENT_ON_DATA (%d +)%d\n", data_len, evt->data_len);
        data = heap_caps_realloc(data, data_len + evt->data_len + 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (data == NULL) {
            printf("data realloc failed\n");
            free(data);
            data = NULL;
            break;
        }
        memcpy(data + data_len, (char *)evt->data, evt->data_len);
        data_len += evt->data_len;
        data[data_len] = '\0';
        break;

    case HTTP_EVENT_ON_FINISH:
        size_t out_size = 0;
        location_num_t location = (location_num_t)evt->user_data;
        size_t decode_maxlen = data_len * 2;
        char *decode_out = heap_caps_malloc(decode_maxlen, MALLOC_CAP_SPIRAM);
        if (NULL == decode_out) {
            printf("Failed allocate mem\n");
            break;
        }
        network_gzip_decompress(data, data_len, decode_out, &out_size, decode_maxlen);
        printf("HTTP_EVENT_ON_FINISH, location:%d, out_size:%d, %s\n", location, out_size, decode_out);
        app_weather_parse_now(decode_out, location);

        if (decode_out) {
            free(decode_out);
            decode_out = NULL;
        }
        break;

    case HTTP_EVENT_DISCONNECTED:
        printf("HTTP_EVENT_DISCONNECTED\n");
        if (data) {
            free(data);
            data = NULL;
        }
        break;

    default:
        break;
    }

    return ESP_OK;
}

esp_err_t app_weather_request(location_num_t location)
{
    char *cityID = NULL;

    char *url_request = heap_caps_malloc(200, MALLOC_CAP_SPIRAM);
    CHECK(url_request, "Failed allocate mem", ESP_ERR_NO_MEM);

    switch (location) {
    case LOCATION_NUM_SHANGHAI:
        cityID = LOCATION_SHANGHAI;
        break;
    case LOCATION_NUM_BEIJING:
        cityID = LOCATION_BEIJING;
        break;
    case LOCATION_NUM_SHENZHEN:
        cityID = LOCATION_SHENZHEN;
        break;
    default:
        cityID = LOCATION_SHANGHAI;
        break;
    }
    sprintf(url_request, WEB_URL_NOW, cityID);
    ESP_LOGI(TAG, "url_request:%s\r\n", url_request);

    esp_http_client_config_t config = {
        .url = url_request,
        .method = HTTP_METHOD_GET,
        .event_handler = response_handler,
        .buffer_size = MAX_HTTP_RECV_BUFFER,
        .timeout_ms = 5000,
        .user_data = (void *)location,
    };
    // Set the headers
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Host", WEATHER_SERVER);
    esp_http_client_set_header(client, "User-Agent", _UA_);
    esp_http_client_set_header(client, "Accept-Encoding", "deflate");
    esp_http_client_set_header(client, "Cache-Control", "no-cache");
    esp_http_client_set_header(client, "Accept", "*/*");

    // Send the request
    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "HTTP POST request failed: %s\n", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);

    if (url_request) {
        heap_caps_free(url_request);
    }

    return ESP_OK;
}

esp_err_t app_weather_start(void)
{
    CHECK(QWEATHER_REQUEST_KEY_LEN == strlen(CONFIG_QWEATHER_REQUEST_KEY),
          "Invalid QWeather request key.\n"
          "Please register at https://console.qweather.com/#/apps",
          ESP_ERR_INVALID_ARG);

    // app_weather_get_air_info(NULL);

    for (int i = 0; i < LOCATION_NUM_MAX; i++) {
        if (NULL == weather_info[i]) {
            weather_info[i] = heap_caps_malloc(sizeof(weather_info_t), MALLOC_CAP_SPIRAM);
            CHECK(weather_info[i], "Failed allocate mem", ESP_ERR_NO_MEM);

            weather_info[i]->describe = heap_caps_malloc(16, MALLOC_CAP_SPIRAM);
            CHECK(weather_info[i]->describe, "Failed allocate mem", ESP_ERR_NO_MEM);

            weather_info[i]->temp = 20;
            weather_info[i]->icon_code = 104;
            weather_info[i]->humid = 80;
            strcpy(weather_info[i]->describe, "N/A");
        }
    }

    return ESP_OK;
}

static esp_err_t app_weather_parse_now(char *buffer, location_num_t location)
{
    cJSON *json = cJSON_Parse(buffer);
    cJSON *json_now = NULL;

    if (NULL != json) {
        json_now = cJSON_GetObjectItem(json, "now");
        if (NULL != json_now) {
            cJSON *json_item_temp = cJSON_GetObjectItem(json_now, "temp");
            cJSON *json_item_icon = cJSON_GetObjectItem(json_now, "icon");
            cJSON *json_item_text = cJSON_GetObjectItem(json_now, "text");
            cJSON *json_item_humidity = cJSON_GetObjectItem(json_now, "humidity");

            ESP_LOGI(TAG, "Temp : [%s]", json_item_temp->valuestring);
            ESP_LOGI(TAG, "Icon : [%s]", json_item_icon->valuestring);
            ESP_LOGI(TAG, "Text : [%s]", json_item_text->valuestring);
            ESP_LOGI(TAG, "Humid: [%s]", json_item_humidity->valuestring);

            weather_info[location]->temp = atoi(json_item_temp->valuestring);
            weather_info[location]->icon_code = atoi(json_item_icon->valuestring);
            weather_info[location]->humid = atoi(json_item_humidity->valuestring);
            strcpy(weather_info[location]->describe, json_item_text->valuestring);
        } else {
            ESP_LOGE(TAG, "Error parsing object - [%s] - [%d]", __FILE__, __LINE__);
            return ESP_FAIL;
        }
        cJSON_Delete(json);
    } else {
        ESP_LOGE(TAG, "Error parsing object - [%s] - [%d]", __FILE__, __LINE__);
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t app_weather_get_current_info(weather_info_t *info, location_num_t location)
{
    CHECK(info, "Parsing NULL of info", ESP_ERR_INVALID_ARG);
    if (weather_info[location]) {
        memcpy(info, weather_info[location], sizeof(weather_info_t));
        return ESP_OK;
    }

    return ESP_FAIL;
}

esp_err_t app_weather_get_air_info(air_info_t *info)
{
    if (NULL == air_info) {
        air_info = heap_caps_malloc(sizeof(air_info_t), MALLOC_CAP_SPIRAM);
        CHECK(air_info, "Failed allocate mem", ESP_ERR_NO_MEM);

        air_info->aqi = heap_caps_malloc(8, MALLOC_CAP_SPIRAM);
        air_info->level = heap_caps_malloc(16, MALLOC_CAP_SPIRAM);
        air_info->co = heap_caps_malloc(8, MALLOC_CAP_SPIRAM);
        air_info->no2 = heap_caps_malloc(8, MALLOC_CAP_SPIRAM);
        air_info->o3 = heap_caps_malloc(8, MALLOC_CAP_SPIRAM);
        air_info->pm2p5 = heap_caps_malloc(8, MALLOC_CAP_SPIRAM);
        air_info->pm10 = heap_caps_malloc(8, MALLOC_CAP_SPIRAM);
        air_info->so2 = heap_caps_malloc(8, MALLOC_CAP_SPIRAM);

        CHECK(air_info->aqi, "Failed allocate mem", ESP_ERR_NO_MEM);
        CHECK(air_info->level, "Failed allocate mem", ESP_ERR_NO_MEM);
        CHECK(air_info->co, "Failed allocate mem", ESP_ERR_NO_MEM);
        CHECK(air_info->no2, "Failed allocate mem", ESP_ERR_NO_MEM);
        CHECK(air_info->o3, "Failed allocate mem", ESP_ERR_NO_MEM);
        CHECK(air_info->pm2p5, "Failed allocate mem", ESP_ERR_NO_MEM);
        CHECK(air_info->pm10, "Failed allocate mem", ESP_ERR_NO_MEM);
        CHECK(air_info->so2, "Failed allocate mem", ESP_ERR_NO_MEM);

        strcpy(air_info->aqi, "18");
        strcpy(air_info->level, "Excellent");
        strcpy(air_info->co, "0.5");
        strcpy(air_info->no2, "12");
        strcpy(air_info->o3, "101");
        strcpy(air_info->pm2p5, "5");
        strcpy(air_info->pm10, "15");
        strcpy(air_info->so2, "3");
    }

    CHECK(info, "Parsing NULL of info", ESP_ERR_INVALID_ARG);

    memcpy(info, air_info, sizeof(air_info_t));

    return ESP_OK;
}

esp_err_t app_weather_get_daily_info(size_t day, weather_daily_info_t *info)
{
    if (NULL == weather_daily_info[day]) {
        weather_daily_info[day] = heap_caps_malloc(sizeof(weather_daily_info_t), MALLOC_CAP_SPIRAM);
        CHECK(weather_daily_info[day], "Failed allocate mem", ESP_ERR_NO_MEM);

        weather_daily_info[day]->describe = heap_caps_malloc(16, MALLOC_CAP_SPIRAM);
        weather_daily_info[day]->icon_code = heap_caps_malloc(8, MALLOC_CAP_SPIRAM);
        weather_daily_info[day]->temp_range = heap_caps_malloc(16, MALLOC_CAP_SPIRAM);
        CHECK(weather_daily_info[day]->describe, "Failed allocate mem", ESP_ERR_NO_MEM);
        CHECK(weather_daily_info[day]->icon_code, "Failed allocate mem", ESP_ERR_NO_MEM);
        CHECK(weather_daily_info[day]->temp_range, "Failed allocate mem", ESP_ERR_NO_MEM);

        /* Give default value */
        strcpy(weather_daily_info[day]->describe, "N/A");
        strcpy(weather_daily_info[day]->icon_code, "999");
        strcpy(weather_daily_info[day]->temp_range, "N/A");
    }

    CHECK(day < 5, "Invalid arg of day", ESP_ERR_INVALID_ARG);
    CHECK(info, "Parsing NULL of info", ESP_ERR_INVALID_ARG);

    memcpy(info, weather_daily_info[day], sizeof(weather_daily_info_t));

    return ESP_OK;
}

esp_err_t app_weather_get_hourly_info(size_t hour, weather_hourly_info_t *info)
{
    CHECK(hour < 24, "Invalid arg of hour", ESP_ERR_INVALID_ARG);

    if (NULL == weather_hourly_info[hour]) {
        weather_hourly_info[hour] = heap_caps_malloc(sizeof(weather_hourly_info_t), MALLOC_CAP_SPIRAM);
        CHECK(weather_hourly_info[hour], "Failed allocate mem", ESP_ERR_NO_MEM);

        weather_hourly_info[hour]->temp = heap_caps_malloc(8, MALLOC_CAP_SPIRAM);
        CHECK(weather_hourly_info[hour]->temp, "Failed allocate mem", ESP_ERR_NO_MEM);

        strcpy(weather_hourly_info[hour]->temp, "10");
    }

    CHECK(info, "Parsing NULL of info", ESP_ERR_INVALID_ARG);

    memcpy(info, weather_hourly_info[hour], sizeof(weather_hourly_info_t));

    return ESP_OK;
}
