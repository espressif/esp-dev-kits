/**
 * @file ui_about.c
 * @brief About page UI code.
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
#include "esp_ota_ops.h"

/* UI function declaration */
ui_func_desc_t ui_about_func = {
    .name = "About",
    .init = ui_about_init,
    .show = ui_about_show,
    .hide = ui_about_hide,
};

/* State of UI */
static ui_state_t ui_about_state = ui_state_dis;

/* LVGL objects defination */
static lv_obj_t *obj_page_about = NULL;
static lv_obj_t *tabel_about = NULL;
static lv_obj_t *tabel_mask = NULL;

void ui_about_init(void *data)
{
    static const esp_app_desc_t *app_desc = NULL;

    /* Show page with title */
    ui_page_show("About");
    obj_page_about = ui_page_get_obj();

    /* Create a mask to hide top and bottom line */
    tabel_mask = lv_objmask_create(obj_page_about, NULL);
    lv_obj_set_click(tabel_mask, false);
    lv_obj_set_size(tabel_mask, 700, 270);
    lv_obj_align(tabel_mask, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -20);

    /* Create tabel t show info about dev board and software version */
    tabel_about = lv_table_create(tabel_mask, NULL);
    lv_obj_set_click(tabel_about, false);
    lv_obj_set_style_local_border_color(tabel_about, LV_TABLE_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_line_color(tabel_about, LV_TABLE_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_pad_top(tabel_about,  LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, 18);
    lv_obj_set_style_local_pad_bottom(tabel_about,  LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, 18);

    /* Set font of text. CELL1 as tittle, CELL2 as content */
    lv_obj_set_style_local_text_font(tabel_about, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, &font_en_20);
    lv_obj_set_style_local_text_font(tabel_about, LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, &font_en_20);
    lv_obj_set_style_local_radius(tabel_about, LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, 20);
    lv_obj_set_style_local_radius(tabel_about, LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, 20);
    
    /* Set tabel rol / col count and size */
    lv_table_set_col_cnt(tabel_about, 2);
    lv_table_set_row_cnt(tabel_about, 5);
    lv_table_set_col_width(tabel_about, 0, 250);
    lv_table_set_col_width(tabel_about, 1, 400);

    /* Set tittle text */
    lv_table_set_cell_value(tabel_about, 0, 0, "App version");
    lv_table_set_cell_value(tabel_about, 1, 0, "IDF version");
    lv_table_set_cell_value(tabel_about, 2, 0, "IC version");
    lv_table_set_cell_value(tabel_about, 3, 0, "Sensor version");
    lv_table_set_cell_value(tabel_about, 4, 0, "STA MAC Addr");

    /* Get app and device information and show in table */
    app_desc = esp_ota_get_app_description();
    uint8_t mac_arr[6];
    char fmt_text[24];
    esp_read_mac(mac_arr, ESP_MAC_WIFI_STA);
    sprintf(fmt_text,
        "%02X:%02X:%02X:%02X:%02X:%02X",
        mac_arr[0], mac_arr[1], mac_arr[2], mac_arr[3], mac_arr[4], mac_arr[5]);
    lv_table_set_cell_value(tabel_about, 0, 1, app_desc->version);
    lv_table_set_cell_value(tabel_about, 1, 1, app_desc->idf_ver);
    lv_table_set_cell_value(tabel_about, 2, 1, "1.0");      //  IC version. Just defined in src
    lv_table_set_cell_value(tabel_about, 3, 1, "1.0");      //  Sensor version. Just defined in src
    lv_table_set_cell_value(tabel_about, 4, 1, fmt_text);
    for (uint16_t row = 0; row < 2; row++) {
        for (uint16_t col = 0; col < 5; col++) {
            lv_table_set_cell_crop(tabel_about, row, col, false);
        }
    }

    /* Place it at bottom of parent */
    lv_obj_align(tabel_about, NULL ,LV_ALIGN_IN_BOTTOM_MID, 0, 35);

    ui_about_state = ui_state_show;
}

void ui_about_show(void *data)
{
    if (ui_state_dis == ui_about_state) {
        ui_about_init(data);
    } else if (ui_state_hide == ui_about_state) {
        ui_page_show("About");
        lv_obj_set_hidden(tabel_mask, false);
    }
}

void ui_about_hide(void *data)
{
    if (ui_state_show == ui_about_state) {
        lv_obj_set_hidden(tabel_mask, true);
        lv_obj_set_hidden(obj_page_about, true);

        ui_about_state = ui_state_hide;
    }
}
