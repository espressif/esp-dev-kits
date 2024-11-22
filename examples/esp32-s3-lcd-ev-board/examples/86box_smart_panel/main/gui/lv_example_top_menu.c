/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "bsp/esp-bsp.h"
#include <time.h>
#include <sys/time.h>
#include "settings.h"

#include "bsp_board_extra.h"
#include "app_weather.h"

#include "lv_example_pub.h"
#include "lv_example_image.h"

static const char *TAG = "TOP_MENU";

#define PAGE_INDEX_USE_POINT    0

typedef void (*lv_btn_create_cb)(lv_obj_t *parent);

static bool main_layer_enter_cb(void *create_layer);
static bool main_layer_exit_cb(void *create_layer);
static void main_layer_timer_cb(lv_timer_t *tmr);

static void lv_btn_create_light_light(lv_obj_t *parent);
static void lv_btn_create_light_color(lv_obj_t *parent);
static void lv_btn_create_page_clock(lv_obj_t *parent);
static void lv_btn_create_page_air_conditioner(lv_obj_t *parent);
static void lv_btn_create_page_weather1(lv_obj_t *parent);
static void lv_btn_create_page_weather3(lv_obj_t *parent);

static void lv_btn_create_page_select_theam(lv_obj_t *parent);
static void lv_btn_create_page_select_light(lv_obj_t *parent);
static void lv_btn_create_page_select_home(lv_obj_t *parent);
static void lv_btn_create_page_select_ac(lv_obj_t *parent);

typedef enum {
    PAGE_NUM_HOME,
    PAGE_NUM_LIGHT,
    PAGE_NUM_AC,
    PAGE_NUM_THEAM,
} PAGE_NUM_t;

lv_layer_t main_Layer = {
    .lv_obj_name    = "main_Layer",
    .lv_obj_parent  = NULL,
    .lv_obj_layer   = NULL,
    .lv_show_layer  = &show_set_layer,
    // .lv_show_layer  = NULL,
    .enter_cb       = main_layer_enter_cb,
    .exit_cb        = main_layer_exit_cb,
    .timer_cb       = main_layer_timer_cb,
};

typedef struct {
    bool icon_enable;
    bool icon_focused;
    uint8_t x_axis;
    uint8_t y_axis;
    uint8_t x_placeholer;
    uint8_t y_placeholer;

    uint8_t *img_src;
    lv_btn_create_cb create_child_cb;
} icon_descrip;

typedef struct {
    lv_obj_t *img_AC_power;
    lv_obj_t *img_AC_add;
    lv_obj_t *img_AC_dec;
    lv_obj_t *label_temp;
} ac_obj_item_t;

icon_descrip icon_descrip_layout_1[] = {
    {true, false,   0, 0, 1, 2, (uint8_t *) &icon_320_weather, lv_btn_create_page_weather1},
    {true, false,   1, 0, 2, 2, (uint8_t *) &icon_320_AC, lv_btn_create_page_clock},
    {false, false,  2, 0, 1, 2, NULL, NULL},

    {false, false,  0, 1, 1, 1, NULL, NULL},
    {false, false,  1, 1, 1, 1, NULL, NULL},
    {false, false,  2, 1, 1, 1, NULL, NULL},

    {true, false,   0, 2, 1, 1, (uint8_t *) &icon_160_theam, lv_btn_create_page_select_theam},
    {true, false,   1, 2, 1, 1, (uint8_t *) &icon_160_light, lv_btn_create_page_select_light},
    {true, false,   2, 2, 1, 1, (uint8_t *) &icon_160_home, lv_btn_create_page_select_ac},
};

icon_descrip icon_descrip_layout_2[] = {
    {true, false,   0, 0, 1, 2, (uint8_t *) &icon_320_weather, lv_btn_create_light_light},
    {true, false,   1, 0, 2, 2, (uint8_t *) &icon_320_AC, lv_btn_create_light_color},
    {false, false,  2, 0, 1, 2, NULL, NULL},


    {false, false,  0, 1, 1, 1, NULL, NULL},
    {false, false,  1, 1, 1, 1, NULL, NULL},
    {false, false,  2, 1, 1, 1, NULL, NULL},

    {true, false,   0, 2, 1, 1, (uint8_t *) &icon_160_theam, lv_btn_create_page_select_theam},
    {true, false,   1, 2, 1, 1, (uint8_t *) &icon_160_light, lv_btn_create_page_select_home},
    {true, false,   2, 2, 1, 1, (uint8_t *) &icon_160_home, lv_btn_create_page_select_ac},
};

icon_descrip icon_descrip_layout_3[] = {
    {true, false,   0, 0, 1, 2, (uint8_t *) &icon_320_weather, lv_btn_create_page_weather3},
    {true, false,   1, 0, 2, 2, (uint8_t *) &icon_320_AC, lv_btn_create_page_air_conditioner},
    {false, false,  2, 0, 1, 2, NULL, NULL},


    {false, false,  0, 1, 1, 1, NULL, NULL},
    {false, false,  1, 1, 1, 1, NULL, NULL},
    {false, false,  2, 1, 1, 1, NULL, NULL},

    {true, false,   0, 2, 1, 1, (uint8_t *) &icon_160_theam, lv_btn_create_page_select_theam},
    {true, false,   1, 2, 1, 1, (uint8_t *) &icon_160_light, lv_btn_create_page_select_light},
    {true, false,   2, 2, 1, 1, (uint8_t *) &icon_160_home, lv_btn_create_page_select_home},
};

icon_descrip *emoji_layout[USER_MAINMENU_MAX] = {
    &icon_descrip_layout_1[0],
    &icon_descrip_layout_2[0],
    &icon_descrip_layout_3[0]
};

static lv_obj_t *label_weather1_temp, *label_weather1_city;
static lv_obj_t *label_weather3_temp, *label_weather3_city;

static lv_obj_t *page1_time_Hour_H, *page1_time_Hour_L;
static lv_obj_t *page1_time_min_H, *page1_time_min_L;

static ac_obj_item_t ac_obj_item;

static uint8_t active_page_index;
static lv_obj_t *cont_page_index;

static lv_obj_t *titleview_home;

static lv_obj_t *tips_btn, *tips_label, *slider_light;
static lv_obj_t *img_AC_power, *img_AC_add,  *img_AC_dec;

static uint8_t tips_delay;
THEAM_SET_SELECT thream_set = THEAM_SET_WARM;

static lv_color16_t light_color_select = {
    .full = 0x00FF,
};
static uint8_t light_brightness_set = 0;

static time_out_count time_500ms;

static void light_set_rgb(lv_color16_t color_set, uint8_t brightness)
{
    bsp_extra_led_set_rgb(0, brightness * color_set.ch.red * 255 / 63 / 100,
                          brightness * color_set.ch.green * 255 / 63 / 100,
                          brightness * color_set.ch.blue * 255 / 63 / 100);
}


static void light_set_onoff(bool onoff)
{
    if (true == onoff) {
        if (0 == light_brightness_set) {
            light_brightness_set = 40;
        }
    } else {
        light_brightness_set = 0;
    }

    light_set_rgb(light_color_select, light_brightness_set);
    lv_slider_set_value(slider_light, light_brightness_set, LV_ANIM_OFF);
    lv_obj_set_tile_id(titleview_home, PAGE_NUM_LIGHT, 0, LV_ANIM_ON);
}

static void top_titleview_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    static lv_event_code_t code_xor;
    if (code_xor != code) {
        code_xor = code;

        if (code == LV_EVENT_SCROLL_BEGIN) {
            // ESP_LOGI(TAG, "Code: begin");
        }
        if (code == LV_EVENT_SCROLL_END) {
            // ESP_LOGI(TAG, "Code: end");
        }
    }

    if (code == LV_EVENT_VALUE_CHANGED) {
        feed_clock_time();
        lv_obj_t *nowObj = lv_tileview_get_tile_act(obj);

        if (cont_page_index) {
            active_page_index = lv_obj_get_index(nowObj);
            lv_event_send(cont_page_index, LV_EVENT_VALUE_CHANGED, &active_page_index);
        } else {
        }
    }
}

static void color_change_press_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        feed_clock_time();
        light_set_onoff(light_brightness_set ? false : true);
    }
}

static void color_select_press_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        feed_clock_time();
        lv_indev_t *indev = lv_indev_get_next(NULL);
        int x, y;

        if (lv_indev_get_type(indev) == LV_INDEV_TYPE_POINTER) {
            x = indev->proc.types.pointer.act_point.x;
            y = indev->proc.types.pointer.act_point.y;
        } else {
            x = y = 0;
        }

        lv_disp_t *disp = lv_disp_get_default();
        if (!disp) {
            LV_LOG_WARN("no display registered");
            return;
        }

        lv_disp_draw_buf_t *draw_buf = lv_disp_get_draw_buf(disp);
        lv_coord_t hor_res = lv_disp_get_hor_res(disp);

        light_color_select.full = *((uint16_t *)draw_buf->buf_act + y * hor_res + x);
        if (0 == light_brightness_set) {
            light_brightness_set = 40;
            lv_slider_set_value(slider_light, light_brightness_set, LV_ANIM_OFF);
        }
        light_set_rgb(light_color_select, light_brightness_set);
    }
}

static void btn_press_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        feed_clock_time();
        int fun_select = (int)lv_event_get_user_data(e);

        switch (fun_select) {
        case PAGE_NUM_THEAM:
            if (THEAM_SET_WARM == thream_set) {
                thream_set = THEAM_SET_COOL;
            } else {
                thream_set = THEAM_SET_WARM;
            }
            lv_func_goto_layer(&main_Layer);
            break;

        default:
        case PAGE_NUM_HOME:
        case PAGE_NUM_LIGHT:
        case PAGE_NUM_AC:
            lv_obj_set_tile_id(titleview_home, fun_select, 0, LV_ANIM_OFF);
            break;
        }
        //LV_LOG_USER("Found fun_select:%d, thream_set:%d", fun_select, thream_set);
    }
}


static void btn_ac_add_press_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    ac_obj_item_t *obj = lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        feed_clock_time();

        char *tempstr = lv_label_get_text(obj->label_temp);
        uint8_t temp_set = atoi(tempstr);
        if (temp_set < 30) {
            temp_set++;
        }
        if (&icon_child_ac_pow2 == lv_img_get_src(img_AC_power) || &icon_child_ac_pow2_warm == lv_img_get_src(img_AC_power)) {
            lv_label_set_text_fmt(obj->label_temp, "%d℃", temp_set);
        }
    }
}

static void btn_ac_dec_press_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    ac_obj_item_t *obj = lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        feed_clock_time();

        char *tempstr = lv_label_get_text(obj->label_temp);
        uint8_t temp_set = atoi(tempstr);
        if (temp_set > 16) {
            temp_set--;
        }
        if (&icon_child_ac_pow2 == lv_img_get_src(img_AC_power) || &icon_child_ac_pow2_warm == lv_img_get_src(img_AC_power)) {
            lv_label_set_text_fmt(obj->label_temp, "%d℃", temp_set);
        }
    }
}

static void btn_ac_power_press_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    ac_obj_item_t *obj = lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        feed_clock_time();

        if (&icon_child_ac_pow2 == lv_img_get_src(obj->img_AC_power) || &icon_child_ac_pow2_warm == lv_img_get_src(obj->img_AC_power)) {
            lv_img_set_src(obj->img_AC_power, thream_set ? &icon_child_ac_pow1_warm : &icon_child_ac_pow1);
            lv_img_set_src(obj->img_AC_add, thream_set ? &icon_child_ac_add1_warm : &icon_child_ac_add1);
            lv_img_set_src(obj->img_AC_dec, thream_set ? &icon_child_ac_dec1_warm : &icon_child_ac_dec1);
        } else if (&icon_child_ac_pow1 == lv_img_get_src(obj->img_AC_power) || &icon_child_ac_pow1_warm == lv_img_get_src(obj->img_AC_power)) {
            lv_img_set_src(obj->img_AC_power, thream_set ? &icon_child_ac_pow2_warm : &icon_child_ac_pow2);
            lv_img_set_src(obj->img_AC_add, thream_set ? &icon_child_ac_add2_warm : &icon_child_ac_add2);
            lv_img_set_src(obj->img_AC_dec, thream_set ? &icon_child_ac_dec2_warm : &icon_child_ac_dec2);
        }
    }
}

static void ac_set_temp(bool add)
{
    char *tempstr = lv_label_get_text(ac_obj_item.label_temp);
    uint8_t temp_set = atoi(tempstr);

    if (false == add) {
        if (temp_set > 16) {
            temp_set--;
        }
    } else {
        if (temp_set < 30) {
            temp_set++;
        }
    }
    lv_label_set_text_fmt(ac_obj_item.label_temp, "%d℃", temp_set);
    lv_img_set_src(img_AC_power, thream_set ? &icon_child_ac_pow2_warm : &icon_child_ac_pow2); //audo open
    lv_img_set_src(img_AC_add, thream_set ? &icon_child_ac_add2_warm : &icon_child_ac_add2);
    lv_img_set_src(img_AC_dec, thream_set ? &icon_child_ac_dec2_warm : &icon_child_ac_dec2);

    //lv_tileview_get_tile_act
    lv_obj_set_tile_id(titleview_home, PAGE_NUM_AC, 0, LV_ANIM_ON);

    //app_sound_play();
}

static void ac_set_onoff(bool power_on)
{
    if ((false == power_on) && \
            (&icon_child_ac_pow2 == lv_img_get_src(ac_obj_item.img_AC_power) || &icon_child_ac_pow2_warm == lv_img_get_src(ac_obj_item.img_AC_power))) {
        lv_img_set_src(ac_obj_item.img_AC_power, thream_set ? &icon_child_ac_pow1_warm : &icon_child_ac_pow1);
        lv_img_set_src(ac_obj_item.img_AC_add, thream_set ? &icon_child_ac_add1_warm : &icon_child_ac_add1);
        lv_img_set_src(ac_obj_item.img_AC_dec, thream_set ? &icon_child_ac_dec1_warm : &icon_child_ac_dec1);
    } else if ((true == power_on) && \
               (&icon_child_ac_pow1 == lv_img_get_src(ac_obj_item.img_AC_power) || &icon_child_ac_pow1_warm == lv_img_get_src(ac_obj_item.img_AC_power))) {
        lv_img_set_src(ac_obj_item.img_AC_power, thream_set ? &icon_child_ac_pow2_warm : &icon_child_ac_pow2);
        lv_img_set_src(ac_obj_item.img_AC_add, thream_set ? &icon_child_ac_add2_warm : &icon_child_ac_add2);
        lv_img_set_src(ac_obj_item.img_AC_dec, thream_set ? &icon_child_ac_dec2_warm : &icon_child_ac_dec2);
    }
    lv_obj_set_tile_id(titleview_home, PAGE_NUM_AC, 0, LV_ANIM_ON);

    //app_sound_play();
}

static void pwm_slider_show_event_cb(lv_event_t *e)
{
    feed_clock_time();
    int pwm_set = 0;
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *slider_light = lv_event_get_target(e);
        pwm_set = (int)lv_slider_get_value(slider_light);
        light_brightness_set = pwm_set;
        light_set_rgb(light_color_select, light_brightness_set);
    }
}

static void update_time(void *arg)
{
    static time_t unix_time;
    static struct tm *time_info;

    unix_time = time(NULL);
    //unix_time += 1642254;
    time_info = localtime(&unix_time);

    int clock_hour = time_info->tm_hour;
    int clock_minutes = time_info->tm_min;

    if ((page1_time_Hour_H != NULL) && (page1_time_min_L != NULL)) {
        lv_label_set_text_fmt(page1_time_Hour_H, "%d", (clock_hour % 24) / 10);
        lv_label_set_text_fmt(page1_time_Hour_L, "%d", (clock_hour % 24) % 10);

        lv_label_set_text_fmt(page1_time_min_H, "%d", clock_minutes / 10);
        lv_label_set_text_fmt(page1_time_min_L, "%d", clock_minutes % 10);
    }
}

static void update_weather(void *arg)
{
    weather_info_t info;
    char *city_name;
    static uint8_t city_poll;

    city_poll++;
    if (ESP_OK == app_weather_get_current_info(&info, (city_poll / 6) % LOCATION_NUM_MAX)) {
        switch ((city_poll / 6) % LOCATION_NUM_MAX) {
        case LOCATION_NUM_BEIJING:
            city_name = sys_set->sr_lang ? "北京" : "BeiJing";
            break;
        case LOCATION_NUM_SHENZHEN:
            city_name = sys_set->sr_lang ? "深圳" : "ShenZhen";
            break;
        default:
        case LOCATION_NUM_SHANGHAI:
            city_name = sys_set->sr_lang ? "上海" : "ShangHai";
            break;
        }

        if ((label_weather1_temp != NULL) && (label_weather1_city != NULL)) {
            lv_label_set_text_fmt(label_weather1_temp, "%d℃", info.temp);
            lv_label_set_text_fmt(label_weather1_city, "%s", city_name);
        }

        if ((label_weather3_temp != NULL) && (label_weather3_city != NULL)) {
            lv_label_set_text_fmt(label_weather3_temp, "%d℃", info.temp);
            lv_label_set_text_fmt(label_weather3_city, "%s", city_name);
        }
    }
}

void lv_btn_create_light_color(lv_obj_t *parent)
{
    lv_obj_t *color_btn = lv_imgbtn_create(parent);
    lv_obj_set_size(color_btn, 250, 250);
    lv_imgbtn_set_src(color_btn, LV_IMGBTN_STATE_RELEASED, NULL, \
                      (false == thream_set) ? &icon_light_color : &icon_light_color_warm, NULL);
    lv_imgbtn_set_src(color_btn, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, \
                      (false == thream_set) ? &icon_light_color : &icon_light_color_warm, NULL);

    lv_obj_align(color_btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(color_btn, lv_color_hex(0xFF0000), 0);
    lv_obj_add_event_cb(color_btn, color_select_press_event_cb, LV_EVENT_SHORT_CLICKED, parent);


    lv_obj_t *change_btn = lv_imgbtn_create(parent);
    lv_obj_set_size(change_btn, 50, 50);
    lv_imgbtn_set_src(change_btn, LV_IMGBTN_STATE_RELEASED, NULL, &icon_light_change, NULL);
    lv_imgbtn_set_src(change_btn, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &icon_light_change, NULL);
    lv_obj_align(change_btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(change_btn, color_change_press_event_cb, LV_EVENT_SHORT_CLICKED, color_btn);
}

void lv_btn_create_light_light(lv_obj_t *parent)
{
    lv_obj_t *icon_light = lv_img_create(parent);
    lv_img_set_src(icon_light, &icon_light_light);
    lv_img_set_src(icon_light, thream_set ? &icon_light_light_warm : &icon_light_light);

    lv_obj_align(icon_light, LV_ALIGN_CENTER, 0, 0);

    slider_light = lv_slider_create(parent);
    lv_obj_remove_style_all(slider_light);        /*Remove the styles coming from the theme*/
    lv_obj_set_size(slider_light, 15, 180);
    lv_obj_align(slider_light, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_style(slider_light, &style_main, LV_PART_MAIN);
    lv_obj_add_style(slider_light, &style_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(slider_light, &style_knob, LV_PART_KNOB);

    lv_obj_add_event_cb(slider_light, pwm_slider_show_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_slider_set_range(slider_light, 0, 100);
    lv_slider_set_value(slider_light, light_brightness_set, LV_ANIM_OFF);
}

static lv_coord_t grid_clock_col_dsc[] = {30, 80, 80, LV_GRID_TEMPLATE_LAST};
static lv_coord_t grid_clock_row_dsc[] = {50, 100, 100, LV_GRID_TEMPLATE_LAST};

void lv_btn_create_page_clock(lv_obj_t *parent)
{
    lv_obj_t *label_HourH = lv_label_create(parent);
    lv_obj_set_style_text_color(label_HourH, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_HourH, &Montserrat_Bold_116, 0);
    // lv_obj_set_style_text_font(label_HourH, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_HourH, "1");

    lv_obj_t *label_HourL = lv_label_create(parent);
    lv_obj_set_style_text_color(label_HourL, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_HourL, &Montserrat_Bold_116, 0);
    // lv_obj_set_style_text_font(label_HourL, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_HourL, "2");

    lv_obj_t *label_MinH = lv_label_create(parent);
    lv_obj_set_style_text_color(label_MinH, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_MinH, &Montserrat_Bold_116, 0);
    // lv_obj_set_style_text_font(label_MinH, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_MinH, "3");

    lv_obj_t *label_MinL = lv_label_create(parent);
    lv_obj_set_style_text_color(label_MinL, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_MinL, &Montserrat_Bold_116, 0);
    // lv_obj_set_style_text_font(label_MinL, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_MinL, "4");

    lv_obj_set_grid_dsc_array(parent, grid_clock_col_dsc, grid_clock_row_dsc);
    lv_obj_set_grid_cell(label_HourH, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_set_grid_cell(label_HourL, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_set_grid_cell(label_MinH, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    lv_obj_set_grid_cell(label_MinL, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 2, 1);

    page1_time_Hour_H = label_HourH;
    page1_time_min_H = label_MinH;

    page1_time_Hour_L = label_HourL;
    page1_time_min_L = label_MinL;
}

void lv_btn_create_page_select_theam(lv_obj_t *parent)
{
    lv_obj_t *little_icon_img = lv_img_create(parent);
    lv_img_set_src(little_icon_img, &icon_child_theam);
    lv_obj_align(little_icon_img, LV_ALIGN_BOTTOM_MID, -20, -20);

    lv_obj_t *label_name = lv_label_create(parent);
    lv_obj_set_style_text_color(label_name, lv_color_hex(COLOUR_WHITE), 0);
    lv_obj_set_style_text_font(label_name, sys_set->sr_lang ? &SourceHanSansCN_Normal_18 : &SourceHanSansCN_Normal_20, 0);
    if (sys_set->sr_lang) {
        lv_label_set_text(label_name, "主题");
        lv_obj_align(label_name, LV_ALIGN_BOTTOM_MID, 20, -20);
    } else {
        lv_label_set_text(label_name, "Theme");
        lv_obj_align(label_name, LV_ALIGN_BOTTOM_MID, 25, -15);
    }

    lv_obj_t *btn_opa = lv_btn_create(parent);
    lv_obj_remove_style_all(btn_opa);
    lv_obj_set_style_bg_color(btn_opa, lv_color_hex(COLOUR_BLACK), 0);
    lv_obj_set_size(btn_opa, (480 / 3) - 10, (480 / 3) - 10);
    lv_obj_set_style_bg_opa(btn_opa, LV_OPA_0, 0);
    lv_obj_add_event_cb(btn_opa, btn_press_event_cb, LV_EVENT_SHORT_CLICKED, (void *)PAGE_NUM_THEAM);
}

void lv_btn_create_page_select_light(lv_obj_t *parent)
{
    lv_obj_t *little_icon_img = lv_img_create(parent);
    lv_img_set_src(little_icon_img, &icon_child_light);
    lv_obj_align(little_icon_img, LV_ALIGN_BOTTOM_MID, -20, -20);

    lv_obj_t *label_name = lv_label_create(parent);
    lv_obj_set_style_text_color(label_name, lv_color_hex(COLOUR_BLACK), 0);
    lv_obj_set_style_text_font(label_name, sys_set->sr_lang ? &SourceHanSansCN_Normal_18 : &SourceHanSansCN_Normal_20, 0);
    if (sys_set->sr_lang) {
        lv_label_set_text(label_name, "照明");
        lv_obj_align(label_name, LV_ALIGN_BOTTOM_MID, 20, -20);
    } else {
        lv_label_set_text(label_name, "Light");
        lv_obj_align(label_name, LV_ALIGN_BOTTOM_MID, 25, -15);
    }

    lv_obj_t *btn_opa = lv_btn_create(parent);
    lv_obj_remove_style_all(btn_opa);
    lv_obj_set_style_bg_color(btn_opa, lv_color_hex(COLOUR_BLACK), 0);
    lv_obj_set_size(btn_opa, (480 / 3) - 10, (480 / 3) - 10);
    lv_obj_set_style_bg_opa(btn_opa, LV_OPA_0, 0);
    lv_obj_add_event_cb(btn_opa, btn_press_event_cb, LV_EVENT_SHORT_CLICKED, (void *)PAGE_NUM_LIGHT);
}

void lv_btn_create_page_select_home(lv_obj_t *parent)
{
    lv_obj_t *little_icon_img = lv_img_create(parent);
    lv_img_set_src(little_icon_img, &icon_child_home);
    lv_obj_align(little_icon_img, LV_ALIGN_BOTTOM_MID, -20, -20);

    lv_obj_t *label_name = lv_label_create(parent);
    lv_obj_set_style_text_color(label_name, lv_color_hex(COLOUR_WHITE), 0);
    lv_obj_set_style_text_font(label_name, sys_set->sr_lang ? &SourceHanSansCN_Normal_18 : &SourceHanSansCN_Normal_20, 0);
    if (sys_set->sr_lang) {
        lv_label_set_text(label_name, "主页");
        lv_obj_align(label_name, LV_ALIGN_BOTTOM_MID, 20, -20);
    } else {
        lv_label_set_text(label_name, "Home");
        lv_obj_align(label_name, LV_ALIGN_BOTTOM_MID, 25, -15);
    }

    lv_obj_t *btn_opa = lv_btn_create(parent);
    lv_obj_remove_style_all(btn_opa);
    lv_obj_set_style_bg_color(btn_opa, lv_color_hex(COLOUR_BLACK), 0);
    lv_obj_set_size(btn_opa, (480 / 3) - 10, (480 / 3) - 10);
    lv_obj_set_style_bg_opa(btn_opa, LV_OPA_0, 0);
    lv_obj_add_event_cb(btn_opa, btn_press_event_cb, LV_EVENT_SHORT_CLICKED, (void *)PAGE_NUM_HOME);
}

void lv_btn_create_page_select_ac(lv_obj_t *parent)
{
    lv_obj_t *little_icon_img = lv_img_create(parent);
    lv_img_set_src(little_icon_img, &icon_child_ac_small);
    lv_obj_align(little_icon_img, LV_ALIGN_BOTTOM_MID, -20, -20);

    lv_obj_t *label_name = lv_label_create(parent);
    lv_obj_set_style_text_color(label_name, lv_color_hex(COLOUR_WHITE), 0);
    lv_obj_set_style_text_font(label_name, sys_set->sr_lang ? &SourceHanSansCN_Normal_18 : &SourceHanSansCN_Normal_20, 0);
    if (sys_set->sr_lang) {
        lv_label_set_text(label_name, "空调");
        lv_obj_align(label_name, LV_ALIGN_BOTTOM_MID, 20, -20);
    } else {
        lv_label_set_text(label_name, "AC");
        lv_obj_align(label_name, LV_ALIGN_BOTTOM_MID, 25, -15);
    }

    lv_obj_t *btn_opa = lv_btn_create(parent);
    lv_obj_remove_style_all(btn_opa);
    lv_obj_set_style_bg_color(btn_opa, lv_color_hex(COLOUR_BLACK), 0);
    lv_obj_set_size(btn_opa, (480 / 3) - 10, (480 / 3) - 10);
    lv_obj_set_style_bg_opa(btn_opa, LV_OPA_0, 0);
    lv_obj_add_event_cb(btn_opa, btn_press_event_cb, LV_EVENT_SHORT_CLICKED, (void *)PAGE_NUM_AC);
}

void lv_btn_create_page_weather1(lv_obj_t *parent)
{
    lv_obj_t *little_icon_img = lv_img_create(parent);
    lv_img_set_src(little_icon_img, thream_set ? &icon_child_weather_warm : &icon_child_weather);
    lv_obj_align(little_icon_img, LV_ALIGN_CENTER, 0, -40);

    label_weather1_temp = lv_label_create(parent);
    lv_obj_set_style_text_color(label_weather1_temp, lv_color_hex(COLOUR_WHITE), 0);
    lv_obj_set_style_text_font(label_weather1_temp, &helveticaneue_32, 0);
    lv_label_set_text(label_weather1_temp, "25℃");
    lv_obj_align(label_weather1_temp, LV_ALIGN_CENTER, 0, 30);

    label_weather1_city = lv_label_create(parent);
    lv_obj_set_style_text_color(label_weather1_city, lv_color_hex(COLOUR_WHITE), 0);
    lv_obj_set_style_text_font(label_weather1_city, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_weather1_city, "上海");
    lv_obj_align(label_weather1_city, LV_ALIGN_CENTER, 0, 60);
}

void lv_btn_create_page_weather3(lv_obj_t *parent)
{
    lv_obj_t *little_icon_img = lv_img_create(parent);
    lv_img_set_src(little_icon_img, thream_set ? &icon_child_weather_warm : &icon_child_weather);
    lv_obj_align(little_icon_img, LV_ALIGN_CENTER, 0, -40);

    label_weather3_temp = lv_label_create(parent);
    lv_obj_set_style_text_color(label_weather3_temp, lv_color_hex(COLOUR_WHITE), 0);
    lv_obj_set_style_text_font(label_weather3_temp, &helveticaneue_32, 0);
    lv_label_set_text(label_weather3_temp, "25℃");
    lv_obj_align(label_weather3_temp, LV_ALIGN_CENTER, 0, 30);

    label_weather3_city = lv_label_create(parent);
    lv_obj_set_style_text_color(label_weather3_city, lv_color_hex(COLOUR_WHITE), 0);
    lv_obj_set_style_text_font(label_weather3_city, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_weather3_city, "上海");
    lv_obj_align(label_weather3_city, LV_ALIGN_CENTER, 0, 60);
}

void lv_btn_create_page_air_conditioner(lv_obj_t *parent)
{
    lv_obj_t *label_name = lv_label_create(parent);
    lv_obj_set_style_text_color(label_name, lv_color_hex(COLOUR_WHITE), 0);
    lv_obj_set_style_text_font(label_name, sys_set->sr_lang ? &SourceHanSansCN_Normal_26 : &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_name, sys_set->sr_lang ? "空调" : "Air-conditioner");
    lv_obj_align(label_name, LV_ALIGN_OUT_TOP_LEFT, 20, 20);

    lv_obj_t *img_AC_BG = lv_img_create(parent);
    lv_img_set_src(img_AC_BG, thream_set ? &icon_child_ac_bg_warm : &icon_child_ac_bg);
    lv_obj_align(img_AC_BG, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t *img_AC_icon = lv_img_create(parent);
    lv_img_set_src(img_AC_icon, thream_set ? &icon_child_ac_warm : &icon_child_ac);
    lv_obj_align(img_AC_icon, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t *label_temp = lv_label_create(img_AC_icon);
    lv_obj_set_style_text_color(label_temp, lv_color_hex(COLOUR_WHITE), 0);
    lv_obj_set_style_text_font(label_temp, &helveticaneue_36, 0);
    lv_label_set_text(label_temp, "26℃");
    lv_obj_center(label_temp);

    /* dec btn */
    lv_obj_t *btn_AC_dec = lv_btn_create(parent);
    lv_obj_remove_style_all(btn_AC_dec);
    lv_obj_set_size(btn_AC_dec, 80, 46);
    lv_obj_align(btn_AC_dec, LV_ALIGN_BOTTOM_LEFT, 20, -34);

    img_AC_dec = lv_img_create(btn_AC_dec);
    lv_img_set_src(img_AC_dec, thream_set ? &icon_child_ac_dec1_warm : &icon_child_ac_dec1);
    lv_obj_center(img_AC_dec);

    /* power btn */
    lv_obj_t *btn_AC_power = lv_btn_create(parent);
    lv_obj_remove_style_all(btn_AC_power);
    lv_obj_set_size(btn_AC_power, 80, 46);
    lv_obj_align(btn_AC_power, LV_ALIGN_BOTTOM_RIGHT, -20, -34);

    img_AC_power = lv_img_create(btn_AC_power);
    lv_img_set_src(img_AC_power, thream_set ? &icon_child_ac_pow1_warm : &icon_child_ac_pow1);
    lv_obj_center(img_AC_power);

    /* Add btn */
    lv_obj_t *btn_AC_add = lv_btn_create(parent);
    lv_obj_remove_style_all(btn_AC_add);
    lv_obj_set_size(btn_AC_add, 80, 46);
    lv_obj_align(btn_AC_add, LV_ALIGN_BOTTOM_MID, 0, -34);

    img_AC_add = lv_img_create(btn_AC_add);
    lv_img_set_src(img_AC_add, thream_set ? &icon_child_ac_add1_warm : &icon_child_ac_add1);
    lv_obj_center(img_AC_add);

    ac_obj_item.img_AC_power = img_AC_power;
    ac_obj_item.img_AC_add = img_AC_add;
    ac_obj_item.img_AC_dec = img_AC_dec;
    ac_obj_item.label_temp = label_temp;

    lv_obj_add_event_cb(btn_AC_add, btn_ac_add_press_event_cb, LV_EVENT_CLICKED, &ac_obj_item);
    lv_obj_add_event_cb(btn_AC_dec, btn_ac_dec_press_event_cb, LV_EVENT_CLICKED, &ac_obj_item);
    lv_obj_add_event_cb(btn_AC_power, btn_ac_power_press_event_cb, LV_EVENT_CLICKED, &ac_obj_item);
}

void lv_create_nine_spaces(lv_obj_t *parent, uint8_t list)
{
    uint8_t count, validcount;

    static lv_coord_t grid_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

    lv_obj_set_grid_dsc_array(parent, grid_col_dsc, grid_row_dsc);

    icon_descrip *layout = emoji_layout[0];
    if (list < 5) {
        layout = emoji_layout[list];
    }

    validcount = 0;
    for (count = 0; count < 9; count++) {

        if (layout[count].icon_enable) {
            validcount++;
            lv_obj_t *btn = lv_btn_create(parent);
            //lv_obj_remove_style_all(btn);
            lv_obj_set_style_border_width(btn, 0, 0);
            lv_obj_set_style_pad_all(btn, 0, 0);
            lv_obj_set_size(btn, (layout[count].x_placeholer * (480 / 3) - 10), \
                            (layout[count].y_placeholer * (480 / 3) - 10));

            if ((emoji_layout[list] + count)->img_src) {
#if 0
                if (&icon_160_home == (emoji_layout[list] + count)->img_src) {
                    lv_obj_set_style_bg_color(btn, lv_color_hex(0x585f6b), 0);
                } else {
                    lv_obj_t *btn_img = lv_img_create(btn);
                    lv_img_set_src(btn_img, (emoji_layout[list] + count)->img_src);
                    lv_obj_center(btn_img);
                }
#else
                if (THEAM_SET_COOL == thream_set) {
                    if (&icon_160_light == (const lv_img_dsc_t *)(emoji_layout[list] + count)->img_src) {
                        lv_obj_set_style_bg_color(btn, lv_color_hex(0xb9d8ec), 0);
                    } else if (&icon_160_home == (const lv_img_dsc_t *)(emoji_layout[list] + count)->img_src) {
                        lv_obj_set_style_bg_color(btn, lv_color_hex(0x585f6b), 0);
                    } else if (&icon_160_theam == (const lv_img_dsc_t *)(emoji_layout[list] + count)->img_src) {
                        lv_obj_set_style_bg_color(btn, lv_color_hex(0x3a93b7), 0);
                    } else if (&icon_320_AC == (const lv_img_dsc_t *)(emoji_layout[list] + count)->img_src) {
                        lv_obj_set_style_bg_color(btn, lv_color_hex(0x9bb8d6), 0);
                    } else if (&icon_320_weather == (const lv_img_dsc_t *)(emoji_layout[list] + count)->img_src) {
                        lv_obj_set_style_bg_color(btn, lv_color_hex(0xadccda), 0);
                    }
                } else {
                    if (&icon_160_light == (const lv_img_dsc_t *)(emoji_layout[list] + count)->img_src) {
                        lv_obj_set_style_bg_color(btn, lv_color_hex(0xbc997a), 0);
                    } else if (&icon_160_home == (const lv_img_dsc_t *)(emoji_layout[list] + count)->img_src) {
                        lv_obj_set_style_bg_color(btn, lv_color_hex(0x6c4b3a), 0);
                    } else if (&icon_160_theam == (const lv_img_dsc_t *)(emoji_layout[list] + count)->img_src) {
                        lv_obj_set_style_bg_color(btn, lv_color_hex(0xbb6e5e), 0);
                    } else if (&icon_320_AC == (const lv_img_dsc_t *)(emoji_layout[list] + count)->img_src) {
                        lv_obj_set_style_bg_color(btn, lv_color_hex(0xe3b07b), 0);
                    } else if (&icon_320_weather == (const lv_img_dsc_t *)(emoji_layout[list] + count)->img_src) {
                        lv_obj_set_style_bg_color(btn, lv_color_hex(0xd5bdad), 0);
                    }
                }
#endif
            } else {
                lv_obj_set_style_bg_color(btn, (false == thream_set) ? lv_color_hex(0xa8c6e5) : lv_color_hex(0xefc190), 0);
            }

            lv_obj_set_grid_cell(btn,
                                 LV_GRID_ALIGN_CENTER, \
                                 layout[count].x_axis, layout[count].x_placeholer, \
                                 LV_GRID_ALIGN_CENTER, \
                                 layout[count].y_axis, layout[count].y_placeholer);

            if ((emoji_layout[list] + count)->create_child_cb) {
                (emoji_layout[list] + count)->create_child_cb(btn);
            }
            lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        }
    }
}

static bool main_layer_enter_cb(void *create_layer)
{
    bool ret = false;
    uint32_t i;

    lv_layer_t *layer = create_layer;
    LV_LOG_USER("lv_obj_name:%s", layer->lv_obj_name);

    if (NULL == layer->lv_obj_layer) {
        ret = true;
        layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_set_size(layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_border_width(layer->lv_obj_layer, 0, 0);
        lv_obj_set_style_pad_all(layer->lv_obj_layer, 0, 0);
        lv_obj_set_style_bg_color(layer->lv_obj_layer, lv_color_hex(COLOUR_GREY_4F), 0);

        titleview_home = lv_tileview_create(layer->lv_obj_layer);
        lv_obj_set_style_bg_color(titleview_home, lv_color_hex(COLOUR_GREY_1F), 0);
        lv_obj_add_event_cb(titleview_home, top_titleview_event_cb, LV_EVENT_ALL, 0);

#if PAGE_INDEX_USE_POINT
        lv_obj_set_style_bg_opa(titleview_home, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(titleview_home, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);

        cont_page_index = lv_obj_create(layer->lv_obj_layer);
        lv_obj_set_flex_flow(cont_page_index, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(cont_page_index, LV_FLEX_ALIGN_SPACE_EVENLY, 0, 0);
        lv_obj_set_size(cont_page_index, lv_pct(50), lv_pct(8));
        lv_obj_add_event_cb(cont_page_index, page_select_event_handler, LV_EVENT_VALUE_CHANGED, &active_page_index);

        lv_obj_center(cont_page_index);
        lv_obj_align(cont_page_index, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_set_style_bg_color(cont_page_index, lv_color_hex(COLOUR_BLACK), LV_PART_MAIN);
        lv_obj_set_style_border_width(cont_page_index, 0, 0);
        lv_obj_set_style_bg_opa(cont_page_index, LV_OPA_0, LV_PART_MAIN);
        lv_obj_clear_flag(cont_page_index, LV_OBJ_FLAG_SCROLLABLE);

        for (i = 0; i < USER_MAINMENU_MAX; i++) {
            lv_obj_t *obj = lv_checkbox_create(cont_page_index);
            lv_checkbox_set_text(obj, "\0");
            lv_obj_add_style(obj, &style_radio, LV_PART_INDICATOR);
            lv_obj_add_style(obj, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            //lv_obj_set_size(obj, 15, 15);
        }

        active_page_index = 0;
        lv_obj_add_state(lv_obj_get_child(cont_page_index, active_page_index), LV_STATE_CHECKED);
#endif
        for (i = 0; i < USER_MAINMENU_MAX; i++) {

            lv_obj_t *tile_bottom =  lv_tileview_add_tile(titleview_home, i, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
            lv_obj_t *obj = lv_btn_create(tile_bottom);
            lv_obj_remove_style_all(obj);
            lv_obj_set_size(obj, LV_HOR_RES, LV_VER_RES);
            lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 0, 0);
            lv_create_nine_spaces(obj, i);
        }

        tips_btn = lv_btn_create(layer->lv_obj_layer);
        //lv_obj_remove_style_all(btn);
        lv_obj_set_style_border_width(tips_btn, 0, 0);
        lv_obj_set_style_pad_all(tips_btn, 0, 0);
        lv_obj_set_size(tips_btn, 300, 60);
        lv_obj_align(tips_btn, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_bg_color(tips_btn, lv_color_hex(COLOUR_GREY_4F), LV_PART_MAIN);

        tips_label = lv_label_create(tips_btn);
        lv_obj_set_style_text_color(tips_label, lv_color_hex(COLOUR_WHITE), 0);
        lv_obj_set_style_text_font(tips_label, sys_set->sr_lang ? &SourceHanSansCN_Normal_18 : &SourceHanSansCN_Normal_20, 0);
        lv_obj_center(tips_label);
        lv_label_set_text(tips_label, " ");
        lv_obj_add_flag(tips_btn, LV_OBJ_FLAG_HIDDEN);
    }

    feed_clock_time();
    update_time(NULL);
    update_weather(NULL);
    set_time_out(&time_500ms, 500);

    return ret;
}

static bool main_layer_exit_cb(void *create_layer)
{
    LV_LOG_USER("exit");
    return true;
}

static void set_tips_info(const char *message)
{
    tips_delay = 14;
    lv_obj_clear_flag(tips_btn, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(tips_label, message);
}

static void main_layer_timer_cb(lv_timer_t *tmr)
{
    lv_event_info_t lvgl_event;

    if (is_time_out(&time_500ms)) {
        update_time(NULL);
        update_weather(NULL);
        if (tips_delay) {
            if (0 == (--tips_delay)) {
                lv_obj_add_flag(tips_btn, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }

    if (pdPASS == app_lvgl_get_event(&lvgl_event, 0)) {
        switch (lvgl_event.event) {

        case LV_EVENT_I_AM_HERE:
            set_tips_info(sys_set->sr_lang ? "请说" : "Say command");
            break;

        case LV_EVENT_LIGHT_ON:
            set_tips_info((const char *)lvgl_event.event_data);
            light_set_onoff(true);
            break;
        case LV_EVENT_LIGHT_OFF:
            set_tips_info((const char *)lvgl_event.event_data);
            light_set_onoff(false);
            break;
        case LV_EVENT_LIGHT_RGB_SET:
            ESP_LOGI(TAG, "light RGB set: %d", lvgl_event.event);
            break;

        case LV_EVENT_AC_SET_ON:
            ac_set_onoff(true);
            set_tips_info((const char *)lvgl_event.event_data);
            break;
        case LV_EVENT_AC_SET_OFF:
            ac_set_onoff(false);
            set_tips_info((const char *)lvgl_event.event_data);
            break;
        case LV_EVENT_AC_TEMP_ADD:
            ac_set_temp(true);
            set_tips_info((const char *)lvgl_event.event_data);
            break;
        case LV_EVENT_AC_TEMP_DEC:
            ac_set_temp(false);
            set_tips_info((const char *)lvgl_event.event_data);
            break;

        default:
        case LV_EVENT_EXIT_CLOCK:
            ESP_LOGI(TAG, "invalid event: %d", lvgl_event.event);
            break;
        }
    }
}
