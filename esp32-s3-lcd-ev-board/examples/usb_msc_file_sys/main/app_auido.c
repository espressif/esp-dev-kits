/* SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_audio.h"
#include "audio_player.h"
#include "bsp/esp32_s3_lcd_ev_board.h"
#include "es8311.h"

#define ES8311_SAMPLE_RATE          (44100)
#define ES8311_DEFAULT_VOLUME       (60)

static i2s_chan_handle_t i2s_tx_chan;

static esp_err_t bsp_i2s_write(void * audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms)
{
    esp_err_t ret = ESP_OK;
    ret = i2s_channel_write(i2s_tx_chan, (char *)audio_buffer, len, bytes_written, timeout_ms);
    return ret;
}

static esp_err_t bsp_i2s_reconfig_clk(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch)
{
    esp_err_t ret = ESP_OK;
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(rate),
        .slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG((i2s_data_bit_width_t)bits_cfg, (i2s_slot_mode_t)ch),
        .gpio_cfg = BSP_I2S_GPIO_CFG,
    };

    ret |= i2s_channel_disable(i2s_tx_chan);
    ret |= i2s_channel_reconfig_std_clock(i2s_tx_chan, &std_cfg.clk_cfg);
    ret |= i2s_channel_reconfig_std_slot(i2s_tx_chan, &std_cfg.slot_cfg);
    ret |= i2s_channel_enable(i2s_tx_chan);
    return ret;
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
    es8311_handle_t es8311_handle = es8311_create(BSP_I2C_NUM, ES8311_ADDRRES_0);

    es8311_clock_config_t clk_cfg = BSP_ES8311_SCLK_CONFIG(ES8311_SAMPLE_RATE);
    ESP_ERROR_CHECK(es8311_init(es8311_handle, &clk_cfg, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16));
    ESP_ERROR_CHECK(es8311_voice_volume_set(es8311_handle, ES8311_DEFAULT_VOLUME, NULL));

    /* Microphone settings */
    // es8311_microphone_config(es8311_handle, false);
    // es8311_microphone_gain_set(es8311_handle, ES8311_MIC_GAIN_42DB);

    /* Configure I2S peripheral and Power Amplifier */
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = BSP_I2S_GPIO_CFG,
    };
    ESP_ERROR_CHECK(bsp_audio_init(&std_cfg, &i2s_tx_chan, NULL));
    ESP_ERROR_CHECK(bsp_audio_poweramp_enable(true));

    audio_player_config_t config = { .mute_fn = audio_mute_function,
                                     .write_fn = bsp_i2s_write,
                                     .clk_set_fn = bsp_i2s_reconfig_clk,
                                     .priority = 5 };
    esp_err_t ret = audio_player_new(config);

    return ret;
}