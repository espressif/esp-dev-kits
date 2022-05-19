/**
 * @file ui_setting.c
 * @brief Setting page UI src.
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

/* UI function declaration */
ui_func_desc_t ui_setting_func = {
    .name = "ui_setting",
    .init = ui_setting_init,
    .show = ui_setting_show,
    .hide = ui_setting_hide,
};

/* LVGL objects defination */
static lv_obj_t *img_app = NULL;
static lv_obj_t *img_about = NULL;
static lv_obj_t *obj_page_app = NULL;
static lv_obj_t *obj_page_about = NULL;

/* Extern image resources data. Loaded in `ui_main.c` */
extern void *data_icon_app;
extern void *data_icon_about;

/* Static function forward declaration */
static void btn_app_cb(lv_obj_t *obj, lv_event_t event);
static void btn_about_cb(lv_obj_t *obj, lv_event_t event);

void ui_setting_init(void *data)
{
    (void)data;

    obj_page_app = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(obj_page_app, 200, 200);
    lv_obj_set_style_local_bg_color(obj_page_app, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
    lv_obj_set_style_local_bg_color(obj_page_app, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_radius(obj_page_app, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 20);
    lv_obj_set_style_local_border_width(obj_page_app, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(obj_page_app, NULL, LV_ALIGN_CENTER, -120, 25);

    obj_page_about = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(obj_page_about, 200, 200);
    lv_obj_set_style_local_bg_color(obj_page_about, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
    lv_obj_set_style_local_bg_color(obj_page_about, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_radius(obj_page_about, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 20);
    lv_obj_set_style_local_border_width(obj_page_about, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(obj_page_about, NULL, LV_ALIGN_CENTER, 120, 25);

    img_app = lv_img_create(obj_page_app, NULL);
    lv_img_set_src(img_app, data_icon_app);
    lv_obj_align(img_app, obj_page_app, LV_ALIGN_CENTER, 0, -30);

    img_about = lv_img_create(obj_page_about, NULL);
    lv_img_set_src(img_about, data_icon_about);
    lv_obj_align(img_about, obj_page_about, LV_ALIGN_CENTER, 0, -30);

    lv_obj_set_style_local_value_str(obj_page_about,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "About");
    lv_obj_set_style_local_value_font(obj_page_about, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_28);
    lv_obj_set_style_local_value_align(obj_page_about,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_ofs_y(obj_page_about,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 50);
    lv_obj_set_style_local_value_color(obj_page_about,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_value_color(obj_page_about,  LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);

    lv_obj_set_style_local_value_str(obj_page_app,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "APP Connect");
    lv_obj_set_style_local_value_font(obj_page_app, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_28);
    lv_obj_set_style_local_value_align(obj_page_app,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_ofs_y(obj_page_app,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 50);
    lv_obj_set_style_local_value_color(obj_page_app,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_value_color(obj_page_app,  LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);
    
    lv_obj_set_event_cb(obj_page_app, btn_app_cb);
    lv_obj_set_event_cb(obj_page_about, btn_about_cb);
}

void ui_setting_show(void *data)
{
    if (NULL == obj_page_about) {
        ui_setting_init(data);
        ui_status_bar_time_show(true);
    } else {
        lv_obj_set_hidden(obj_page_about, false);
        lv_obj_set_hidden(obj_page_app, false);
        ui_status_bar_time_show(true);
    }
}

void ui_setting_hide(void *data)
{
    (void)data;

    if (NULL != obj_page_about) {
        lv_obj_set_hidden(obj_page_app, true);
        lv_obj_set_hidden(obj_page_about, true);
    }

}

/* ******************************** Event Handler(s) ******************************** */
static void btn_app_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        ui_show(&ui_app_func, UI_SHOW_PEDDING);
    }
}

static void btn_about_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        ui_show(&ui_about_func, UI_SHOW_PEDDING);
    }
}
