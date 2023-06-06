/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_spiffs.h"
#include "esp_vfs.h"

#include "esp_alc.h"
#include "app_audio.h"
#include "audio_player.h"
#include "esp32_c3_lcd_ev_board.h"

static const char *TAG = "app_audio";

static void *volume_handle;

esp_err_t audio_force_quite(bool ret)
{
    return audio_player_stop();
}

esp_err_t app_audio_write(void *audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms)
{
    esp_err_t ret = ESP_OK;

    int result = alc_volume_setup_process(audio_buffer, len, 1, volume_handle, 10);
    // ESP_LOGE(TAG, "len:%d. %d", len, result);
    // ESP_ERROR_CHECK(alc_volume_setup_process(audio_buffer, len, 1, volume_handle, 5));
    if (bsp_audio_write(audio_buffer, len, bytes_written, 1000) != ESP_OK) {
        ESP_LOGE(TAG, "Write Task: i2s write failed");
        ret = ESP_FAIL;
    }

    return ret;
}

esp_err_t audio_handle_info(PDM_SOUND_TYPE voice)
{
    char filepath[30];
    esp_err_t ret = ESP_OK;

    switch (voice) {
    case SOUND_TYPE_KNOB:
        sprintf(filepath, "%s/%s", CONFIG_BSP_SPIFFS_MOUNT_POINT, "knob_1ch.mp3");
        break;
    case SOUND_TYPE_SNORE:
        sprintf(filepath, "%s/%s", CONFIG_BSP_SPIFFS_MOUNT_POINT, "snore_cute_1ch.mp3");
        break;
    case SOUND_TYPE_WASH_END_CN:
        sprintf(filepath, "%s/%s", CONFIG_BSP_SPIFFS_MOUNT_POINT, "wash_end_zh_1ch.mp3");
        break;
    case SOUND_TYPE_WASH_END_EN:
        sprintf(filepath, "%s/%s", CONFIG_BSP_SPIFFS_MOUNT_POINT, "wash_end_en_1ch.mp3");
        break;
    case SOUND_TYPE_FACTORY:
        sprintf(filepath, "%s/%s", CONFIG_BSP_SPIFFS_MOUNT_POINT, "factory.mp3");
        break;
    }

    FILE *fp = fopen(filepath, "r");
    ESP_GOTO_ON_FALSE(fp, ESP_FAIL, err, TAG,  "Failed open file:%s", filepath);

    ESP_LOGI(TAG, "play: %s", filepath);
    ret = audio_player_play(fp);
err:
    return ret;
}

static esp_err_t app_mute_function(AUDIO_PLAYER_MUTE_SETTING setting)
{
    return ESP_OK;
}

static void audio_callback(audio_player_cb_ctx_t *ctx)
{
    switch (ctx->audio_event) {
    case AUDIO_PLAYER_CALLBACK_EVENT_IDLE: /**< Player is idle, not playing audio */
        ESP_LOGI(TAG, "IDLE");
        break;
    case AUDIO_PLAYER_CALLBACK_EVENT_COMPLETED_PLAYING_NEXT:
        ESP_LOGI(TAG, "NEXT");
        break;
    case AUDIO_PLAYER_CALLBACK_EVENT_PLAYING:
        ESP_LOGI(TAG, "PLAYING");
        break;
    case AUDIO_PLAYER_CALLBACK_EVENT_PAUSE:
        ESP_LOGI(TAG, "PAUSE");
        break;
    case AUDIO_PLAYER_CALLBACK_EVENT_SHUTDOWN:
        ESP_LOGI(TAG, "SHUTDOWN");
        break;
    case AUDIO_PLAYER_CALLBACK_EVENT_UNKNOWN_FILE_TYPE:
        ESP_LOGI(TAG, "UNKNOWN FILE");
        break;
    case AUDIO_PLAYER_CALLBACK_EVENT_UNKNOWN:
        ESP_LOGI(TAG, "UNKNOWN");
        break;
    }
}

esp_err_t audio_play_start()
{
    esp_err_t ret = ESP_OK;
    volume_handle = alc_volume_setup_open();
    if (volume_handle == NULL) {
        ESP_LOGE(TAG, "Failed to create ALC handle. (line %d)", __LINE__);
    }

    audio_player_config_t config = { .mute_fn = app_mute_function,
                                     .write_fn = app_audio_write,
                                     .clk_set_fn = bsp_audio_reconfig_clk,
                                     .priority = 5
                                   };
    ESP_ERROR_CHECK(audio_player_new(config));
    audio_player_callback_register(audio_callback, NULL);
    return ret;
}
