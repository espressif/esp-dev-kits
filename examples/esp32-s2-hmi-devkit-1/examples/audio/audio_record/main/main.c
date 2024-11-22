/**
 * @file main.c
 * @brief Audio record example
 * @version 0.1
 * @date 2021-03-04
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

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"

#include "driver/adc.h"
#include "driver/timer.h"

#include "esp_heap_caps.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "bsp_i2c.h"
#include "lvgl_port.h"

#include "ft5x06.h"
#include "tpl0401.h"
#include "tca9554.h"

#include "bsp_ext_io.h"
#include "adc_isr.h"
#include "ui_record.h"
#include "sdkconfig.h"

#ifdef CONFIG_AUDIO_DAC_OUTPUT
#include "dac_audio.h"
#endif

#ifdef CONFIG_AUDIO_PWM_OUTPUT
#include "pwm_audio.h"
#endif

#define CONFIG_AUDIO_SAMPLE_RATE     (20000)

static const char *TAG = "rec_example";

static size_t audio_index = 0;
static size_t audio_total = 0;
static uint8_t *audio_buffer = NULL;

static void timer_group_init(void)
{
    timer_config_t config = {
        .divider = 80,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .intr_type = TIMER_INTR_LEVEL,
        .auto_reload = TIMER_AUTORELOAD_EN,
    };

    timer_init(TIMER_GROUP_0, TIMER_0, &config);
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0ULL);
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, (1000000ULL / (uint64_t) CONFIG_AUDIO_SAMPLE_RATE));

    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
}

static IRAM_ATTR bool audio_acquire_isr(void *data)
{
    static DRAM_ATTR int adc_val = 0;

    adc_val = adc1_get_raw_from_isr(ADC_CHANNEL_8);

    audio_buffer[audio_index] = adc_val >> 5;

    if (audio_index < 3 * 20000 - 1) {
        audio_index++;
    } else {
        timer_pause(TIMER_GROUP_0, TIMER_0);
    }

    return false;
}

static IRAM_ATTR bool audio_play_isr(void *data)
{
#ifdef CONFIG_AUDIO_DAC_OUTPUT
    dac_set_value_fast(audio_buffer[audio_index]);
#endif

#if CONFIG_AUDIO_PWM_OUTPUT
    ledc_set_duty_fast(audio_buffer[audio_index]);
#endif

    if (audio_index < audio_total) {
        audio_index++;
    } else {
        timer_pause(TIMER_GROUP_0, TIMER_0);
        
#ifdef CONFIG_AUDIO_DAC_OUTPUT
        dac_set_value_fast(0);
#endif

#if CONFIG_AUDIO_PWM_OUTPUT
        ledc_set_duty_fast(0);
#endif
    }

    return false;
}

static IRAM_ATTR void audio_set_index(size_t index)
{
    audio_index = index;
}

void audio_play_start(void)
{
    timer_pause(TIMER_GROUP_0, TIMER_0);
    timer_isr_callback_remove(TIMER_GROUP_0, TIMER_0);
    timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, audio_play_isr, NULL, ESP_INTR_FLAG_IRAM);
    audio_total = audio_index;
    audio_set_index(0);
    timer_start(TIMER_GROUP_0, TIMER_0);
}

void audio_record_start(void)
{
    timer_pause(TIMER_GROUP_0, TIMER_0);
    timer_isr_callback_remove(TIMER_GROUP_0, TIMER_0);
    timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, audio_acquire_isr, NULL, ESP_INTR_FLAG_IRAM);
    audio_set_index(0);
    timer_start(TIMER_GROUP_0, TIMER_0);
}

void app_main(void)
{
    ESP_ERROR_CHECK(bsp_i2c_init(I2C_NUM_0, 400000));

    /* Volume digital potentiometer */
    ESP_ERROR_CHECK(tpl0401_init());
    ESP_ERROR_CHECK(tpl0401_set_resistance(30 * 127 / 100));

    /* Initialize LCD */
    ESP_ERROR_CHECK(bsp_lcd_init());

    /* LCD touch IC init */
    ESP_ERROR_CHECK(ft5x06_init());

    /* Initialize LVGL */
    ESP_ERROR_CHECK(lvgl_init(LVGL_SCR_SIZE / 8, LV_BUF_ALLOC_INTERNAL));

    /* Config ADC */
    adc1_config_width(ADC_WIDTH_BIT_13);
    adc1_config_channel_atten(GPIO_ADC_CH, ADC_ATTEN_DB_6);

#if CONFIG_AUDIO_DAC_OUTPUT
    /* Config DAC output for audio interface */
    dac_output_enable(GPIO_DAC_CH);
#endif

#if CONFIG_AUDIO_PWM_OUTPUT
    /* Config LEDC as PWM output for audio interface */
    static pwm_audio_config_t pac = {
        .duty_resolution    = LEDC_TIMER_8_BIT,
        .gpio_num           = GPIO_NUM_18,
        .ledc_channel       = LEDC_CHANNEL_1,
        .ledc_timer_sel     = LEDC_TIMER_1,
    };
    pwm_audio_init(&pac);
#endif

    /* Allocate audio buffer. Limited by DRAM size, only allocate 3S */
    audio_buffer = heap_caps_malloc(CONFIG_AUDIO_SAMPLE_RATE * 3 * sizeof(uint8_t),
        MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

    if (NULL == audio_buffer) {
        ESP_LOGE(TAG, "Failed allocate mem for audio buffer.");
        return;
    }

    /* Start timer group to call isr periodically */
    timer_group_init();

    /* Init UI of example */
    ui_record();
}
