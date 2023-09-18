/* SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_audio.h"
#include "audio_player.h"
#include "bsp/esp32_s3_lcd_ev_board.h"
#include "esp_check.h"

static const char *TAG = "app_audio";
static esp_codec_dev_handle_t codec_dev = NULL;

static esp_err_t bsp_i2s_write(void * audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms)
{
    ESP_RETURN_ON_FALSE(esp_codec_dev_write(codec_dev, audio_buffer, len) == ESP_CODEC_DEV_OK, ESP_FAIL, TAG, "codec dev write fail");
    *bytes_written = len;
    return ESP_OK;
}

static esp_err_t bsp_i2s_reconfig_clk(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch)
{
    ESP_RETURN_ON_FALSE(esp_codec_dev_close(codec_dev) == ESP_CODEC_DEV_OK, ESP_FAIL, TAG, "codec dev set vol fail");

    esp_codec_dev_sample_info_t fs = {
        .bits_per_sample = bits_cfg,
        .sample_rate = rate,
        .channel = (int)ch,
    };
    ESP_RETURN_ON_FALSE(esp_codec_dev_open(codec_dev, &fs) == ESP_CODEC_DEV_OK, ESP_FAIL, TAG, "codec dev open fail");
    return ESP_OK;
}

static esp_err_t audio_mute_function(AUDIO_PLAYER_MUTE_SETTING setting)
{
    switch (setting) {
        case AUDIO_PLAYER_MUTE:
            bsp_audio_poweramp_enable(false);
            break;
        case AUDIO_PLAYER_UNMUTE:
            bsp_audio_poweramp_enable(true);
            break;
        default:
            break;
    }
    return ESP_OK;
}

esp_err_t audio_init(void)
{
    ESP_RETURN_ON_FALSE(codec_dev == NULL, ESP_FAIL, TAG, "codec dev has been initialized");
    codec_dev = bsp_audio_codec_speaker_init();
    ESP_RETURN_ON_FALSE(codec_dev != NULL, ESP_FAIL, TAG, "codec dev can't be null");
    esp_codec_dev_sample_info_t fs = {
        .bits_per_sample = 16,
        .sample_rate = 48000,
        .channel = 2,
    };
    ESP_RETURN_ON_FALSE(esp_codec_dev_open(codec_dev, &fs) == ESP_CODEC_DEV_OK, ESP_FAIL, TAG, "codec dev open fail");
    ESP_RETURN_ON_FALSE(esp_codec_dev_set_out_vol(codec_dev, 70.0) == ESP_CODEC_DEV_OK, ESP_FAIL, TAG, "codec dev set vol fail");

    audio_player_config_t config = {.mute_fn = audio_mute_function,
                                    .write_fn = bsp_i2s_write,
                                    .clk_set_fn = bsp_i2s_reconfig_clk,
                                    .priority = 5};
    ESP_ERROR_CHECK(bsp_audio_poweramp_enable(true));
    esp_err_t ret = audio_player_new(config);
    return ret;
}