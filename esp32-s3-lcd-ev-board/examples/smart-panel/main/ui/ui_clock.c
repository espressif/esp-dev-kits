/**
 * @file ui_clock.c
 * @brief 
 * @version 0.1
 * @date 2021-01-11
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

#include "ui_main.h"
#include "app_weather.h"

/* UI function declaration */
ui_func_desc_t ui_clock_func = {
    .name = "ui_clock",
    .init = ui_clock_init,
    .show = ui_clock_show,
    .hide = ui_clock_hide,
};

static ui_state_t ui_clock_state = ui_state_dis;

/* LVGL objects defination */
static lv_obj_t *page_weather = NULL;
static lv_obj_t *page_humidity = NULL;
static lv_obj_t *page_uv = NULL;
static lv_obj_t *bar_div = NULL;
static lv_obj_t *page_air = NULL;
static lv_obj_t *bar_time_date_div = NULL;
static lv_obj_t *label_time = NULL;
static lv_obj_t *label_date = NULL;
static lv_obj_t *label_location = NULL;
static lv_obj_t *label_weather = NULL;
static lv_obj_t *label_temp = NULL;
static lv_obj_t *label_humid = NULL;
static lv_obj_t *label_humid_val = NULL;
static lv_obj_t *label_uv = NULL;
static lv_obj_t *label_uv_val = NULL;
static lv_obj_t *label_aqi = NULL;
static lv_obj_t *label_aqi_val = NULL;
static lv_obj_t *label_air = NULL;
static lv_obj_t *img_weather = NULL;
static lv_obj_t *img_humid = NULL;
static lv_obj_t *img_uv = NULL;
static lv_obj_t *img_pm2p5 = NULL;

/* Air and weather information */
static air_info_t air_info;
static weather_info_t weather_info;

/* Extern image variable(s) */
extern void *data_icon_weather;
extern void *data_icon_humidity;
extern void *data_icon_uv;
extern void *data_icon_pm2p5;
extern void *data_icon_pm2p5_press;

/* Static function forward declaration */
static void time_update_task(lv_task_t *task);
static void page_detail_cb(lv_obj_t *obj, lv_event_t event);

void ui_clock_init(void *data)
{
    (void)data;

    app_weather_get_current_info(&weather_info);

    /* Weather page */
    page_weather = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(page_weather, 400, 160);
    lv_obj_set_event_cb(page_weather, page_detail_cb);
    lv_obj_set_style_local_bg_color(page_weather, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(page_weather, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_radius(page_weather, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_border_width(page_weather, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(page_weather, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -30, 80);

    img_weather = lv_img_create(page_weather, NULL);
    lv_img_set_src(img_weather, data_icon_weather);
    lv_obj_align(img_weather, page_weather, LV_ALIGN_IN_LEFT_MID, 30, 0);

    label_temp = lv_label_create(page_weather, NULL);
    lv_label_set_text(label_temp, weather_info.temp);
    lv_obj_set_style_local_text_font(label_temp, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_60);
    lv_obj_align(label_temp, img_weather, LV_ALIGN_OUT_RIGHT_TOP, 40, 20);

    label_weather = lv_label_create(page_weather, NULL);
    lv_label_set_text(label_weather, weather_info.describe);
    lv_obj_set_style_local_text_font(label_weather, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_36);
    lv_obj_align(label_weather, label_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    /* Humidity page */
    page_humidity = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(page_humidity, 190, 90);
    lv_obj_set_event_cb(page_humidity, page_detail_cb);
    lv_obj_set_style_local_bg_color(page_humidity, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(page_humidity, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_radius(page_humidity, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_border_width(page_humidity, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(page_humidity, page_weather, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    
    img_humid = lv_img_create(page_humidity, NULL);
    lv_img_set_src(img_humid, data_icon_humidity);
    lv_obj_align(img_humid, page_humidity, LV_ALIGN_IN_LEFT_MID, 15, 0);

    label_humid = lv_label_create(page_humidity, NULL);
    lv_label_set_text(label_humid, "Humidity");
    lv_obj_set_style_local_text_font(label_humid, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_align(label_humid, img_humid, LV_ALIGN_OUT_RIGHT_TOP, 10, -10);

    label_humid_val = lv_label_create(page_humidity, NULL);
    lv_label_set_text(label_humid_val, weather_info.humid);
    lv_obj_set_style_local_text_font(label_humid_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_28);
    lv_obj_align(label_humid_val, img_humid, LV_ALIGN_OUT_RIGHT_BOTTOM, 15, 10);

    /* UV light page */
    page_uv = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(page_uv, 190, 90);
    lv_obj_set_event_cb(page_uv, page_detail_cb);
    lv_obj_set_style_local_bg_color(page_uv, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(page_uv, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_radius(page_uv, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_border_width(page_uv, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(page_uv, page_weather, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 16);

    img_uv = lv_img_create(page_uv, NULL);
    lv_img_set_src(img_uv, data_icon_uv);
    lv_obj_align(img_uv, page_uv, LV_ALIGN_IN_LEFT_MID, 15, 0);

    label_uv = lv_label_create(page_uv, NULL);
    lv_label_set_text(label_uv, "UV Index");
    lv_obj_set_style_local_text_font(label_uv, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_align(label_uv, img_uv, LV_ALIGN_OUT_RIGHT_TOP, 10, -10);

    label_uv_val = lv_label_create(page_uv, NULL);
    lv_label_set_text(label_uv_val, weather_info.uv_val);
    lv_obj_set_style_local_text_font(label_uv_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_28);
    lv_obj_align(label_uv_val, img_uv, LV_ALIGN_OUT_RIGHT_BOTTOM, 15, 10);

    page_air = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(page_air, 400, 90);
    lv_obj_set_event_cb(page_air, page_detail_cb);
    lv_obj_set_style_local_bg_color(page_air, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(page_air, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_radius(page_air, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_border_width(page_air, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(page_air, page_humidity, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    img_pm2p5 = lv_img_create(page_air, NULL);
    lv_img_set_src(img_pm2p5, data_icon_pm2p5);
    lv_obj_align(img_pm2p5, page_air, LV_ALIGN_IN_LEFT_MID, 20, 0);

    label_aqi = lv_label_create(page_air, NULL);
    lv_label_set_text(label_aqi, "AQI");
    lv_obj_set_style_local_text_font(label_aqi, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_align(label_aqi, img_pm2p5, LV_ALIGN_OUT_RIGHT_TOP, 15, -10);

    label_aqi_val = lv_label_create(page_air, NULL);
    lv_label_set_text(label_aqi_val, "N/A");
    lv_obj_set_style_local_text_font(label_aqi_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_28);
    lv_obj_set_style_local_text_color(label_aqi_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_align(label_aqi_val, img_pm2p5, LV_ALIGN_OUT_RIGHT_BOTTOM, 15, 10);

    bar_div = lv_bar_create(page_air, NULL);
    lv_obj_set_size(bar_div, 6, 45);
    lv_obj_set_style_local_bg_color(bar_div, LV_BAR_PART_BG, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_align(bar_div, NULL, LV_ALIGN_CENTER, -40, 0);

    label_air = lv_label_create(page_air, NULL);
    lv_label_set_text(label_air, "Good");
    lv_obj_set_style_local_text_color(label_air, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_text_font(label_air, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_36);
    lv_obj_align(label_air, NULL, LV_ALIGN_CENTER, 80, 0);

    /* Main time */
    label_time = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_clock_108);
    lv_obj_set_style_local_text_color(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_color_primary());
    lv_label_set_text(label_time, "08:00");
    lv_obj_set_pos(label_time, 40, 120);

    bar_time_date_div = lv_bar_create(lv_scr_act(), NULL);
    lv_obj_set_size(bar_time_date_div, 100, 6);
    lv_obj_set_style_local_bg_color(bar_time_date_div, LV_BAR_PART_BG, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_align(bar_time_date_div, label_time, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 100);

    label_date = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_text_font(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_28);
    lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_color_primary());
    lv_label_set_text(label_date, "Friday, January 1st");
    lv_obj_align(label_date, bar_time_date_div, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);

    label_location = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_recolor(label_location, true);
    lv_obj_set_style_local_text_font(label_location, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_28);
    lv_obj_set_style_local_text_color(label_location, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_color_primary());
    lv_label_set_text(label_location, "Shanghai, China" "#f6ae3d " LV_SYMBOL_EXTRA_MAP_MARKER_ALT "#");
    lv_obj_align(label_location, label_date, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    /* Set time zone and create a task to update time */
    setenv("TZ", "CST-8", 1);
    tzset();
    lv_task_create(time_update_task, 500, 1, NULL);

    ui_clock_state = ui_state_show;
}

void ui_clock_show(void *data)
{
    if (ui_state_dis == ui_clock_state) {
        ui_clock_init(data);
    } else {
        lv_obj_set_hidden(label_time, false);
        lv_obj_set_hidden(bar_time_date_div, false);
        lv_obj_set_hidden(label_date, false);
        lv_obj_set_hidden(label_location, false);
        lv_obj_set_hidden(page_weather, false);
        lv_obj_set_hidden(page_humidity, false);
        lv_obj_set_hidden(page_uv, false);
        lv_obj_set_hidden(page_air, false);
        ui_clock_state = ui_state_show;
    }

    ui_status_bar_time_show(false);
}

void ui_clock_hide(void *data)
{
    if (ui_state_show == ui_clock_state) {
        lv_obj_set_hidden(label_time, true);
        lv_obj_set_hidden(bar_time_date_div, true);
        lv_obj_set_hidden(label_date, true);
        lv_obj_set_hidden(label_location, true);
        lv_obj_set_hidden(page_weather, true);
        lv_obj_set_hidden(page_humidity, true);
        lv_obj_set_hidden(page_uv, true);
        lv_obj_set_hidden(page_air, true);

        ui_clock_state = ui_state_hide;
    }
}

void ui_clock_set_item_val(ui_clock_item_t item, const char *text)
{
    switch (item) {
    case ui_clock_item_temp:
        if (NULL != label_temp) {
            lv_label_set_text(label_temp, text);
        }
        break;
    case ui_clock_item_humid:
        if (NULL != label_humid_val) {
           lv_label_set_text(label_humid_val, text); 
        }
        break;
    case ui_clock_item_uv:
        if (NULL != label_uv_val) {
            lv_label_set_text(label_uv_val, text);
        }
        break;
    default:
        break;
    }
}

static void time_update_task(lv_task_t *task)
{
    static char fmt_text[8];
    static uint8_t disp_flag = 0;
    static time_t time_utc;
    static struct tm time_local;

    /* Get UTC time */
    time(&time_utc);

    /* Localization time */
    localtime_r(&time_utc, &time_local);

    if (disp_flag) {
        sprintf(fmt_text, "%02d:%02d", time_local.tm_hour, time_local.tm_min);
    } else {
        sprintf(fmt_text, "%02d %02d", time_local.tm_hour, time_local.tm_min);
    }

    disp_flag = !disp_flag;

    if (lv_obj_get_hidden(label_time)) {
        ui_status_bar_set_item_text(status_bar_item_little_time, fmt_text);
    } else {
        lv_label_set_text_static(label_time, fmt_text);
    }
}

static void page_detail_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED == event) {
        if (page_weather == obj) {
            lv_obj_set_style_local_text_color(label_temp, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_text_color(label_weather, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        }

        if (page_air == obj) {
            lv_obj_set_style_local_text_color(label_aqi, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_text_color(label_aqi_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_bg_color(bar_div, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_text_color(label_air, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_img_set_src(img_pm2p5, data_icon_pm2p5_press);
        }

        if (page_uv == obj) {
            lv_obj_set_style_local_text_color(label_uv, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_text_color(label_uv_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        }

        if (page_humidity == obj) {
            lv_obj_set_style_local_text_color(label_humid, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_text_color(label_humid_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        }
    }

    if (LV_EVENT_PRESS_LOST == event) {
        if (page_weather == obj) {
            lv_obj_set_style_local_text_color(label_temp, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
            lv_obj_set_style_local_text_color(label_weather, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
        }

        if (page_air == obj) {
            lv_obj_set_style_local_text_color(label_aqi, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
            lv_obj_set_style_local_text_color(label_aqi_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
            lv_obj_set_style_local_bg_color(bar_div, LV_BAR_PART_BG, LV_STATE_DEFAULT, COLOR_THEME);
            lv_obj_set_style_local_text_color(label_air, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
            lv_img_set_src(img_pm2p5, data_icon_pm2p5);
        }

        if (page_uv == obj) {
            lv_obj_set_style_local_text_color(label_uv, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
            lv_obj_set_style_local_text_color(label_uv_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
        }

        if (page_humidity == obj) {
            lv_obj_set_style_local_text_color(label_humid, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
            lv_obj_set_style_local_text_color(label_humid_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
        }
    }

    if (LV_EVENT_CLICKED == event) {
        if (page_weather == obj) {
            lv_obj_set_style_local_text_color(label_temp, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
            lv_obj_set_style_local_text_color(label_weather, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);

            ui_show(&ui_weather_func, UI_SHOW_PEDDING);
        }

        if (page_air == obj) {
            lv_obj_set_style_local_text_color(label_aqi, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
            lv_obj_set_style_local_text_color(label_aqi_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);

            lv_obj_set_style_local_bg_color(bar_div, LV_BAR_PART_BG, LV_STATE_DEFAULT, COLOR_THEME);
            lv_obj_set_style_local_text_color(label_air, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);

            lv_img_set_src(img_pm2p5, data_icon_pm2p5);

            ui_show(&ui_air_func, UI_SHOW_PEDDING);
        }

        if (page_uv == obj) {
            lv_obj_set_style_local_text_color(label_uv, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
            lv_obj_set_style_local_text_color(label_uv_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);

            ui_show(&ui_uv_func, UI_SHOW_PEDDING);
        }

        if (page_humidity == obj) {
            lv_obj_set_style_local_text_color(label_humid, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
            lv_obj_set_style_local_text_color(label_humid_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);

            ui_show(&ui_humidity_func, UI_SHOW_PEDDING);
        }
    }
}

static void ui_clock_icon_update(void)
{
    lv_fs_file_t file;
    char file_path[64];
    void *old_icon_ptr = data_icon_weather;

    sprintf(file_path, "S:/Icon/%s.bin", weather_info.icon_code);

    if (LV_FS_RES_OK == lv_fs_open(&file, file_path, LV_FS_MODE_RD)) {
        lv_fs_close(&file);
    } else {
        strcpy(file_path, "S:/Icon/999.bin");
    }

    ui_laod_resource(file_path, &data_icon_weather);

    lv_img_set_src(img_weather, data_icon_weather);

    free(old_icon_ptr);
}

void ui_clock_update(void)
{
    static char temp_text[8];
    static char humid_text[8];

    app_weather_get_current_info(&weather_info);
    app_weather_get_air_info(&air_info);

    if (ui_state_dis == ui_clock_state) {
        return;
    }

    sprintf(temp_text, "%s°C", weather_info.temp);
    sprintf(humid_text, "%s%%", weather_info.humid);

    lv_label_set_text_static(label_temp, temp_text);
    lv_label_set_text_static(label_humid_val, humid_text);
    lv_label_set_text_static(label_uv_val, weather_info.uv_val);
    lv_label_set_text_static(label_weather, weather_info.describe);
    lv_label_set_text_static(label_aqi_val, air_info.aqi);
    lv_label_set_text_static(label_air, air_info.level);
    lv_obj_align(label_air, NULL, LV_ALIGN_CENTER, 80, 0);

    ui_clock_icon_update();
}

void ui_clock_update_date(void)
{
    const char *week_str[] = {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday",
    };

    const char *month_str[] = {
        "January",
        "February",
        "March",
        "April",
        "May",
        "June",
        "July",
        "August",
        "September",
        "October",
        "November",
        "December"
    };

    time_t time_now;
    struct tm *tm_now;
    char fmt_text[32];

    if (NULL == label_date) {
        return;
    }

    time(&time_now);
    tm_now = localtime(&time_now);
    sprintf(fmt_text, "%s, %s %d%s",
        week_str[tm_now->tm_wday],
        month_str[tm_now->tm_mon],
        tm_now->tm_mday,
        1 == tm_now->tm_mday ? "st" : 2 == tm_now->tm_mday ? "nd" : 3 == tm_now->tm_mday ? "rd" : "th");

    lv_label_set_text(label_date, fmt_text);
}
