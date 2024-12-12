/**
 * @file pwm_audio.c
 * @brief PWM audio
 * @version 0.1
 * @date 2021-03-07
 * 
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "pwm_audio.h"

#ifndef CONFIG_IDF_TARGET_ESP32S2
#error Not defined idf target to esp32s2
#endif

static const char *TAG = "pwm_audio";

#define PWM_AUDIO_CHECK(a, str, ret_val)                          \
    if (!(a))                                                     \
    {                                                             \
        ESP_LOGE(TAG, "%s(%d): %s", __FUNCTION__, __LINE__, str); \
        return (ret_val);                                         \
    }

static const char *PWM_AUDIO_PARAM_ADDR_ERROR = "PWM AUDIO PARAM ADDR ERROR";
static const char *PWM_AUDIO_ALLOC_ERROR      = "PWM AUDIO ALLOC ERROR";
static const char *PWM_AUDIO_RESOLUTION_ERROR = "PWM AUDIO RESOLUTION ERROR";

#define SAMPLE_RATE_MAX     (48000)
#define SAMPLE_RATE_MIN     (8000)

typedef struct {
    pwm_audio_config_t    config;           /**< pwm audio config struct */
    ledc_channel_config_t ledc_channel;     /**< ledc channel config */
    ledc_timer_config_t   ledc_timer;       /**< ledc timer config  */
    timg_dev_t            *timg_dev;        /**< timer group register pointer */
} pwm_audio_handle;
typedef pwm_audio_handle *pwm_audio_handle_t;

/**< ledc some register pointer */
static volatile uint32_t *g_ledc_conf0_val  = NULL;
static volatile uint32_t *g_ledc_conf1_val  = NULL;
static volatile uint32_t *g_ledc_duty_val   = NULL;

/**< pwm audio handle pointer */
static pwm_audio_handle_t g_pwm_audio_handle = NULL;

/*
 * Note:
 * In order to improve efficiency, register is operated directly
 */
IRAM_ATTR void ledc_set_duty_fast(uint32_t duty_val)
{
    *g_ledc_duty_val = (duty_val) << 4; /* Discard decimal part */
    *g_ledc_conf0_val |= 0x00000014;
    *g_ledc_conf1_val |= 0x80000000;
}

esp_err_t pwm_audio_init(const pwm_audio_config_t *cfg)
{
    esp_err_t res = ESP_OK;
    PWM_AUDIO_CHECK(cfg != NULL, PWM_AUDIO_PARAM_ADDR_ERROR, ESP_ERR_INVALID_ARG);
    PWM_AUDIO_CHECK(cfg->duty_resolution <= 10 && cfg->duty_resolution >= 8, PWM_AUDIO_RESOLUTION_ERROR, ESP_ERR_INVALID_ARG);
    PWM_AUDIO_CHECK(NULL == g_pwm_audio_handle, "Already initiate", ESP_ERR_INVALID_STATE);

    // ESP_LOGI(TAG, "io: %d | resolution: %dBIT",
    //          cfg->gpio_num, cfg->duty_resolution);

    pwm_audio_handle_t handle = NULL;

    handle = heap_caps_malloc(sizeof(pwm_audio_handle), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    PWM_AUDIO_CHECK(handle != NULL, PWM_AUDIO_ALLOC_ERROR, ESP_ERR_NO_MEM);
    memset(handle, 0, sizeof(pwm_audio_handle));

    handle->config = *cfg;
    g_pwm_audio_handle = handle;

    /**
     * config ledc to generate pwm
     */
    handle->ledc_channel.channel = handle->config.ledc_channel;
    handle->ledc_channel.duty = 0;
    handle->ledc_channel.gpio_num = handle->config.gpio_num;
    handle->ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
    handle->ledc_channel.hpoint = 0;
    handle->ledc_channel.timer_sel = handle->config.ledc_timer_sel;
    handle->ledc_channel.intr_type = LEDC_INTR_DISABLE;
    res = ledc_channel_config(&handle->ledc_channel);
    PWM_AUDIO_CHECK(ESP_OK == res, "LEDC channel configuration failed", ESP_ERR_INVALID_ARG);

    handle->ledc_timer.clk_cfg = LEDC_USE_APB_CLK;
    handle->ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;
    handle->ledc_timer.duty_resolution = handle->config.duty_resolution;
    handle->ledc_timer.timer_num = handle->config.ledc_timer_sel;
    uint32_t freq = (APB_CLK_FREQ / (1 << handle->ledc_timer.duty_resolution));
    handle->ledc_timer.freq_hz = freq - (freq % 1000); // fixed PWM frequency ,It's a multiple of 1000
    ledc_timer_config(&handle->ledc_timer);
    PWM_AUDIO_CHECK(res == ESP_OK, "LEDC timer configuration failed", ESP_ERR_INVALID_ARG);

    /**
     * Get the address of LEDC register to reduce the addressing time
     */
    g_ledc_duty_val = &LEDC.channel_group[handle->ledc_timer.speed_mode].\
                           channel[handle->ledc_channel.channel].duty.val;
    g_ledc_conf0_val = &LEDC.channel_group[handle->ledc_timer.speed_mode].\
                            channel[handle->ledc_channel.channel].conf0.val;
    g_ledc_conf1_val = &LEDC.channel_group[handle->ledc_timer.speed_mode].\
                            channel[handle->ledc_channel.channel].conf1.val;

    return ESP_OK;
}

esp_err_t pwm_audio_deinit(void)
{
    pwm_audio_handle_t handle = g_pwm_audio_handle;
    PWM_AUDIO_CHECK(handle != NULL, PWM_AUDIO_PARAM_ADDR_ERROR, ESP_FAIL);

    if (handle->ledc_channel.gpio_num >= 0) {
        ledc_stop(handle->ledc_channel.speed_mode, handle->ledc_channel.channel, 0);
    }

    /**< set the channel gpios input mode */
    if (handle->ledc_channel.gpio_num >= 0) {
        gpio_set_direction(handle->ledc_channel.gpio_num, GPIO_MODE_INPUT);
    }

    heap_caps_free(handle);
    g_pwm_audio_handle = NULL;
    return ESP_OK;
}
