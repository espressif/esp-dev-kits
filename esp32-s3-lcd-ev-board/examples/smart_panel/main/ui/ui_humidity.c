/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ui_main.h"

/* UI function declaration */
ui_func_desc_t ui_humidity_func = {
    .name = "Humidity",
    .init = ui_humidity_init,
    .show = ui_humidity_show,
    .hide = ui_humidity_hide,
};

static ui_state_t ui_humidity_state = ui_state_dis;

static int humid_val = 65;

/* LVGL objects defination */
static lv_obj_t *obj_page_humidity = NULL;
static lv_obj_t *bar_humid_bg = NULL;
static lv_obj_t *bar_humid_val = NULL;
static lv_obj_t *label_humid_val = NULL;
static lv_obj_t *label_humid_level = NULL;
static lv_obj_t *chart_humid_val = NULL;
static lv_obj_t *mask_humid_val = NULL;
static lv_obj_t *bar_max = NULL;
static lv_obj_t *bar_min = NULL;
static lv_obj_t *bar_horizontal = NULL;
static lv_obj_t *bar_vertical = NULL;

static uint8_t humid_val_min[] = { 55, 57, 61, 58, 62 };
static uint8_t humid_val_max[] = { 64, 63, 66, 62, 67 };

static lv_chart_series_t *humid_data_min_series = NULL;
static lv_chart_series_t *humid_data_max_series = NULL;

/* Static function forward declaration */
static void anim_humid_task(void *data);

void ui_humidity_init(void *data)
{
    if (ui_state_dis == ui_humidity_state) {
        ui_page_show("Humidity");
        obj_page_humidity = ui_page_get_obj();

        bar_humid_bg = lv_bar_create(obj_page_humidity, NULL);
        lv_obj_set_size(bar_humid_bg, 230, 230);
        lv_obj_align(bar_humid_bg, NULL, LV_ALIGN_IN_LEFT_MID, 50, 35);
        lv_obj_set_style_local_bg_color(bar_humid_bg, LV_BAR_PART_BG, LV_STATE_DEFAULT, COLOR_BG);

        mask_humid_val = lv_objmask_create(bar_humid_bg, NULL);
        lv_obj_set_size(mask_humid_val, 230, 230);
        lv_obj_align(mask_humid_val, NULL, LV_ALIGN_CENTER, 0, 2 * (100 - 65));

        bar_humid_val = lv_bar_create(mask_humid_val, NULL);
        lv_obj_set_size(bar_humid_val, 200, 200);
        lv_obj_align(bar_humid_val, bar_humid_bg, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_local_bg_color(bar_humid_val, LV_BAR_PART_BG, LV_STATE_DEFAULT, COLOR_THEME);

        label_humid_val = lv_label_create(bar_humid_bg, NULL);
        lv_label_set_text(label_humid_val, "65%");
        lv_obj_set_style_local_text_font(label_humid_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_48);
        lv_obj_align(label_humid_val, bar_humid_bg, LV_ALIGN_CENTER, 0, -20);

        label_humid_level = lv_label_create(bar_humid_bg, NULL);
        lv_label_set_text(label_humid_level, "Humidity");
        lv_obj_set_style_local_text_font(label_humid_level, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_28);
        lv_obj_align(label_humid_level, label_humid_val, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

        chart_humid_val = lv_chart_create(obj_page_humidity, NULL);
        lv_obj_set_size(chart_humid_val, 440, 260);
        lv_obj_set_style_local_value_font(chart_humid_val, LV_CHART_PART_BG, LV_STATE_DEFAULT, &font_en_20);
        lv_obj_set_style_local_value_str(chart_humid_val, LV_CHART_PART_BG, LV_STATE_DEFAULT, "Maximum & Minimum Humidity");
        lv_obj_set_style_local_value_ofs_x(chart_humid_val, LV_CHART_PART_BG, LV_STATE_DEFAULT, -25);
        lv_obj_set_style_local_value_ofs_y(chart_humid_val, LV_CHART_PART_BG, LV_STATE_DEFAULT, -145);
        lv_chart_set_range(chart_humid_val, 54, 68);
        lv_chart_set_point_count(chart_humid_val, 6);

        lv_obj_set_style_local_border_width(chart_humid_val, LV_CHART_PART_BG, LV_STATE_DEFAULT, 0);
        lv_obj_set_style_local_pad_left(chart_humid_val,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 60);
        lv_obj_set_style_local_pad_bottom(chart_humid_val,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 36);
        lv_obj_set_style_local_pad_right(chart_humid_val,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 25);
        lv_obj_set_style_local_pad_top(chart_humid_val,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 20);
        lv_obj_set_style_local_text_font(chart_humid_val, LV_CHART_PART_BG, LV_STATE_DEFAULT, &font_en_20);
        lv_obj_set_style_local_text_color(chart_humid_val, LV_CHART_PART_BG, LV_STATE_DEFAULT, COLOR_BAR);

        lv_chart_set_div_line_count(chart_humid_val, 6, 0);
        lv_chart_set_x_tick_length(chart_humid_val, 0, 0);
        lv_chart_set_y_tick_length(chart_humid_val, 0, 0);
        lv_chart_set_x_tick_texts(chart_humid_val, " \n" "12/28\n" "12/29\n" "12/30\n" "12/31\n" "1/1", 0, LV_CHART_AXIS_DRAW_LAST_TICK);
        lv_chart_set_y_tick_texts(chart_humid_val, "68%\n" "66%\n" "64%\n" "62%\n" "60%\n" "58%\n" "56%\n", 0, LV_CHART_AXIS_DRAW_LAST_TICK);

        lv_obj_align(chart_humid_val, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 50);

        bar_max = lv_bar_create(obj_page_humidity, NULL);
        lv_obj_set_size(bar_max, 64, 4);
        lv_obj_set_style_local_bg_color(bar_max, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_make(239, 144, 60));

        bar_min = lv_bar_create(obj_page_humidity, NULL);
        lv_obj_set_size(bar_min, 30, 4);
        lv_obj_set_style_local_bg_color(bar_min, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_make(248, 198, 39));

        lv_obj_align(bar_max, chart_humid_val, LV_ALIGN_IN_TOP_RIGHT, -20, -20);
        lv_obj_align(bar_min, chart_humid_val, LV_ALIGN_IN_TOP_RIGHT, -20, -10);

        bar_horizontal = lv_bar_create(chart_humid_val, NULL);
        lv_obj_set_size(bar_horizontal, 2, 230);
        lv_obj_align(bar_horizontal, NULL, LV_ALIGN_IN_LEFT_MID, 60, -20);

        bar_vertical = lv_bar_create(chart_humid_val, NULL);
        lv_obj_set_size(bar_vertical, 360, 2);
        lv_obj_align(bar_vertical, NULL, LV_ALIGN_IN_BOTTOM_MID, 20, -35);

        ui_humidity_state = ui_state_show;
    }
}

void ui_humidity_show(void *data)
{
    if (ui_state_dis == ui_humidity_state) {
        ui_humidity_init(data);
        ui_status_bar_time_show(true);

        xTaskCreate(
            (TaskFunction_t)        anim_humid_task,
            (const char *const)    "Humid Animate Task",
            (const uint32_t)        3 * 1024,
            (void *const)          NULL,
            (UBaseType_t)           1,
            (TaskHandle_t *const)  NULL);
    } else if (ui_state_hide == ui_humidity_state) {
        lv_obj_set_hidden(bar_humid_bg, false);
        lv_obj_set_hidden(chart_humid_val, false);
        lv_obj_set_hidden(bar_max, false);
        lv_obj_set_hidden(bar_min, false);

        ui_page_show("Humidity");
        ui_status_bar_time_show(true);

        ui_humidity_state = ui_state_show;

        xTaskCreate(
            (TaskFunction_t)        anim_humid_task,
            (const char *const)    "Humid Animate Task",
            (const uint32_t)        3 * 1024,
            (void *const)          NULL,
            (UBaseType_t)           1,
            (TaskHandle_t *const)  NULL);
    }
}

void ui_humidity_hide(void *data)
{
    if (ui_state_show == ui_humidity_state) {
        lv_obj_set_hidden(bar_humid_bg, true);
        lv_obj_set_hidden(chart_humid_val, true);
        lv_obj_set_hidden(obj_page_humidity, true);
        lv_obj_set_hidden(bar_max, true);
        lv_obj_set_hidden(bar_min, true);

        lv_chart_clear_series(chart_humid_val, humid_data_min_series);
        lv_chart_clear_series(chart_humid_val, humid_data_max_series);

        ui_humidity_state = ui_state_hide;
    }
}

void ui_anim_set_humidity(const char *humidity)
{
    int humid = 0;
    if (strlen(humidity) <= 3) {
        for (int i = 0; i < strlen(humidity); i++) {
            humid *= 10;
            humid += humidity[i] - '0';
        }
        humid_val = humid;
    }
}

static void anim_humid_task(void *data)
{
    static TickType_t tick = 0;
    static char fmt_text[16];
    int i = 0;

    /* Reset State */
    bsp_display_lock(0);
    lv_label_set_text(label_humid_val, "0%");
    lv_obj_align(mask_humid_val, NULL, LV_ALIGN_CENTER, 0, 2 * (100 - 0));
    lv_obj_align(bar_humid_val, bar_humid_bg, LV_ALIGN_CENTER, 0, 0);
    humid_data_min_series = lv_chart_add_series(chart_humid_val, lv_color_make(248, 198, 39));
    humid_data_max_series = lv_chart_add_series(chart_humid_val, lv_color_make(239, 144, 60));
    bsp_display_unlock();

    tick = xTaskGetTickCount();

    while (1) {
        /* Delete task if went to other UI */
        if (ui_state_hide == ui_humidity_state) {
            vTaskDelete(NULL);
        }

        if (i <= humid_val) {
            sprintf(fmt_text, "%2d%%", i);

            bsp_display_lock(0);
            lv_label_set_text(label_humid_val, fmt_text);
            lv_obj_align(mask_humid_val, NULL, LV_ALIGN_CENTER, 0, 2 * (100 - i));
            lv_obj_align(bar_humid_val, bar_humid_bg, LV_ALIGN_CENTER, 0, 0);

            if ((i % 6 == 0) && (i / 6 <= 4)) {
                lv_chart_set_next(chart_humid_val, humid_data_min_series, humid_val_min[i / 5]);
                lv_chart_set_next(chart_humid_val, humid_data_max_series, humid_val_max[i / 5]);
            }

            bsp_display_unlock();

            i++;
            vTaskDelayUntil(&tick, pdMS_TO_TICKS(20));
        } else {
            vTaskDelete(NULL);
        }
    }
}
