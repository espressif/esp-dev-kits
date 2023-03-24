/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "bsp/esp-bsp.h"
#include <time.h>
#include <sys/time.h>
#include "settings.h"

#include "esp_err.h"
#include "esp_log.h"

#include "lv_example_pub.h"
#include "lv_example_image.h"
#include "ui_sr.h"

#define LV_DEMO_MUSIC_HANDLE_SIZE  20

typedef struct{
    const lv_img_dsc_t * addr_open;
    const lv_img_dsc_t * addr_close;
    const char *btn_name;
}warm_fan_info_list_t;

static bool show_set_layer_enter_cb(void *create_layer);
static bool show_set_layer_exit_cb(void *create_layer);
static void show_set_layer_timer_cb(lv_timer_t * tmr);

lv_layer_t show_set_layer ={
    .lv_obj_name    = "show_set_layer",
    .lv_obj_layer   = NULL,
    .enter_cb       = show_set_layer_enter_cb,
    .exit_cb        = show_set_layer_exit_cb,
    .timer_cb       = show_set_layer_timer_cb,
};

const warm_fan_info_list_t title_set_info[5] ={
    {&set_icon_temp_y,      &set_icon_temp_B,       "天气"},
    {&set_icon_screen_y,    &set_icon_screen_B,     "关闭屏幕"},
    {&set_icon_WIFI_y,      &set_icon_WIFI_B,       "WiFi"},
    {&set_icon_BLE_y,       &set_icon_BLE_B,        "蓝牙"},
    {&set_icon_set_y,       &set_icon_set_B,        "设置"},

};

static time_out_count time_500ms;
static lv_obj_t * main_cont, * setting_page;

static void ctrlBtn_set_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * button = lv_event_get_target(e);

    if(code == LV_EVENT_SHORT_CLICKED) {
        int btn = (int)lv_event_get_user_data(e);
        feed_clock_time();
        //app_sound_play();

        lv_obj_t * btnchild = lv_obj_get_child(button, 0);

        if(LV_STATE_CHECKED == (button->state&LV_STATE_CHECKED)){
            lv_obj_clear_state(button, LV_STATE_CHECKED);
            lv_img_set_src(btnchild, title_set_info[btn].addr_open);
            LV_LOG_USER("clear checked:%X", button->state&LV_STATE_CHECKED);
        }
        else {
            lv_obj_add_state(button, LV_STATE_CHECKED);
            lv_img_set_src(btnchild, title_set_info[btn].addr_close);
            LV_LOG_USER("set checked:%X", button->state&LV_STATE_CHECKED);
        }

        /* set button */
        if(4 == btn){
            lv_func_goto_layer(&set_layer);
        }
        /* screenoff button */
        else if(1 == btn){
            enter_clock_time();
        }
    }
}

static void slider_show_event_cb(lv_event_t * e)
{
   lv_obj_t * slider = lv_event_get_target(e);
   feed_clock_time();

   sys_param_t * sys_param = settings_get_parameter();
   int volume_set = (int)lv_slider_get_value(slider);
   sys_param->brightness = volume_set;
   //LV_LOG_USER("brightness:%d", volume_set);
}

static void btn_hidden_event_cb(lv_event_t * e)
{
    lv_point_t scroll_end;
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_SHORT_CLICKED) {
        lv_obj_get_scroll_end(main_cont, &scroll_end);
        if(0 == lv_obj_get_scroll_y(main_cont)){
            lv_obj_scroll_by(main_cont, 0, 0-(LV_VER_RES - 0 *  LV_DEMO_MUSIC_HANDLE_SIZE), LV_ANIM_OFF);
            LV_LOG_USER("main_cont ->%d", 0-(LV_VER_RES - 0 *  LV_DEMO_MUSIC_HANDLE_SIZE));
        }
        else{
            lv_obj_scroll_by(main_cont, 0, 0+(LV_VER_RES - 0 *  LV_DEMO_MUSIC_HANDLE_SIZE), LV_ANIM_OFF);
            LV_LOG_USER("main_cont ->%d", 0+(LV_VER_RES - 0 *  LV_DEMO_MUSIC_HANDLE_SIZE));
        }
    }
    //app_sound_play();
}

static void top_pulldown_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_point_t scroll_end;

    lv_obj_get_scroll_end(main_cont, &scroll_end);

    if(code == LV_EVENT_SCROLL_END) {

        if((480 == lv_obj_get_scroll_y(main_cont))){
            LV_LOG_USER("End TOP,[%d,%d]", lv_obj_get_scroll_y(main_cont), scroll_end.y);
            lv_obj_set_style_opa(setting_page, LV_OPA_0, 0);
        }
        else if(0 == lv_obj_get_scroll_y(main_cont)){
            LV_LOG_USER("End Bottom,[%d,%d]", lv_obj_get_scroll_y(main_cont), scroll_end.y);
        }
    }
    else if(code == LV_EVENT_SCROLL_BEGIN) {
        LV_LOG_USER("begin, [%d,%d]", lv_obj_get_scroll_y(main_cont), scroll_end.y);
        lv_obj_set_style_opa(setting_page, LV_OPA_100, 0);
    }
    else if(code == LV_EVENT_SCROLL){
        feed_clock_time();
        //lv_obj_set_style_opa(setting_page, LV_OPA_80, 0);
        //LV_LOG_USER("LV_EVENT_SCROLL, [%d:%d]", lv_obj_get_x(main_cont),lv_obj_get_y(main_cont));
    }
}

static lv_obj_t * lv_example_slider_backLight(lv_obj_t * parent)
{
    /*Create a transition*/
    static const lv_style_prop_t props[] = {LV_STYLE_BG_COLOR, 0};
    static lv_style_transition_dsc_t transition_dsc;
    lv_style_transition_dsc_init(&transition_dsc, props, lv_anim_path_linear, 300, 0, NULL);

    static lv_style_t style_main;
    static lv_style_t style_indicator;
    static lv_style_t style_pressed_color;

    lv_style_init(&style_main);
    lv_style_set_bg_opa(&style_main, LV_OPA_COVER);
    lv_style_set_bg_color(&style_main, lv_color_hex(0xFFFFFF));
    lv_style_set_radius(&style_main, LV_RADIUS_CIRCLE);
    lv_style_set_pad_ver(&style_main, -2); /*Makes the indicator larger*/

    lv_style_init(&style_indicator);//click
    lv_style_set_bg_opa(&style_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indicator, lv_color_hex(COLOUR_YELLOW));
    lv_style_set_radius(&style_indicator, LV_RADIUS_CIRCLE);
    lv_style_set_transition(&style_indicator, &transition_dsc);

    lv_style_init(&style_pressed_color);//slider
    lv_style_set_bg_color(&style_pressed_color, lv_color_hex(COLOUR_YELLOW));

    /*Create a slider and add the style*/
    lv_obj_t * slider = lv_slider_create(parent);
    lv_obj_remove_style_all(slider);        /*Remove the styles coming from the theme*/
    lv_obj_set_size(slider, lv_pct(90), 20);

    lv_obj_add_style(slider, &style_main, LV_PART_MAIN);
    lv_obj_add_style(slider, &style_indicator, LV_PART_INDICATOR);//click
    lv_obj_add_style(slider, &style_pressed_color, LV_PART_INDICATOR | LV_STATE_PRESSED);
    lv_obj_add_style(slider, &style_pressed_color, LV_PART_KNOB | LV_STATE_PRESSED);

    lv_obj_add_event_cb(slider,slider_show_event_cb,LV_EVENT_VALUE_CHANGED,NULL); //设置回调显示
    sys_param_t * sys_param = settings_get_parameter();
    lv_slider_set_value(slider, sys_param->brightness, LV_ANIM_OFF);
    //lv_slider_set_range(slider,5,100);

    return slider;
}

static void lv_create_set_pulldown(lv_obj_t * parent)
{
    int i;
    static lv_coord_t grid_2_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_2_row_dsc[] = {
                50,  /*power key*/
                50,  /*current temp*/
                50,  /*Title "current temp*/
                70,  /*Title "current temp*/
                LV_GRID_TEMPLATE_LAST
        };

    lv_obj_t *bg_setpage, *set_btn, * bg_set_btn, * label_set;

    bg_setpage = lv_list_create(parent);
    //lv_obj_remove_style_all(bg_setpage);
    lv_obj_set_style_opa(bg_setpage, LV_OPA_100, 0);
    lv_obj_set_size(bg_setpage, LV_PCT(100), LV_PCT(55));
    lv_obj_align(bg_setpage, LV_ALIGN_TOP_MID, 0, 15);

    lv_obj_set_style_bg_color(bg_setpage, lv_color_hex(COLOUR_GREY_2F), LV_PART_MAIN);
    lv_obj_set_style_border_width(bg_setpage, 0, 0);
    lv_obj_set_style_radius(bg_setpage, 0, 10);

    lv_obj_set_grid_dsc_array(bg_setpage, grid_2_col_dsc, grid_2_row_dsc);

    lv_obj_t * bg_empty = lv_list_create(parent);
    lv_obj_set_style_opa(bg_empty, LV_OPA_0, 0);
    lv_obj_set_size(bg_empty, LV_PCT(100), LV_PCT(45));
    lv_obj_align(bg_empty, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_event_cb(bg_empty, btn_hidden_event_cb, LV_EVENT_SHORT_CLICKED, bg_empty);

    for(i = 0; i< sizeof(title_set_info)/sizeof(title_set_info[0]); i++){
        set_btn = lv_btn_create(bg_setpage);
        lv_obj_remove_style_all(set_btn);
        lv_obj_set_size(set_btn, 70, 70);
        lv_obj_add_event_cb(set_btn, ctrlBtn_set_event_cb, LV_EVENT_SHORT_CLICKED, (void *)i);
        lv_obj_clear_state(set_btn, LV_STATE_CHECKED);
        lv_obj_set_grid_cell(set_btn, LV_GRID_ALIGN_CENTER, i, 1, LV_GRID_ALIGN_CENTER, 1, 1);

        bg_set_btn = lv_img_create(set_btn);
        lv_img_set_src(bg_set_btn, title_set_info[i].addr_open);
        lv_obj_align(bg_set_btn, LV_ALIGN_CENTER, 0, 0);

        label_set = lv_label_create(bg_setpage);
        lv_obj_remove_style_all(label_set);
        lv_obj_set_style_text_color(label_set, lv_color_white(), 0);
        lv_obj_set_style_text_font(label_set, font20.font, 0);

        lv_label_set_text(label_set, title_set_info[i].btn_name);
        lv_obj_set_grid_cell(label_set, LV_GRID_ALIGN_CENTER, i, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    }

    lv_obj_t * slider = lv_example_slider_backLight(bg_setpage);
    lv_obj_set_grid_cell(slider, LV_GRID_ALIGN_CENTER, 0, 5, LV_GRID_ALIGN_CENTER, 3, 1);
}

void lv_create_pulldown_bg(lv_obj_t * parent)
{
    /*A transparent container in which the setting_page section will be scrolled*/
    main_cont = lv_obj_create(parent);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_remove_style_all(main_cont);                            /*Make it transparent*/
    lv_obj_set_size(main_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_scroll_snap_y(main_cont, LV_SCROLL_SNAP_CENTER);    /*Snap the children to the center*/

    /* Create a container for the setting_page
     * size: 520 == 20 +480 +20, position:-20
     */
    setting_page = lv_obj_create(main_cont);
    lv_obj_set_size(setting_page, LV_HOR_RES, LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE * 2);
    lv_obj_set_y(setting_page, - LV_DEMO_MUSIC_HANDLE_SIZE);
    LV_LOG_INFO("setting_page, [%d:%d], y:%d",LV_HOR_RES, LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE * 2, - LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_set_style_bg_color(setting_page, lv_color_hex(COLOUR_BLACK), 0);
    lv_obj_set_style_opa(setting_page, LV_OPA_0, 0);
    lv_obj_set_style_border_width(setting_page, 0, 0);
    lv_obj_set_style_pad_all(setting_page, 0, 0);
    lv_obj_set_scroll_dir(setting_page, LV_DIR_VER);

    lv_obj_t * label_down = lv_label_create(setting_page);
    lv_obj_set_style_text_color(label_down, lv_color_hex(COLOUR_GREY_BF), 0);
    lv_obj_set_style_text_font(label_down, &lv_font_montserrat_16, 0);
    lv_label_set_text(label_down, LV_SYMBOL_DOWN);
    lv_obj_align(label_down, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_opa(label_down, LV_OPA_100, 0);

    /* A transparent placeholder below the setting_page container
     * It is used only to snap it to center.
     * size: 440 == 480 -20 -20, position:500, 20 left from bottom
     */
    lv_obj_t * placeholder = lv_obj_create(main_cont);
    lv_obj_remove_style_all(placeholder);
    lv_obj_set_size(placeholder, lv_pct(100), LV_VER_RES -2 *  LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_set_y(placeholder, LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_clear_flag(placeholder, LV_OBJ_FLAG_CLICKABLE);
    LV_LOG_INFO("placeholder, [%d:%d], y:%d",LV_HOR_RES, LV_VER_RES -2 *  LV_DEMO_MUSIC_HANDLE_SIZE, LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE);

    lv_obj_update_layout(main_cont);
    lv_obj_add_event_cb(main_cont, top_pulldown_event_cb, LV_EVENT_ALL, 0);

    lv_create_set_pulldown(setting_page);
    lv_obj_scroll_by(main_cont, 0, 0-(LV_VER_RES - 0 *  LV_DEMO_MUSIC_HANDLE_SIZE), LV_ANIM_OFF);
    return ;
}

static bool show_set_layer_enter_cb(void *layer)
{
    bool ret = false;
    lv_layer_t *create_layer = layer;
    LV_LOG_USER("Enter:%s", create_layer->lv_obj_name);

    if (NULL == create_layer->lv_obj_layer) {
        ret = true;
        lv_create_pulldown_bg(create_layer->lv_obj_parent);
    }

    return ret;
}

static bool show_set_layer_exit_cb(void *create_layer)
{
    LV_LOG_USER("exit");
    return true;
}

static void show_set_layer_timer_cb(lv_timer_t * tmr)
{
    anim_handle_cb amin_func_handle = sr_anim_task(&show_set_layer);

    if(is_time_out(&time_500ms)){
        if(amin_func_handle){
            amin_func_handle();
        }
    }
}
