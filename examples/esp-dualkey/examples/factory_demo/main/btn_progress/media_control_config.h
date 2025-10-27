/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bsp/keycodes.h"
#include "tinyusb_hid.h"

#ifdef __cplusplus
extern "C" {
#endif

// Media control key mapping structure
typedef struct {
    uint16_t keycode;              /**< Keycode from BSP */
    uint16_t hid_usage;            /**< Corresponding HID usage */
} media_key_mapping_t;

// Extended media control key mappings with fallbacks
static const media_key_mapping_t extended_media_key_map[] = {
    // Volume and audio control
    {KC_KB_MUTE,           HID_USAGE_CONSUMER_MUTE},
    {KC_KB_VOLUME_UP,      HID_USAGE_CONSUMER_VOLUME_INCREMENT},
    {KC_KB_VOLUME_DOWN,    HID_USAGE_CONSUMER_VOLUME_DECREMENT},

    // Basic media transport
    {KC_MEDIA_PREV_TRACK,  HID_USAGE_CONSUMER_SCAN_PREVIOUS_TRACK},
    {KC_MEDIA_NEXT_TRACK,  HID_USAGE_CONSUMER_SCAN_NEXT_TRACK},
    {KC_MEDIA_STOP,        HID_USAGE_CONSUMER_STOP},
    {KC_MEDIA_PLAY_PAUSE,  HID_USAGE_CONSUMER_PLAY_PAUSE},

    // Extended media control (with fallbacks)
    {KC_MEDIA_FAST_FORWARD, HID_USAGE_CONSUMER_SCAN_NEXT_TRACK},
    {KC_MEDIA_REWIND,      HID_USAGE_CONSUMER_SCAN_PREVIOUS_TRACK},
    {KC_MEDIA_EJECT,       HID_USAGE_CONSUMER_AL_CONSUMER_CONTROL_CONFIGURATION},
    {KC_MEDIA_SELECT,      HID_USAGE_CONSUMER_AL_CONSUMER_CONTROL_CONFIGURATION},

    // Additional media keys that might be useful
    {KC_MEDIA_FAST_FORWARD, HID_USAGE_CONSUMER_SCAN_NEXT_TRACK},
    {KC_MEDIA_REWIND,      HID_USAGE_CONSUMER_SCAN_PREVIOUS_TRACK},

    {KC_AUDIO_MUTE,        HID_USAGE_CONSUMER_MUTE},
    {KC_AUDIO_VOL_UP,      HID_USAGE_CONSUMER_VOLUME_INCREMENT},
    {KC_AUDIO_VOL_DOWN,    HID_USAGE_CONSUMER_VOLUME_DECREMENT},
    {KC_MEDIA_NEXT_TRACK,  HID_USAGE_CONSUMER_SCAN_NEXT_TRACK},
    {KC_MEDIA_PREV_TRACK,  HID_USAGE_CONSUMER_SCAN_PREVIOUS_TRACK},
    {KC_MEDIA_STOP,        HID_USAGE_CONSUMER_STOP},
    {KC_MEDIA_PLAY_PAUSE,  HID_USAGE_CONSUMER_PLAY_PAUSE},
    {KC_MEDIA_SELECT,      HID_USAGE_CONSUMER_AL_CONSUMER_CONTROL_CONFIGURATION},
    {KC_MEDIA_EJECT,       HID_USAGE_CONSUMER_AL_CONSUMER_CONTROL_CONFIGURATION},
};

#define EXTENDED_MEDIA_KEY_MAP_SIZE (sizeof(extended_media_key_map) / sizeof(extended_media_key_map[0]))

// Function to find media key mapping
static inline bool find_extended_media_key_mapping(uint16_t keycode, uint16_t *hid_usage)
{
    for (size_t i = 0; i < EXTENDED_MEDIA_KEY_MAP_SIZE; i++) {
        if (extended_media_key_map[i].keycode == keycode) {
            *hid_usage = extended_media_key_map[i].hid_usage;
            return true;
        }
    }
    return false;
}

// Function to check if a keycode is a media control key
static inline bool is_media_control_key(uint16_t keycode)
{
    uint16_t hid_usage;
    return find_extended_media_key_mapping(keycode, &hid_usage);
}

#ifdef __cplusplus
}
#endif
