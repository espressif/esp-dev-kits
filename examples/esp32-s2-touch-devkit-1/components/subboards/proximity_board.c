// Copyright 2021 Espressif Systems (Shanghai) PTE LTD
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

#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "hal/touch_sensor_hal.h"
#include "esp_log.h"

#include "subboards.h"

static const char *TAG = "Touch proximity";

#define PROXIMITY_COUNT               40
#define PROXIMITY_CHANNEL_NUM         3

static const touch_pad_t proxinmity_channel[PROXIMITY_CHANNEL_NUM] = {
    TOUCH_PAD_NUM5,
    TOUCH_PAD_NUM7,
    TOUCH_PAD_NUM9
};

static const float proxinmity_channel_sense[PROXIMITY_CHANNEL_NUM] = {
    0.0015F,
    0.0015F,
    0.0015F
};

typedef struct {
    touch_pad_t channel;
    bool state;
} touch_isr_info_t;

static TaskHandle_t proximity_task_handle = NULL;
static QueueHandle_t proximity_state_queue = NULL;
static SemaphoreHandle_t proximity_sample_sem = NULL;
static volatile uint32_t proximity_signal[PROXIMITY_CHANNEL_NUM] = {0};

void touch_sensor_isr(void *arg)
{
    (void) arg;
    static bool is_set_threshold = false;
    static int pass_count = 3;
    int task_awoken = pdFALSE;
    uint32_t intr_mask = touch_pad_read_intr_status_mask();
    touch_pad_t channel = touch_pad_get_current_meas_channel();
    
    if (intr_mask & TOUCH_PAD_INTR_MASK_SCAN_DONE) {
        if (pass_count >= 0) {
            pass_count--;
            return;
        }
        for (int i = 0; i < PROXIMITY_CHANNEL_NUM; ++i) {
            uint32_t current_meas_cnt = 0;
            touch_hal_proximity_read_meas_cnt(proxinmity_channel[i], &current_meas_cnt);
            if (current_meas_cnt == PROXIMITY_COUNT) {
                continue;
            } else {
                return;
            }
        }

        for (int i = 0; i < PROXIMITY_CHANNEL_NUM; ++i) {
            uint32_t signal = 0;
            touch_pad_proximity_get_data(proxinmity_channel[i], &signal);
            proximity_signal[i] = signal;
        }

        if (!is_set_threshold) {
            is_set_threshold = true;
            touch_hal_stop_fsm();
            for (int i = 0; i < PROXIMITY_CHANNEL_NUM; ++i) {
                uint32_t signal = 0;
                uint32_t smooth_signal = 0;
                touch_pad_proximity_get_data(proxinmity_channel[i], &signal);
                touch_pad_filter_read_smooth(proxinmity_channel[i], &smooth_signal);
                touch_hal_set_threshold(proxinmity_channel[i], signal + signal * proxinmity_channel_sense[i]);
            }
            touch_hal_start_fsm();
        }
        xSemaphoreGiveFromISR(proximity_sample_sem, &task_awoken);
    } else if (intr_mask & TOUCH_PAD_INTR_MASK_ACTIVE) {
        touch_isr_info_t isr_info;
        isr_info.channel = channel;
        isr_info.state = true;
        xQueueSendFromISR(proximity_state_queue, &isr_info, &task_awoken);
    } else if (intr_mask & TOUCH_PAD_INTR_MASK_INACTIVE) {
        touch_isr_info_t isr_info;
        isr_info.channel = channel;
        isr_info.state = false;
        xQueueSendFromISR(proximity_state_queue, &isr_info, &task_awoken);
    }
    if (task_awoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

static void proximity_task(void *arg)
{
    xSemaphoreTake(proximity_sample_sem, portMAX_DELAY);
    ESP_LOGI(TAG, "Touch proximity initialize finished");
    touch_isr_info_t isr_info;
    while (1) {
        BaseType_t ret = xQueueReceive(proximity_state_queue, &isr_info, pdMS_TO_TICKS(10));
        if (ret == pdTRUE) {
            buzzer_set_voice(isr_info.state);
            if (isr_info.state) {
                ESP_LOGI(TAG, "channel[%d]  pressed", isr_info.channel);
            } else {
                ESP_LOGI(TAG, "channel[%d]  released", isr_info.channel);
            }
        }
        ret = xSemaphoreTake(proximity_sample_sem, pdMS_TO_TICKS(10));
        if (ret == pdTRUE) {
            uint32_t proximity_signal_diff[PROXIMITY_CHANNEL_NUM] = {0};
            for (int i = 0; i < PROXIMITY_CHANNEL_NUM; ++i) {
                uint32_t signal_threshold = 0;
                touch_pad_get_thresh(proxinmity_channel[i], &signal_threshold);
                int diff_signal = proximity_signal[i] - signal_threshold;
                if (diff_signal > 0) {
                    proximity_signal_diff[i] = diff_signal;
                }
            }
            uint32_t red = proximity_signal_diff[0] * 0.1;
            uint32_t green = proximity_signal_diff[0] * 0.1;
            uint32_t blue = proximity_signal_diff[0] * 0.1;
            if (red > 255)  red = 255;
            if (green > 255)  green = 255;
            if (blue > 255)  blue = 255;
            rgb_light_set_characters(red, green, blue);
        }
    }
}

esp_err_t proximity_board_driver_install(void)
{
    esp_err_t de_ret;
    proximity_sample_sem = xSemaphoreCreateBinary();
    proximity_state_queue = xQueueCreate(10, sizeof(touch_isr_info_t));
    esp_err_t ret = touch_pad_init();
    if(ret != ESP_OK)   return ret;
    for (int i = 0; i < PROXIMITY_CHANNEL_NUM; ++i) {
        ret = touch_pad_config(proxinmity_channel[i]);
        if(ret != ESP_OK)   goto cleanup;
        ret = touch_pad_proximity_enable(proxinmity_channel[i], true);
        if(ret != ESP_OK)   goto cleanup;
        ret = touch_pad_proximity_set_count(proxinmity_channel[i], PROXIMITY_COUNT);
        if(ret != ESP_OK)   goto cleanup;
    }
    touch_pad_denoise_t denoise_config = {
        .cap_level = TOUCH_PAD_DENOISE_CAP_L0,
        .grade = TOUCH_PAD_DENOISE_BIT4
    };
    ret = touch_pad_denoise_set_config(&denoise_config);
    if(ret != ESP_OK)   goto cleanup;
    ret = touch_pad_denoise_enable();
    if(ret != ESP_OK)   goto cleanup;
    touch_filter_config_t filter_info = {
        .mode = TOUCH_PAD_FILTER_IIR_16,
        .debounce_cnt = 2,
        .noise_thr = 0,
        .jitter_step = 0,
        .smh_lvl = TOUCH_PAD_SMOOTH_IIR_2,
    };
    ret = touch_pad_filter_set_config(&filter_info);
    if(ret != ESP_OK)   goto cleanup;
    ret = touch_pad_filter_enable();
    if(ret != ESP_OK)   goto cleanup;
    ret = touch_pad_waterproof_enable();
    if(ret != ESP_OK)   goto cleanup;
    ret = touch_pad_set_meas_time(0, 500);
    if(ret != ESP_OK)   goto cleanup;
    ret = touch_pad_set_voltage(TOUCH_HVOLT_2V4, TOUCH_PAD_LOW_VOLTAGE_THRESHOLD, TOUCH_PAD_ATTEN_VOLTAGE_THRESHOLD);
    if(ret != ESP_OK)   goto cleanup;
    ret = touch_pad_isr_register(touch_sensor_isr, NULL, TOUCH_PAD_INTR_MASK_ALL);
    if(ret != ESP_OK)   goto cleanup;
    ret = touch_pad_intr_enable(TOUCH_PAD_INTR_MASK_ACTIVE | TOUCH_PAD_INTR_MASK_INACTIVE | TOUCH_PAD_INTR_MASK_SCAN_DONE);
    if(ret != ESP_OK)   goto cleanup;
    ret = touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    if(ret != ESP_OK)   goto cleanup;
    ret = touch_pad_fsm_start();
    if(ret != ESP_OK)   goto cleanup;
    digital_tube_show_reload(100);
    BaseType_t os_ret = xTaskCreate(proximity_task, "proxinity_task", 4 * 1024, NULL, 10, &proximity_task_handle);
    if (os_ret != pdPASS)   goto cleanup;
    return ESP_OK;

cleanup:
    de_ret = touch_pad_deinit();
    if (de_ret != ESP_OK) {
        abort();
    }
    return ret;
}

esp_err_t proximity_board_driver_uninstall(void)
{
    while (eTaskGetState(proximity_task_handle) == eRunning) {
        vTaskDelay(pdTICKS_TO_MS(1));
    }
    vTaskDelete(proximity_task_handle);
    vQueueDelete(proximity_state_queue);
    vSemaphoreDelete(proximity_sample_sem);

    esp_err_t ret = touch_pad_isr_deregister(touch_sensor_isr, NULL);
    if (ret != ESP_OK)  return ret;
    ret = touch_pad_deinit();
    if (ret != ESP_OK)  return ret;
    return ESP_OK;
}
