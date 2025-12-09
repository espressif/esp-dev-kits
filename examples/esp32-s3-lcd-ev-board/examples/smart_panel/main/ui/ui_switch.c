/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "ui_main.h"

/* UI function declaration */
ui_func_desc_t ui_switch_func = {
    .name = "ui_switch",
    .init = ui_switch_init,
    .show = ui_switch_show,
    .hide = ui_switch_hide,
};

static ui_state_t ui_switch_state = ui_state_dis;

/* LVGL objects defination */
static lv_obj_t *_obj_light_control = NULL;
static lv_obj_t *_obj_air_conditioner = NULL;
static lv_obj_t *_obj_pomodoro_timer = NULL;
static lv_obj_t *_obj_habit_tracker = NULL;
static lv_obj_t *_obj_fan = NULL;
static lv_obj_t *_obj_video_entertainment = NULL;
static lv_obj_t *_obj_smart_security = NULL;
static lv_obj_t *_obj_other_device = NULL;
static lv_obj_t *msgbox = NULL;

/* Extern image resources data. Loaded in `ui_main.c` */
extern void *light_control;
extern void *air_conditioner;
extern void *pomodoro_timer;
extern void *habit_tracker;
extern void *fan;
extern void *video_entertainment;
extern void *smart_security;
extern void *other_device;

/* Static function forward declaration */
static void show_msg(const char *text);
static void btn_obj_click_cb(lv_obj_t *obj, lv_event_t event);
static lv_obj_t *ui_img_obj_vertical_create(lv_obj_t *par, lv_coord_t width, lv_coord_t height, lv_style_int_t radius, const void *src_img, const char *text);

void ui_switch_init(void *data)
{
    _obj_light_control = ui_img_obj_vertical_create(lv_scr_act(), 160, 160, 15, light_control, "  Light\nControl");
    _obj_air_conditioner = ui_img_obj_vertical_create(lv_scr_act(), 160, 160, 15, air_conditioner, "       Air\nConditioner");
    _obj_pomodoro_timer = ui_img_obj_vertical_create(lv_scr_act(), 160, 160, 15, pomodoro_timer, "Pomodoro\n    Timer");
    _obj_habit_tracker = ui_img_obj_vertical_create(lv_scr_act(), 160, 160, 15, habit_tracker, "  Habit\nTracker");
    _obj_fan = ui_img_obj_vertical_create(lv_scr_act(), 160, 160, 15, fan, "Fan");
    _obj_video_entertainment = ui_img_obj_vertical_create(lv_scr_act(), 160, 160, 15, video_entertainment, "Media");
    _obj_smart_security = ui_img_obj_vertical_create(lv_scr_act(), 160, 160, 15, smart_security, " Smart\nSecurity");
    _obj_other_device = ui_img_obj_vertical_create(lv_scr_act(), 160, 160, 15, other_device, "Other");

    lv_obj_align(_obj_light_control, NULL, LV_ALIGN_CENTER, -270, -65);
    lv_obj_align(_obj_air_conditioner, NULL, LV_ALIGN_CENTER, -90, -65);
    lv_obj_align(_obj_pomodoro_timer, NULL, LV_ALIGN_CENTER, 90, -65);
    lv_obj_align(_obj_habit_tracker, NULL, LV_ALIGN_CENTER, 270, -65);
    lv_obj_align(_obj_fan, NULL, LV_ALIGN_CENTER, -270, 115);
    lv_obj_align(_obj_video_entertainment, NULL, LV_ALIGN_CENTER, -90, 115);
    lv_obj_align(_obj_smart_security, NULL, LV_ALIGN_CENTER, 90, 115);
    lv_obj_align(_obj_other_device, NULL, LV_ALIGN_CENTER, 270, 115);

    lv_obj_set_event_cb(_obj_light_control, btn_obj_click_cb);
    lv_obj_set_event_cb(_obj_air_conditioner, btn_obj_click_cb);
    lv_obj_set_event_cb(_obj_other_device, btn_obj_click_cb);
    lv_obj_set_event_cb(_obj_pomodoro_timer, btn_obj_click_cb);
    lv_obj_set_event_cb(_obj_habit_tracker, btn_obj_click_cb);
    lv_obj_set_event_cb(_obj_fan, btn_obj_click_cb);
    lv_obj_set_event_cb(_obj_video_entertainment, btn_obj_click_cb);
    lv_obj_set_event_cb(_obj_smart_security, btn_obj_click_cb);

    ui_switch_state = ui_state_show;
}

void ui_switch_show(void *data)
{
    if (ui_state_dis == ui_switch_state) {
        ui_switch_init(data);
        ui_status_bar_time_show(true);
    } else if (ui_state_hide == ui_switch_state) {
        lv_obj_set_hidden(_obj_light_control, false);
        lv_obj_set_hidden(_obj_air_conditioner, false);
        lv_obj_set_hidden(_obj_pomodoro_timer, false);
        lv_obj_set_hidden(_obj_habit_tracker, false);
        lv_obj_set_hidden(_obj_fan, false);
        lv_obj_set_hidden(_obj_video_entertainment, false);
        lv_obj_set_hidden(_obj_smart_security, false);
        lv_obj_set_hidden(_obj_other_device, false);

        ui_status_bar_time_show(true);

        ui_switch_state = ui_state_show;
    }
}

void ui_switch_hide(void *data)
{
    if (ui_state_show == ui_switch_state) {
        lv_obj_set_hidden(_obj_light_control, true);
        lv_obj_set_hidden(_obj_air_conditioner, true);
        lv_obj_set_hidden(_obj_pomodoro_timer, true);
        lv_obj_set_hidden(_obj_habit_tracker, true);
        lv_obj_set_hidden(_obj_fan, true);
        lv_obj_set_hidden(_obj_video_entertainment, true);
        lv_obj_set_hidden(_obj_smart_security, true);
        lv_obj_set_hidden(_obj_other_device, true);

        ui_switch_state = ui_state_hide;
    }
}

static void btn_obj_click_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        if (_obj_light_control == obj) {
            ui_show(&ui_led_func, UI_SHOW_PEDDING);
        } else if (_obj_air_conditioner == obj) {
            ui_show(&ui_air_conditioner_func, UI_SHOW_PEDDING);
        } else if (_obj_pomodoro_timer == obj) {
            ui_show(&ui_pomodoro_timer_func, UI_SHOW_PEDDING); 
        } else if (_obj_habit_tracker == obj) {
            ui_show(&ui_habit_tracker_func, UI_SHOW_PEDDING); 
        } else {
            show_msg("Not support yet!");
        }
    }
}

static lv_obj_t *ui_img_obj_vertical_create(lv_obj_t *par, lv_coord_t width, lv_coord_t height, lv_style_int_t radius, const void *src_img, const char *text)
{
    lv_obj_t *_obj = NULL;
    lv_obj_t *_img = NULL;

    _obj = lv_obj_create(par, NULL);
    _img = lv_img_create(_obj, NULL);

    lv_obj_set_size(_obj, width, height);
    lv_obj_set_style_local_radius(_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, radius);
    lv_obj_set_style_local_bg_color(_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_border_color(_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);
    lv_obj_set_style_local_border_color(_obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, COLOR_BG);

    lv_img_set_src(_img, src_img);

    lv_obj_set_style_local_value_str(_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text);
    lv_obj_set_style_local_value_font(_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_style_local_value_align(_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_line_space(_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, -8);
    lv_obj_set_style_local_value_ofs_y(_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 40);
    lv_obj_set_style_local_value_color(_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_value_color(_obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);

    lv_obj_align(_img, NULL, LV_ALIGN_CENTER, 0, -20);

    return _obj;
}

static void msgbox_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        lv_obj_set_hidden(obj, true);
    }
}

static void show_msg(const char *text)
{
    static const char *btns[] = { "OK", ""};
    if (NULL == msgbox) {
        msgbox = lv_msgbox_create(lv_scr_act(), NULL);
        lv_msgbox_set_text(msgbox, text);
        lv_msgbox_add_btns(msgbox, btns);
        lv_obj_set_style_local_text_font(msgbox, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, &font_en_24);
        lv_obj_set_style_local_text_font(msgbox, LV_MSGBOX_PART_BTN, LV_STATE_DEFAULT, &font_en_24);

        lv_obj_align(msgbox, NULL, LV_ALIGN_CENTER, 0, 0);

        lv_obj_set_event_cb(msgbox, msgbox_cb);
    } else {
        lv_msgbox_set_text(msgbox, text);
        lv_obj_set_hidden(msgbox, false);
    }
}
