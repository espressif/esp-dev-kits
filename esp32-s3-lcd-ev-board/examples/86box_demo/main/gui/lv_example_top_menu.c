/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "bsp/esp-bsp.h"
#include "lv_example_pub.h"
#include "lv_example_image.h"

const warm_fan_info_list_t warm_openclose_icon[7] = {
    {&warmIcon6_open, &warmIcon6_close,     "电源"},
    {&warmIcon_open, &warmIcon_close,      "照明"},
    {&warmIcon1_open, &warmIcon1_close,    "自然风"},
    {&warmIcon2_open, &warmIcon2_close,    "换气"},
    {&warmIcon3_open, &warmIcon3_close,    "干燥"},
    {&warmIcon4_open, &warmIcon4_close,    "风暖"},
    {&warmIcon5_open, &warmIcon5_close,    "风向"},
};

const weather_info_list_t weather_icon_list[8] = {
    {&weather_icon_rain_home,       "雨",   "上海", 20,     "13-28"},
    {&weather_icon_cloudy_home,     "多云", "北京", 18,     "15-26"},
    {&weather_icon_smog_home,       "雾",   "深圳", 26,     "18-27"},
    {&weather_icon_floatingdust_home, "霾",  "广州", 27,     "19-28"},
    {&weather_icon_snow_home,       "雪",   "哈尔滨", 1,    "0-9"},
    {&weather_icon_forst_home,      "雾",   "重庆", 5,      "3-12"},
    {&weather_icon_summer_home,     "晴",   "青岛", 28,     "21-30"},
    {&weather_icon_overcast_home,   "阴",   "西安", 19,     "15-23"},
};

static bool main_layer_enter_cb(void *layer);
static bool main_layer_exit_cb(void *layer);
static void main_layer_timer_cb(lv_timer_t *tmr);

lv_layer_t main_Layer = {
    .lv_obj_name    = "main_Layer",
    .lv_obj_layer   = NULL,
    .lv_show_layer  = &show_set_layer,
    //.lv_show_layer  = NULL,
    .enter_cb       = main_layer_enter_cb,
    .exit_cb        = main_layer_exit_cb,
    .timer_cb       = main_layer_timer_cb,
};

static lv_obj_t *bg_top_title, *label_top_time;
static lv_obj_t *lv_top_menu_btn[USER_MAINMENU_MAX];
static lv_obj_t *tile_bottomHead;

static lv_obj_t *label_home_time;
static lv_obj_t *bg_home_device;
static lv_obj_t *tabview_home_weather;

static lv_obj_t *btn_fan_ctrl[USER_CTRLBTN_MAX];
static lv_obj_t *bg_fan_ctrl[USER_CTRLBTN_MAX];
static lv_obj_t *label_fan_ctrl[USER_CTRLBTN_MAX];

static lv_obj_t *label_device_power, *label_device_order, *label_device_temp;

static lv_obj_t *titleview_home_base;

static uint8_t slide_enter_filter = 0;

static time_out_count time_1000ms, time_1500ms;

static void top_home_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        if (slide_enter_filter) {
            LV_LOG_USER("slide_enter_filter:%d", slide_enter_filter);
        } else {
            lv_func_goto_layer(&weather_Layer);
        }
    }
}

static void top_home_music_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        LV_LOG_USER("LV_EVENT_CLICKED");
        lv_func_goto_layer(&show_music_layer);
    }
}

static void top_home_device_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_func_goto_layer(&ctrl_Layer);
    }
}

static void top_ctrl_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        if (slide_enter_filter) {
            LV_LOG_USER("slide_enter_filter:%d", slide_enter_filter);
        } else {
            lv_func_goto_layer(&ctrl_Layer);
        }
        feed_clock_time();
    }
}

static void top_titleview_event_cb(lv_event_t *e)
{
    lv_obj_t *obj, *nowObj;
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_VALUE_CHANGED) {

        slide_enter_filter = 6;
        feed_clock_time();

        obj = lv_event_get_target(e);
        nowObj = lv_tileview_get_tile_act(obj);

        if ((tile_bottomHead == nowObj)) {
            lv_obj_add_flag(bg_top_title, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(bg_top_title, LV_OBJ_FLAG_HIDDEN);

        }
    }
    if (code == LV_EVENT_SCROLL) {
        LV_LOG_USER("LV_EVENT_SCROLL");
    }
}

static void ctrlBtn_event_cb(lv_event_t *e)
{
    uint8_t i;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *button = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        int btn = (int)lv_event_get_user_data(e);
        feed_clock_time();

        if (0 == btn) {
            if (LV_STATE_CHECKED == (button->state & LV_STATE_CHECKED)) {
                for (i = 0; i < USER_CTRLBTN_MAX; i++) {
                    lv_obj_clear_state(btn_fan_ctrl[i], LV_STATE_CHECKED);
                    lv_img_set_src(bg_fan_ctrl[i], warm_openclose_icon[i].addr_open);
                }
            } else {
                for (i = 0; i < USER_CTRLBTN_MAX; i++) {
                    lv_obj_add_state(btn_fan_ctrl[i], LV_STATE_CHECKED);
                    lv_img_set_src(bg_fan_ctrl[i], warm_openclose_icon[i].addr_close);
                }
            }
        } else {
            if (LV_STATE_CHECKED == (button->state & LV_STATE_CHECKED)) {
                lv_obj_clear_state(button, LV_STATE_CHECKED);
                lv_img_set_src(bg_fan_ctrl[btn], warm_openclose_icon[btn].addr_open);
            } else {
                lv_obj_add_state(button, LV_STATE_CHECKED);
                lv_img_set_src(bg_fan_ctrl[btn], warm_openclose_icon[btn].addr_close);

                lv_obj_add_state(btn_fan_ctrl[0], LV_STATE_CHECKED);
                lv_img_set_src(bg_fan_ctrl[0], warm_openclose_icon[0].addr_close);
            }
        }
    }
}

static void ctrlBtn_power_event_cb(lv_event_t *e)
{
    char tempBUf[30];
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *button = lv_event_get_target(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        lv_obj_t *btn_img = lv_event_get_user_data(e);

        feed_clock_time();

        if (LV_STATE_CHECKED == (button->state & LV_STATE_CHECKED)) {
            lv_obj_clear_state(button, LV_STATE_CHECKED);
            lv_img_set_src(btn_img, warm_openclose_icon[0].addr_open);

            lv_obj_set_style_text_color(label_device_power, lv_color_hex(COLOUR_GREY), 0);
            memset(tempBUf, 0, sizeof(tempBUf));
            sprintf(tempBUf, "%s", "已关机");
            lv_label_set_text(label_device_power, tempBUf);

            lv_obj_set_style_text_color(label_device_order, lv_color_hex(COLOUR_GREY), 0);
            memset(tempBUf, 0, sizeof(tempBUf));
            sprintf(tempBUf, "%s", "9min开机");
            lv_label_set_text(label_device_order, tempBUf);
            lv_obj_set_style_text_color(label_device_temp, lv_color_hex(COLOUR_GREY), 0);
            LV_LOG_USER("clear checked:%X", button->state & LV_STATE_CHECKED);
        } else {
            lv_obj_add_state(button, LV_STATE_CHECKED);
            lv_img_set_src(btn_img, warm_openclose_icon[0].addr_close);

            lv_obj_set_style_text_color(label_device_power, lv_color_hex(0xFFFFFF), 0);
            memset(tempBUf, 0, sizeof(tempBUf));
            sprintf(tempBUf, "%s", "已开机");
            lv_label_set_text(label_device_power, tempBUf);

            lv_obj_set_style_text_color(label_device_order, lv_color_hex(COLOUR_GREY), 0);
            memset(tempBUf, 0, sizeof(tempBUf));
            sprintf(tempBUf, "%s", "--");
            lv_label_set_text(label_device_order, tempBUf);
            lv_obj_set_style_text_color(label_device_temp, lv_color_hex(0xFFFFFF), 0);

            LV_LOG_USER("set checked:%X", button->state & LV_STATE_CHECKED);
        }
    }
}

void lv_create_top_ctrl_2_page(lv_obj_t *parent)
{
    int i;
    lv_obj_t *labelName;

    static lv_coord_t grid_2_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    //static lv_coord_t grid_2_col_dsc[] = {150, 150, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_2_row_dsc[] = {
        80,  /*Title*/
        80,  /*Box title*/

        30,
        50,

        30,
        50,

        30,
        50,
        LV_GRID_TEMPLATE_LAST
    };
    lv_obj_set_grid_dsc_array(parent, grid_2_col_dsc, grid_2_row_dsc);

    labelName = lv_label_create(parent);
    lv_obj_remove_style_all(labelName);
    lv_obj_set_style_text_color(labelName, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelName, &font_lanting24, 0);

    lv_label_set_text(labelName, "暖风机");
    lv_obj_set_grid_cell(labelName, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    for (i = 0; i < USER_CTRLBTN_MAX; i++) {

        btn_fan_ctrl[i] = lv_btn_create(parent);
        lv_obj_remove_style_all(btn_fan_ctrl[i]);

        lv_obj_set_size(btn_fan_ctrl[i], 80, 80);
        if (0 != i) {
            label_fan_ctrl[i] = lv_label_create(parent);
            lv_obj_remove_style_all(label_fan_ctrl[i]);
            lv_obj_set_style_text_color(label_fan_ctrl[i], lv_color_hex(COLOUR_GREY), 0);
            lv_obj_set_style_text_font(label_fan_ctrl[i], &font_lantingMid18, 0);
            lv_label_set_text(label_fan_ctrl[i], warm_openclose_icon[i].btn_name);

            lv_obj_set_grid_cell(btn_fan_ctrl[i], LV_GRID_ALIGN_CENTER, (i - 1) % 2, 1, LV_GRID_ALIGN_CENTER, 2 + ((i - 1) / 2) * 2, 1);
            lv_obj_set_grid_cell(label_fan_ctrl[i], LV_GRID_ALIGN_CENTER, (i - 1) % 2, 1, LV_GRID_ALIGN_CENTER, 2 + ((i - 1) / 2) * 2 + 1, 1);
        } else {
            lv_obj_set_grid_cell(btn_fan_ctrl[i], LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
        }
        lv_obj_add_event_cb(btn_fan_ctrl[i], ctrlBtn_event_cb, LV_EVENT_CLICKED, (void *)i);

        lv_obj_clear_state(btn_fan_ctrl[i], LV_STATE_CHECKED);
        bg_fan_ctrl[i] = lv_img_create(btn_fan_ctrl[i]);
        lv_img_set_src(bg_fan_ctrl[i], warm_openclose_icon[i].addr_open);
        lv_obj_align(bg_fan_ctrl[i], LV_ALIGN_CENTER, 0, 0);
    }

    return;
}

void lv_create_top_ctrl_1_page(lv_obj_t *parent)
{
    char temp[30];

    lv_obj_t *labelName, * powerBtn, * powerBtnImg, *labelStatus;

    static lv_coord_t grid_2_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_2_row_dsc[] = {
        60,  /*Title*/
        120,  /*Box title*/
        60,  /*Box title*/
        80,  /*Title*/
        80,  /*Title*/
        LV_GRID_TEMPLATE_LAST
    };
    lv_obj_set_grid_dsc_array(parent, grid_2_col_dsc, grid_2_row_dsc);

    labelName = lv_label_create(parent);
    lv_obj_remove_style_all(labelName);
    lv_obj_set_style_text_color(labelName, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelName, &font_lanting24, 0);
    lv_label_set_text(labelName, "电热水器");
    lv_obj_set_grid_cell(labelName, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    powerBtn = lv_btn_create(parent);
    lv_obj_remove_style_all(powerBtn);

    lv_obj_set_size(powerBtn, 80, 80);
    lv_obj_set_grid_cell(powerBtn, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_clear_state(powerBtn, LV_STATE_CHECKED);
    powerBtnImg = lv_img_create(powerBtn);
    lv_img_set_src(powerBtnImg, warm_openclose_icon[0].addr_open);
    lv_obj_align(powerBtnImg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(powerBtn, ctrlBtn_power_event_cb, LV_EVENT_SHORT_CLICKED, powerBtnImg);

    label_device_temp = lv_label_create(parent);
    lv_obj_remove_style_all(label_device_temp);
    lv_obj_set_style_text_color(label_device_temp, lv_color_hex(COLOUR_GREY), 0);
    //lv_obj_set_style_text_font(labelTemp, &font_lanting36, 0);
    lv_obj_set_style_text_font(label_device_temp, &font_num50, 0);
    sprintf(temp, "%d", 32);
    lv_label_set_text(label_device_temp, temp);
    lv_obj_set_grid_cell(label_device_temp, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 2, 1);

    labelStatus = lv_label_create(parent);
    lv_obj_remove_style_all(labelStatus);
    lv_obj_set_style_text_color(labelStatus, lv_color_hex(COLOUR_GREY), 0);
    lv_obj_set_style_text_font(labelStatus, &font_lanting24, 0);
    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%s", "当前温度");
    lv_label_set_text(labelStatus, temp);
    //lv_obj_center(labelData);
    lv_obj_set_grid_cell(labelStatus, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 3, 1);

    label_device_power = lv_label_create(parent);
    lv_obj_remove_style_all(label_device_power);
    lv_obj_set_style_text_color(label_device_power, lv_color_hex(COLOUR_GREY), 0);
    lv_obj_set_style_text_font(label_device_power, &font_lanting24, 0);
    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%s", "已关机");
    lv_label_set_text(label_device_power, temp);
    //lv_obj_center(labelData);
    lv_obj_set_grid_cell(label_device_power, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 4, 1);

    label_device_order = lv_label_create(parent);
    lv_obj_remove_style_all(label_device_order);
    lv_obj_set_style_text_color(label_device_order, lv_color_hex(COLOUR_GREY), 0);
    lv_obj_set_style_text_font(label_device_order, &font_lanting24, 0);
    lv_label_set_long_mode(label_device_order, LV_LABEL_LONG_WRAP);
    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%s", "9min开机");
    lv_label_set_text(label_device_order, temp);
    //lv_obj_center(labelData);
    lv_obj_set_grid_cell(label_device_order, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 4, 1);

    return;
}

lv_obj_t *lv_create_weather_Anmi(lv_obj_t *parent)
{
    int i;
    char temp[20];
    lv_obj_t *tabview, * tab;
    lv_obj_t *tabimage, * titleimage, * labelTemp, *labelCity, *labelStatus;

    //tabview_home_weather = lv_tabview_create(parent, LV_DIR_TOP, 0);
    tabview = lv_tabview_create(parent, LV_DIR_TOP, 0);
    lv_obj_remove_style_all(tabview);
    lv_obj_set_size(tabview, 220, 210);
    //lv_obj_align(tabview_home_weather, LV_ALIGN_TOP_LEFT, 0, 100);
    lv_obj_clear_flag(tabview, LV_OBJ_FLAG_SCROLLABLE);

    for (i = 0; i < sizeof(weather_icon_list) / sizeof(weather_icon_list[0]); i++) {
        tab = lv_tabview_add_tab(tabview, "1");
        //lv_obj_remove_style_all(tab);
        lv_obj_align(tab, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_obj_clear_flag(tab, LV_OBJ_FLAG_SCROLLABLE);

        tabimage = lv_img_create(tab);
        lv_img_set_src(tabimage, weather_icon_list[i].addr_weather_icon);
        lv_obj_align(tabimage, LV_ALIGN_TOP_LEFT, 0, 0);

        labelTemp = lv_label_create(tabimage);
        lv_obj_remove_style_all(labelTemp);
        lv_obj_set_style_text_color(labelTemp, lv_color_white(), 0);
        lv_obj_set_style_text_font(labelTemp, &font_lantingMid18, 0);
        sprintf(temp, "%d 摄氏度", weather_icon_list[i].temperature);
        lv_label_set_text(labelTemp, temp);
        //lv_obj_center(labelTemp);
        lv_obj_align(labelTemp, LV_ALIGN_CENTER, 0, -10);

        labelStatus = lv_label_create(tabimage);
        lv_obj_remove_style_all(labelStatus);
        lv_obj_set_style_text_color(labelStatus, lv_color_white(), 0);
        lv_obj_set_style_text_font(labelStatus, &font_lantingMid18, 0);
        memset(temp, 0, sizeof(temp));
        sprintf(temp, "%s", weather_icon_list[i].status);
        lv_label_set_text(labelStatus, temp);
        //lv_obj_center(labelData);
        lv_obj_align_to(labelStatus, labelTemp, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

        titleimage = lv_img_create(tabimage);
        lv_img_set_src(titleimage, &icon_title_bg);
        lv_obj_align(titleimage, LV_ALIGN_TOP_LEFT, 0, 0);

        labelCity = lv_label_create(titleimage);
        lv_obj_remove_style_all(labelCity);
        lv_obj_set_style_text_color(labelCity, lv_color_black(), 0);
        lv_obj_set_style_text_font(labelCity, &font_lanting16, 0);

        sprintf(temp, "%s", weather_icon_list[i].location);
        lv_label_set_text(labelCity, temp);
        lv_obj_center(labelCity);

        lv_obj_t *tabBtn = lv_btn_create(tabimage);
        lv_obj_remove_style_all(tabBtn);
        lv_obj_set_size(tabBtn, 207, 104);
        lv_obj_add_event_cb(tabBtn, top_home_btn_event_cb, LV_EVENT_CLICKED, (void *)i);
    }

    return tabview;
}

lv_obj_t *lv_create_devInfo_Anmi(lv_obj_t *parent)
{
    char temp[20];
    lv_obj_t *devimage, *titleimage, *labelname, *labelTemp, *labelStatus;

    lv_obj_t *devbtn = lv_btn_create(parent);
    lv_obj_remove_style_all(devbtn);

    devimage = lv_img_create(devbtn);
    lv_img_set_src(devimage, &weather_icon_summer_home);
    lv_img_set_src(devimage, &home_btn);
    lv_obj_align(devimage, LV_ALIGN_CENTER, 0, 0);

    titleimage = lv_img_create(devimage);
    lv_img_set_src(titleimage, &icon_title_bg);

    labelname = lv_label_create(titleimage);
    lv_obj_remove_style_all(labelname);
    lv_obj_set_style_text_color(labelname, lv_color_black(), 0);
    lv_obj_set_style_text_font(labelname, &font_lanting16, 0);

    sprintf(temp, "%s", "热水器");
    lv_label_set_text(labelname, temp);
    lv_obj_center(labelname);

    labelTemp = lv_label_create(devimage);
    lv_obj_remove_style_all(labelTemp);
    lv_obj_set_style_text_color(labelTemp, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelTemp, &font_lantingMid18, 0);
    sprintf(temp, "%d 摄氏度", 30);
    lv_label_set_text(labelTemp, temp);
    //lv_obj_center(labelTemp);
    lv_obj_align(labelTemp, LV_ALIGN_CENTER, 0, -10);

    labelStatus = lv_label_create(devimage);
    lv_obj_remove_style_all(labelStatus);
    lv_obj_set_style_text_color(labelStatus, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelStatus, &font_lantingMid18, 0);
    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%s", "当前温度");
    lv_label_set_text(labelStatus, temp);
    //lv_obj_center(labelData);
    lv_obj_align_to(labelStatus, labelTemp, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    return devbtn;
}


void lv_create_topTitle(lv_obj_t *parent)
{
    lv_obj_t *label_indoor_status;

    bg_top_title = lv_label_create(parent);
    lv_obj_set_size(bg_top_title, 480, 38);

    label_top_time = lv_label_create(bg_top_title);
    lv_obj_remove_style_all(label_top_time);
    lv_obj_set_style_text_color(label_top_time, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_top_time, &font_num36, 0);

    lv_label_set_text(label_top_time, "16:17");
    lv_obj_center(label_top_time);
    lv_obj_align(label_top_time, LV_ALIGN_TOP_MID, 0, 1);

    label_indoor_status = lv_label_create(bg_top_title);
    lv_obj_remove_style_all(label_indoor_status);
    lv_obj_set_style_text_color(label_indoor_status, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_indoor_status, &font_lantingMid18, 0);

    lv_label_set_text(label_indoor_status, "室内20度  58\%");
    lv_obj_center(label_indoor_status);
    lv_obj_align(label_indoor_status, LV_ALIGN_TOP_RIGHT, 0, 5);

    lv_obj_add_flag(bg_top_title, LV_OBJ_FLAG_HIDDEN);

    return;
}

void lv_create_top_home_page(lv_obj_t *parent)
{
    char timeBuf[15];

    static lv_coord_t grid_2_col_dsc[] = {8, LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_2_row_dsc[] = {
        80,  /*Title*/
        80,  /*Box title*/
        80,  /*Box title*/
        80,  /*Title*/
        80,  /*Box title*/
        80,  /*Box title*/
        80,  /*Box title*/
        80,  /*Box title*/
        LV_GRID_TEMPLATE_LAST
    };

    lv_obj_set_grid_dsc_array(parent, grid_2_col_dsc, grid_2_row_dsc);

    label_home_time = lv_label_create(parent);
    lv_obj_remove_style_all(label_home_time);
    lv_obj_set_style_text_color(label_home_time, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_home_time, &font_num50, 0);

    int temp = sys_clock_getSecond();
    sprintf(timeBuf, "%02d:%02d", (temp / 60) % 24, temp % 60);
    lv_label_set_text(label_home_time, timeBuf);
    lv_obj_center(label_home_time);
    //lv_obj_align(label_home_time, LV_ALIGN_CENTER, 50, 0);
    lv_obj_set_grid_cell(label_home_time, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *labelData = lv_label_create(parent);
    lv_obj_remove_style_all(labelData);
    lv_obj_set_style_text_color(labelData, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelData, &font_num36, 0);

    lv_label_set_text(labelData, "2022.07.25");
    lv_obj_center(labelData);
    //lv_obj_align(labelData, LV_ALIGN_CENTER, 50, 0);
    lv_obj_set_grid_cell(labelData, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_t *btn_music = lv_btn_create(parent);
    lv_obj_remove_style_all(btn_music);
    lv_obj_t *image_music = lv_img_create(btn_music);
    lv_img_set_src(image_music, &icon_player_yellow);
    lv_obj_align(image_music, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_grid_cell(btn_music, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 2);
    lv_obj_add_event_cb(btn_music, top_home_music_btn_event_cb, LV_EVENT_CLICKED, 0);
    //lv_obj_remove_style_all(devbtn);

    bg_home_device = lv_create_devInfo_Anmi(parent);
    lv_obj_set_grid_cell(bg_home_device, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 4, 1);
    lv_obj_add_event_cb(bg_home_device, top_home_device_btn_event_cb, LV_EVENT_CLICKED, 0);

    tabview_home_weather = lv_create_weather_Anmi(parent);
    lv_obj_set_grid_cell(tabview_home_weather, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER, 4, 2);

    return;
}


static bool main_layer_enter_cb(void *layer)
{
    bool ret = false;
    uint32_t i;

    lv_layer_t *create_layer = layer;
    LV_LOG_USER("lv_obj_name:%s", create_layer->lv_obj_name);

    if (NULL == create_layer->lv_obj_layer) {
        ret = true;
        create_layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(create_layer->lv_obj_layer);
        lv_obj_set_size(create_layer->lv_obj_layer, BSP_LCD_H_RES, BSP_LCD_V_RES);

        titleview_home_base = lv_tileview_create(create_layer->lv_obj_layer);
        lv_obj_set_style_bg_color(titleview_home_base, lv_color_black(), 0);
        lv_obj_add_event_cb(titleview_home_base, top_titleview_event_cb, LV_EVENT_VALUE_CHANGED, 0);

        for (i = 0; i < USER_MAINMENU_MAX; i++) {

            lv_obj_t *tile_bottom =  lv_tileview_add_tile(titleview_home_base, i, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
            if (i == 0) {
                tile_bottomHead = tile_bottom;
            }

            lv_top_menu_btn[i] = lv_btn_create(tile_bottom);
            lv_obj_remove_style_all(lv_top_menu_btn[i]);
            lv_obj_set_size(lv_top_menu_btn[i], BSP_LCD_H_RES, BSP_LCD_V_RES);
            lv_obj_align(lv_top_menu_btn[i], LV_ALIGN_TOP_LEFT, 0, 0);

            lv_obj_t *lv_top_menu_bg;
            lv_top_menu_bg = lv_img_create(lv_top_menu_btn[i]);
            //lv_img_set_src(lv_top_menu_bg, &wallpaper);
            lv_img_set_src(lv_top_menu_bg, &global_bg1);
            lv_obj_align(lv_top_menu_bg, LV_ALIGN_TOP_LEFT, 0, 0);

            if (0 == i) {
                lv_create_top_home_page(lv_top_menu_btn[i]);
                //lv_obj_add_event_cb(lv_top_menu_btn[i], top_home_btn_event_cb, LV_EVENT_CLICKED, i);
            } else {
                lv_obj_t *lv_ctrl_menu_bg = lv_obj_create(lv_top_menu_bg);
                //lv_obj_remove_style_all(lv_ctrl_menu_bg);
                lv_obj_set_size(lv_ctrl_menu_bg, 362, 362);
                lv_obj_set_style_border_width(lv_ctrl_menu_bg, 0, 0);
                lv_obj_set_style_pad_all(lv_ctrl_menu_bg, 5, 0);
                lv_obj_set_style_bg_color(lv_ctrl_menu_bg, lv_color_hex(COLOUR_BG_GREY), 0);
                lv_obj_align(lv_ctrl_menu_bg, LV_ALIGN_CENTER, 0, 0);

                if (1 == i) {
                    lv_create_top_ctrl_1_page(lv_top_menu_btn[i]);
                    lv_obj_add_event_cb(lv_top_menu_btn[i], top_ctrl_btn_event_cb, LV_EVENT_CLICKED, (void *)i);
                    lv_obj_add_event_cb(lv_ctrl_menu_bg, top_ctrl_btn_event_cb, LV_EVENT_CLICKED, (void *)i);
                } else {
                    lv_create_top_ctrl_2_page(lv_top_menu_btn[i]);
                }
            }
        }

        lv_create_topTitle(create_layer->lv_obj_layer);

        if (0) {
            //if (create_layer->lv_obj_user == &ctrl_Layer) {
            lv_obj_set_tile_id(titleview_home_base, 1, 0, LV_ANIM_OFF);
        } else {
            lv_obj_set_tile_id(titleview_home_base, 0, 0, LV_ANIM_OFF);
        }
        lv_event_send(titleview_home_base, LV_EVENT_VALUE_CHANGED, NULL);
    }

    feed_clock_time();
    set_time_out(&time_1000ms, 1000);
    set_time_out(&time_1500ms, 1500);

    return ret;
}

static bool main_layer_exit_cb(void *layer)
{
    LV_LOG_USER("exit");
    return true;
}

static void main_layer_timer_cb(lv_timer_t *tmr)
{
    int temp = 0;
    char timeBuf[20];
    static uint32_t list;

    if (is_time_out(&time_1500ms)) {
        list++;
        int weatherTabviewnum = sizeof(weather_icon_list) / sizeof(weather_icon_list[0]);
        temp = (list) % (weatherTabviewnum);
        if (temp == (weatherTabviewnum - 1)) {
            lv_tabview_set_act(tabview_home_weather, 0, LV_ANIM_OFF);
        } else {
            lv_tabview_set_act(tabview_home_weather, temp, LV_ANIM_ON);
        }
        // lv_func_goto_layer(&weather_Layer);
        //lv_func_goto_layer(&main_Layer);
    }

    if (is_time_out(&time_1000ms)) {
        temp = sys_clock_getSecond();
        sprintf(timeBuf, "%02d:%02d", (temp / 60) % 24, temp % 60);
        lv_label_set_text(label_home_time, timeBuf);
        lv_label_set_text(label_top_time, timeBuf);
    }

    if (slide_enter_filter) {
        slide_enter_filter--;
    }
}
