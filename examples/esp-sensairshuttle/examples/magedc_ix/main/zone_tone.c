/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "zone_tone.h"

#if !ENABLE_ZONE_TONE

void zone_tone_init(void) {}

void zone_tone_set_zone(int zone_id)
{
    (void)zone_id;
}

void zone_tone_update(int zone_id)
{
    (void)zone_id;
}

void zone_tone_set_enabled(bool enabled)
{
    (void)enabled;
}

bool zone_tone_is_enabled(void)
{
    return false;
}

void zone_tone_get_config(zone_tone_config_t *cfg)
{
    (void)cfg;
}

void zone_tone_set_config(const zone_tone_config_t *cfg)
{
    (void)cfg;
}

void zone_tone_reset_config(void) {}

void zone_tone_play_test(int zone_id)
{
    (void)zone_id;
}

#else

#include <string.h>

#include "driver/gpio.h"
#include "driver/i2s_pdm.h"
#include "esp_board_manager.h"
#include "esp_board_manager_includes.h"
#include "esp_log.h"
#include "esp_check.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/gpio_sig_map.h"
#include "soc/io_mux_reg.h"

static const char *TAG = "ZONE_TONE";

#define TONE_SAMPLE_RATE_HZ 24000
#define TONE_DMA_DESC_NUM 6
#define TONE_DMA_FRAME_NUM 240
#define TONE_PDM_UPSAMPLE_FS (TONE_SAMPLE_RATE_HZ / 50) /* SensairShuttle BSP: 24k/50=480 */
#define TONE_PDM_P_GPIO GPIO_NUM_7
#define TONE_PDM_N_GPIO GPIO_NUM_8
#define TONE_PA_GPIO GPIO_NUM_1
#define TONE_DEFAULT_VOLUME 1
#define TONE_PDM_SD_SCALE I2S_PDM_SIG_SCALING_MUL_4
#define TONE_PDM_HP_SCALE I2S_PDM_SIG_SCALING_MUL_4
#define TONE_PDM_LP_SCALE I2S_PDM_SIG_SCALING_MUL_4
#define TONE_PDM_SINC_SCALE I2S_PDM_SIG_SCALING_MUL_4
#define TONE_DEFAULT_MAX_PLAY_MS 1200
#define TONE_DEFAULT_PA_SETTLE_MS 4
#define TONE_DEFAULT_PA_HOLD_MS 0
#define TONE_DEFAULT_ATTACK_PCT 1
#define TONE_DEFAULT_RELEASE_PCT 1
#define ZONE_TONE_SILENT_ID 5
#define ZONE_TONE_CONFIRM_ID 1
#define ZONE_TONE_NVS_NS "zone_tone"
#define ZONE_TONE_NVS_KEY_EN "enabled"

#if TONE_PDM_UPSAMPLE_FS != (TONE_SAMPLE_RATE_HZ / 50)
#error "PDM up_sample_fs must equal sample_rate_hz / 50"
#endif

extern const uint8_t _binary_zone1_wav_start[] asm("_binary_zone1_wav_start");
extern const uint8_t _binary_zone1_wav_end[] asm("_binary_zone1_wav_end");
extern const uint8_t _binary_zone2_wav_start[] asm("_binary_zone2_wav_start");
extern const uint8_t _binary_zone2_wav_end[] asm("_binary_zone2_wav_end");
extern const uint8_t _binary_zone3_wav_start[] asm("_binary_zone3_wav_start");
extern const uint8_t _binary_zone3_wav_end[] asm("_binary_zone3_wav_end");
extern const uint8_t _binary_zone4_wav_start[] asm("_binary_zone4_wav_start");
extern const uint8_t _binary_zone4_wav_end[] asm("_binary_zone4_wav_end");
extern const uint8_t _binary_zone6_wav_start[] asm("_binary_zone6_wav_start");
extern const uint8_t _binary_zone6_wav_end[] asm("_binary_zone6_wav_end");
extern const uint8_t _binary_zone7_wav_start[] asm("_binary_zone7_wav_start");
extern const uint8_t _binary_zone7_wav_end[] asm("_binary_zone7_wav_end");
extern const uint8_t _binary_zone8_wav_start[] asm("_binary_zone8_wav_start");
extern const uint8_t _binary_zone8_wav_end[] asm("_binary_zone8_wav_end");
extern const uint8_t _binary_zone9_wav_start[] asm("_binary_zone9_wav_start");
extern const uint8_t _binary_zone9_wav_end[] asm("_binary_zone9_wav_end");

typedef struct {
    const uint8_t *wav;
    const uint8_t *wav_end;
} zone_wav_t;

#define ZONE_WAV(start, end) {(start), (end)}

static const zone_wav_t s_zone_wav[10] = {
    [0] = {NULL, NULL},
    [1] = ZONE_WAV(_binary_zone1_wav_start, _binary_zone1_wav_end),
    [2] = ZONE_WAV(_binary_zone2_wav_start, _binary_zone2_wav_end),
    [3] = ZONE_WAV(_binary_zone3_wav_start, _binary_zone3_wav_end),
    [4] = ZONE_WAV(_binary_zone4_wav_start, _binary_zone4_wav_end),
    [5] = {NULL, NULL},
    [6] = ZONE_WAV(_binary_zone6_wav_start, _binary_zone6_wav_end),
    [7] = ZONE_WAV(_binary_zone7_wav_start, _binary_zone7_wav_end),
    [8] = ZONE_WAV(_binary_zone8_wav_start, _binary_zone8_wav_end),
    [9] = ZONE_WAV(_binary_zone9_wav_start, _binary_zone9_wav_end),
};

static size_t zone_wav_len(const zone_wav_t *zw)
{
    if (!zw || !zw->wav || !zw->wav_end || zw->wav_end <= zw->wav) {
        return 0;
    }
    return (size_t)(zw->wav_end - zw->wav);
}

static gpio_num_t s_pa_gpio = TONE_PA_GPIO;
static bool s_pa_from_board = false;
static i2s_chan_handle_t s_tx_handle;
static TaskHandle_t s_tone_task;
static volatile int s_pending_zone = 0;
static bool s_ready = false;
static bool s_i2s_enabled = false;
static volatile bool s_enabled = true;
static zone_tone_config_t s_tone_cfg = {
    .volume_pct = TONE_DEFAULT_VOLUME,
    .max_play_ms = TONE_DEFAULT_MAX_PLAY_MS,
    .pa_settle_ms = TONE_DEFAULT_PA_SETTLE_MS,
    .pa_hold_ms = TONE_DEFAULT_PA_HOLD_MS,
    .attack_pct = TONE_DEFAULT_ATTACK_PCT,
    .release_pct = TONE_DEFAULT_RELEASE_PCT,
};

static int clamp_int(int v, int min_v, int max_v)
{
    if (v < min_v) {
        return min_v;
    }
    if (v > max_v) {
        return max_v;
    }
    return v;
}

static uint32_t clamp_u32(uint32_t v, uint32_t min_v, uint32_t max_v)
{
    if (v < min_v) {
        return min_v;
    }
    if (v > max_v) {
        return max_v;
    }
    return v;
}

static zone_tone_config_t zone_tone_sanitize_config(const zone_tone_config_t *cfg)
{
    zone_tone_config_t out = *cfg;
    out.volume_pct = clamp_int(out.volume_pct, 0, 100);
    out.max_play_ms = clamp_u32(out.max_play_ms, 80, 1200);
    out.pa_settle_ms = clamp_u32(out.pa_settle_ms, 0, 40);
    out.pa_hold_ms = clamp_u32(out.pa_hold_ms, 0, 120);
    out.attack_pct = clamp_int(out.attack_pct, 0, 20);
    out.release_pct = clamp_int(out.release_pct, 0, 35);
    return out;
}

static bool zone_tone_load_enabled(void)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(ZONE_TONE_NVS_NS, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        return true;
    }

    uint8_t enabled_u8 = 1;
    err = nvs_get_u8(handle, ZONE_TONE_NVS_KEY_EN, &enabled_u8);
    nvs_close(handle);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return true;
    }
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Load tone state failed: 0x%x", err);
        return true;
    }
    return enabled_u8 != 0;
}

static void zone_tone_store_enabled(bool enabled)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(ZONE_TONE_NVS_NS, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Open tone NVS failed: 0x%x", err);
        return;
    }

    err = nvs_set_u8(handle, ZONE_TONE_NVS_KEY_EN, enabled ? 1 : 0);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Save tone state failed: 0x%x", err);
    }
}

static void pa_bind_from_board(void)
{
    periph_gpio_handle_t *gpio = NULL;
    if (esp_board_manager_get_periph_handle("gpio_speaker_enable", (void **)&gpio) == ESP_OK && gpio != NULL) {
        s_pa_gpio = gpio->gpio_num;
        s_pa_from_board = true;
        ESP_LOGI(TAG, "PA GPIO from Board Manager: %d", (int)s_pa_gpio);
    }
}

static void pa_set(bool on)
{
    gpio_set_level(s_pa_gpio, on ? 1 : 0);
}

static float envelope_gain(size_t idx, size_t total, int attack_pct, int release_pct)
{
    if (total <= 1) {
        return 1.0f;
    }

    const size_t attack = (total * (size_t)attack_pct) / 100U;
    const size_t release = (total * (size_t)release_pct) / 100U;
    float gain = 1.0f;

    if (attack > 0 && idx < attack) {
        const float t = (float)idx / (float)attack;
        gain = t * t;
    }
    if (release > 0 && idx + 1U > total - release) {
        const float t = (float)(total - 1U - idx) / (float)release;
        gain *= t * t;
    }
    return gain;
}

static void apply_pcm_process(int16_t *buf, size_t chunk_start, size_t chunk_samples, size_t play_samples,
                              const zone_tone_config_t *cfg)
{
    for (size_t i = 0; i < chunk_samples; i++) {
        const size_t idx = chunk_start + i;
        const float env = envelope_gain(idx, play_samples, cfg->attack_pct, cfg->release_pct);
        int32_t v = (int32_t)buf[i];
        v = (v * cfg->volume_pct) / 100;
        v = (int32_t)((float)v * env);

        if (v > 32767) {
            v = 32767;
        } else if (v < -32768) {
            v = -32768;
        }
        buf[i] = (int16_t)v;
    }
}

static void speaker_diff_gpio_init(void)
{
    gpio_set_drive_capability(TONE_PDM_P_GPIO, GPIO_DRIVE_CAP_0);
    if (TONE_PDM_N_GPIO != GPIO_NUM_NC) {
        PIN_FUNC_SELECT(IO_MUX_GPIO8_REG, PIN_FUNC_GPIO);
        gpio_set_direction(TONE_PDM_N_GPIO, GPIO_MODE_OUTPUT);
        esp_rom_gpio_connect_out_signal(TONE_PDM_N_GPIO, I2SO_SD_OUT_IDX, 1, 0);
        gpio_set_drive_capability(TONE_PDM_N_GPIO, GPIO_DRIVE_CAP_0);
    }
}

static esp_err_t audio_out_hw_init(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;
    chan_cfg.dma_desc_num = TONE_DMA_DESC_NUM;
    chan_cfg.dma_frame_num = TONE_DMA_FRAME_NUM;
    ESP_RETURN_ON_ERROR(i2s_new_channel(&chan_cfg, &s_tx_handle, NULL), TAG, "i2s ch");

    i2s_pdm_tx_config_t pdm_cfg = {
        .clk_cfg = I2S_PDM_TX_CLK_DEFAULT_CONFIG(TONE_SAMPLE_RATE_HZ),
        .slot_cfg = I2S_PDM_TX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg =
        {
            .clk = GPIO_NUM_NC,
            .dout = TONE_PDM_P_GPIO,
            .invert_flags = {.clk_inv = false},
        },
    };
    pdm_cfg.clk_cfg.up_sample_fs = TONE_PDM_UPSAMPLE_FS;
    /* SensairShuttle speaker path needs high PDM gain to stay audible. */
    pdm_cfg.slot_cfg.sd_scale = TONE_PDM_SD_SCALE;
    pdm_cfg.slot_cfg.hp_scale = TONE_PDM_HP_SCALE;
    pdm_cfg.slot_cfg.lp_scale = TONE_PDM_LP_SCALE;
    pdm_cfg.slot_cfg.sinc_scale = TONE_PDM_SINC_SCALE;

    ESP_RETURN_ON_ERROR(i2s_channel_init_pdm_tx_mode(s_tx_handle, &pdm_cfg), TAG, "pdm tx");

    /* 与 setup_device.c adc_audio_out_init 顺序一致：先 enable，再差分 GPIO */
    ESP_RETURN_ON_ERROR(i2s_channel_enable(s_tx_handle), TAG, "pdm enable");
    s_i2s_enabled = true;
    speaker_diff_gpio_init();

    pa_bind_from_board();
    if (!s_pa_from_board) {
        gpio_config_t pa_cfg = {
            .pin_bit_mask = (1ULL << s_pa_gpio),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        ESP_RETURN_ON_ERROR(gpio_config(&pa_cfg), TAG, "pa gpio");
    }
    pa_set(false);

    return ESP_OK;
}

static bool wav_parse_pcm(const uint8_t *wav, size_t len, const int16_t **pcm_out, size_t *sample_count,
                          uint32_t *sample_rate)
{
    if (!wav || len < 44 || memcmp(wav, "RIFF", 4) != 0 || memcmp(wav + 8, "WAVE", 4) != 0) {
        return false;
    }

    size_t offset = 12;
    uint16_t channels = 0;
    uint16_t bits = 0;
    uint32_t rate = 0;
    const uint8_t *data = NULL;
    size_t data_len = 0;

    while (offset + 8 <= len) {
        const char *id = (const char *)(wav + offset);
        uint32_t chunk_len = (uint32_t)wav[offset + 4] | ((uint32_t)wav[offset + 5] << 8) |
                             ((uint32_t)wav[offset + 6] << 16) | ((uint32_t)wav[offset + 7] << 24);
        offset += 8;
        if (offset + chunk_len > len) {
            break;
        }
        if (memcmp(id, "fmt ", 4) == 0 && chunk_len >= 16) {
            channels = (uint16_t)wav[offset + 2] | ((uint16_t)wav[offset + 3] << 8);
            rate = (uint32_t)wav[offset + 4] | ((uint32_t)wav[offset + 5] << 8) | ((uint32_t)wav[offset + 6] << 16) |
                   ((uint32_t)wav[offset + 7] << 24);
            bits = (uint16_t)wav[offset + 14] | ((uint16_t)wav[offset + 15] << 8);
        } else if (memcmp(id, "data", 4) == 0) {
            data = wav + offset;
            data_len = chunk_len;
        }
        offset += chunk_len + (chunk_len & 1U);
    }

    if (!data || bits != 16 || channels != 1 || rate == 0) {
        return false;
    }

    *pcm_out = (const int16_t *)data;
    *sample_count = data_len / sizeof(int16_t);
    *sample_rate = rate;
    return true;
}

static bool i2s_write_pcm(const int16_t *pcm, size_t samples, const zone_tone_config_t *cfg)
{
    if (!s_i2s_enabled || !pcm || samples == 0 || !s_enabled) {
        return false;
    }

    size_t offset = 0;
    const size_t chunk_samples = 256;
    int16_t chunk_buf[256];

    while (offset < samples) {
        if (!s_enabled) {
            return false;
        }
        const size_t n = (offset + chunk_samples > samples) ? (samples - offset) : chunk_samples;
        const size_t bytes = n * sizeof(int16_t);
        memcpy(chunk_buf, &pcm[offset], bytes);
        apply_pcm_process(chunk_buf, offset, n, samples, cfg);

        size_t written = 0;
        esp_err_t err = i2s_channel_write(s_tx_handle, chunk_buf, bytes, &written, portMAX_DELAY);
        if (err != ESP_OK || written != bytes) {
            ESP_LOGE(TAG, "i2s write failed err=0x%x written=%u/%u", err, (unsigned)written, (unsigned)bytes);
            return false;
        }
        offset += n;
    }
    return true;
}

static bool i2s_drain_pcm(void)
{
    /* Write more than a full DMA ring of silence. Accepting these buffers
     * requires all preceding PCM to have been transmitted, including a
     * partially filled final buffer. Keep the PA on until then. */
    const int16_t silence[TONE_DMA_FRAME_NUM] = {0};
    for (int i = 0; i <= TONE_DMA_DESC_NUM; i++) {
        if (!s_enabled) {
            return false;
        }
        size_t written = 0;
        const esp_err_t err = i2s_channel_write(s_tx_handle, silence, sizeof(silence), &written, 1000);
        if (err != ESP_OK || written != sizeof(silence)) {
            ESP_LOGE(TAG, "i2s drain failed err=0x%x written=%u", err, (unsigned)written);
            return false;
        }
    }
    return true;
}

static void play_zone_wav(int zone_id)
{
    const zone_wav_t *zw = &s_zone_wav[zone_id];
    const size_t wav_len = zone_wav_len(zw);
    if (wav_len == 0) {
        return;
    }

    const int16_t *pcm = NULL;
    size_t samples = 0;
    uint32_t rate = 0;
    if (!wav_parse_pcm(zw->wav, wav_len, &pcm, &samples, &rate)) {
        ESP_LOGE(TAG, "WAV parse failed for zone %d", zone_id);
        return;
    }
    if (rate != TONE_SAMPLE_RATE_HZ) {
        ESP_LOGE(TAG, "zone %d wav rate %u != %d, skip", zone_id, (unsigned)rate, TONE_SAMPLE_RATE_HZ);
        return;
    }

    zone_tone_config_t cfg = s_tone_cfg;
    const size_t max_samples = ((size_t)TONE_SAMPLE_RATE_HZ * cfg.max_play_ms) / 1000U;
    if (samples > max_samples) {
        samples = max_samples;
    }

    pa_set(true);
    if (cfg.pa_settle_ms > 0) {
        vTaskDelay(pdMS_TO_TICKS(cfg.pa_settle_ms));
    }

    const bool ok = i2s_write_pcm(pcm, samples, &cfg) && i2s_drain_pcm();
    if (ok && cfg.pa_hold_ms > 0) {
        vTaskDelay(pdMS_TO_TICKS(cfg.pa_hold_ms));
    }
    pa_set(false);

    if (ok) {
        ESP_LOGI(TAG, "Played zone %d (%u ms @ %d Hz, volume=%d)", zone_id,
                 (unsigned)((samples * 1000U) / TONE_SAMPLE_RATE_HZ), TONE_SAMPLE_RATE_HZ, cfg.volume_pct);
    }
}

static void tone_task(void *arg)
{
    (void)arg;
    while (1) {
        int zone = s_pending_zone;
        if (zone == 0) {
            (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            continue;
        }
        s_pending_zone = 0;
        play_zone_wav(zone);
    }
}

void zone_tone_init(void)
{
    if (s_tone_task != NULL) {
        return;
    }

    if (audio_out_hw_init() != ESP_OK) {
        ESP_LOGE(TAG, "Audio out init failed");
        return;
    }

    s_ready = true;
    s_enabled = zone_tone_load_enabled();
    BaseType_t ok = xTaskCreate(tone_task, "zone_tone", 6144, NULL, 3, &s_tone_task);
    if (ok != pdPASS) {
        ESP_LOGE(TAG, "tone task create failed");
        s_tone_task = NULL;
        s_ready = false;
        return;
    }

    ESP_LOGI(TAG, "Zone WAV tone ready (piano, vol=%d, enabled=%d)", s_tone_cfg.volume_pct, (int)s_enabled);
}

void zone_tone_set_zone(int zone_id)
{
    if (!s_ready || s_tone_task == NULL || !s_enabled) {
        return;
    }
    if (zone_id < 1 || zone_id > 9) {
        s_pending_zone = 0;
        return;
    }
    if (zone_id == ZONE_TONE_SILENT_ID || s_zone_wav[zone_id].wav == NULL) {
        s_pending_zone = 0;
        return;
    }

    s_pending_zone = zone_id;
    xTaskNotifyGive(s_tone_task);
}

void zone_tone_update(int zone_id)
{
    (void)zone_id;
}

void zone_tone_set_enabled(bool enabled)
{
    if (s_enabled == enabled) {
        return;
    }
    s_enabled = enabled;
    zone_tone_store_enabled(enabled);
    ESP_LOGW(TAG, "Zone tone %s", enabled ? "enabled" : "disabled");
    if (!enabled) {
        s_pending_zone = 0;
        pa_set(false);
    } else if (s_ready && s_tone_task != NULL && s_zone_wav[ZONE_TONE_CONFIRM_ID].wav != NULL) {
        /* Audible confirmation when speaker is turned on */
        s_pending_zone = ZONE_TONE_CONFIRM_ID;
        xTaskNotifyGive(s_tone_task);
    }
}

bool zone_tone_is_enabled(void)
{
    return s_enabled;
}

void zone_tone_get_config(zone_tone_config_t *cfg)
{
    if (!cfg) {
        return;
    }
    *cfg = s_tone_cfg;
}

void zone_tone_set_config(const zone_tone_config_t *cfg)
{
    if (!cfg) {
        return;
    }
    s_tone_cfg = zone_tone_sanitize_config(cfg);
    ESP_LOGI(TAG, "Tone cfg: vol=%d max=%u settle=%u hold=%u attack=%d release=%d", s_tone_cfg.volume_pct,
             (unsigned)s_tone_cfg.max_play_ms, (unsigned)s_tone_cfg.pa_settle_ms, (unsigned)s_tone_cfg.pa_hold_ms,
             s_tone_cfg.attack_pct, s_tone_cfg.release_pct);
}

void zone_tone_reset_config(void)
{
    const zone_tone_config_t defaults = {
        .volume_pct = TONE_DEFAULT_VOLUME,
        .max_play_ms = TONE_DEFAULT_MAX_PLAY_MS,
        .pa_settle_ms = TONE_DEFAULT_PA_SETTLE_MS,
        .pa_hold_ms = TONE_DEFAULT_PA_HOLD_MS,
        .attack_pct = TONE_DEFAULT_ATTACK_PCT,
        .release_pct = TONE_DEFAULT_RELEASE_PCT,
    };
    zone_tone_set_config(&defaults);
}

void zone_tone_play_test(int zone_id)
{
    zone_tone_set_zone(zone_id);
}

#endif /* ENABLE_ZONE_TONE */
