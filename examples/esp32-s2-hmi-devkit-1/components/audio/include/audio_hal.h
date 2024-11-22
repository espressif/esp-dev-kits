/**
 * @file audio_hal.h
 * @brief Audio HAL header file
 * @version 0.1
 * @date 2021-02-24
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

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "driver/timer.h"
#include "driver/ledc.h"

#include "esp_err.h"
#include "esp_heap_caps.h"

#include "ring_buffer.h"

#include "sdkconfig.h"

#ifdef CONFIG_AUDIO_DAC_OUTPUT
#include "dac_audio.h"
#endif

#ifdef CONFIG_AUDIO_PWM_OUTPUT
#include "pwm_audio.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AUDIO_STATE_STOPPED = 0,
    AUDIO_STATE_PLAY,
    AUDIO_STATE_PAUSE,
    AUDIO_STATE_PENDING,
} audio_hal_state_t;

typedef struct {
    size_t buffer_size;
    size_t audio_freq;
    uint32_t malloc_caps;
} audio_hal_config_t;

/**
 * @brief Init audio hal component
 * 
 * @param cfg 
 * @return esp_err_t 
 */
esp_err_t audio_hal_init(audio_hal_config_t *cfg);

/**
 * @brief Deinit audio hal component and free memory
 * 
 * @return esp_err_t 
 */
esp_err_t audio_hal_deinit(void);

/**
 * @brief Start playing audio with audio hal component
 * 
 */
void audio_hal_start(void);

/**
 * @brief Pause audio playing with audio hal component
 * 
 */
void audio_hal_pause(void);

/**
 * @brief Clear audio data in ring buffer and reset data pointer
 * 
 * @return esp_err_t 
 */
esp_err_t audio_hal_clear_data(void);

/**
 * @brief Add data to audio ring buffer
 * 
 * @param data 
 * @return esp_err_t 
 */
esp_err_t audio_hal_add_byte(uint8_t data);

/**
 * @brief Get audio play state.
 * 
 * @return audio_hal_state_t 
 */
audio_hal_state_t audio_hal_get_state(void);

#ifdef __cplusplus
}
#endif
