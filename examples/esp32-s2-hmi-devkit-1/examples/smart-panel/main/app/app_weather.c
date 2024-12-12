/**
 * @file app_weather.c
 * @brief Weather app src.
 * @version 0.1
 * @date 2021-04-16
 * 
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "app_weather.h"

static const char *TAG = "app_weather";

#define LOG_TRACE(...)  ESP_LOGI(TAG, ##__VA_ARGS__)
#define CHECK(a, str, ret_val) do { \
        if (!(a)) { \
            ESP_LOGE(TAG,"%s(%d): %s", __FUNCTION__, __LINE__, str); \
            return (ret_val); \
        } \
    } while(0)

/* User Agent string */
#define _UA_                        "esp-idf/1.0 esp32s2"
/* QWeather key string, get it at https://console.qweather.com/#/apps */
#define _KEY_VALUE_                 CONFIG_QWEATHER_REQUEST_KEY
#define _KEY_                       "key=" _KEY_VALUE_
#define QWEATHER_REQUEST_KEY_LEN    (32)

/**
 * @brief HTTPS request param. You can found location code at QWeather API page.
 *      And please keep gzip=n, otherwise you need to unzip the data by your self and use more Flash and RAM.
 * 
 */
#define _OPTION_                    "location=101020600&gzip=n&lang=en"

/* See more at https://dev.qweather.com/docs/api/ */
#define WEATHER_SERVER  "devapi.qweather.com"
#define WEB_URL_NOW "https://" WEATHER_SERVER "/v7/weather/now?" _OPTION_ "&" _KEY_
#define WEB_URL_AIR "https://" WEATHER_SERVER "/v7/air/now?"     _OPTION_ "&" _KEY_
#define WEB_URL_7D  "https://" WEATHER_SERVER "/v7/weather/7d?"  _OPTION_ "&" _KEY_
#define WEB_URL_24H "https://" WEATHER_SERVER "/v7/weather/24h?" _OPTION_ "&" _KEY_

/* Macro to build https request URL */
#define URL_CONSTRUCT(WEB_URL, WEB_SERVER) \
    "GET "          WEB_URL     " HTTP/1.0\r\n" \
    "Host: "        WEB_SERVER  "\r\n" \
    "User-Agent: "  _UA_        "\r\n" \
    "\r\n"

/* Data parse functions */
typedef esp_err_t (*parse_func_t)(char *);
static esp_err_t app_weather_parse_now(char *buffer);
static esp_err_t app_weather_parse_air(char *buffer);
static esp_err_t app_weather_parse_7d(char *buffer);
static esp_err_t app_weather_parse_24h(char *buffer);

/* Data parse function list */
static parse_func_t parse_func[] = {
    app_weather_parse_now,
    app_weather_parse_air,
    app_weather_parse_24h,
    app_weather_parse_7d,
};

/* HTTPS request list */
static https_request_t request_list[] = {
    { .url = WEB_URL_NOW, .rsp = NULL, .rsp_size = 4096, .req = URL_CONSTRUCT(WEB_URL_NOW, WEATHER_SERVER)},
    { .url = WEB_URL_AIR, .rsp = NULL, .rsp_size = 4096, .req = URL_CONSTRUCT(WEB_URL_AIR, WEATHER_SERVER)},
    { .url = WEB_URL_24H, .rsp = NULL, .rsp_size = 8192, .req = URL_CONSTRUCT(WEB_URL_24H, WEATHER_SERVER)},
    { .url = WEB_URL_7D , .rsp = NULL, .rsp_size = 8192, .req = URL_CONSTRUCT(WEB_URL_7D , WEATHER_SERVER)},
};

static air_info_t *air_info = NULL;
static weather_info_t *weather_info = NULL;
static weather_daily_info_t *weather_daily_info[7] = { [0 ... 6] = NULL };
static weather_hourly_info_t *weather_hourly_info[24] = { [0 ... 23] = NULL };

static void app_weather_task(void *pvParam)
{
    (void) pvParam;

    TickType_t tick;
    char *rx_msg = NULL;

    /* Allocate size for HTTPS response data */
    for (int i = 0; i < sizeof(request_list) / sizeof(request_list[0]); i++) {
        request_list[i].rsp = heap_caps_malloc(request_list[i].rsp_size, MALLOC_CAP_SPIRAM);
    }

    /* Allocate mem for weather data */
    app_weather_get_air_info(NULL);

    for (size_t i = 0; i < sizeof(weather_daily_info) / sizeof(weather_daily_info[0]); i++) {
        app_weather_get_daily_info(i, NULL);
    }

    for (size_t i = 0; i < sizeof(weather_hourly_info) / sizeof(weather_hourly_info[0]); i++) {
        app_weather_get_hourly_info(i, NULL);
    }

    app_weather_get_current_info(NULL);

    tick = xTaskGetTickCount();

    /* Start HTTPS request */
    while (1) {
        for (int i = 0; i < sizeof(request_list) / sizeof(request_list[0]); i++) {
            https_request(&request_list[i]);
            https_request_get_msg_body(request_list[i].rsp, &rx_msg);

            if (NULL != rx_msg) {
                parse_func[i](rx_msg);
            }
        }

        /* Wait until next minutes */
        vTaskDelayUntil(&tick, pdMS_TO_TICKS(60 * 1000));
    }
}

esp_err_t app_weather_start(void)
{
    CHECK(QWEATHER_REQUEST_KEY_LEN == strlen(CONFIG_QWEATHER_REQUEST_KEY),
        "Invalid QWeather request key.\n"
        "Please register at https://console.qweather.com/#/apps",
        ESP_ERR_INVALID_ARG);

    CHECK(pdPASS == xTaskCreate(
        (TaskFunction_t)        app_weather_task,
        (const char * const)    "Weather Task",
        (const uint32_t)        4 * 1024,
        (void * const)          NULL,
        (UBaseType_t)           1,
        (TaskHandle_t * const)  NULL),
        "No mem for weather task",
        ESP_ERR_NO_MEM);

    return ESP_OK;
}

static esp_err_t app_weather_parse_now(char *buffer)
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

            /* Print parse result */
            ESP_LOGI(TAG, "Temp : [%s]", json_item_temp->valuestring);
            ESP_LOGI(TAG, "Icon : [%s]", json_item_icon->valuestring);
            ESP_LOGI(TAG, "Text : [%s]", json_item_text->valuestring);
            ESP_LOGI(TAG, "Humid: [%s]", json_item_humidity->valuestring);

            strcpy(weather_info->temp, json_item_temp->valuestring);
            strcpy(weather_info->humid, json_item_humidity->valuestring);
            strcpy(weather_info->describe, json_item_text->valuestring);
            strcpy(weather_info->icon_code, json_item_icon->valuestring);
            strcpy(weather_info->temp, json_item_temp->valuestring);
        } else {
            ESP_LOGE(TAG, "Error parsing object - [%s] - [%d]", __FILE__, __LINE__);
            return ESP_FAIL;
        }

        /* Delete object */
        cJSON_Delete(json);
    } else {
        ESP_LOGE(TAG, "Error parsing object - [%s] - [%d]", __FILE__, __LINE__);
        return ESP_FAIL;
    }

    return ESP_OK;
}

static esp_err_t app_weather_parse_air(char *buffer)
{
    cJSON *json = cJSON_Parse(buffer);
    cJSON *json_air = NULL;
    
    if (NULL != json) {
        json_air = cJSON_GetObjectItem(json, "now");

        if (NULL != json_air) {
            cJSON *json_item_category = cJSON_GetObjectItem(json_air, "category");
            cJSON *json_item_pm2p5 = cJSON_GetObjectItem(json_air, "pm2p5");
            cJSON *json_item_aqi = cJSON_GetObjectItem(json_air, "aqi");
            cJSON *json_item_pm10 = cJSON_GetObjectItem(json_air, "pm10");
            cJSON *json_item_no2 = cJSON_GetObjectItem(json_air, "no2");
            cJSON *json_item_so2 = cJSON_GetObjectItem(json_air, "so2");
            cJSON *json_item_co = cJSON_GetObjectItem(json_air, "co");
            cJSON *json_item_o3 = cJSON_GetObjectItem(json_air, "o3");

            LOG_TRACE("aqi  : %s", json_item_aqi->valuestring);
            LOG_TRACE("level: %s" , json_item_category->valuestring);
            LOG_TRACE("pm10 : %s", json_item_pm10->valuestring);
            LOG_TRACE("pm2.5 : %s", json_item_pm2p5->valuestring);
            LOG_TRACE("no2  : %s", json_item_no2->valuestring);
            LOG_TRACE("so2  : %s", json_item_so2->valuestring);
            LOG_TRACE("co   : %s", json_item_co->valuestring);
            LOG_TRACE("o3   : %s", json_item_o3->valuestring);

            strcpy(air_info->aqi, json_item_aqi->valuestring);
            strcpy(air_info->level, json_item_category->valuestring);
            strcpy(air_info->co, json_item_co->valuestring);
            strcpy(air_info->no2, json_item_no2->valuestring);
            strcpy(air_info->o3, json_item_o3->valuestring);
            strcpy(air_info->pm2p5, json_item_pm2p5->valuestring);
            strcpy(air_info->pm10, json_item_pm10->valuestring);
            strcpy(air_info->so2, json_item_so2->valuestring);

            ui_air_update_value();
            ui_clock_update();

            sensor_data_update_stop();
        } else {
            ESP_LOGE(TAG, "Error parsing object - [%s] - [%d]", __FILE__, __LINE__);
            return ESP_FAIL;
        }

        /* Delete object */
        cJSON_Delete(json);
    } else {
        ESP_LOGE(TAG, "Error parsing object - [%s] - [%d]", __FILE__, __LINE__);
        return ESP_FAIL;
    }

    
    return ESP_OK;
}

static esp_err_t app_weather_parse_7d(char *buffer)
{
    cJSON *json = cJSON_Parse(buffer);
    cJSON *json_day = NULL;
    cJSON *json_day_data = NULL;
    char fmt_text[16];

    size_t json_arr_len = 0;

    if (NULL != json) {
        json_day = cJSON_GetObjectItem(json, "daily");

        if (NULL != json_day) {
            json_arr_len = cJSON_GetArraySize(json_day);

            for (size_t i = 0; i < json_arr_len; i++) {
                json_day_data = cJSON_GetArrayItem(json_day, i);
                LOG_TRACE("%s~%s°C", cJSON_GetObjectItem(json_day_data, "tempMin")->valuestring, cJSON_GetObjectItem(json_day_data, "tempMax")->valuestring);
                sprintf(fmt_text, "%s~%s°C", cJSON_GetObjectItem(json_day_data, "tempMin")->valuestring, cJSON_GetObjectItem(json_day_data, "tempMax")->valuestring);
                strcpy(weather_daily_info[i]->temp_range, fmt_text);
                strcpy(weather_daily_info[i]->icon_code, cJSON_GetObjectItem(json_day_data, "iconDay")->valuestring);
                strcpy(weather_daily_info[i]->describe, cJSON_GetObjectItem(json_day_data, "textDay")->valuestring);
            }

            lv_port_sem_take();
            weather_update_widget_caption();
            lv_port_sem_give();
        } else {
            ESP_LOGE(TAG, "Error parsing object - [%s] - [%d]", __FILE__, __LINE__);
        }

        /* Delete object */
        cJSON_Delete(json);
    } else {
        ESP_LOGE(TAG, "Error parsing object - [%s] - [%d]", __FILE__, __LINE__);
    }

    return ESP_OK;
}

static esp_err_t app_weather_parse_24h(char *buffer)
{
    size_t json_arr_len = 0;
    cJSON *json = cJSON_Parse(buffer);
    cJSON *json_hour = NULL;
    cJSON *json_hour_data = NULL;

    if (NULL != json) {
        json_hour = cJSON_GetObjectItem(json, "hourly");

        if (NULL != json_hour) {
            json_arr_len = cJSON_GetArraySize(json_hour);

            for (size_t i = 0; i < json_arr_len; i++) {
                json_hour_data = cJSON_GetArrayItem(json_hour, i);
                if (NULL != json_hour_data) {
                    LOG_TRACE("Temp[%d] : %s", i, cJSON_GetObjectItem(json_hour_data, "temp")->valuestring);
                    strcpy(weather_hourly_info[i]->temp, cJSON_GetObjectItem(json_hour_data, "temp")->valuestring);
                } else {
                    ESP_LOGE(TAG, "Error parsing object - [%s] - [%d]", __FILE__, __LINE__);
                    return ESP_FAIL;
                }
            }

            /* Parse done, update to chart */
            lv_port_sem_take();
            weather_update_widget_caption();
            lv_port_sem_give();
        } else {
            ESP_LOGE(TAG, "Error parsing object - [%s] - [%d]", __FILE__, __LINE__);
            return ESP_FAIL;
        }

        /* Delete object */
        cJSON_Delete(json);
    } else {
        ESP_LOGE(TAG, "Error parsing object - [%s] - [%d]", __FILE__, __LINE__);
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

esp_err_t app_weather_get_current_info(weather_info_t *info)
{
    if (NULL == weather_info) {
        weather_info = heap_caps_malloc(sizeof(weather_info_t), MALLOC_CAP_SPIRAM);

        CHECK(weather_info, "Failed allocate mem", ESP_ERR_NO_MEM);

        weather_info->temp = heap_caps_malloc(8, MALLOC_CAP_SPIRAM);
        weather_info->humid = heap_caps_malloc(8, MALLOC_CAP_SPIRAM);
        weather_info->describe = heap_caps_malloc(16, MALLOC_CAP_SPIRAM);
        weather_info->icon_code = heap_caps_malloc(8, MALLOC_CAP_SPIRAM);
        weather_info->uv_val = heap_caps_malloc(8, MALLOC_CAP_SPIRAM);

        CHECK(weather_info->temp, "Failed allocate mem", ESP_ERR_NO_MEM);
        CHECK(weather_info->humid, "Failed allocate mem", ESP_ERR_NO_MEM);
        CHECK(weather_info->describe, "Failed allocate mem", ESP_ERR_NO_MEM);
        CHECK(weather_info->icon_code, "Failed allocate mem", ESP_ERR_NO_MEM);
        CHECK(weather_info->uv_val, "Failed allocate mem", ESP_ERR_NO_MEM);

        strcpy(weather_info->temp, "0°C");
        strcpy(weather_info->humid, "N/A");
        strcpy(weather_info->describe, "N/A");
        strcpy(weather_info->icon_code, "999");
        strcpy(weather_info->uv_val, "3.0");
    }

    CHECK(info, "Parsing NULL of info", ESP_ERR_INVALID_ARG);
    
    memcpy(info, weather_info, sizeof(weather_info_t));

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
