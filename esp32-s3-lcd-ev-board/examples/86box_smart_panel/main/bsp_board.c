/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "esp_log.h"
#include "esp_check.h"
#include "esp_spiffs.h"
#include "hal/i2s_hal.h"
#include "driver/rmt_tx.h"
#include "led_strip.h"
#include "led_strip_interface.h"

#include "audio_player.h"
#include "bsp_board.h"
#include "es7210.h"
#include "es8311.h"

#define ES8311_SAMPLE_RATE          (16000)
#define ES8311_DEFAULT_VOLUME       (60)

#define ES7210_I2C_ADDR             (0x41)
#define ES7210_SAMPLE_RATE          (16000)
#define ES7210_I2S_FORMAT           ES7210_I2S_FMT_I2S
#define ES7210_MCLK_MULTIPLE        (256)
#define ES7210_BIT_WIDTH            ES7210_I2S_BITS_16B
#define ES7210_MIC_BIAS             ES7210_MIC_BIAS_2V87
#define ES7210_MIC_GAIN             ES7210_MIC_GAIN_30DB
#define ES7210_ADC_VOLUME           (0)

#define BLINK_GPIO                  GPIO_NUM_4

static led_strip_handle_t led_strip;
static bsp_codec_config_t g_codec_handle;

static i2s_chan_handle_t i2s_tx_chan;
static i2s_chan_handle_t i2s_rx_chan;

static es7210_dev_handle_t es7210_handle = NULL;
static es8311_handle_t es8311_handle = NULL;

static file_iterator_instance_t *file_iterator;

static const char *TAG = "board";

/* LED strip initialization with the GPIO and pixels number*/
led_strip_config_t strip_config = {
    .strip_gpio_num = BLINK_GPIO, // The GPIO that connected to the LED strip's data line
    .max_leds = 1, // The number of LEDs in the strip,
    .led_pixel_format = LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
    .led_model = LED_MODEL_WS2812, // LED strip model
    .flags.invert_out = false, // whether to invert the output signal (useful when your hardware has a level inverter)
};

led_strip_rmt_config_t rmt_config = {
    .clk_src = RMT_CLK_SRC_DEFAULT, // different clock source can lead to different power consumption
    .resolution_hz = 10 * 1000 * 1000, // 10MHz
    .flags.with_dma = false, // whether to enable the DMA feature
};

static esp_err_t bsp_led_init()
{
    ESP_LOGI(TAG, "BLINK_GPIO setting %d", BLINK_GPIO);

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_set_pixel(led_strip, 0, 0x00, 0x00, 0x00);
    led_strip_refresh(led_strip);
    return ESP_OK;
}

esp_err_t bsp_led_set_rgb(uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
    led_strip_set_pixel(led_strip, index, r, g, b);
    led_strip_refresh(led_strip);
    return ESP_OK;
}

static esp_err_t bsp_i2s_read(void *audio_buffer, size_t len, size_t *bytes_read, uint32_t timeout_ms)
{
    esp_err_t ret = ESP_OK;
    ret = i2s_channel_read(i2s_rx_chan, (char *)audio_buffer, len, bytes_read, timeout_ms);
    return ret;
}

static esp_err_t bsp_i2s_write(void *audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms)
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

static esp_err_t bsp_codec_volume_set(int volume, int *volume_set)
{
    esp_err_t ret = ESP_OK;
    float v = volume;//volume;
    v *= 0.6f;
    v = -0.01f * (v * v) + 2.0f * v;
    ret = es8311_voice_volume_set(es8311_handle, (int)v, volume_set);
    return ret;
}

static esp_err_t bsp_codec_mute_set(bool enable)
{
    esp_err_t ret = ESP_OK;
    ret = es8311_voice_mute(es8311_handle, enable);
    return ret;
}

static esp_err_t bsp_codec_es7210_set()
{
    esp_err_t ret = ESP_OK;

    es7210_codec_config_t codec_conf = {
        .i2s_format = ES7210_I2S_FORMAT,
        .mclk_ratio = ES7210_MCLK_MULTIPLE,
        .sample_rate_hz = ES7210_SAMPLE_RATE,
        .bit_width = ES7210_BIT_WIDTH,
        .mic_bias = ES7210_MIC_BIAS,
        .mic_gain = ES7210_MIC_GAIN,
        .flags.tdm_enable = false
    };
    ret |= es7210_config_codec(es7210_handle, &codec_conf);
    ret |= es7210_config_volume(es7210_handle, ES7210_ADC_VOLUME);
    return ret;
}

static void bsp_codec_init()
{
    /* Create ES7210 device handle */
    es7210_i2c_config_t es7210_i2c_conf = {
        .i2c_port = BSP_I2C_NUM,
        .i2c_addr = ES7210_I2C_ADDR
    };
    es7210_new_codec(&es7210_i2c_conf, &es7210_handle);
    bsp_codec_es7210_set();

    es8311_handle = es8311_create(BSP_I2C_NUM, ES8311_ADDRRES_0);

    es8311_clock_config_t clk_cfg = BSP_ES8311_SCLK_CONFIG(ES8311_SAMPLE_RATE);
    es8311_init(es8311_handle, &clk_cfg, ES8311_RESOLUTION_32, ES8311_RESOLUTION_32);
    es8311_voice_volume_set(es8311_handle, ES8311_DEFAULT_VOLUME, NULL);

    /* Microphone settings */
    es8311_microphone_config(es8311_handle, false);
    es8311_microphone_gain_set(es8311_handle, ES8311_MIC_GAIN_42DB);

    /* Configure I2S peripheral and Power Amplifier */
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = BSP_I2S_GPIO_CFG,
    };
    bsp_audio_init(&std_cfg, &i2s_tx_chan, &i2s_rx_chan);
    bsp_audio_poweramp_enable(true);

    bsp_codec_config_t *codec_config = bsp_board_get_codec_handle();
    codec_config->volume_set_fn = bsp_codec_volume_set;
    codec_config->mute_set_fn = bsp_codec_mute_set;
    codec_config->codec_reconfig_fn = bsp_codec_es7210_set;
    codec_config->i2s_read_fn = bsp_i2s_read;
    codec_config->i2s_write_fn = bsp_i2s_write;
    codec_config->i2s_reconfig_clk_fn = bsp_i2s_reconfig_clk;
}

bsp_codec_config_t *bsp_board_get_codec_handle(void)
{
    return &g_codec_handle;
}

esp_err_t bsp_board_init(void)
{
    esp_err_t ret = ESP_OK;

    bsp_led_init();
    bsp_codec_init();

    return ret;
}

esp_err_t bsp_spiffs_mount(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = CONFIG_BSP_SPIFFS_MOUNT_POINT,
        .partition_label = CONFIG_BSP_SPIFFS_PARTITION_LABEL,
        .max_files = CONFIG_BSP_SPIFFS_MAX_FILES,
#ifdef CONFIG_BSP_SPIFFS_FORMAT_ON_MOUNT_FAIL
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif
    };

    esp_err_t ret_val = esp_vfs_spiffs_register(&conf);

    ESP_ERROR_CHECK(ret_val);

    size_t total = 0, used = 0;
    ret_val = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret_val != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret_val));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    return ret_val;
}

esp_err_t bsp_spiffs_unmount(void)
{
    return esp_vfs_spiffs_unregister(CONFIG_BSP_SPIFFS_PARTITION_LABEL);
}

file_iterator_instance_t *get_file_iterator_instance(void)
{
    return file_iterator;
}

static esp_err_t audio_mute_function(AUDIO_PLAYER_MUTE_SETTING setting)
{
    // Volume saved when muting and restored when unmuting. Restoring volume is necessary
    // as es8311_set_voice_mute(true) results in voice volume (REG32) being set to zero.
    static int last_volume;
    bsp_codec_config_t *codec_handle = bsp_board_get_codec_handle();

    codec_handle->mute_set_fn(setting == AUDIO_PLAYER_MUTE ? true : false);

    if (setting == AUDIO_PLAYER_UNMUTE) {
        codec_handle->volume_set_fn(80, NULL);
    }
    ESP_LOGI(TAG, "mute setting %d, volume:%d", setting, last_volume);

    return ESP_OK;
}

esp_err_t bsp_audio_player_init(void)
{
    bsp_codec_config_t *codec_handle = bsp_board_get_codec_handle();
    file_iterator = file_iterator_new("/spiffs/mp3");
    assert(file_iterator != NULL);
    audio_player_config_t config = { .mute_fn = audio_mute_function,
                                     .write_fn = codec_handle->i2s_write_fn,
                                     .clk_set_fn = codec_handle->i2s_reconfig_clk_fn,
                                     .priority = 5
                                   };
    ESP_ERROR_CHECK(audio_player_new(config));

    return ESP_OK;
}
