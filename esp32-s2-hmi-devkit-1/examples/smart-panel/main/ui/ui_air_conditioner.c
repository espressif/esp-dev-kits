/**
 * @file ui_air_conditioner.c
 * @brief Air conditioner control page.
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
ui_func_desc_t ui_air_conditioner_func = {
    .name = "Aic Conditioner",
    .init = ui_air_conditioner_init,
    .show = ui_air_conditioner_show,
    .hide = ui_air_conditioner_hide,
};

/* Private enum type definition */
typedef enum {
    ac_mode_auto = 0,
    ac_mode_cool,
    ac_mode_heat,
    ac_mode_dry,
    ac_mode_fan,
} ac_mode_t;

typedef enum {
    ac_fan_speed_low = 0,
    ac_fan_speed_mid,
    ac_fan_speed_high,
} ac_fan_speed_t;

/* Private struct declaration */
/* Struct of air conditioner mode */
typedef struct {
    ac_mode_t       ac_mode;
    ac_fan_speed_t  fan_speed;
    bool            sleep_en;
    bool            on;
    int8_t          set_temp;
    int8_t          room_temp;
} ac_state_t;

/* Struct of imgbtn information. Save pointer of image when clicked and released */
typedef struct {
    void *img_default;
    void *img_active;
} imgbtn_img_info_t;

/* State of UI */
static ui_state_t ui_air_conditioner_state = ui_state_dis;

static ac_state_t ac_state = {
    .ac_mode = ac_mode_cool,
    .fan_speed = ac_fan_speed_mid,
    .sleep_en = false,
    .on = false,
    .set_temp = 18,
    .room_temp = 26,
};

/* LVGL objects defination */
static lv_obj_t *_btn_auto = NULL;
static lv_obj_t *_btn_cool = NULL;
static lv_obj_t *_btn_heat = NULL;
static lv_obj_t *_btn_dry = NULL;
static lv_obj_t *_btn_fan = NULL;
static lv_obj_t *_btn_sleep = NULL;
static lv_obj_t *_btn_pwr = NULL;
static lv_obj_t *_main_panel = NULL;
static lv_obj_t *_fan_panel = NULL;
static lv_obj_t *_time_panel = NULL;
static lv_obj_t *_obj_room_temp = NULL;
static lv_obj_t *_label_room_temp_val = NULL;
static lv_obj_t *_label_set_temp_val = NULL;
static lv_obj_t *_img_icon_auto = NULL;
static lv_obj_t *_img_icon_cool = NULL;
static lv_obj_t *_img_icon_heat = NULL;
static lv_obj_t *_img_icon_dry = NULL;
static lv_obj_t *_img_icon_fan = NULL;
static lv_obj_t *_label_auto = NULL;
static lv_obj_t *_label_cool = NULL;
static lv_obj_t *_label_heat = NULL;
static lv_obj_t *_label_dry = NULL;
static lv_obj_t *_label_fan = NULL;
static lv_obj_t *_label_sleep = NULL;
static lv_obj_t *_sw_sleep = NULL;
static lv_obj_t *_label_air_on_off = NULL;
static lv_obj_t *_label_fan_speed = NULL;
static lv_obj_t *_label_time_on_off = NULL;
static lv_obj_t *_bar_div_fan = NULL;
static lv_obj_t *_btn_temp_inc = NULL;
static lv_obj_t *_btn_temp_dec = NULL;
static lv_obj_t *_obj_fan_speed = NULL;
static lv_obj_t *_bar_fan_speed[3] = { NULL, NULL, NULL };
static lv_obj_t *_slider_fan_speed = NULL;
static lv_obj_t *_label_time_on[4] = { NULL, NULL, NULL, NULL };
static lv_obj_t *_label_time_on_div = NULL;
static lv_obj_t *_bar_time_on_off_div = NULL;
static lv_obj_t *_sw_time_on_off = NULL;
static lv_obj_t *_roller_hour = NULL;
static lv_obj_t *_roller_min = NULL;
static lv_obj_t *_label_hour = NULL;
static lv_obj_t *_label_min = NULL;
static lv_obj_t *_div_led[3] = { NULL, NULL, NULL };
static lv_obj_t *obj_air_conditioner = NULL;

/* Extern image resources data. Loaded in `ui_main.c` */
extern void *data_ico_auto;
extern void *data_ico_cool;
extern void *data_ico_heat;
extern void *data_ico_dry;
extern void *data_ico_fan;
extern void *data_ico_auto_down;
extern void *data_ico_cool_down;
extern void *data_ico_heat_down;
extern void *data_ico_dry_down;
extern void *data_ico_fan_down;

/* Static function forward declaration */
static void ui_ac_update(void);
static void btn_mode_click(lv_obj_t *obj, lv_event_t event);
static void btn_sleep_click(lv_obj_t *obj, lv_event_t event);
static void btn_temp_inc_dec_click(lv_obj_t *obj, lv_event_t event);
static void btn_pwr_click(lv_obj_t *obj, lv_event_t event);
static void slider_fan_speed_changed(lv_obj_t *obj, lv_event_t event);
static void sw_time_on_off_cb(lv_obj_t *obj, lv_event_t event);

void ui_air_conditioner_init(void *data)
{
    /* Show page with title */
    ui_page_show("Air Conditioner");
    obj_air_conditioner = ui_page_get_obj();

    /* Create a button with image. Use base object to implement */
    _btn_auto = lv_obj_create(obj_air_conditioner, NULL);
    lv_obj_set_size(_btn_auto, 85, 83);
    lv_obj_align(_btn_auto, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 70);
    lv_obj_set_style_local_border_width(_btn_auto, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_bg_color(_btn_auto, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);
    lv_obj_set_style_local_bg_color(_btn_auto, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    
    /* Create copy of buttons */
    _btn_cool = lv_obj_create(obj_air_conditioner, _btn_auto);
    _btn_heat = lv_obj_create(obj_air_conditioner, _btn_auto);
    _btn_dry = lv_obj_create(obj_air_conditioner, _btn_auto);
    _btn_fan = lv_obj_create(obj_air_conditioner, _btn_auto);
    _btn_sleep = lv_obj_create(obj_air_conditioner, _btn_auto);

    /* Align objects */
    lv_obj_align(_btn_cool, _btn_auto, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
    lv_obj_align(_btn_heat, NULL, LV_ALIGN_IN_LEFT_MID, 20, 25);
    lv_obj_align(_btn_dry, _btn_heat, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
    lv_obj_align(_btn_fan, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 20, -20);
    lv_obj_align(_btn_sleep, _btn_fan, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

    /* Add imgbtn to every object and create label */
    _img_icon_auto = lv_imgbtn_create(_btn_auto, NULL);
    lv_obj_set_click(_img_icon_auto, false);
    lv_imgbtn_set_src(_img_icon_auto, LV_BTN_STATE_RELEASED, data_ico_auto);
    lv_imgbtn_set_src(_img_icon_auto, LV_BTN_STATE_PRESSED, data_ico_auto_down);
    lv_obj_align(_img_icon_auto, NULL, LV_ALIGN_CENTER, 0, -10);
    
    _label_auto = lv_label_create(_btn_auto, NULL);
    lv_label_set_text(_label_auto, "AUTO");
    lv_obj_align(_label_auto, NULL, LV_ALIGN_CENTER, 0, 25);

    _img_icon_cool = lv_imgbtn_create(_btn_cool, NULL);
    lv_obj_set_click(_img_icon_cool, false);
    lv_imgbtn_set_src(_img_icon_cool, LV_BTN_STATE_RELEASED, data_ico_cool);
    lv_imgbtn_set_src(_img_icon_cool, LV_BTN_STATE_PRESSED, data_ico_cool_down);
    lv_obj_align(_img_icon_cool, NULL, LV_ALIGN_CENTER, 0, -10);

    _label_cool = lv_label_create(_btn_cool, NULL);
    lv_label_set_text(_label_cool, "COOL");
    lv_obj_align(_label_cool, NULL, LV_ALIGN_CENTER, 0, 25);

    _img_icon_heat = lv_imgbtn_create(_btn_heat, NULL);
    lv_obj_set_click(_img_icon_heat, false);
    lv_imgbtn_set_src(_img_icon_heat, LV_BTN_STATE_RELEASED, data_ico_heat);
    lv_imgbtn_set_src(_img_icon_heat, LV_BTN_STATE_PRESSED, data_ico_heat_down);
    lv_obj_align(_img_icon_heat, NULL, LV_ALIGN_CENTER, 0, -10);

    _label_heat = lv_label_create(_btn_heat, NULL);
    lv_label_set_text(_label_heat, "HEAT");
    lv_obj_align(_label_heat, NULL, LV_ALIGN_CENTER, 0, 25);

    _img_icon_dry = lv_imgbtn_create(_btn_dry, NULL);
    lv_obj_set_click(_img_icon_dry, false);
    lv_imgbtn_set_src(_img_icon_dry, LV_BTN_STATE_RELEASED, data_ico_dry);
    lv_imgbtn_set_src(_img_icon_dry, LV_BTN_STATE_PRESSED, data_ico_dry_down);
    lv_obj_align(_img_icon_dry, NULL, LV_ALIGN_CENTER, 0, -10);

    _label_dry = lv_label_create(_btn_dry, NULL);
    lv_label_set_text(_label_dry, "DRY");
    lv_obj_align(_label_dry, NULL, LV_ALIGN_CENTER, 0, 25);

    _img_icon_fan = lv_imgbtn_create(_btn_fan, NULL);
    lv_obj_set_click(_img_icon_fan, false);
    lv_imgbtn_set_src(_img_icon_fan, LV_BTN_STATE_RELEASED, data_ico_fan);
    lv_imgbtn_set_src(_img_icon_fan, LV_BTN_STATE_PRESSED, data_ico_fan_down);
    lv_obj_align(_img_icon_fan, NULL, LV_ALIGN_CENTER, 0, -10);

    _label_fan = lv_label_create(_btn_fan, NULL);
    lv_label_set_text(_label_fan, "FAN");
    lv_obj_align(_label_fan, NULL, LV_ALIGN_CENTER, 0, 25);

    _label_sleep = lv_label_create(_btn_sleep, NULL);
    lv_label_set_text(_label_sleep, "SLEEP");
    lv_obj_align(_label_sleep, NULL, LV_ALIGN_CENTER, 0, 25);

    /* Set default icon to objects' user data */
    lv_obj_set_user_data(_btn_auto, _img_icon_auto);
    lv_obj_set_user_data(_btn_cool, _img_icon_cool);
    lv_obj_set_user_data(_btn_heat, _img_icon_heat);
    lv_obj_set_user_data(_btn_dry, _img_icon_dry);
    lv_obj_set_user_data(_btn_fan, _img_icon_fan);

    /* Set event callback function */
    lv_obj_set_event_cb(_btn_auto, btn_mode_click);
    lv_obj_set_event_cb(_btn_cool, btn_mode_click);
    lv_obj_set_event_cb(_btn_heat, btn_mode_click);
    lv_obj_set_event_cb(_btn_dry, btn_mode_click);
    lv_obj_set_event_cb(_btn_fan, btn_mode_click);
    lv_obj_set_event_cb(_btn_sleep, btn_sleep_click);

    /* Set font of text */
    lv_obj_set_style_local_text_font(_label_auto, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_18);
    lv_obj_set_style_local_text_font(_label_cool, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_18);
    lv_obj_set_style_local_text_font(_label_heat, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_18);
    lv_obj_set_style_local_text_font(_label_dry, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_18);
    lv_obj_set_style_local_text_font(_label_fan, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_18);
    lv_obj_set_style_local_text_font(_label_sleep, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_18);

    /* Add switch to object */
    _sw_sleep = lv_switch_create(_btn_sleep, NULL);
    lv_obj_set_click(_sw_sleep, false);
    lv_obj_set_style_local_bg_color(_sw_sleep, LV_SWITCH_PART_INDIC, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_align(_sw_sleep, NULL, LV_ALIGN_CENTER, 0, -10);

    /* Create a main panel as switch and show temp */
    _main_panel = lv_obj_create(obj_air_conditioner, NULL);
    lv_obj_set_click(_main_panel, false);
    lv_obj_set_size(_main_panel, 260, 290);
    lv_obj_align(_main_panel, NULL, LV_ALIGN_IN_BOTTOM_MID, -15, -20);
    lv_obj_set_style_local_border_color(_main_panel, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_border_color(_main_panel, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, COLOR_THEME);

    /* Fan control panel */
    _fan_panel = lv_obj_create(obj_air_conditioner, NULL);
    lv_obj_set_size(_fan_panel, 220, 135);
    lv_obj_align(_fan_panel, NULL, LV_ALIGN_IN_TOP_RIGHT, -20, 70); 

    /* Timing panel */
    _time_panel = lv_obj_create(obj_air_conditioner, _fan_panel);
    lv_obj_set_click(_time_panel, false);
    lv_obj_align(_time_panel, _fan_panel, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    /* A label with diffrent color to show ON / OFF */
    _label_air_on_off = lv_label_create(_main_panel, NULL);
    lv_label_set_recolor(_label_air_on_off, true);
    lv_obj_set_style_local_text_font(_label_air_on_off, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_label_set_text(_label_air_on_off, "ON / #f6ae3d OFF#");
    lv_obj_align(_label_air_on_off, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);

    /* Object to show temperature */
    _obj_room_temp = lv_obj_create(_main_panel, NULL);
    lv_obj_set_size(_obj_room_temp, 260, 155);
    lv_obj_set_focus_parent(_obj_room_temp, true);
    lv_obj_align(_obj_room_temp, NULL, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_style_local_border_width(_obj_room_temp, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_bg_color(_obj_room_temp, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);

    /* Label of temperature */
    _label_room_temp_val = lv_label_create(_obj_room_temp, NULL);
    lv_obj_set_style_local_text_font(_label_room_temp_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_72);
    lv_label_set_text(_label_room_temp_val, "26°C");
    lv_obj_align(_label_room_temp_val, NULL, LV_ALIGN_CENTER, 0, 0);

    /* Power button, has external click area */         
    _btn_pwr = lv_btn_create(_main_panel, NULL);
    lv_obj_set_size(_btn_pwr, 40, 40);
    lv_obj_set_ext_click_area(_btn_pwr, 200, 100, 10, 160);
    lv_obj_set_style_local_radius(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
    lv_obj_set_style_local_value_str(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_POWER);
    lv_obj_set_style_local_bg_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_bg_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_value_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_value_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_obj_set_style_local_border_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_border_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_obj_set_style_local_border_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, LV_COLOR_WHITE);
    lv_obj_set_event_cb(_btn_pwr, btn_pwr_click);
    lv_obj_set_focus_parent(_btn_pwr, true);
    lv_obj_align(_btn_pwr, NULL, LV_ALIGN_IN_TOP_RIGHT, -10, 10);

    /* Label of expected temperature */
    _label_set_temp_val = lv_label_create(_main_panel, NULL);
    lv_obj_set_style_local_text_font(_label_set_temp_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_28);
    lv_label_set_text(_label_set_temp_val, "18°C");
    lv_obj_align(_label_set_temp_val, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -20);

    /* Buttons to increase or decrease temperature */
    _btn_temp_inc = lv_btn_create(_main_panel, NULL);
    lv_obj_set_size(_btn_temp_inc, 50, 50);
    lv_obj_set_ext_click_area(_btn_temp_inc, 50, 10, 10, 10);
    lv_obj_set_style_local_radius(_btn_temp_inc, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 5);
    lv_obj_set_style_local_border_color(_btn_temp_inc, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_border_color(_btn_temp_inc, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_border_color(_btn_temp_inc, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, COLOR_THEME);
    lv_obj_set_style_local_outline_color(_btn_temp_inc, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_outline_color(_btn_temp_inc, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_outline_color(_btn_temp_inc, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, COLOR_THEME);
    lv_obj_set_style_local_bg_color(_btn_temp_inc, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(_btn_temp_inc, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_bg_color(_btn_temp_inc, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, COLOR_THEME);
    lv_obj_set_style_local_value_color(_btn_temp_inc, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_value_color(_btn_temp_inc, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_obj_set_style_local_value_color(_btn_temp_inc, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, LV_COLOR_WHITE);
    lv_obj_set_style_local_value_font(_btn_temp_inc, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &font_en_32);
    lv_obj_set_style_local_value_align(_btn_temp_inc, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_str(_btn_temp_inc, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "+");
    lv_obj_set_style_local_value_ofs_y(_btn_temp_inc, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(_btn_temp_inc, _label_set_temp_val, LV_ALIGN_OUT_RIGHT_MID, 30, 0);
    lv_obj_set_event_cb(_btn_temp_inc, btn_temp_inc_dec_click);

    _btn_temp_dec = lv_btn_create(_main_panel, NULL);
    lv_obj_set_size(_btn_temp_dec, 50, 50);
    lv_obj_set_ext_click_area(_btn_temp_dec, 10, 50, 10, 10);
    lv_obj_set_style_local_radius(_btn_temp_dec, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 5);
    lv_obj_set_style_local_value_font(_btn_temp_dec, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &font_en_32);
    lv_obj_set_style_local_value_str(_btn_temp_dec, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "-");
    lv_obj_set_style_local_border_color(_btn_temp_dec, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_border_color(_btn_temp_dec, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_border_color(_btn_temp_dec, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, COLOR_THEME);
    lv_obj_set_style_local_outline_color(_btn_temp_dec, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_outline_color(_btn_temp_dec, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_outline_color(_btn_temp_dec, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, COLOR_THEME);
    lv_obj_set_style_local_bg_color(_btn_temp_dec, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(_btn_temp_dec, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_bg_color(_btn_temp_dec, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, COLOR_THEME);
    lv_obj_set_style_local_value_color(_btn_temp_dec, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_value_color(_btn_temp_dec, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_obj_set_style_local_value_color(_btn_temp_dec, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, LV_COLOR_WHITE);
    lv_obj_set_style_local_value_align(_btn_temp_dec, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_ofs_y(_btn_temp_dec, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(_btn_temp_dec, _label_set_temp_val, LV_ALIGN_OUT_LEFT_MID, -30, 0);
    lv_obj_set_event_cb(_btn_temp_dec, btn_temp_inc_dec_click);

    /* Object mask to show only 2 radius of object */
    static lv_obj_t *_mask = NULL;
    _mask = lv_obj_create(_fan_panel, NULL);
    lv_obj_set_size(_mask, 230, 20);
    lv_obj_set_style_local_bg_color(_mask, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_radius(_mask, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_width(_mask, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(_mask, NULL, LV_ALIGN_IN_TOP_MID, 0, 25);

    /* Fan speed indicator */
    _obj_fan_speed = lv_obj_create(_fan_panel, NULL);
    lv_obj_set_size(_obj_fan_speed, 230, 45);
    lv_obj_set_click(_obj_fan_speed, false);
    lv_obj_set_style_local_bg_color(_obj_fan_speed, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_radius(_obj_fan_speed, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 20);
    lv_obj_set_style_local_border_width(_obj_fan_speed, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(_obj_fan_speed, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
    lv_obj_set_user_data(_obj_fan_speed, _mask);

    /* Label of fan speed */
    _label_fan_speed = lv_label_create(_obj_fan_speed, NULL);
    lv_obj_set_style_local_text_color(_label_fan_speed, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_text_font(_label_fan_speed, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    lv_label_set_text(_label_fan_speed, "Fan Speed");
    lv_obj_align(_label_fan_speed, NULL, LV_ALIGN_IN_LEFT_MID, 20, 0);

    /* Create 3 bar to indicate fan level */
    for (int i = 0; i < 3; i++) {
        _bar_fan_speed[i] = lv_bar_create(_obj_fan_speed, NULL);
        lv_obj_set_style_local_border_width(_bar_fan_speed[i], LV_BAR_PART_INDIC, LV_STATE_DEFAULT, 0);
        lv_obj_set_style_local_border_width(_bar_fan_speed[i], LV_BAR_PART_BG, LV_STATE_DEFAULT, 0);
        lv_obj_set_style_local_bg_color(_bar_fan_speed[i], LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_set_style_local_bg_color(_bar_fan_speed[i], LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_set_size(_bar_fan_speed[i], 20, 4);
        lv_bar_set_range(_bar_fan_speed[i], 0, 10);
    }

    lv_obj_align(_bar_fan_speed[2], NULL, LV_ALIGN_IN_RIGHT_MID, -20, 0);
    lv_obj_align(_bar_fan_speed[1], _bar_fan_speed[2], LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_obj_align(_bar_fan_speed[0], _bar_fan_speed[1], LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_bar_set_value(_bar_fan_speed[0], 10, LV_ANIM_ON);
    lv_bar_set_value(_bar_fan_speed[1], 10, LV_ANIM_ON);

    /* Slider of fan speed */
    _slider_fan_speed = lv_slider_create(_fan_panel, NULL);
    lv_obj_set_ext_click_area(_slider_fan_speed, 0, 0, 20, 20);
    lv_bar_set_range(_slider_fan_speed, 0, 2);
    lv_obj_set_size(_slider_fan_speed, 160, 8);
    lv_obj_set_style_local_bg_color(_slider_fan_speed, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, COLOR_BG);
    lv_obj_set_style_local_bg_color(_slider_fan_speed, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, COLOR_BG);
    lv_obj_set_style_local_bg_color(_slider_fan_speed, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_COLOR_MAKE(198, 198, 198));
    lv_obj_set_style_local_border_width(_slider_fan_speed, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_outline_width(_slider_fan_speed, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_align(_slider_fan_speed, NULL, LV_ALIGN_CENTER, 0, 30);
    lv_slider_set_value(_slider_fan_speed, 1, LV_ANIM_ON);
    lv_obj_set_event_cb(_slider_fan_speed, slider_fan_speed_changed);

    for (int i = 0; i < 3; i++) {
        _div_led[i] = lv_obj_create(_fan_panel, NULL);
        lv_obj_set_size(_div_led[i], 10, 10);
        lv_obj_set_click(_div_led[i], false);
        lv_obj_set_style_local_bg_color(_div_led[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_set_style_local_border_width(_div_led[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
        
    }

    lv_obj_set_style_local_bg_color(_div_led[0], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_style_local_bg_color(_div_led[1], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_style_local_bg_color(_div_led[2], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_align(_div_led[0], _slider_fan_speed, LV_ALIGN_IN_LEFT_MID, -5, 0);
    lv_obj_align(_div_led[1], _slider_fan_speed, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(_div_led[2], _slider_fan_speed, LV_ALIGN_IN_RIGHT_MID, 5, 0);
    lv_obj_set_style_local_value_str(_slider_fan_speed, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, "Low        Mid        High");
    lv_obj_set_style_local_value_font(_slider_fan_speed, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, &font_en_18);
    lv_obj_set_style_local_value_align(_slider_fan_speed, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_IN_TOP_MID);
    lv_obj_set_style_local_value_ofs_y(_slider_fan_speed, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, -30);

    /* A divided line in object */
    _bar_div_fan = lv_bar_create(_fan_panel, NULL);
    lv_obj_set_size(_bar_div_fan, 220, 2);
    lv_obj_align(_bar_div_fan, NULL, LV_ALIGN_IN_TOP_MID, 0, 40);

    /* Objects of timing panel */
    _label_time_on_off = lv_label_create(_time_panel, NULL);
    lv_label_set_text(_label_time_on_off, "TIME ON");
    lv_obj_set_style_local_text_font(_label_time_on_off, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    lv_obj_align(_label_time_on_off, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);

    _bar_time_on_off_div = lv_bar_create(_time_panel, NULL);
    lv_obj_set_size(_bar_time_on_off_div, 220, 2);
    lv_obj_align(_bar_time_on_off_div, NULL, LV_ALIGN_IN_TOP_MID, 0, 40);

    /* Switch of timing page */
    _sw_time_on_off = lv_switch_create(_time_panel, NULL);
    lv_obj_set_height(_sw_time_on_off, 25);
    lv_obj_set_ext_click_area(_sw_time_on_off, 150, 10, 10, 10);
    lv_obj_set_style_local_bg_color(_sw_time_on_off, LV_SWITCH_PART_INDIC, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_outline_color(_sw_time_on_off, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_align(_sw_time_on_off, NULL, LV_ALIGN_IN_TOP_RIGHT, -10, 10);
    lv_switch_off(_sw_time_on_off, LV_ANIM_ON);
    lv_obj_set_event_cb(_sw_time_on_off, sw_time_on_off_cb);

    for (int i = 0; i < 4; i++) {
        _label_time_on[i] = lv_obj_create(_time_panel, NULL);
        lv_obj_set_size(_label_time_on[i], 35, 45);
        lv_obj_set_click(_label_time_on[i], false);
        lv_obj_set_style_local_radius(_label_time_on[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
        lv_obj_set_style_local_border_width(_label_time_on[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
        lv_obj_set_style_local_bg_color(_label_time_on[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);
        lv_obj_set_style_local_value_font(_label_time_on[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &font_en_40);
        lv_obj_set_style_local_value_align(_label_time_on[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    }

    lv_obj_align(_label_time_on[0], NULL, LV_ALIGN_CENTER, -70, 20);
    lv_obj_align(_label_time_on[1], NULL, LV_ALIGN_CENTER, -30, 20);
    lv_obj_align(_label_time_on[2], NULL, LV_ALIGN_CENTER, 30, 20);
    lv_obj_align(_label_time_on[3], NULL, LV_ALIGN_CENTER, 70, 20);

    /* Label to show `:` between HH and MM, like `HH:MM` */
    _label_time_on_div = lv_label_create(_time_panel, NULL);
    lv_obj_set_style_local_text_font(_label_time_on_div, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_40);
    lv_label_set_text(_label_time_on_div, ":");
    lv_obj_align(_label_time_on_div, NULL, LV_ALIGN_CENTER, 0, 15);

    const char *text_hour = {
        "0   \n"
        "1   \n"
        "2   \n"
        "3   \n"
        "4   \n"
        "5   \n"
        "6   \n"
        "7   \n"
        "8   \n"
        "9   \n"
        "10   \n"
        "11   \n"
        "12   \n"
        "13   \n"
        "14   \n"
        "15   \n"
        "16   \n"
        "17   \n"
        "18   \n"
        "19   \n"
        "20   \n"
        "21   \n"
        "22   \n"
        "23   "                
    };
    _roller_hour = lv_roller_create(_time_panel, NULL);
    lv_roller_set_options(_roller_hour, text_hour, LV_ROLLER_MODE_INFINITE);
    const char *text_min = {
        "0   \n"
        "1   \n"
        "2   \n"
        "3   \n"
        "4   \n"
        "5   \n"
        "6   \n"
        "7   \n"
        "8   \n"
        "9   \n"
        "10   \n"
        "11   \n"
        "12   \n"
        "13   \n"
        "14   \n"
        "15   \n"
        "16   \n"
        "17   \n"
        "18   \n"
        "19   \n"
        "20   \n"
        "21   \n"
        "22   \n"
        "23   \n"
        "24   \n"
        "25   \n"
        "26   \n"
        "27   \n"
        "28   \n"
        "29   \n"
        "30   \n"
        "31   \n"
        "32   \n"
        "33   \n"
        "34   \n"
        "35   \n"
        "36   \n"
        "37   \n"
        "38   \n"
        "39   \n"
        "40   \n"
        "41   \n"
        "42   \n"
        "43   \n"
        "44   \n"
        "45   \n"
        "46   \n"
        "47   \n"
        "48   \n"
        "49   \n"
        "50   \n"
        "51   \n"
        "52   \n"
        "53   \n"
        "54   \n"
        "55   \n"
        "56   \n"
        "57   \n"
        "58   \n"
        "59   "
    };
    _roller_min = lv_roller_create(_time_panel, NULL);
    lv_roller_set_options(_roller_min, text_min, LV_ROLLER_MODE_INFINITE);

    lv_roller_set_fix_width(_roller_hour, 110);
    lv_roller_set_fix_width(_roller_min, 110);

    lv_obj_set_style_local_bg_color(_roller_hour, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(_roller_hour, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, COLOR_BG);
    lv_obj_set_style_local_text_color(_roller_hour, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_style_local_text_color(_roller_hour, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_border_width(_roller_hour, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, 0);

    lv_obj_set_style_local_bg_color(_roller_min, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(_roller_min, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, COLOR_BG);
    lv_obj_set_style_local_text_color(_roller_min, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_style_local_text_color(_roller_min, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_border_width(_roller_min, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, 0);

    lv_obj_set_style_local_text_font(_roller_hour, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, &font_en_bold_20);
    lv_obj_set_style_local_text_font(_roller_hour, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, &font_en_bold_20);
    lv_obj_set_style_local_text_font(_roller_min, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, &font_en_bold_20);
    lv_obj_set_style_local_text_font(_roller_min, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, &font_en_bold_20);

    lv_obj_set_style_local_text_line_space(_roller_hour, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, 8);
    lv_obj_set_style_local_text_line_space(_roller_hour, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, 8);
    lv_obj_set_style_local_text_line_space(_roller_min, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, 8);
    lv_obj_set_style_local_text_line_space(_roller_min, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, 8);

    lv_roller_set_visible_row_count(_roller_hour, 3);
    lv_roller_set_visible_row_count(_roller_min, 3);

    lv_obj_set_style_local_bg_opa(_roller_hour, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_bg_opa(_roller_min, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    _label_hour = lv_label_create(_time_panel, NULL);
    lv_label_set_text(_label_hour, "hour");
    lv_obj_set_style_local_text_font(_label_hour, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_18);

    _label_min = lv_label_create(_time_panel, NULL);
    lv_label_set_text(_label_min, "min");
    lv_obj_set_style_local_text_font(_label_min, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_18);

    lv_obj_align(_roller_hour, NULL, LV_ALIGN_IN_LEFT_MID, 0, 20);
    lv_obj_align(_roller_min, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 20);
    lv_obj_align(_label_hour, _roller_hour, LV_ALIGN_IN_RIGHT_MID, -5, 0);
    lv_obj_align(_label_min, _roller_min, LV_ALIGN_IN_RIGHT_MID, -15, 0);

    ui_air_conditioner_state = ui_state_show;
}

void ui_air_conditioner_show(void *data)
{
    if (ui_state_dis == ui_air_conditioner_state) {
        ui_air_conditioner_init(data);
    } else if (ui_state_hide == ui_air_conditioner_state) {
        lv_obj_set_hidden(_btn_auto, false);
        lv_obj_set_hidden(_btn_cool, false);
        lv_obj_set_hidden(_btn_heat, false);
        lv_obj_set_hidden(_btn_dry, false);
        lv_obj_set_hidden(_btn_fan, false);
        lv_obj_set_hidden(_btn_sleep, false);
        lv_obj_set_hidden(_main_panel, false);
        lv_obj_set_hidden(_fan_panel, false);
        lv_obj_set_hidden(_time_panel, false);

        ui_page_show("Air Conditioner");
        
        ui_air_conditioner_state = ui_state_show;
    }

    /* Update air conditioner UI */
    ui_ac_update();
}

void ui_air_conditioner_hide(void *data)
{
    if (ui_state_show == ui_air_conditioner_state) {
        lv_obj_set_hidden(_btn_auto, true);
        lv_obj_set_hidden(_btn_cool, true);
        lv_obj_set_hidden(_btn_heat, true);
        lv_obj_set_hidden(_btn_dry, true);
        lv_obj_set_hidden(_btn_fan, true);
        lv_obj_set_hidden(_btn_sleep, true);
        lv_obj_set_hidden(_main_panel, true);
        lv_obj_set_hidden(_fan_panel, true);
        lv_obj_set_hidden(_time_panel, true);
        lv_obj_set_hidden(obj_air_conditioner, true);
        
        ui_air_conditioner_state = ui_state_hide;
    }
}

static void ui_obj_set_highlight(lv_obj_t *obj, bool highlighted)
{
    if (NULL != obj) {
        if (highlighted) {
            lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
        } else {
            lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);
        }
    }
}

static void ui_ac_update_mode(void)
{
    if (ac_state.on) {
        lv_obj_set_style_local_bg_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
        lv_obj_set_style_local_bg_color(_slider_fan_speed, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, lv_color_make(252, 199, 0));
        lv_obj_set_style_local_bg_color(_slider_fan_speed, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, lv_color_make(252, 199, 0));
        switch (ac_state.ac_mode) {
        case ac_mode_auto:
            lv_imgbtn_set_src(_img_icon_auto, LV_BTN_STATE_RELEASED, data_ico_auto_down);
            lv_imgbtn_set_src(_img_icon_cool, LV_BTN_STATE_RELEASED, data_ico_cool);
            lv_imgbtn_set_src(_img_icon_heat, LV_BTN_STATE_RELEASED, data_ico_heat);
            lv_imgbtn_set_src(_img_icon_dry, LV_BTN_STATE_RELEASED, data_ico_dry);
            lv_imgbtn_set_src(_img_icon_fan, LV_BTN_STATE_RELEASED, data_ico_fan);
            
            ui_obj_set_highlight(_btn_auto, true);
            ui_obj_set_highlight(_btn_cool, false);
            ui_obj_set_highlight(_btn_heat, false);
            ui_obj_set_highlight(_btn_dry, false);
            ui_obj_set_highlight(_btn_fan, false);
            break;
        case ac_mode_cool:
            lv_imgbtn_set_src(_img_icon_auto, LV_BTN_STATE_RELEASED, data_ico_auto);
            lv_imgbtn_set_src(_img_icon_cool, LV_BTN_STATE_RELEASED, data_ico_cool_down);
            lv_imgbtn_set_src(_img_icon_heat, LV_BTN_STATE_RELEASED, data_ico_heat);
            lv_imgbtn_set_src(_img_icon_dry, LV_BTN_STATE_RELEASED, data_ico_dry);
            lv_imgbtn_set_src(_img_icon_fan, LV_BTN_STATE_RELEASED, data_ico_fan);

            ui_obj_set_highlight(_btn_auto, false);
            ui_obj_set_highlight(_btn_cool, true);
            ui_obj_set_highlight(_btn_heat, false);
            ui_obj_set_highlight(_btn_dry, false);
            ui_obj_set_highlight(_btn_fan, false);
            break;
        case ac_mode_heat:
            lv_imgbtn_set_src(_img_icon_auto, LV_BTN_STATE_RELEASED, data_ico_auto);
            lv_imgbtn_set_src(_img_icon_cool, LV_BTN_STATE_RELEASED, data_ico_cool);
            lv_imgbtn_set_src(_img_icon_heat, LV_BTN_STATE_RELEASED, data_ico_heat_down);
            lv_imgbtn_set_src(_img_icon_dry, LV_BTN_STATE_RELEASED, data_ico_dry);
            lv_imgbtn_set_src(_img_icon_fan, LV_BTN_STATE_RELEASED, data_ico_fan);
            ui_obj_set_highlight(_btn_auto, false);
            ui_obj_set_highlight(_btn_cool, false);
            ui_obj_set_highlight(_btn_heat, true);
            ui_obj_set_highlight(_btn_dry, false);
            ui_obj_set_highlight(_btn_fan, false);
            break;
        case ac_mode_dry:
            lv_imgbtn_set_src(_img_icon_auto, LV_BTN_STATE_RELEASED, data_ico_auto);
            lv_imgbtn_set_src(_img_icon_cool, LV_BTN_STATE_RELEASED, data_ico_cool);
            lv_imgbtn_set_src(_img_icon_heat, LV_BTN_STATE_RELEASED, data_ico_heat);
            lv_imgbtn_set_src(_img_icon_dry, LV_BTN_STATE_RELEASED, data_ico_dry_down);
            lv_imgbtn_set_src(_img_icon_fan, LV_BTN_STATE_RELEASED, data_ico_fan);

            ui_obj_set_highlight(_btn_auto, false);
            ui_obj_set_highlight(_btn_cool, false);
            ui_obj_set_highlight(_btn_heat, false);
            ui_obj_set_highlight(_btn_dry, true);
            ui_obj_set_highlight(_btn_fan, false);
            break;
        case ac_mode_fan:
            lv_imgbtn_set_src(_img_icon_auto, LV_BTN_STATE_RELEASED, data_ico_auto);
            lv_imgbtn_set_src(_img_icon_cool, LV_BTN_STATE_RELEASED, data_ico_cool);
            lv_imgbtn_set_src(_img_icon_heat, LV_BTN_STATE_RELEASED, data_ico_heat);
            lv_imgbtn_set_src(_img_icon_dry, LV_BTN_STATE_RELEASED, data_ico_dry);
            lv_imgbtn_set_src(_img_icon_fan, LV_BTN_STATE_RELEASED, data_ico_fan_down);

            ui_obj_set_highlight(_btn_auto, false);
            ui_obj_set_highlight(_btn_cool, false);
            ui_obj_set_highlight(_btn_heat, false);
            ui_obj_set_highlight(_btn_dry, false);
            ui_obj_set_highlight(_btn_fan, true);
            break;
        default:
            break;
        }
    } else {
        lv_obj_set_style_local_bg_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
        lv_obj_set_style_local_bg_color(_slider_fan_speed, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, COLOR_BAR);
        lv_obj_set_style_local_bg_color(_slider_fan_speed, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, COLOR_BAR);
        ui_obj_set_highlight(_btn_auto, false);
        ui_obj_set_highlight(_btn_cool, false);
        ui_obj_set_highlight(_btn_heat, false);
        ui_obj_set_highlight(_btn_dry, false);
        ui_obj_set_highlight(_btn_fan, false);
        lv_imgbtn_set_src(_img_icon_auto, LV_BTN_STATE_RELEASED, data_ico_auto);
        lv_imgbtn_set_src(_img_icon_cool, LV_BTN_STATE_RELEASED, data_ico_cool);
        lv_imgbtn_set_src(_img_icon_heat, LV_BTN_STATE_RELEASED, data_ico_heat);
        lv_imgbtn_set_src(_img_icon_dry, LV_BTN_STATE_RELEASED, data_ico_dry);
        lv_imgbtn_set_src(_img_icon_fan, LV_BTN_STATE_RELEASED, data_ico_fan);
    }
}

static void ui_ac_update_sleep_mode(void)
{
    if (ac_state.on) {
        if (ac_state.sleep_en) {
            lv_switch_on(_sw_sleep, LV_ANIM_ON);
        } else {
            lv_switch_off(_sw_sleep, LV_ANIM_ON);
            lv_obj_set_style_local_border_width(_btn_sleep, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

        }
    } else {
        lv_switch_off(_sw_sleep, LV_ANIM_ON);
    }
}

static void ui_ac_update_fan_speed(void)
{
    if (ac_state.on) {
        lv_obj_set_style_local_bg_color(_obj_fan_speed, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
        lv_obj_set_style_local_bg_color(_obj_fan_speed->user_data, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
        lv_obj_set_hidden(_bar_div_fan, true);
        lv_obj_set_style_local_text_color(_label_fan_speed, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_set_style_local_bg_color(_bar_fan_speed[0], LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_make(251, 217, 163));
        lv_obj_set_style_local_bg_color(_bar_fan_speed[1], LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_make(251, 217, 163));
        lv_obj_set_style_local_bg_color(_bar_fan_speed[2], LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_make(251, 217, 163));
        
        switch (ac_state.fan_speed) {
        case ac_fan_speed_low:
            lv_bar_set_value(_bar_fan_speed[0], 10, LV_ANIM_ON);
            lv_bar_set_value(_bar_fan_speed[1], 0, LV_ANIM_ON);
            lv_bar_set_value(_bar_fan_speed[2], 0, LV_ANIM_ON);
            break;
        case ac_fan_speed_mid:
            lv_bar_set_value(_bar_fan_speed[0], 10, LV_ANIM_ON);
            lv_bar_set_value(_bar_fan_speed[1], 10, LV_ANIM_ON);
            lv_bar_set_value(_bar_fan_speed[2], 0, LV_ANIM_ON);
            break;
        case ac_fan_speed_high:
            lv_bar_set_value(_bar_fan_speed[0], 10, LV_ANIM_ON);
            lv_bar_set_value(_bar_fan_speed[1], 10, LV_ANIM_ON);
            lv_bar_set_value(_bar_fan_speed[2], 10, LV_ANIM_ON);
            break;
        }
    } else {
        lv_obj_set_hidden(_bar_div_fan, false);
        lv_obj_set_style_local_bg_color(_obj_fan_speed, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_set_style_local_bg_color(_obj_fan_speed->user_data, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_set_style_local_text_color(_label_fan_speed, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
        lv_obj_set_style_local_bg_color(_bar_fan_speed[0], LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_set_style_local_bg_color(_bar_fan_speed[1], LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_set_style_local_bg_color(_bar_fan_speed[2], LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    }
}

static void ui_ac_update_set_temp(void)
{
    static char *temp_str = NULL;

    if (NULL == temp_str) {
        temp_str = heap_caps_malloc(8, MALLOC_CAP_SPIRAM);
    }

    sprintf(temp_str, "%d°C", ac_state.set_temp);
    lv_label_set_text_static(_label_set_temp_val, temp_str);
}

/**
 * @brief Update timing panel
 * 
 */
static void ui_ac_update_time_panel(void)
{
    static uint8_t hour = 0, min = 0;
    static char fmt_text_h1[4];
    static char fmt_text_h2[4];
    static char fmt_text_m1[4];
    static char fmt_text_m2[4];

    if (lv_switch_get_state(_sw_time_on_off)) {
        hour = lv_roller_get_selected(_roller_hour);
        min = lv_roller_get_selected(_roller_min);

        lv_obj_set_hidden(_roller_hour, true);
        lv_obj_set_hidden(_roller_min, true);
        lv_obj_set_hidden(_label_hour, true);
        lv_obj_set_hidden(_label_min, true);
        lv_obj_set_hidden(_label_time_on[0], false);
        lv_obj_set_hidden(_label_time_on[1], false);
        lv_obj_set_hidden(_label_time_on[2], false);
        lv_obj_set_hidden(_label_time_on[3], false);
        lv_obj_set_hidden(_label_time_on_div, false);

        sprintf(fmt_text_h1, "%d", hour / 10);
        lv_obj_set_style_local_value_str(_label_time_on[0], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, fmt_text_h1);
        sprintf(fmt_text_h2, "%d", hour % 10);
        lv_obj_set_style_local_value_str(_label_time_on[1], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, fmt_text_h2);

        sprintf(fmt_text_m1, "%d", min / 10);
        lv_obj_set_style_local_value_str(_label_time_on[2], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, fmt_text_m1);
        sprintf(fmt_text_m2, "%d", min % 10);
        lv_obj_set_style_local_value_str(_label_time_on[3], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, fmt_text_m2);
    } else {
        lv_obj_set_hidden(_roller_hour, false);
        lv_obj_set_hidden(_roller_min, false);
        lv_obj_set_hidden(_label_hour, false);
        lv_obj_set_hidden(_label_min, false);
        lv_obj_set_hidden(_label_time_on[0], true);
        lv_obj_set_hidden(_label_time_on[1], true);
        lv_obj_set_hidden(_label_time_on[2], true);
        lv_obj_set_hidden(_label_time_on[3], true);
        lv_obj_set_hidden(_label_time_on_div, true);
    }
}

/**
 * @brief Update air conditioner mode.
 * 
 */
static void ui_ac_update(void)
{
    ui_ac_update_mode();
    ui_ac_update_sleep_mode();
    ui_ac_update_fan_speed();
    ui_ac_update_set_temp();
    ui_ac_update_time_panel();
}

static void sw_time_on_off_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_VALUE_CHANGED == event) {
        ui_ac_update_time_panel();
    }
}

static void btn_mode_click(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED == event) {
        lv_imgbtn_set_state(lv_obj_get_user_data(obj), LV_BTN_STATE_PRESSED);
    }

    if (LV_EVENT_RELEASED == event) {
        lv_imgbtn_set_state(lv_obj_get_user_data(obj), LV_BTN_STATE_RELEASED);
    }

    if (LV_EVENT_PRESS_LOST == event) {
        lv_imgbtn_set_state(lv_obj_get_user_data(obj), LV_BTN_STATE_RELEASED);
    }

    if (LV_EVENT_CLICKED == event) {
        if (_btn_auto == obj) {
            ac_state.ac_mode = ac_mode_auto;
        } else if (_btn_cool == obj) {
            ac_state.ac_mode = ac_mode_cool;
        } else if (_btn_heat == obj) {
            ac_state.ac_mode = ac_mode_heat;
        } else if (_btn_dry == obj) {
            ac_state.ac_mode = ac_mode_dry;
        } else if (_btn_fan == obj) {
            ac_state.ac_mode = ac_mode_fan;
        }
    }

    ui_ac_update_mode();
}

static void btn_sleep_click(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        ac_state.sleep_en = !ac_state.sleep_en;
        ui_ac_update_sleep_mode();
    }
}

static void btn_temp_inc_dec_click(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        if (_btn_temp_dec == obj) {
            if (ac_state.set_temp > 16) {
                ac_state.set_temp--;
            }
        } else if (_btn_temp_inc == obj) {
            if (ac_state.set_temp < 30) {
                ac_state.set_temp++;
            }
        }
    }

    ui_ac_update_set_temp();
}

static void btn_pwr_click(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        ac_state.on = !ac_state.on;

        if (ac_state.on) {
            lv_label_set_text(_label_air_on_off, "#f6ae3d ON# / OFF");
            lv_obj_set_style_local_border_color(_main_panel, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
            lv_obj_set_style_local_border_color(_fan_panel, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
            lv_obj_set_style_local_border_color(_time_panel, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
            lv_obj_set_style_local_border_color(_main_panel, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, COLOR_THEME);
            lv_obj_set_style_local_border_color(_fan_panel, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, COLOR_THEME);
            lv_obj_set_style_local_border_color(_time_panel, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, COLOR_THEME);
            lv_obj_set_style_local_bg_color(_div_led[0], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
            lv_obj_set_style_local_bg_color(_div_led[1], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
            lv_obj_set_style_local_bg_color(_div_led[2], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
        } else {
            lv_label_set_text(_label_air_on_off, "ON / #f6ae3d OFF#");

            lv_obj_set_style_local_border_color(_main_panel, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);
            lv_obj_set_style_local_border_color(_fan_panel, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);
            lv_obj_set_style_local_border_color(_time_panel, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);
            lv_obj_set_style_local_border_color(_main_panel, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, COLOR_BG);
            lv_obj_set_style_local_border_color(_fan_panel, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, COLOR_BG);
            lv_obj_set_style_local_border_color(_time_panel, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, COLOR_BG);
            lv_obj_set_style_local_bg_color(_div_led[0], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
            lv_obj_set_style_local_bg_color(_div_led[1], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
            lv_obj_set_style_local_bg_color(_div_led[2], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
        }

        ui_ac_update();
    }
}

static void slider_fan_speed_changed(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_VALUE_CHANGED == event) {
        ac_state.fan_speed = lv_slider_get_value(obj);

        ui_ac_update_fan_speed();
    }
}
