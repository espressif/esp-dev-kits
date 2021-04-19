/**
 * @file ui_weather.c
 * @brief Weather UI page src.
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
#include "time.h"
#include "app_weather.h"

#define DAYS_TO_SHOW    5

static const char *TAG = "ui_weather";

/* UI function declaration */
ui_func_desc_t ui_weather_func = {
    .name = "Weather",
    .init = ui_weather_init,
    .show = ui_weather_show,
    .hide = ui_weather_hide,
};

static ui_state_t ui_weather_state = ui_state_dis;

/* LVGL objects defination */
static lv_obj_t *obj_page_weater = NULL;
static lv_obj_t *img_weather[DAYS_TO_SHOW] = {[0 ... (DAYS_TO_SHOW - 1)] = NULL};
static lv_obj_t *label_date[DAYS_TO_SHOW] = {[0 ... (DAYS_TO_SHOW - 1)] = NULL};
static lv_obj_t *label_temp[DAYS_TO_SHOW] = {[0 ... (DAYS_TO_SHOW - 1)] = NULL};
static lv_obj_t *label_weather[DAYS_TO_SHOW] = {[0 ... (DAYS_TO_SHOW - 1)] = NULL};
static lv_obj_t *chart_temp = NULL;
static lv_obj_t *bar_div[4] = { NULL, NULL, NULL, NULL };
static lv_obj_t *label_temp_val[9] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static lv_chart_series_t *temp_data_series = NULL;

static weather_daily_info_t weather_daily_info[5];
static weather_hourly_info_t weather_hourly_info[24];

/* Extern image resources data. Loaded in `ui_main.c` */
extern void *data_icon_weather;
extern void *weather_icon_data[];

void ui_weather_init(void *data)
{
    if (ui_state_dis == ui_weather_state) {
        for (size_t i = 0; i < sizeof(weather_daily_info) / sizeof(weather_daily_info[0]); i++) {
            app_weather_get_daily_info(i, &weather_daily_info[i]);
        }

        for (size_t i = 0; i < sizeof(weather_hourly_info) / sizeof(weather_hourly_info[0]); i++) {
            app_weather_get_hourly_info(i, &weather_hourly_info[i]);
        }
        ui_page_show("Weather");
        obj_page_weater = ui_page_get_obj();

        for (int i = 0; i < DAYS_TO_SHOW; i++) {
            img_weather[i] = lv_img_create(obj_page_weater, NULL);
            lv_img_set_src(img_weather[i], weather_icon_data[i]);
            lv_img_set_zoom(img_weather[i], 96);
        }

        /* Align weather icons */
        lv_obj_align(img_weather[DAYS_TO_SHOW / 2], NULL, LV_ALIGN_CENTER, 0, -50);
        lv_obj_align(img_weather[DAYS_TO_SHOW / 2 - 1], NULL, LV_ALIGN_CENTER, -150, -50);
        lv_obj_align(img_weather[DAYS_TO_SHOW / 2 + 1], NULL, LV_ALIGN_CENTER, 150, -50);
        lv_obj_align(img_weather[DAYS_TO_SHOW / 2 - 2], NULL, LV_ALIGN_CENTER, -300, -50);
        lv_obj_align(img_weather[DAYS_TO_SHOW / 2 + 2], NULL, LV_ALIGN_CENTER, 300, -50);

        /* Add div bar between icons */
        for (int i = 0; i < 4; i++) {
            bar_div[i] = lv_bar_create(obj_page_weater, NULL);
            lv_obj_set_size(bar_div[i], 2, 150);
            lv_obj_set_style_local_bg_color(bar_div[i], LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_make(198, 198, 198));
        }

        /* Make bars align */
        lv_obj_align(bar_div[0], NULL, LV_ALIGN_CENTER, -225, -40);
        lv_obj_align(bar_div[1], NULL, LV_ALIGN_CENTER, -75, -40);
        lv_obj_align(bar_div[2], NULL, LV_ALIGN_CENTER, 75, -40);
        lv_obj_align(bar_div[3], NULL, LV_ALIGN_CENTER, 225, -40);
        
        /* Set default value */
        for (int i = 0; i < DAYS_TO_SHOW; i++) {
            label_date[i] = lv_label_create(obj_page_weater, NULL);
            lv_obj_set_style_local_text_font(label_date[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_thin_20);
            lv_label_set_align(label_date[i], LV_LABEL_ALIGN_CENTER);
            lv_label_set_text(label_date[i], "12/6\nThursday");
            label_temp[i] = lv_label_create(obj_page_weater, NULL);
            lv_obj_set_style_local_text_font(label_temp[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_24);
            lv_label_set_text(label_temp[i], weather_daily_info[i].temp_range);
            label_weather[i] = lv_label_create(obj_page_weater, NULL);
            lv_obj_set_style_local_text_font(label_weather[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
            lv_label_set_text(label_weather[i], weather_daily_info[i].describe);

            lv_obj_align(label_date[i], img_weather[i], LV_ALIGN_IN_TOP_MID, 0, -5);
            lv_obj_align(label_temp[i], img_weather[i], LV_ALIGN_IN_BOTTOM_MID, 0, -5);
            lv_obj_align(label_weather[i], img_weather[i], LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
        }

        /* Add temp chart */
        chart_temp = lv_chart_create(obj_page_weater, NULL);
        lv_obj_set_size(chart_temp, 700, 130);
        lv_obj_align(chart_temp, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
        lv_chart_set_type(chart_temp, LV_CHART_TYPE_LINE);
        lv_chart_set_point_count(chart_temp, 9);
        lv_obj_set_style_local_border_width(chart_temp, LV_CHART_PART_BG, LV_STATE_DEFAULT, 0);

        /* Set default data */
        temp_data_series = lv_chart_add_series(chart_temp, COLOR_THEME);

        lv_obj_set_style_local_pad_left(chart_temp,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 25);
        lv_obj_set_style_local_pad_bottom(chart_temp,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 35);
        lv_obj_set_style_local_pad_right(chart_temp,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 25);
        lv_obj_set_style_local_pad_top(chart_temp,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 0);
        lv_obj_set_style_local_text_font(chart_temp, LV_CHART_PART_BG, LV_STATE_DEFAULT, &font_en_20);
        lv_obj_set_style_local_text_color(chart_temp, LV_CHART_PART_BG, LV_STATE_DEFAULT, COLOR_BAR);
        lv_chart_set_div_line_count(chart_temp, 3, 0);
        lv_chart_set_y_tick_length(chart_temp, 0, 0);
        lv_chart_set_x_tick_length(chart_temp, 0, 0);

        ui_status_bar_time_show(true);
        ui_weather_state = ui_state_show;
    }
}

void ui_weather_show(void *data)
{
    if (ui_state_dis == ui_weather_state) {
        ui_weather_init(data);

        weather_update_widget_caption();

        ui_status_bar_time_show(true);
    } else if (ui_state_hide == ui_weather_state) {
        for (int i = 0; i < DAYS_TO_SHOW; i++) {
            lv_obj_set_hidden(img_weather[i], false);
            lv_obj_set_hidden(label_date[i], false);
            lv_obj_set_hidden(label_temp[i], false);
            lv_obj_set_hidden(label_weather[i], false);
        }

        for (int i = 0; i < 4; i++) {
            lv_obj_set_hidden(bar_div[i], false);
        }

        for (int i = 0; i < 9; i++) {
            lv_obj_set_hidden(label_temp_val[i], false);
        }

        lv_obj_set_hidden(chart_temp, false);
        
        ui_page_show("Weather");

        weather_update_widget_caption();

        ui_status_bar_time_show(true);

        ui_weather_state = ui_state_show;
    }
}

void ui_weather_hide(void *data)
{
    if (ui_state_show == ui_weather_state) {
        for (int i = 0; i < DAYS_TO_SHOW; i++) {
            lv_obj_set_hidden(img_weather[i], true);
            lv_obj_set_hidden(label_date[i], true);
            lv_obj_set_hidden(label_temp[i], true);
            lv_obj_set_hidden(label_weather[i], true);
        }

        for (int i = 0; i < 4; i++) {
            lv_obj_set_hidden(bar_div[i], true);
        }

        for (int i = 0; i < 9; i++) {
            lv_obj_set_hidden(label_temp_val[i], true);
        }

        lv_obj_set_hidden(chart_temp, true);
        
        lv_obj_set_hidden(obj_page_weater, true);

        ui_weather_state = ui_state_hide;
    }
}
/***************** UI Weather Interface **********************/
static lv_coord_t min = 0;
static lv_coord_t max = 0;
static lv_coord_t hourly_temp_val[24] = {
    10, 11, 10, 10, 9, 8, 8, 8, 7, 7,
    6, 6, 6, 7, 7, 8, 9, 10, 11, 11,
    12, 12, 12, 11 };

static void weather_update_daily_date(void)
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

    /* Check widgets */
    for (int i = 0; i < sizeof(label_date) / sizeof(label_date[0]); i++) {
        if (NULL == label_date[i]) {
            return;
        }
    }

    time_t time_now;
    struct tm *tm_now;
    char fmt_text[20];

    /* Set "Today" */
    time(&time_now);
    tm_now = localtime(&time_now);
    sprintf(fmt_text, "Today\n%s", week_str[tm_now->tm_wday]);
    lv_label_set_text(label_date[0], fmt_text);

    /* Set next 4 days' date */
    for (int i = 1; i < 5; i++) {
        time_now += 86400;
        tm_now = localtime(&time_now);
        sprintf(fmt_text, "%d/%d\n%s", tm_now->tm_mon + 1, tm_now->tm_mday, week_str[tm_now->tm_wday]);
        lv_label_set_text(label_date[i], fmt_text);
    }

    /* Align object */
    for (int i = 0; i < DAYS_TO_SHOW; i++) {
        lv_obj_align(label_date[i], img_weather[i], LV_ALIGN_IN_TOP_MID, 0, 0);
    }
}

static void weather_update_daily_detail(void)
{
    /* Check widgets */
    for (int i = 0; i < 5; i++) {
        if ((NULL == label_temp[i]) || (NULL == label_weather[i])) {
            return;
        }
    }

    /* Update labels' text */
    for (int i = 0; i < 5; i++) {
        lv_label_set_text(label_temp[i], weather_daily_info[i].temp_range);
        lv_label_set_text(label_weather[i], weather_daily_info[i].describe);
    }

    /* Align objects */
    for (int i = 0; i < DAYS_TO_SHOW; i++) {
        lv_obj_align(label_temp[i], img_weather[i], LV_ALIGN_IN_BOTTOM_MID, 0, -10);
        lv_obj_align(label_weather[i], img_weather[i], LV_ALIGN_OUT_BOTTOM_MID, 0, -10);
    }
}

static void weather_update_daily_icon(void)
{
    lv_fs_file_t file;
    char file_path[32];
    void *old_icon_ptr = NULL;

    static char *prev_icon_code[5];

    for (int i = 0; i < 5; i++) {
        if (NULL == prev_icon_code[i]) {
            prev_icon_code[i] = heap_caps_malloc(8, MALLOC_CAP_SPIRAM);

            if (NULL == prev_icon_code[i]) {
                continue;
            } else {
                strcpy(prev_icon_code[i] , DRAM_STR("999"));
            }
        }

        if (0 != strcmp(weather_daily_info[i].icon_code, prev_icon_code[i])) {
            /* Get file name, use N/A if icon code not found */
            sprintf(file_path, "S:/Icon/%s.bin", weather_daily_info[i].icon_code);
            if (LV_FS_RES_OK == lv_fs_open(&file, file_path, LV_FS_MODE_RD)) {
                lv_fs_close(&file);
            } else {
                strcpy(file_path, "S:/Icon/999.bin");
            }

            old_icon_ptr = weather_icon_data[i];
            ui_laod_resource(file_path, &weather_icon_data[i]);

            if (ui_state_dis != ui_weather_state) {
                lv_img_set_src(img_weather[i], weather_icon_data[i]);
            }

            strcpy(prev_icon_code[i], weather_daily_info[i].icon_code);
            
            if (NULL != old_icon_ptr) {
                free(old_icon_ptr);
            }
        }
    }
}

static void weather_update_hourly_chart_axis_x(void)
{
    if (NULL == chart_temp) {
        return;
    }

    time_t time_now;
    struct tm *tm_now;
    char apd_text[8];
    static char dst_text[128];

    /* Set "Today" */
    time(&time_now);
    tm_now = localtime(&time_now);

    dst_text[0] = '\0';

    strcpy(dst_text, "Now");

    for (int i = 0; i < 8; i++) {
        time_now += 3 * 60 * 60;
        tm_now = localtime(&time_now);
        sprintf(apd_text, "\n%02d:00", tm_now->tm_hour);
        strcat(dst_text, apd_text);
    }

    lv_chart_set_x_tick_texts(chart_temp, dst_text, 0, LV_CHART_AXIS_DRAW_LAST_TICK);
    ESP_LOGD(TAG, "Dest text : [%s]", dst_text);
}

static void weather_update_hourly_chart_axis_y(void)
{

    min = hourly_temp_val[0];
    max = hourly_temp_val[0];

    for (int i = 1; i < 24; i++) {
        if (min > hourly_temp_val[i]) {
            min = hourly_temp_val[i];
        }

        if (max < hourly_temp_val[i]) {
            max = hourly_temp_val[i];
        }
    }

    min -= 1;

    switch ((max - min) % 3) {
    case 0:
        break;
    case 1:
        max += 2;
        break;
    case 2:
        max += 1;
        break;
    }

    lv_chart_set_y_range(chart_temp, LV_CHART_AXIS_PRIMARY_Y, min, max);

    ESP_LOGD(TAG, "Range : [%d - %d]", min, max);
    ESP_LOGD(TAG , "Points : [%d], [%d], [%d], [%d], [%d], [%d], [%d], [%d]",
        hourly_temp_val[0], hourly_temp_val[3], hourly_temp_val[6], hourly_temp_val[9],
        hourly_temp_val[12], hourly_temp_val[15], hourly_temp_val[18], hourly_temp_val[21]);
}

static void weather_update_hourly_chart_value(void)
{
    /* Remove previous series and add new series */
    lv_chart_remove_series(chart_temp, temp_data_series);
    temp_data_series = lv_chart_add_series(chart_temp, COLOR_THEME);

    for (int i = 0; i < 9; i++) {
        lv_chart_set_next(chart_temp, temp_data_series, hourly_temp_val[3 * i <= 23 ? 3 * i : 23]);
        ESP_LOGD(TAG, "Add data [%d] to series", hourly_temp_val[3 * i <= 23 ? 3 * i : 23]);
    }
}

static void weather_update_hourly_chart_label(void)
{
    static char fmt_text[16];
    static lv_coord_t value;
    static lv_coord_t ofs_x, ofs_y;

    for (int i = 0; i < 9; i++) {
        value = hourly_temp_val[3 * i <= 23 ? 3 * i : 23];

        sprintf(fmt_text, "%d°C", value);

        if (NULL == label_temp_val[i]) {
            label_temp_val[i] = lv_label_create(obj_page_weater, NULL);
            lv_label_set_text(label_temp_val[i], fmt_text);
            lv_obj_set_style_local_text_font(label_temp_val[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
        } else {
            lv_label_set_text(label_temp_val[i], fmt_text);
        }

        ofs_x = 81 * i + 15;
        ofs_y = -36 - 100.0f * (value - min) / (max - min);


        lv_obj_align(label_temp_val[i], chart_temp, LV_ALIGN_IN_BOTTOM_LEFT, ofs_x, ofs_y);
    }
}

static void weather_update_hourly_temp(void)
{
    bool neg = false;
    char *text = NULL;
    lv_coord_t value = 0;

    for (int i = 0; i < sizeof(weather_hourly_info) / sizeof(weather_hourly_info[0]); i++) {
        /* Skip if NULL */
        if (NULL == weather_hourly_info[i].temp) {
            continue;
        } else {
            text = weather_hourly_info[i].temp;
        }

        /* Reset flags */
        value = 0;
        neg = false;
        size_t dig = 0;

        /* Convert from text to int */
        do {
            if ('-' == text[dig]) {
                neg = true;
            } else if (('0' <= text[dig]) && ('9' >= text[dig])) {
                value *= 10;
                value += text[dig] - '0';
            } else {
                break;
            }
            dig++;
        } while (1);

        if (neg) {
            value = -value;
        }

        hourly_temp_val[i] = value;
    }
}

void weather_update_widget_caption(void)
{
    for (size_t i = 0; i < sizeof(weather_daily_info) / sizeof(weather_daily_info[0]); i++) {
        app_weather_get_daily_info(i, &weather_daily_info[i]);
    }

    for (size_t i = 0; i < sizeof(weather_hourly_info) / sizeof(weather_hourly_info[0]); i++) {
        app_weather_get_hourly_info(i, &weather_hourly_info[i]);
    }

    /* Hourly part update */
    weather_update_hourly_temp();

    /* Daily part update */
    weather_update_daily_date();
    weather_update_daily_detail();
    weather_update_daily_icon();

    if (ui_state_dis == ui_weather_state) {
        return;
    }

    /* Hourly part update */
    weather_update_hourly_chart_axis_x();
    weather_update_hourly_chart_axis_y();
    weather_update_hourly_chart_value();
    weather_update_hourly_chart_label();
}
