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
#include "chinese_tts.h"

int iot_dac_audio_play(const uint8_t *data, int length, TickType_t ticks_to_wait)
{
    size_t bytes_write = 0;
    i2s_write(0, (const char *) data, length, &bytes_write, ticks_to_wait);
    return ESP_OK;
}


void tts_output_chinese(esp_tts_handle_t *tts_handle,  char *data)
{
    if (esp_tts_parse_chinese(tts_handle, data)) {
        int len[1] = {0};

        do {
            short *data = esp_tts_stream_play(tts_handle, len, 4);
            iot_dac_audio_play(data, len[0] * 2, portMAX_DELAY);
            // printf("data:?%d \n", len[0]);
        } while (len[0] > 0);

        i2s_zero_dma_buffer(0);
    }
}