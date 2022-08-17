// Copyright 2015-2022 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _VIRTUAL_MIC_H_
#define _VIRTUAL_MIC_H_

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t sample_rate;
    uint8_t ch_num;
    uint8_t data_width;
} virtual_mic_config_t;

typedef void *vmic_handle_t;

vmic_handle_t virtual_mic_create(const virtual_mic_config_t *cfg);
esp_err_t virtual_mic_delete(vmic_handle_t handle);
esp_err_t virtual_mic_set_clk(vmic_handle_t handle, uint32_t rate, uint32_t bits_cfg, uint32_t ch);
esp_err_t virtual_mic_read(vmic_handle_t handle, void *dest, size_t size, size_t *bytes_read, TickType_t ticks_to_wait);
esp_err_t virtual_mic_input_file(vmic_handle_t handle, const char *filepath);
esp_err_t virtual_mic_input_buffer(vmic_handle_t handle, const uint8_t *inbuf, size_t length);

#ifdef __cplusplus
}
#endif

#endif

