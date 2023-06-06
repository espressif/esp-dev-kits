// Copyright 2020 Espressif Systems (Shanghai) Co. Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "esp_log.h"
#include "esp_check.h"
#include "unity.h"
#include "audio_player.h"
#include "driver/gpio.h"
#include "test_utils.h"
#include "freertos/semphr.h"

static const char *TAG = "AUDIO PLAYER TEST";

#define CONFIG_BSP_I2S_NUM 1

/* Audio */
#define BSP_I2S_SCLK          (GPIO_NUM_17)
#define BSP_I2S_MCLK          (GPIO_NUM_2)
#define BSP_I2S_LCLK          (GPIO_NUM_47)
#define BSP_I2S_DOUT          (GPIO_NUM_15) // To Codec ES8311
#define BSP_I2S_DSIN          (GPIO_NUM_16) // From ADC ES7210
#define BSP_POWER_AMP_IO      (GPIO_NUM_46)
#define BSP_MUTE_STATUS       (GPIO_NUM_1)

/**
 * @brief ESP-BOX I2S pinout
 *
 * Can be used for i2s_std_gpio_config_t and/or i2s_std_config_t initialization
 */
#define BSP_I2S_GPIO_CFG       \
    {                          \
        .mclk = BSP_I2S_MCLK,  \
        .bclk = BSP_I2S_SCLK,  \
        .ws = BSP_I2S_LCLK,    \
        .dout = BSP_I2S_DOUT,  \
        .din = BSP_I2S_DSIN,   \
        .invert_flags = {      \
            .mclk_inv = false, \
            .bclk_inv = false, \
            .ws_inv = false,   \
        },                     \
    }

/**
 * @brief Mono Duplex I2S configuration structure
 *
 * This configuration is used by default in bsp_audio_init()
 */
#define BSP_I2S_DUPLEX_MONO_CFG(_sample_rate)                                                         \
    {                                                                                                 \
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(_sample_rate),                                          \
        .slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO), \
        .gpio_cfg = BSP_I2S_GPIO_CFG,                                                                 \
    }

static i2s_chan_handle_t i2s_tx_chan;
static i2s_chan_handle_t i2s_rx_chan;

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

static esp_err_t audio_mute_function(AUDIO_PLAYER_MUTE_SETTING setting) {
    ESP_LOGI(TAG, "mute setting %d", setting);
    return ESP_OK;
}

TEST_CASE("audio player can be newed and deleted", "[audio player]")
{
    audio_player_config_t config = { .mute_fn = audio_mute_function,
                                     .write_fn = bsp_i2s_write,
                                     .clk_set_fn = bsp_i2s_reconfig_clk,
                                     .priority = 0 };
    esp_err_t ret = audio_player_new(config);
    TEST_ASSERT_EQUAL(ret, ESP_OK);

    ret = audio_player_delete();
    TEST_ASSERT_EQUAL(ret, ESP_OK);

    audio_player_state_t state = audio_player_get_state();
    TEST_ASSERT_EQUAL(state, AUDIO_PLAYER_STATE_SHUTDOWN);
}

static esp_err_t bsp_audio_init(const i2s_std_config_t *i2s_config, i2s_chan_handle_t *tx_channel, i2s_chan_handle_t *rx_channel)
{
    /* Setup I2S peripheral */
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(CONFIG_BSP_I2S_NUM, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true; // Auto clear the legacy data in the DMA buffer
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, tx_channel, rx_channel));

    /* Setup I2S channels */
    const i2s_std_config_t std_cfg_default = BSP_I2S_DUPLEX_MONO_CFG(22050);
    const i2s_std_config_t *p_i2s_cfg = &std_cfg_default;
    if (i2s_config != NULL) {
        p_i2s_cfg = i2s_config;
    }

    if (tx_channel != NULL) {
        ESP_ERROR_CHECK(i2s_channel_init_std_mode(*tx_channel, p_i2s_cfg));
        ESP_ERROR_CHECK(i2s_channel_enable(*tx_channel));
    }
    if (rx_channel != NULL) {
        ESP_ERROR_CHECK(i2s_channel_init_std_mode(*rx_channel, p_i2s_cfg));
        ESP_ERROR_CHECK(i2s_channel_enable(*rx_channel));
    }

    /* Setup power amplifier pin */
    const gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = BIT64(BSP_POWER_AMP_IO),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLDOWN_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    return ESP_OK;
}

static audio_player_callback_event_t expected_event;
static QueueHandle_t event_queue;

static void audio_player_callback(audio_player_cb_ctx_t *ctx)
{
    TEST_ASSERT_EQUAL(ctx->audio_event, expected_event);

    // wake up the test so it can continue to the next step
    TEST_ASSERT_EQUAL(xQueueSend(event_queue, &(ctx->audio_event), 0), pdPASS);
}

TEST_CASE("audio player states and callbacks are correct", "[audio player]")
{
    audio_player_callback_event_t event;

    /* Configure I2S peripheral and Power Amplifier */
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(44100),
        .slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = BSP_I2S_GPIO_CFG,
    };
    esp_err_t ret = bsp_audio_init(&std_cfg, &i2s_tx_chan, &i2s_rx_chan);
    TEST_ASSERT_EQUAL(ret, ESP_OK);

    audio_player_config_t config = { .mute_fn = audio_mute_function,
                                     .write_fn = bsp_i2s_write,
                                     .clk_set_fn = bsp_i2s_reconfig_clk,
                                     .priority = 0 };
    ret = audio_player_new(config);
    TEST_ASSERT_EQUAL(ret, ESP_OK);

    event_queue = xQueueCreate(1, sizeof(audio_player_callback_event_t));
    TEST_ASSERT_NOT_NULL(event_queue);

    ret = audio_player_callback_register(audio_player_callback, NULL);
    TEST_ASSERT_EQUAL(ret, ESP_OK);

    audio_player_state_t state = audio_player_get_state();
    TEST_ASSERT_EQUAL(state, AUDIO_PLAYER_STATE_IDLE);

    extern const char mp3_start[] asm("_binary_gs_16b_1c_44100hz_mp3_start");
    extern const char mp3_end[]   asm("_binary_gs_16b_1c_44100hz_mp3_end");

    // -1 due to the size being 1 byte too large, I think because end is the byte
    // immediately after the last byte in the memory but I'm not sure - cmm 2022-08-20
    //
    // Suppression as these are linker symbols and cppcheck doesn't know how to ensure
    // they are the same object
    // cppcheck-suppress comparePointers
    size_t mp3_size = (mp3_end - mp3_start) - 1;
    ESP_LOGI(TAG, "mp3_size %zu bytes", mp3_size);

    FILE *fp = fmemopen((void*)mp3_start, mp3_size, "rb");
    TEST_ASSERT_NOT_NULL(fp);



    ///////////////
    expected_event = AUDIO_PLAYER_CALLBACK_EVENT_PLAYING;
    ret = audio_player_play(fp);
    TEST_ASSERT_EQUAL(ret, ESP_OK);

    // wait for playing event to arrive
    TEST_ASSERT_EQUAL(xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100)), pdPASS);

    // confirm state is playing
    state = audio_player_get_state();
    TEST_ASSERT_EQUAL(state, AUDIO_PLAYER_STATE_PLAYING);



    ///////////////
    expected_event = AUDIO_PLAYER_CALLBACK_EVENT_PAUSE;
    ret = audio_player_pause();
    TEST_ASSERT_EQUAL(ret, ESP_OK);

    // wait for paused event to arrive
    TEST_ASSERT_EQUAL(xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100)), pdPASS);

    state = audio_player_get_state();
    TEST_ASSERT_EQUAL(state, AUDIO_PLAYER_STATE_PAUSE);



    ////////////////
    expected_event = AUDIO_PLAYER_CALLBACK_EVENT_PLAYING;
    ret = audio_player_resume();
    TEST_ASSERT_EQUAL(ret, ESP_OK);

    // wait for paused event to arrive
    TEST_ASSERT_EQUAL(xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100)), pdPASS);



    ///////////////
    expected_event = AUDIO_PLAYER_CALLBACK_EVENT_IDLE;

    // the track is 16 seconds long so lets wait a bit here
    int sleep_seconds = 16;
    ESP_LOGI(TAG, "sleeping for %d seconds for playback to complete", sleep_seconds);
    vTaskDelay(pdMS_TO_TICKS(sleep_seconds * 1000));

    // wait for idle event to arrive
    TEST_ASSERT_EQUAL(xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100)), pdPASS);

    state = audio_player_get_state();
    TEST_ASSERT_EQUAL(state, AUDIO_PLAYER_STATE_IDLE);



    ///////////////
    expected_event = AUDIO_PLAYER_CALLBACK_EVENT_SHUTDOWN;
    ret = audio_player_delete();
    TEST_ASSERT_EQUAL(ret, ESP_OK);

    // wait for idle event to arrive
    TEST_ASSERT_EQUAL(xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100)), pdPASS);

    state = audio_player_get_state();
    TEST_ASSERT_EQUAL(state, AUDIO_PLAYER_STATE_SHUTDOWN);

    vQueueDelete(event_queue);

    TEST_ESP_OK(i2s_channel_disable(i2s_tx_chan));
    TEST_ESP_OK(i2s_channel_disable(i2s_rx_chan));
    TEST_ESP_OK(i2s_del_channel(i2s_tx_chan));
    TEST_ESP_OK(i2s_del_channel(i2s_rx_chan));

    ESP_LOGI(TAG, "NOTE: a memory leak will be reported the first time this test runs.\n");
    ESP_LOGI(TAG, "esp-idf v4.4.1 and v4.4.2 both leak memory between i2s_driver_install() and i2s_driver_uninstall()\n");
}
