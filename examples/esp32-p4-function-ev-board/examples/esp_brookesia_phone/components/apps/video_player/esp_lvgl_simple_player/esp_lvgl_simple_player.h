/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Player states
 */
typedef enum
{
    PLAYER_STATE_PLAYING,
    PLAYER_STATE_PAUSED,
    PLAYER_STATE_STOPPED,
} player_state_t;

/**
 * @brief Player configuration structure
 */
typedef struct {
    const char        *video_path;      /* File path to play */
    const char        *bgm_path;        /* File path to play */
    lv_obj_t    *screen;    /* LVGL screen to put the player */
    uint32_t    buff_size;      /* Size of the buffer for one video frame */
    uint32_t    cache_buff_size;      /* Size of the buffer for one video frame */
    bool        cache_buff_in_psram;    /* Use PSRAM for split buffer */
    uint32_t    screen_width;   /* Width of the video player object */
    uint32_t    screen_height;  /* Height of the video player object */
    struct {
        unsigned int hide_controls: 1;  /* Hide control buttons */
        unsigned int hide_slider: 1;  /* Hide indication slider */
        unsigned int hide_status: 1;  /* Hide status icons in video (paused, stopped) */

        unsigned int auto_width: 1;  /* Set automatic width by video size */
        unsigned int auto_height: 1;  /* Set automatic height by video size */
    } flags;
} esp_lvgl_simple_player_cfg_t;

/**
 * @brief Create Player
 *
 * This function initializes video decoder (JPEG, ...), creates LVGL objects and starts handling task.
 *
 * @return
 *      - ESP_OK                 On success
 */
lv_obj_t * esp_lvgl_simple_player_create(esp_lvgl_simple_player_cfg_t * params);

/**
 * @brief Get player state
 */
player_state_t esp_lvgl_simple_player_get_state(void);

/**
 * @brief Hide player controls
 */
void esp_lvgl_simple_player_hide_controls(bool hide);

/**
 * @brief Change file for playing
 */
void esp_lvgl_simple_player_change_file(const char *video_file);

/**
 * @brief Play player
 */
void esp_lvgl_simple_player_play(void);

/**
 * @brief Pause player
 */
void esp_lvgl_simple_player_pause(void);

void esp_lvgl_simple_player_resume(void);

/**
 * @brief Stop player
 */
void esp_lvgl_simple_player_stop(void);

/**
 * @brief Set repeat playing
 */
void esp_lvgl_simple_player_repeat(bool repeat);

/**
 * @brief Delete Player
 *
 * @return
 *      - ESP_OK                 On success
 */
esp_err_t esp_lvgl_simple_player_del(void);

esp_err_t esp_lvgl_simple_player_wait_task_stop(int timeout_ms);

#ifdef __cplusplus
}
#endif
