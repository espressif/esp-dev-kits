/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "virtual_mic.h"

static const char *TAG = "vmic";

typedef struct {
    virtual_mic_config_t cfg;
    uint32_t bytes_per_ms;
    uint64_t last_ts;

    const uint8_t *buf;
    uint32_t buf_len;
    uint32_t offset;
} vmic_data_t;

static uint64_t ts = 0;

vmic_handle_t virtual_mic_create(const virtual_mic_config_t *cfg)
{
    vmic_data_t *vmic = heap_caps_calloc(1, sizeof(vmic_data_t), MALLOC_CAP_INTERNAL);
    ESP_RETURN_ON_FALSE(NULL != vmic, NULL, TAG, "No memory for virtual mic");

    vmic->cfg = *cfg;
    vmic->bytes_per_ms = (cfg->sample_rate * cfg->ch_num * cfg->data_width / 8) / 1000;
    vmic->last_ts = esp_timer_get_time();

    return vmic;
}

esp_err_t virtual_mic_delete(vmic_handle_t handle)
{
    vmic_data_t *vmic = (vmic_data_t *)handle;
    if (vmic) {
        heap_caps_free(vmic);
    }
    return ESP_OK;
}

esp_err_t virtual_mic_set_clk(vmic_handle_t handle, uint32_t rate, uint32_t bits_cfg, uint32_t ch)
{
    vmic_data_t *vmic = (vmic_data_t *)handle;
    ESP_RETURN_ON_FALSE(NULL != vmic, ESP_ERR_INVALID_ARG, TAG, "handle is NULL");
    vmic->cfg.ch_num = ch;
    vmic->cfg.data_width = bits_cfg;
    vmic->cfg.sample_rate = rate;
    vmic->bytes_per_ms = (rate * ch * bits_cfg / 8) / 1000;
    return ESP_OK;
}

esp_err_t virtual_mic_read(vmic_handle_t handle, void *dest, size_t size, size_t *bytes_read, TickType_t ticks_to_wait)
{
    vmic_data_t *vmic = (vmic_data_t *)handle;
    ESP_RETURN_ON_FALSE(NULL != vmic, ESP_ERR_INVALID_ARG, TAG, "handle is NULL");
    uint32_t duration_ms = size / vmic->bytes_per_ms;
    int64_t dt = vmic->last_ts + duration_ms * 1000 - esp_timer_get_time();
    uint8_t *out = dest;
    if (dt <= 0) {
        vmic->last_ts = esp_timer_get_time();
        if (vmic->buf) {
            uint32_t remain = vmic->buf_len - vmic->offset;
            if (remain >= size) {
                memcpy(out, vmic->buf + vmic->offset, size);
                vmic->offset += size;
            } else {
                memcpy(out, vmic->buf + vmic->offset, remain);
                vmic->offset = 0;
                memcpy(out + remain, vmic->buf + vmic->offset, size - remain);
                vmic->offset += size;
            }
        } else {
            memset(out, 0, size);
        }
        *bytes_read = size;
    } else {
        if (vmic->buf) {
            uint32_t remain = vmic->buf_len - vmic->offset;
            if (remain >= size) {
                memcpy(out, vmic->buf + vmic->offset, size);
                vmic->offset += size;
            } else {
                memcpy(out, vmic->buf + vmic->offset, remain);
                vmic->offset = 0;
                memcpy(out + remain, vmic->buf + vmic->offset, size - remain);
                vmic->offset += size;
            }
        } else {
            memset(out, 0, size);
        }
        *bytes_read = size;
        TickType_t xLastWakeTime = xTaskGetTickCount();
        xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1 + dt / 1000));
        vmic->last_ts = esp_timer_get_time();
    }
    ESP_LOGI(TAG, "t=%ums", (uint32_t)(esp_timer_get_time() - ts) / 1000);
    ts = esp_timer_get_time();
    return ESP_OK;
}

esp_err_t virtual_mic_input_file(vmic_handle_t handle, const char *filepath)
{
    vmic_data_t *vmic = (vmic_data_t *)handle;
    ESP_RETURN_ON_FALSE(NULL != vmic, ESP_ERR_INVALID_ARG, TAG, "handle is NULL");
    return ESP_OK;
}

esp_err_t virtual_mic_input_buffer(vmic_handle_t handle, const uint8_t *inbuf, size_t length)
{
    vmic_data_t *vmic = (vmic_data_t *)handle;
    ESP_RETURN_ON_FALSE(NULL != vmic, ESP_ERR_INVALID_ARG, TAG, "handle is NULL");
    vmic->buf = inbuf;
    vmic->offset = 0;
    vmic->buf_len = length;
    return ESP_OK;
}




// static void audio_test_task(void *pvParam)
// {
//     const board_res_desc_t *brd = bsp_board_get_description();
//     size_t bytes_read = 0;
//     size_t read_len = 0;
//     int time_s = 3;
//     size_t buf_len = 16000 * time_s * sizeof(int16_t) * 2;
//     /* Allocate audio buffer and check for result */
//     int16_t *audio_buffer = heap_caps_malloc(buf_len, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
//     if (NULL == audio_buffer) {
//         esp_system_abort("No mem for audio buffer");
//     }

//     if (!brd->FUNC_I2S_EN) {
//         // bsp_adc_init();

//         char *wave_get(void);
//         uint32_t wave_get_size(void);
//         uint32_t wave_get_framerate(void);
//         uint32_t wave_get_bits(void);
//         uint32_t wave_get_ch(void);

//         // virtual_mic_input_buffer((const uint8_t *)wave_get(), wave_get_size());
//     }
//     uint64_t ts = esp_timer_get_time();


//     while (true) {

//         bsp_codec_read(audio_buffer, 1024 * I2S_CHANNEL_NUM * sizeof(int16_t), &bytes_read, portMAX_DELAY);
//         read_len += bytes_read;
//         ESP_LOGI(TAG, "t=%ums", (uint32_t)(esp_timer_get_time() - ts) / 1000);
//         ts = esp_timer_get_time();


//         if (read_len >= buf_len) {
//             ESP_LOGI(TAG, "playstart");
//             size_t num;
//             bsp_codec_write(audio_buffer, buf_len, &num, portMAX_DELAY);
//             read_len = 0;
//             ESP_LOGI(TAG, "playend");
//         }

//     }
// }

//    ret_val = xTaskCreatePinnedToCore(audio_test_task, "test Task", 4 * 1024, g_sr_data->afe_data, 5, &g_sr_data->feed_task, 1);
//     ESP_GOTO_ON_FALSE(pdPASS == ret_val, ESP_FAIL, err, TAG,  "Failed create audio test task");