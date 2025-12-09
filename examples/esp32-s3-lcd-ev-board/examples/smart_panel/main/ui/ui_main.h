/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#pragma once

#include "bsp/esp-bsp.h"

#include "lvgl.h"
#include "ui_font_declare.h"
#include "lv_symbol_extra_def.h"

#define COLOR_BAR   lv_color_make(86, 94, 102)
#define COLOR_THEME lv_color_make(252, 199, 0)
#define COLOR_DEEP  lv_color_make(246, 174, 61)
#define COLOR_TEXT  lv_color_make(56, 56, 56)
#define COLOR_BG    lv_color_make(238, 241, 245)

#define _UI_FUNC_DEF_(ui_name)                  \
    extern ui_func_desc_t ui_##ui_name##_func;  \
    void ui_##ui_name##_init(void *data);       \
    void ui_##ui_name##_show(void *data);       \
    void ui_##ui_name##_hide(void *data);

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ui_state_dis = 0,       /* UI not initialized yet */
    ui_state_show,          /* UI is active and showing */
    ui_state_hide,          /* UI is hidden */
} ui_state_t;

typedef enum {
    status_bar_item_wifi = 0,
    status_bar_item_battery,
    status_bar_item_bluetooth,
    status_bar_item_little_time,
    status_bar_item_max,
} status_bar_item_t;

typedef enum {
    UI_SHOW_OVERRIDE = 0,   /* Show new page and replace its position in call queue */
    UI_SHOW_PEDDING,        /* Show UI and add it behind call queue's tail */
    UI_SHOW_BACKPORT,       /* Show UI in previous call queue */
} ui_show_mode_t;

typedef enum {
    ui_clock_item_temp = 0,
    ui_clock_item_humid,
    ui_clock_item_weather,
    ui_clock_item_weather_code,
    ui_clock_item_uv,
    ui_clock_item_aqi,
    ui_clock_item_aqi_level,
} ui_clock_item_t;

typedef struct {
    char *path;             /* File path */
    void **data;            /* Pointer of data to store in RAM */
} ui_data_fetch_t;

typedef struct {
    char *name;             /* UI name */
    void (*init)(void *);   /* init function pointer */
    void (*show)(void *);   /* show function pointer */
    void (*hide)(void *);   /* hide function pointer */
} ui_func_desc_t;

/**
 * @brief Initialize main UI, including resources loading.
 *
 */
void ui_main(void);

/**
 * @brief Load resource from SD card to RAM.
 *
 * @param path The path of image.
 * @param dst Address to save image data.
 */
void ui_laod_resource(const char *path, void **dst);


/**
 * @brief Show UI with call queue control.
 *
 * @param ui UI to show. Can be NULL if mode is UI_SHOW_BACKPORT.
 * @param mode UI show mode, see `ui_show_mode_t`. All function is listed below.
 */
void ui_show(ui_func_desc_t *ui, ui_show_mode_t mode);

/* UI function defination */
_UI_FUNC_DEF_(page);

_UI_FUNC_DEF_(about);
_UI_FUNC_DEF_(air);
_UI_FUNC_DEF_(air_conditioner);
_UI_FUNC_DEF_(pomodoro_timer);
_UI_FUNC_DEF_(habit_tracker);
_UI_FUNC_DEF_(app);
_UI_FUNC_DEF_(bluetooth);
_UI_FUNC_DEF_(clock);
_UI_FUNC_DEF_(humidity);
_UI_FUNC_DEF_(led);
_UI_FUNC_DEF_(music);
_UI_FUNC_DEF_(setting);
_UI_FUNC_DEF_(switch);
_UI_FUNC_DEF_(uv);
_UI_FUNC_DEF_(weather);
_UI_FUNC_DEF_(guide);
_UI_FUNC_DEF_(sr);
// _UI_FUNC_DEF_(ui_pomodoro_analytics_func);

/**
 * @brief Init status bar. Objects to show time, qucik action buttons and Wi-Fi signal indicate.
 *
 */
void ui_status_bar_init(void);

/**
 * @brief Show or hide time on status bar.
 *
 * @param show True if show.
 */
void ui_status_bar_time_show(bool show);

/**
 * @brief Set text of status bar.
 *
 * @param item Type of item. See defination at `status_bar_item_t`.
 * @param text Text of item.
 */
void ui_status_bar_set_item_text(status_bar_item_t item, const char *text);

/**
 * @brief Get page object used as container.
 *
 * @return lv_obj_t* object pointer of page. NULL if not initialized.
 */
lv_obj_t *ui_page_get_obj(void);

/**
 * @brief Set clock page item text.
 *
 * @param item Item to set. See `ui_clock_item_t`.
 * @param text Text to show or parse.
 */
void ui_clock_set_item_val(ui_clock_item_t item, const char *text);

/**
 * @brief Update LED UI, can be called when network data received.
 *
 */
void ui_led_update(void);

/**
 * @brief Update widgets capton of weather page. Call it when weather data is updated.
 *
 */
void weather_update_widget_caption(void);

/**
 * @brief Update air information of AQI page. Call it when air data is updated.
 *
 */
void ui_air_update_value(void);

/**
 * @brief Update weather information of clock page. Call it when weather data is updated.
 *
 */
void ui_clock_update(void);

/**
 * @brief Update date text. Call it when time is successfully updated via NTP.
 *
 */
void ui_clock_update_date(void);

/**
 * @brief Update length of music file in music page.
 *
 * @param data_size Size of audio file. Length will automiclly calculated by `SAMPLE_RATE` in `app_music.h`.
 */
void ui_music_update_length(size_t data_size);

/**
 * @brief Update current play time of music file in music page.
 *
 * @param data_index Current file index.
 */
void ui_music_update_update_time(size_t data_index);

/**
 * @brief Reset lyric time count.
 *
 */
void lyric_reset_count(void);

#ifdef __cplusplus
}
#endif
