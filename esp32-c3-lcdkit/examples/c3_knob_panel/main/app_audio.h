
/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#pragma once

typedef enum{
    SOUND_TYPE_KNOB,
    SOUND_TYPE_SNORE,
    SOUND_TYPE_WASH_END_CN,
    SOUND_TYPE_WASH_END_EN,
    SOUND_TYPE_FACTORY,
}PDM_SOUND_TYPE;

esp_err_t audio_force_quite(bool ret);

esp_err_t audio_handle_info(PDM_SOUND_TYPE voice);

esp_err_t audio_play_start();