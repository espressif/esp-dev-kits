/**
 * @file ui_air.c
 * @brief AQI page src.
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
ui_func_desc_t ui_air_func = {
    .name = "Air Quality",
    .init = ui_air_init,
    .show = ui_air_show,
    .hide = ui_air_hide,
};

static air_info_t air_info;
static ui_state_t ui_air_state = ui_state_dis;

/* LVGL objects defination */
static lv_obj_t *obj_page_air = NULL;
static lv_obj_t *label_aqi = NULL;
static lv_obj_t *label_level = NULL;
static lv_obj_t *bar_level = NULL;
static lv_obj_t *label_level_detail = NULL;
static lv_obj_t *tabel_air = NULL;
static lv_obj_t *bar_table_top = NULL;
static lv_obj_t *bar_table_bot = NULL;
static lv_obj_t *obj_level[6] = { [0 ... 5] = NULL };

/* Color of objects in different AQI level */
static uint32_t obj_level_color[6] = {
    0xf9c629, // [249, 198,  41],
    0xf2ad47, // [242, 173,  71],
    0xeef1f5, // [238, 241, 245],
    0xc6c6c6, // [198, 198, 198],
    0x575e66, // [ 87,  94, 102],
    0x383838, // [ 56,  56,  56],
};

/* Font used in example is not a monospace font, spacing has been adjusted */
static char *obj_level_text[] = {
    "0-50           Excellent",
    "51-100       Good",
    "101-150     Lightly",
    "151-200     Moderately",
    "201-300     Heavily",
    "300-500     Hazardous",
};

/* Currently only shows specified text. You can change the text accroding to the AQI */
static char *obj_level_detail[] = {
    "The air quality is good, but some pollutants\n"
    "may bring health effects to very small numer\n"
    "of extremely sensitive people.",
};

void ui_air_init(void *data)
{
    ui_page_show("Air Quality Index");
    obj_page_air = ui_page_get_obj();

    tabel_air = lv_table_create(obj_page_air, NULL);
    lv_obj_set_click(tabel_air, false);
    lv_obj_set_style_local_border_color(tabel_air, LV_TABLE_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_line_color(tabel_air, LV_TABLE_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);

    /* Adjust line height */
    lv_obj_set_style_local_pad_top(tabel_air,  LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_pad_bottom(tabel_air,  LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, 5);
    lv_obj_set_style_local_pad_left(tabel_air,  LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_pad_right(tabel_air,  LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, 10);

    lv_obj_set_style_local_pad_top(tabel_air,  LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_pad_bottom(tabel_air,  LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_pad_left(tabel_air,  LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_pad_right(tabel_air,  LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, 10);

    lv_obj_set_style_local_text_font(tabel_air, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, &font_en_20);
    lv_obj_set_style_local_text_font(tabel_air, LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, &font_en_bold_28);
    lv_obj_set_style_local_margin_all(tabel_air, LV_TABLE_PART_BG, LV_STATE_DEFAULT, 0);

    /* Hide inner line */
    lv_obj_set_style_local_border_color(tabel_air, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_border_color(tabel_air, LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, LV_COLOR_WHITE);

    lv_table_set_col_cnt(tabel_air, 5);
    lv_table_set_row_cnt(tabel_air, 2);

    lv_table_set_col_width(tabel_air, 0, 90);
    lv_table_set_col_width(tabel_air, 1, 80);
    lv_table_set_col_width(tabel_air, 2, 80);
    lv_table_set_col_width(tabel_air, 3, 80);
    lv_table_set_col_width(tabel_air, 4, 60);

    for (int i = 0; i < 5; i++) {
        lv_table_set_cell_crop(tabel_air, 0, i, true);
        lv_table_set_cell_crop(tabel_air, 1, i, true);
        lv_table_set_cell_type(tabel_air, 0, i, 1);
        lv_table_set_cell_type(tabel_air, 1, i, 2);
    }

    lv_table_set_cell_align(tabel_air, 0, 0, LV_LABEL_ALIGN_CENTER);
    lv_table_set_cell_align(tabel_air, 0, 1, LV_LABEL_ALIGN_CENTER);
    lv_table_set_cell_align(tabel_air, 0, 2, LV_LABEL_ALIGN_CENTER);
    lv_table_set_cell_align(tabel_air, 0, 3, LV_LABEL_ALIGN_CENTER);
    lv_table_set_cell_align(tabel_air, 0, 4, LV_LABEL_ALIGN_CENTER);
    lv_table_set_cell_align(tabel_air, 1, 0, LV_LABEL_ALIGN_CENTER);
    lv_table_set_cell_align(tabel_air, 1, 1, LV_LABEL_ALIGN_CENTER);
    lv_table_set_cell_align(tabel_air, 1, 2, LV_LABEL_ALIGN_CENTER);
    lv_table_set_cell_align(tabel_air, 1, 3, LV_LABEL_ALIGN_CENTER);
    lv_table_set_cell_align(tabel_air, 1, 4, LV_LABEL_ALIGN_CENTER);

    /* Since the value will update later, there is no need to give a default value. */
    lv_table_set_cell_value(tabel_air, 0, 0, "PM 2.5");
    lv_table_set_cell_value(tabel_air, 0, 1, "PM 10");
    lv_table_set_cell_value(tabel_air, 0, 2, "NO2");
    lv_table_set_cell_value(tabel_air, 0, 3, "SO2");
    lv_table_set_cell_value(tabel_air, 0, 4, "CO");

    lv_obj_align(tabel_air, NULL, LV_ALIGN_IN_LEFT_MID, 10, 35);

    label_aqi = lv_label_create(obj_page_air, NULL);
    lv_label_set_text(label_aqi, "18");
    lv_obj_set_style_local_text_font(label_aqi, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_clock_108);
    /* Change to align style */
    lv_obj_set_pos(label_aqi, 100, 80);

    label_level = lv_label_create(obj_page_air, NULL);
    lv_label_set_text(label_level, "Good");
    lv_obj_set_style_local_text_font(label_level, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_28);
    lv_obj_align(label_level, label_aqi, LV_ALIGN_OUT_RIGHT_MID, 30, -5);

    bar_level = lv_bar_create(obj_page_air, NULL);
    lv_obj_set_height(bar_level, 5);
    lv_obj_set_width(bar_level, lv_obj_get_width(label_level));
    lv_obj_set_style_local_bg_color(bar_level, LV_BAR_PART_BG, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_align(bar_level, label_level, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    for (int i = 0; i < 6; i++) {
        obj_level[i] = lv_obj_create(obj_page_air, NULL);
        lv_obj_set_click(obj_level[i], false);
        lv_obj_set_size(obj_level[i], 265, 40);
        lv_obj_set_pos(obj_level[i], 460, 70 + 50 * i);
        lv_obj_set_style_local_bg_color(obj_level[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(obj_level_color[i]));
        lv_obj_set_style_local_value_color(obj_level[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, i < 4 ? COLOR_BAR : LV_COLOR_WHITE);
        lv_obj_set_style_local_value_str(obj_level[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, obj_level_text[i]);
        lv_obj_set_style_local_value_font(obj_level[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
        lv_obj_set_style_local_value_align(obj_level[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
        lv_obj_set_style_local_value_ofs_x(obj_level[i], LV_TABLE_PART_BG, LV_STATE_DEFAULT, 10);
        lv_obj_set_style_local_border_width(obj_level[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    }

    label_level_detail = lv_label_create(obj_page_air, NULL);
    lv_obj_set_style_local_text_font(label_level_detail, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_thin_20);
    lv_label_set_text_static(label_level_detail, obj_level_detail[0]);
    lv_obj_set_style_local_text_line_space(label_level_detail, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 5);
    lv_obj_set_style_local_text_letter_space(label_level_detail, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 1);
    lv_label_set_align(label_level_detail, LV_LABEL_ALIGN_LEFT);
    lv_obj_align(label_level_detail, tabel_air, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    ui_air_state = ui_state_show;

    bar_table_top = lv_bar_create(obj_page_air, NULL);
    lv_obj_set_size(bar_table_top, 410, 2);
    lv_obj_set_style_local_bg_color(bar_table_top, LV_BAR_PART_BG, LV_STATE_DEFAULT, COLOR_BG);
    lv_obj_align(bar_table_top, tabel_air, LV_ALIGN_IN_TOP_MID, 0, 20);

    bar_table_bot = lv_bar_create(obj_page_air, NULL);
    lv_obj_set_size(bar_table_bot, 410, 2);
    lv_obj_set_style_local_bg_color(bar_table_bot, LV_BAR_PART_BG, LV_STATE_DEFAULT, COLOR_BG);
    lv_obj_align(bar_table_bot, tabel_air, LV_ALIGN_IN_BOTTOM_MID, 0, -25);
}

void ui_air_show(void *data)
{
    if (ui_state_dis == ui_air_state) {
        ui_air_init(data);
        ui_air_update_value();
    } else if (ui_state_hide == ui_air_state) {
        lv_obj_set_hidden(label_aqi, false);
        lv_obj_set_hidden(tabel_air, false);
        lv_obj_set_hidden(label_level, false);
        lv_obj_set_hidden(bar_level, false);
        lv_obj_set_hidden(bar_table_top, false);
        lv_obj_set_hidden(bar_table_bot, false);
        lv_obj_set_hidden(label_level_detail, false);
        
        for (int i = 0; i < 6; i++) {
            lv_obj_set_hidden(obj_level[i], false);
        }

        ui_page_show("Air Quality Index");

        ui_air_state = ui_state_show;

        /* Make sure `ui_air_state` is updated, or mode check will fail */
        ui_air_update_value();
    }

    /* Show time on status bar */
    ui_status_bar_time_show(true);
}

void ui_air_hide(void *data)
{
    if (ui_state_show == ui_air_state) {
        lv_obj_set_hidden(label_aqi, true);
        lv_obj_set_hidden(tabel_air, true);
        lv_obj_set_hidden(label_level, true);
        lv_obj_set_hidden(bar_level, true);
        lv_obj_set_hidden(label_level_detail, true);
        lv_obj_set_hidden(bar_table_top, true);
        lv_obj_set_hidden(bar_table_bot, true);

        for (int i = 0; i < 6; i++) {
            lv_obj_set_hidden(obj_level[i], true);
        }

        lv_obj_set_hidden(obj_page_air, true);

        ui_air_state = ui_state_hide;
    }
}

static void ui_air_update_item_value(ui_air_item_t item)
{
    if (ui_state_show == ui_air_state) {    
        switch (item) {
        case air_item_pm2p5:
            lv_table_set_cell_value(tabel_air, 1, 0, air_info.pm2p5);
            break;
        case air_item_aqi:
            lv_label_set_text_static(label_aqi, air_info.aqi);
            break;
        case air_item_level:
            lv_label_set_text_static(label_level,  air_info.level);
            lv_obj_align(label_level, label_aqi, LV_ALIGN_OUT_RIGHT_MID, 30, -5);
            lv_obj_set_width(bar_level, lv_obj_get_width(label_level));
            lv_obj_align(bar_level, label_level, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
            break;
        case air_item_pm10:
            lv_table_set_cell_value(tabel_air, 1, 1,  air_info.pm10);
            break;
        case air_item_no2:
            lv_table_set_cell_value(tabel_air, 1, 2,  air_info.no2);
            break;
        case air_item_so2:
            lv_table_set_cell_value(tabel_air, 1, 3,  air_info.so2);
            break;
        case air_item_co:
            lv_table_set_cell_value(tabel_air, 1, 4,  air_info.co);
            break;
        }
    }
}

void ui_air_update_value(void)
{
    app_weather_get_air_info(&air_info);

    ui_air_update_item_value(air_item_aqi);
    ui_air_update_item_value(air_item_level);
    ui_air_update_item_value(air_item_co);
    ui_air_update_item_value(air_item_no2);
    ui_air_update_item_value(air_item_pm2p5);
    ui_air_update_item_value(air_item_pm10);
    ui_air_update_item_value(air_item_so2);
}
