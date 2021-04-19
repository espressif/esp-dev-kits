/**
 * @file audio_hal.c
 * @brief Audio HAL src
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

#include "audio_hal.h"

static DRAM_ATTR ringbuf_handle_t audio_buffer = NULL;
static DRAM_ATTR volatile audio_hal_state_t audio_hal_state = AUDIO_STATE_STOPPED;

static IRAM_ATTR bool audio_hal_isr(void *data);

esp_err_t audio_hal_init(audio_hal_config_t *cfg)
{
    /* Init audio interface */
#if CONFIG_AUDIO_DAC_OUTPUT
    dac_output_enable(DAC_CHANNEL_2);
#endif

#if CONFIG_AUDIO_PWM_OUTPUT
    static pwm_audio_config_t pac = {
        .duty_resolution    = LEDC_TIMER_8_BIT,
        .gpio_num           = GPIO_NUM_18,
        .ledc_channel       = LEDC_CHANNEL_1,
        .ledc_timer_sel     = LEDC_TIMER_1,
    };
    pwm_audio_init(&pac);
#endif

    /* Init audio isr via timer group */
    timer_config_t config = {
        .divider = 2,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .intr_type = TIMER_INTR_LEVEL,
        .auto_reload = TIMER_AUTORELOAD_EN,
    };

    timer_init(TIMER_GROUP_0, TIMER_0, &config);
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0ULL);
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 40000000ULL / (uint64_t) cfg->audio_freq);

    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, audio_hal_isr, NULL, ESP_INTR_FLAG_IRAM);

    /* Create audio buffer */
    audio_buffer = rb_create(cfg->buffer_size, cfg->malloc_caps);

    return ESP_OK;
}

IRAM_ATTR void audio_hal_start(void)
{
    audio_hal_state = AUDIO_STATE_PLAY;
    timer_start(TIMER_GROUP_0, TIMER_0);
}

IRAM_ATTR void audio_hal_pause(void)
{
    audio_hal_state = AUDIO_STATE_PAUSE;
    timer_pause(TIMER_GROUP_0, TIMER_0);
}

IRAM_ATTR esp_err_t audio_hal_deinit(void)
{
    audio_hal_state = AUDIO_STATE_STOPPED;

    return ESP_ERR_NOT_SUPPORTED;
}

static IRAM_ATTR bool audio_hal_isr(void *data)
{
    /* Interrupt flag has been reset. No need to clear again. */

    /* The expected output value */
    static uint8_t output = 0;

    /* It's designed like that. 'break' is not missing */
    switch (audio_hal_state) {
    case AUDIO_STATE_STOPPED:
    case AUDIO_STATE_PAUSE:
        audio_hal_pause();
    case AUDIO_STATE_PENDING:
        return false;
    default:
        break;
    }

    /* Check if data avaliable */
    if (ESP_OK != rb_read_byte(audio_buffer, &output)) {
        // output = 0;     /* Make it silent */
        return false;
    }

#ifdef CONFIG_AUDIO_DAC_OUTPUT
    dac_set_value_fast(output);
#endif

#ifdef CONFIG_AUDIO_PWM_OUTPUT
    ledc_set_duty_fast(output);
#endif

    /* DO NOT YIELD */
    return false;
}

IRAM_ATTR esp_err_t audio_hal_add_byte(uint8_t data)
{
    return rb_write_byte(audio_buffer, data);
}

IRAM_ATTR esp_err_t audio_hal_clear_data(void)
{
    audio_hal_state = AUDIO_STATE_PAUSE;

    rb_flush(audio_buffer);
    return ESP_OK;
}

IRAM_ATTR audio_hal_state_t audio_hal_get_state(void)
{
    return audio_hal_state;
}
