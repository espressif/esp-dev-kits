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

#include "driver/timer.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "sdkconfig.h"
#include "audio_hal.h"

static DRAM_ATTR volatile audio_hal_state_t audio_hal_state = AUDIO_STATE_STOPPED;

esp_err_t audio_hal_init(audio_hal_config_t *cfg)
{


    return ESP_OK;
}

IRAM_ATTR void audio_hal_start(void)
{
    audio_hal_state = AUDIO_STATE_PLAY;
}

IRAM_ATTR void audio_hal_pause(void)
{
    audio_hal_state = AUDIO_STATE_PAUSE;
    
}

IRAM_ATTR esp_err_t audio_hal_deinit(void)
{
    audio_hal_state = AUDIO_STATE_STOPPED;

    return ESP_ERR_NOT_SUPPORTED;
}


IRAM_ATTR esp_err_t audio_hal_add_byte(uint8_t data)
{
    return ESP_OK;
}

IRAM_ATTR esp_err_t audio_hal_clear_data(void)
{
    
    return ESP_OK;
}

IRAM_ATTR audio_hal_state_t audio_hal_get_state(void)
{
    return audio_hal_state;
}
