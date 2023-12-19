/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_vfs.h"
#include "app_sr.h"

#include "lv_example_pub.h"

#include "bsp_board_extra.h"
#include "bsp/esp-bsp.h"
#include "app_sr_handler.h"
#include "settings.h"


static const char *TAG = "sr_handler";

static bool b_audio_playing = false;

//extern file_iterator_instance_t* file_iterator;

typedef enum {
    AUDIO_WAKE,
    AUDIO_OK,
    AUDIO_END,
    AUDIO_PRESS,
    AUDIO_MAX,
} audio_segment_t;

static esp_err_t sr_echo_play(audio_segment_t audio)
{
    typedef struct {
        // The "RIFF" chunk descriptor
        uint8_t ChunkID[4];
        int32_t ChunkSize;
        uint8_t Format[4];
        // The "fmt" sub-chunk
        uint8_t Subchunk1ID[4];
        int32_t Subchunk1Size;
        int16_t AudioFormat;
        int16_t NumChannels;
        int32_t SampleRate;
        int32_t ByteRate;
        int16_t BlockAlign;
        int16_t BitsPerSample;
        // The "data" sub-chunk
        uint8_t Subchunk2ID[4];
        int32_t Subchunk2Size;
    } wav_header_t;

    char filepath[30];
    FILE *fd = NULL;
    struct stat file_stat;
    size_t chunk_size = 4096;

    uint8_t *audio_buffer = NULL;
    sys_param_t *param = settings_get_parameter();

    char *files[2][4] = {
        {"echo_en_wake.wav", "echo_en_ok.wav", "echo_en_end.wav", "echo_en_wake.wav"},
        {"echo_cn_wake.wav", "echo_cn_ok.wav", "echo_cn_end.wav", "echo_en_wake.wav"},
    };
    sprintf(filepath, "%s/%s", CONFIG_BSP_SPIFFS_MOUNT_POINT, files[param->sr_lang][audio]);

    if (stat(filepath, &file_stat) == -1) {
        ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
        goto EXIT;
    }

    ESP_LOGI(TAG, "file stat info: %s (%ld bytes)...", filepath, file_stat.st_size);
    fd = fopen(filepath, "r");
    if (NULL == fd) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
        goto EXIT;
    }

    audio_buffer = malloc(chunk_size);
    if (NULL == audio_buffer) {
        ESP_LOGE(TAG, "audio data audio_buffer malloc failed");
        goto EXIT;
    }

    /**
     * read head of WAV file
     */
    wav_header_t wav_head;
    int len = fread(&wav_head, 1, sizeof(wav_header_t), fd);
    if (len <= 0) {
        ESP_LOGE(TAG, "Read wav header failed");
        goto EXIT;
    }
    if (NULL == strstr((char *)wav_head.Subchunk1ID, "fmt") &&
            NULL == strstr((char *)wav_head.Subchunk2ID, "data")) {
        ESP_LOGE(TAG, "Header of wav format error");
        goto EXIT;
    }

    bsp_extra_codec_set_fs(wav_head.SampleRate, wav_head.BitsPerSample, wav_head.NumChannels);
    ESP_LOGD(TAG, "frame_rate= %" PRIi32 ", ch=%d, width=%d", wav_head.SampleRate, wav_head.NumChannels, wav_head.BitsPerSample);

    bsp_extra_codec_mute_set(true);
    bsp_extra_codec_mute_set(false);
    bsp_extra_codec_volume_set(80, NULL);
    vTaskDelay(pdMS_TO_TICKS(50));

    size_t cnt;
    do {
        /* Read file in chunks into the scratch audio_buffer */
        len = fread(audio_buffer, 1, chunk_size, fd);
        if (len <= 0) {
            break;
        } else {
            if (bsp_extra_i2s_write(audio_buffer, len, &cnt, 1000) != ESP_OK) {
                ESP_LOGI(TAG, "Write Task: i2s write failed");
            }
        }
    } while (1);

EXIT:
    fclose(fd);
    if (audio_buffer) {
        free(audio_buffer);
    }

    return ESP_OK;
}

bool sr_echo_is_playing(void)
{
    return b_audio_playing;
}

sr_language_t sr_detect_language()
{
    static sr_language_t sr_current_lang = SR_LANG_MAX;
    const sys_param_t *param = settings_get_parameter();

    if (param->sr_lang ^ sr_current_lang) {
        sr_current_lang = param->sr_lang;
        ESP_LOGI(TAG, "boardcast language change to = %s", (SR_LANG_EN == param->sr_lang ? "EN" : "CN"));
    }
    return sr_current_lang;
}

void sr_handler_task(void *pvParam)
{
    static lv_event_info_t event;
    event.event = LV_EVENT_EXIT_CLOCK;

    while (true) {
        sr_result_t result;
        app_sr_get_result(&result, portMAX_DELAY);

        if (true == result.beep_enable) {
            sr_echo_play(AUDIO_PRESS);
            continue;
        }

        if (ESP_MN_STATE_TIMEOUT == result.state) {
#if !SR_RUN_TEST
            event.event = LV_EVENT_I_AM_LEAVING;
            app_lvgl_send_event(&event);
            sr_echo_play(AUDIO_END);
#endif
            continue;
        }

        if (WAKENET_DETECTED == result.wakenet_mode) {
            event.event = LV_EVENT_I_AM_HERE;
            app_lvgl_send_event(&event);
#if !SR_RUN_TEST
            sr_echo_play(AUDIO_WAKE);
#endif
            continue;
        }

        if (ESP_MN_STATE_DETECTED & result.state) {
            const sr_cmd_t *cmd = app_sr_get_cmd_from_id(result.command_id);
            if (NULL == cmd) {
                continue;
            }

            ESP_LOGI(TAG, "command:%s, act:%d", cmd->str, cmd->cmd);
            event.event_data = (void *)cmd->str;

            switch (cmd->cmd) {
            case SR_CMD_SET_RED:
                event.event = LV_EVENT_LIGHT_RGB_SET;
                app_lvgl_send_event(&event);
                break;
            case SR_CMD_SET_WHITE:
                event.event = LV_EVENT_LIGHT_RGB_SET;
                app_lvgl_send_event(&event);
                break;
            case SR_CMD_SET_YELLOW:
                event.event = LV_EVENT_LIGHT_RGB_SET;
                app_lvgl_send_event(&event);
                break;
            case SR_CMD_SET_BLUE:
                event.event = LV_EVENT_LIGHT_RGB_SET;
                app_lvgl_send_event(&event);
                break;
            case SR_CMD_LIGHT_ON:
                event.event = LV_EVENT_LIGHT_ON;
                app_lvgl_send_event(&event);
                break;
            case SR_CMD_LIGHT_OFF:
                event.event = LV_EVENT_LIGHT_OFF;
                app_lvgl_send_event(&event);
                break;

            case SR_CMD_AC_SET_ON:
                event.event = LV_EVENT_AC_SET_ON;
                app_lvgl_send_event(&event);
                break;
            case SR_CMD_AC_SET_OFF:
                event.event = LV_EVENT_AC_SET_OFF;
                app_lvgl_send_event(&event);
                break;
            case SR_CMD_AC_TEMP_ADD:
                event.event = LV_EVENT_AC_TEMP_ADD;
                app_lvgl_send_event(&event);
                break;
            case SR_CMD_AC_TEMP_DEC:
                event.event = LV_EVENT_AC_TEMP_DEC;
                app_lvgl_send_event(&event);
                break;
            default:
                ESP_LOGE(TAG, "Unknow cmd");
                break;
            }
#if !SR_RUN_TEST
            sr_echo_play(AUDIO_OK);
#endif
        }
    }
    vTaskDelete(NULL);
}
