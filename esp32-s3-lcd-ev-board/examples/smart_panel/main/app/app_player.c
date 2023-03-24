/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "driver/i2s.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "app_player.h"
#include "bsp_board.h"
#include "bsp_codec.h"
#include "file_manager.h"

#define DR_MP3_IMPLEMENTATION
#include "../dr_libs/dr_mp3.h"

static const char *TAG = "player";

/* **************** AUDIO CALLBACK **************** */
static audio_cb_t s_audio_cb = NULL;
static void *audio_cb_usrt_ctx = NULL;
static QueueHandle_t audio_event_queue = NULL;
player_state_t g_player_state;
static drmp3 *mp3 = NULL;
static uint16_t g_file_num = 0;
static uint16_t audio_index = 0;
static char **g_file_list = NULL;
static uint64_t music_total_frame = 0;

static char *lrc_str;
static uint32_t *lrc_timestampe;
static uint32_t lrc_lines;

static esp_err_t load_current_lyric(void);
static void free_current_lyric(void);

esp_err_t app_player_callback_register(audio_cb_t call_back, void *user_ctx)
{
    s_audio_cb = call_back;
    audio_cb_usrt_ctx = user_ctx;

    return ESP_OK;
}


size_t app_player_get_index(void)
{
    return audio_index;
}

const char *app_player_get_name_from_index(size_t index)
{
    ESP_RETURN_ON_FALSE(index < g_file_num, NULL, TAG, "File index out of range");
    ESP_RETURN_ON_FALSE(NULL != g_file_list, NULL, TAG, "Audio file not found");
    ESP_RETURN_ON_FALSE(NULL != g_file_list[index], NULL, TAG, "Audio file not found");

    return g_file_list[index];
}

const char **app_player_get_file_list(size_t *length)
{
    ESP_RETURN_ON_FALSE(NULL != g_file_list, NULL, TAG, "Audio file not found");
    *length = g_file_num;
    return g_file_list;
}


static esp_err_t play_mp3(const char *path)
{
    esp_err_t ret = ESP_OK;
    ESP_LOGI(TAG, "start to decode %s", path);

    mp3 = malloc(sizeof(drmp3));
    ESP_RETURN_ON_FALSE(NULL != mp3, ESP_ERR_NO_MEM, TAG, "Failed allocate mp3 decoder");

    if (!drmp3_init_file(mp3, path, NULL)) {
        ESP_RETURN_ON_FALSE(false, ESP_FAIL, TAG, "Failed create MP3 decoder");
    }

    load_current_lyric();

    int sample_rate = 0;
    uint32_t bits_cfg = 0;
    uint32_t nChans = 0;
    player_event_t audio_event = AUDIO_EVENT_NONE;
    uint8_t *output = malloc(2048 * sizeof(int16_t) * 2);
    ESP_GOTO_ON_FALSE(NULL != output, ESP_ERR_NO_MEM, clean_up, TAG, "Failed allocate output buffer");

    /* Start MP3 decoding */
    ESP_LOGI(TAG, "drmp3_get_pcm_frame_count start");
    music_total_frame = drmp3_get_pcm_frame_count(mp3);
    drmp3_uint64 framesToRead = 2048;
    ESP_LOGI(TAG, "pcm_frame_count=%llu", music_total_frame);

    if (esp_ptr_executable(s_audio_cb)) {
        player_cb_ctx_t ctx = {
            .audio_event = AUDIO_EVENT_PLAY_BRFORE,
            .user_ctx = audio_cb_usrt_ctx,
        };
        s_audio_cb(&ctx);
    }

    do {
        /* Process audio event sent from other task */
        if (pdPASS == xQueueReceive(audio_event_queue, &audio_event, 0)) {
            if (AUDIO_EVENT_PAUSE == audio_event) {
                g_player_state = PLAYER_STATE_PAUSE;
                bsp_codec_zero_dma_buffer();
                xQueuePeek(audio_event_queue, &audio_event, portMAX_DELAY);
                if (esp_ptr_executable(s_audio_cb)) {
                    player_cb_ctx_t ctx = {
                        .audio_event = AUDIO_EVENT_PLAY,
                        .user_ctx = audio_cb_usrt_ctx,
                    };
                    s_audio_cb(&ctx);
                }
                continue;
            }

            if (AUDIO_EVENT_CHANGE == audio_event ||
                    AUDIO_EVENT_NEXT == audio_event ||
                    AUDIO_EVENT_PREV == audio_event) {
                bsp_codec_zero_dma_buffer();
                ret = ESP_FAIL;
                goto clean_up;
            }
        }
        g_player_state = PLAYER_STATE_PLAYING;

        drmp3_uint64 framesRead = drmp3_read_pcm_frames_s16(mp3, framesToRead, (drmp3_int16 *)output);

        if (framesRead > 0) {
            /* Configure I2S clock if sample rate changed. Always reconfigure at first frame */
            if (sample_rate != mp3->sampleRate ||
                    nChans != mp3->channels ||
                    bits_cfg != 16) {
                sample_rate = mp3->sampleRate;
                bits_cfg = 16;
                nChans = mp3->channels;
                ESP_LOGI(TAG, "audio info: sr=%d, bit=%d, ch=%d", sample_rate, bits_cfg, nChans);
                i2s_channel_t channel = (nChans == 1) ? I2S_CHANNEL_MONO : I2S_CHANNEL_STEREO;
                bsp_codec_set_clk(sample_rate, bits_cfg, nChans);
            }

            /* Write decoded data to audio decoder */
            size_t i2s_bytes_written = 0;
            size_t output_size = framesRead * nChans * bits_cfg / 8;
            bsp_codec_write(output, output_size, &i2s_bytes_written, portMAX_DELAY);
        } else {
            ESP_LOGE(TAG, "no more pcm frame can be found");
            break;
        }
    } while (true);

clean_up:
    /* This will prevent from sending dumy data to audio decoder */
    bsp_codec_zero_dma_buffer();

    free_current_lyric();

    /* Clean up resources */
    drmp3_uninit(mp3);
    if (NULL != mp3) {
        free(mp3);
    }
    if (NULL != output) {
        free(output);
    }

    return ret;
}

static void audio_task(void *pvParam)
{
    char *base_path = (char *) pvParam;
    fm_init(base_path);
    /* Scan audio file */
    fm_print_dir(base_path, 2);
    fm_file_table_create(&g_file_list, &g_file_num, ".mp3");
    for (size_t i = 0; i < g_file_num; i++) {
        ESP_LOGI(TAG, "have file [%d:%s]", i, g_file_list[i]);
    }
    if (0 == g_file_num) {
        ESP_LOGW(TAG, "Can't found any mp3 file!");
        vTaskDelete(NULL);
    }

    if (esp_ptr_executable(s_audio_cb)) {
        player_cb_ctx_t ctx = {
            .audio_event = AUDIO_EVENT_FILE_SCAN_DONE,
            .user_ctx = audio_cb_usrt_ctx,
        };
        s_audio_cb(&ctx);
    }

    /* Audio control event queue */
    audio_event_queue = xQueueCreate(4, sizeof(player_event_t));
    if (NULL == audio_event_queue) {
        vTaskDelete(NULL);
    }

    /* Get name of first song to play */
    char full_name[256];
    memset(full_name, '\0', sizeof(full_name));
    strcpy(full_name, base_path);
    strcat(full_name, "/");
    strcat(full_name, app_player_get_name_from_index(audio_index));

    /* Default state is pause */
    app_player_pause();

    /* Start play music */
    while (1) {
        esp_err_t ret_val = play_mp3(full_name);

        /* Get next audio's name if audio played without error */
        if (ESP_OK == ret_val) {
            audio_index ++;
            if (audio_index >= g_file_num) {
                audio_index = 0;
            }
        }

        /* Callback for audio index change */
        if (esp_ptr_executable(s_audio_cb)) {
            player_cb_ctx_t ctx = {
                .audio_event = AUDIO_EVENT_CHANGE,
                .user_ctx = audio_cb_usrt_ctx,
            };
            s_audio_cb(&ctx);
        }

        /* Get next song's file name */
        memset(full_name, '\0', sizeof(full_name));
        strcpy(full_name, base_path);
        strcat(full_name, "/");
        strcat(full_name, app_player_get_name_from_index(audio_index));
    }

    /* Task never returns */
    vTaskDelete(NULL);
}

uint32_t app_player_get_total_time(void)
{
    if (mp3 && 0 == mp3->sampleRate) {
        return 0;
    }

    return 1000 * music_total_frame / mp3->sampleRate;
}

uint32_t app_player_get_current_time(void)
{
    if (mp3 && 0 == mp3->sampleRate) {
        return 0;
    }
    return 1000 * mp3->currentPCMFrame / mp3->sampleRate;
}

static size_t lyric_parse_line(char *inbuf, size_t len, char *out, uint32_t *timestampe)
{
    if ('[' != inbuf[0]) {
        return 0;
    }

    char *p = strchr(inbuf, ']');
    if (p) {
        *p = '\0';
    }

    uint32_t m = 0, s = 0, mm = 0;
    sscanf(inbuf + 1, "%u:%u.%u", &m, &s, &mm);
    // printf("ts = %d, %d, %d\n", m, s, mm);
    *timestampe = m * 60 * 1000 + s * 1000 + mm;
    len = len - (1 + p - inbuf);
    memcpy(out, p + 1, len);
    out[len++] = '\n';

    return len;
}

static esp_err_t load_current_lyric(void)
{
    esp_err_t ret = ESP_OK;
    FILE *fp = NULL;
    lrc_str = NULL;
    lrc_lines = 0;
    size_t length = 0;
    char full_name[256];
    memset(full_name, '\0', sizeof(full_name));
    strcpy(full_name, fm_get_rootpath());
    strcat(full_name, "/");
    strcat(full_name, app_player_get_name_from_index(audio_index));
    int strl = strlen(full_name);
    for (int i = strl - 1; i >= 0; i--) {
        if ('.' == full_name[i]) {
            strcpy(&full_name[i + 1], "lrc");
            break;
        }
    }

    fp = fopen(full_name, "rb");
    ESP_RETURN_ON_FALSE(NULL != fp, ESP_ERR_NOT_FOUND, TAG, "File \"%s\" does not exist", full_name);
    size_t len = fm_get_file_size(full_name);
    char *lyric = heap_caps_malloc(2 * len, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    ESP_GOTO_ON_FALSE(NULL != lyric, ESP_ERR_NO_MEM, err, TAG, "No memory for lyric buffer");
    char *temp = lyric + len;
    length = fread(temp, 1, len, fp);
    fclose(fp);

    for (size_t i = 0; i < length; i++) {
        if ('\r' == temp[i] && '\n' == temp[i + 1]) {
            i++; // skip the CRLF
            (lrc_lines)++;
        }
    }
    ESP_LOGI(TAG, "lyric line=%d", lrc_lines);
    lrc_timestampe = heap_caps_malloc(sizeof(uint32_t) * (lrc_lines), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    ESP_GOTO_ON_FALSE(NULL != lrc_timestampe, ESP_ERR_NO_MEM, err, TAG, "No memory for lrc_timestampe");

    int j = 0, k = 0;
    char *head = temp;
    for (size_t i = 0; i < length; i++) {
        if ('\r' == temp[i] && '\n' == temp[i + 1]) {
            j += lyric_parse_line(head, &temp[i] - head, lyric + j, &lrc_timestampe[k++]);
            head = &temp[i + 2];
            i++; // skip the CRLF
        }
    }

    char *buffer = heap_caps_realloc(lyric, j + 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    ESP_GOTO_ON_FALSE(NULL != buffer, ESP_ERR_NO_MEM, err, TAG, "No memory for lyric");
    buffer[j] = '\0';
    lrc_str = buffer;

    return ret;
err:
    if (fp) {
        fclose(fp);
    }
    if (lyric) {
        heap_caps_free(lyric);
    }

    if (lrc_timestampe) {
        heap_caps_free(lrc_timestampe);
    }

    return ret;
}

static void free_current_lyric(void)
{
    if (lrc_str) {
        heap_caps_free(lrc_str);
        lrc_str = NULL;
    }

    if (lrc_timestampe) {
        heap_caps_free(lrc_timestampe);
        lrc_timestampe = NULL;
    }
}

esp_err_t app_player_get_current_lyric(char **out, uint32_t **timestamp_list, uint32_t *line_num)
{
    *out = lrc_str;
    *timestamp_list = lrc_timestampe;
    *line_num = lrc_lines;
    return ESP_OK;
}

/* **************** AUDIO PLAY CONTROL **************** */
esp_err_t app_player_play(void)
{
    ESP_RETURN_ON_FALSE(NULL != audio_event_queue, ESP_ERR_INVALID_STATE, TAG, "Audio task not started yet");

    player_event_t event = AUDIO_EVENT_PLAY;
    BaseType_t ret_val = xQueueSend(audio_event_queue, &event, 0);
    ESP_RETURN_ON_FALSE(pdPASS == ret_val, ESP_ERR_INVALID_STATE, TAG, "The last event has not been processed yet");

    return ESP_OK;
}

esp_err_t app_player_pause(void)
{
    ESP_RETURN_ON_FALSE(NULL != audio_event_queue, ESP_ERR_INVALID_STATE, TAG, "Audio task not started yet");

    player_event_t event = AUDIO_EVENT_PAUSE;
    BaseType_t ret_val = xQueueSend(audio_event_queue, &event, 0);
    ESP_RETURN_ON_FALSE(pdPASS == ret_val, ESP_ERR_INVALID_STATE, TAG, "The last event has not been processed yet");

    return ESP_OK;
}

esp_err_t app_player_play_next(void)
{
    ESP_RETURN_ON_FALSE(NULL != audio_event_queue, ESP_ERR_INVALID_STATE, TAG, "Audio task not started yet");

    audio_index ++;
    if (audio_index >= g_file_num) {
        audio_index = 0;
    }
    player_event_t event = AUDIO_EVENT_NEXT;
    BaseType_t ret_val = xQueueSend(audio_event_queue, &event, 0);
    ESP_RETURN_ON_FALSE(pdPASS == ret_val, ESP_ERR_INVALID_STATE, TAG, "The last event has not been processed yet");

    return ESP_OK;
}

esp_err_t app_player_play_prev(void)
{
    ESP_RETURN_ON_FALSE(NULL != audio_event_queue, ESP_ERR_INVALID_STATE, TAG, "Audio task not started yet");

    if (audio_index == 0) {
        audio_index = g_file_num;
    }
    audio_index--;
    player_event_t event = AUDIO_EVENT_PREV;
    BaseType_t ret_val = xQueueSend(audio_event_queue, &event, 0);
    ESP_RETURN_ON_FALSE(pdPASS == ret_val, ESP_ERR_INVALID_STATE, TAG, "The last event has not been processed yet");

    return ESP_OK;
}

esp_err_t app_player_play_index(size_t index)
{
    ESP_RETURN_ON_FALSE(NULL != audio_event_queue, ESP_ERR_INVALID_STATE, TAG, "Audio task not started yet");
    ESP_RETURN_ON_FALSE(index < g_file_num, ESP_ERR_INVALID_ARG, TAG, "File index out of range");

    audio_index = index;
    player_event_t event = AUDIO_EVENT_CHANGE;
    BaseType_t ret_val = xQueueSend(audio_event_queue, &event, 0);
    ESP_RETURN_ON_FALSE(pdPASS == ret_val, ESP_ERR_INVALID_STATE, TAG, "The last event has not been processed yet");

    return ESP_OK;
}

/* **************** START AUDIO PLAYER **************** */
esp_err_t app_player_start(char *file_path)
{
    // sys_param_t *param = settings_get_parameter();
    // bsp_codec_set_voice_volume(param->volume);

    ESP_RETURN_ON_FALSE(NULL != file_path, ESP_ERR_INVALID_ARG, TAG,  "Invalid base path");
    BaseType_t ret_val = xTaskCreatePinnedToCore(audio_task, "Audio Task", 32 * 1024, file_path, configMAX_PRIORITIES - 5, NULL, 1);
    ESP_RETURN_ON_FALSE(pdPASS == ret_val, ESP_FAIL, TAG,  "Failed create audio task");
    return ESP_OK;
}

player_state_t app_player_get_state(void)
{
    return g_player_state;
}
