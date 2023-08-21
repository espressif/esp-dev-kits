/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_spiffs.h"

#include "bsp/esp-bsp.h"
#include "audio_player.h"
#include "file_iterator.h"
#include "es8311.h"

#include "lv_example_pub.h"

static char *TAG = "app_main";

#define LOG_MEM_INFO    (0)

void memory_monitor()
{
    static char buffer[128];    /* Make sure buffer is enough for `sprintf` */
    if (1) {
        sprintf(buffer, "   Biggest /     Free /    Total\n"
                "\t  SRAM : [%8d / %8d / %8d]\n"
                "\t PSRAM : [%8d / %8d / %8d]",
                heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL),
                heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                heap_caps_get_total_size(MALLOC_CAP_INTERNAL),
                heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM),
                heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
                heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
        ESP_LOGI("MEM", "%s", buffer);
    }
    int currentMem = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    static int LastMem = 0;
    static int originMem = 0;
    static int total_dec = 0;
    if (currentMem ^ LastMem) {
        if (LastMem) {
            total_dec = (currentMem - originMem);
        } else {
            originMem = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        }
        ESP_LOGI("MEM Lost", "%d, %d", currentMem - LastMem, total_dec);
        LastMem = currentMem;
    }
}

static file_iterator_instance_t *file_iterator;
static i2s_chan_handle_t i2s_tx_chan;
static es8311_handle_t es8311_dev = NULL;

/* Buffer for reading/writing to I2S driver. Same length as SPIFFS buffer and I2S buffer, for optimal read/write performance.
   Recording audio data path:
   I2S peripheral -> I2S buffer (DMA) -> App buffer (RAM) -> SPIFFS buffer -> External SPI Flash.
   Vice versa for playback. */
#define BUFFER_SIZE     (1024)
#define SAMPLE_RATE     (44100)
#define DEFAULT_VOLUME  (60)

esp_err_t audio_codec_set_voice_volume(uint8_t volume)
{
    float v = volume;
    v *= 0.6f;
    v = -0.01f * (v * v) + 2.0f * v;

    return es8311_voice_volume_set(es8311_dev, volume, NULL);
}

static esp_err_t audio_mute_function(AUDIO_PLAYER_MUTE_SETTING setting)
{
    // Volume saved when muting and restored when unmuting. Restoring volume is necessary
    // as es8311_set_voice_mute(true) results in voice volume (REG32) being set to zero.
    static uint8_t last_volume;

    uint8_t volume = 60;//get_sys_volume();
    if (volume != 0) {
        last_volume = volume;
    }

    ESP_RETURN_ON_ERROR(es8311_voice_mute(es8311_dev, setting == AUDIO_PLAYER_MUTE ? true : false), TAG, "set voice mute");

    // restore the voice volume upon unmuting
    if (setting == AUDIO_PLAYER_UNMUTE) {
        audio_codec_set_voice_volume(last_volume);
    }

    return ESP_OK;
}

static esp_err_t audio_i2s_write(void *audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms)
{
    esp_err_t ret = ESP_OK;
    ret = i2s_channel_write(i2s_tx_chan, (char *)audio_buffer, len, bytes_written, timeout_ms);
    return ret;
}

static esp_err_t audio_i2s_reconfig_clk(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch)
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

void app_main(void)
{
#if CONFIG_BSP_LCD_SUB_BOARD_480_480
    // For the newest version sub board, we need to set `BSP_LCD_VSYNC` to high before initialize LCD
    // It's a requirement of the LCD module and will be added into BSP in the future
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = BIT64(BSP_LCD_VSYNC);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = true;
    gpio_config(&io_conf);
    gpio_set_level(BSP_LCD_VSYNC, 1);
#endif

    bsp_i2c_init();
    bsp_display_start();

#if CONFIG_BSP_DISPLAY_LVGL_AVOID_TEAR
    ESP_LOGI(TAG, "Avoid lcd tearing effect");
#if CONFIG_BSP_DISPLAY_LVGL_FULL_REFRESH
    ESP_LOGI(TAG, "LVGL full-refresh");
#elif CONFIG_BSP_DISPLAY_LVGL_DIRECT_MODE
    ESP_LOGI(TAG, "LVGL direct-mode");
#endif
#endif

    ESP_LOGI(TAG, "Display LVGL demo");

    bsp_display_lock(0);
    lv_style_pre_init();
    lv_create_home(&main_Layer);
    lv_create_clock(&clock_screen_layer, TIME_ENTER_CLOCK_2MIN);
    bsp_display_unlock();

    bsp_spiffs_mount();

    file_iterator = file_iterator_new(CONFIG_BSP_SPIFFS_MOUNT_POINT);
    assert(file_iterator != NULL);

    /* Create and configure ES8311 I2C driver */
    es8311_dev = es8311_create(BSP_I2C_NUM, ES8311_ADDRRES_0);
    const es8311_clock_config_t clk_cfg = BSP_ES8311_SCLK_CONFIG(SAMPLE_RATE);
    es8311_init(es8311_dev, &clk_cfg, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16);
    es8311_voice_volume_set(es8311_dev, DEFAULT_VOLUME, NULL);

    /* Configure I2S peripheral and Power Amplifier */
    bsp_audio_init(NULL, &i2s_tx_chan, NULL);
    bsp_audio_poweramp_enable(true);

    audio_player_config_t config = { .mute_fn = audio_mute_function,
                                     .write_fn = audio_i2s_write,
                                     .clk_set_fn = audio_i2s_reconfig_clk,
                                     .priority = 5
                                   };

    ESP_ERROR_CHECK(audio_player_new(config));

#if LOG_MEM_INFO
    static char buffer[128];    /* Make sure buffer is enough for `sprintf` */
    while (1) {
        sprintf(buffer, "   Biggest /     Free /    Total\n"
                "\t  SRAM : [%8d / %8d / %8d]\n"
                "\t PSRAM : [%8d / %8d / %8d]",
                heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL),
                heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                heap_caps_get_total_size(MALLOC_CAP_INTERNAL),
                heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM),
                heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
                heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
        ESP_LOGI("MEM", "%s", buffer);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
#endif
}
