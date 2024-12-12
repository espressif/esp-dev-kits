// Copyright 2019 Espressif Systems (Shanghai) PTE LTD
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
#pragma once

#include "esp_tts.h"
#include "esp_tts_voice_xiaole.h"
#include "esp_tts_player.h"
#include "driver/i2s.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief output chinese data.
 *
 * @param tts_handle a pointer of Handle to the TTS for  example:    
 *                      esp_tts_voice_t *voice = &esp_tts_voice_xiaole;
 *                      esp_tts_handle_t *tts_handle = esp_tts_create(voice);
 *
 * @param data data of chinese sentense.
 *
 */
void tts_output_chinese(esp_tts_handle_t *tts_handle,  char *data);

#ifdef __cplusplus
}
#endif

