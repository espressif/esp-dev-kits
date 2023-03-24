/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "bsp/esp-bsp.h"
#include <time.h>
#include <sys/time.h>
#include "settings.h"

#include "esp_wifi.h"
#include "app_wifi.h"

#include "lv_example_pub.h"
#include "lv_example_image.h"

static bool set_layer_enter_cb(void *create_layer);
static bool set_layer_exit_cb(void *create_layer);
static void set_layer_timer_cb(lv_timer_t * tmr);

static void lv_create_set_home(lv_obj_t * set_background);

static void lv_create_set_wifi(lv_obj_t * set_background);
static void lv_create_set_voice(lv_obj_t * set_background);
static void lv_create_set_bright(lv_obj_t * set_background);
static void lv_create_set_updata(lv_obj_t * set_background);
static void lv_create_set_aboutus(lv_obj_t * set_background);

static void lv_create_wifi_keyboard(lv_obj_t * set_background);
static void lv_create_display_standby(lv_obj_t * set_background);

lv_layer_t set_layer ={
    .lv_obj_name    = "set_layer",
    .lv_obj_parent  = NULL,
    .lv_obj_layer   = NULL,
    .lv_show_layer  = NULL,
    .enter_cb       = set_layer_enter_cb,
    .exit_cb        = set_layer_exit_cb,
    .timer_cb       = set_layer_timer_cb,
};

typedef enum{
    SPRITE_SET_PAGE_HOME    = 0x00,

    SPRITE_SET_PAGE_WiFi,
    SPRITE_SET_PAGE_VOICE,
    SPRITE_SET_PAGE_BRIGHT,
    //SPRITE_SET_PAGE_SWITCH,
    SPRITE_SET_PAGE_UPDATA,
    //SPRITE_SET_PAGE_ADVANCE,
    SPRITE_SET_PAGE_ABOUTUS,

    SPRITE_SET_WIFI_KEYBOARD,//8
    SPRITE_SET_DISPLAY_STANDBY,//9

    SPRITE_SET_PAGE_MAX,
    SPRITE_SET_PAGE_SWITCH,
    SPRITE_SET_PAGE_ADVANCE,
}SPRITE_SET_PAGE_LIST;

static SPRITE_SET_PAGE_LIST sprite_focus_page = SPRITE_SET_PAGE_MAX;
static uint8_t ssid_list_select = 0xFF;

static sprite_create_func_t sprite_create_list[] = {
    {"设置",        lv_create_set_home,    NULL},

    {"WiFi",        lv_create_set_wifi,    NULL},
    {"声音",        lv_create_set_voice,   NULL},
    {"亮度与显示",  lv_create_set_bright,  NULL},
    {"升级",        lv_create_set_updata,  NULL},
    {"关于",        lv_create_set_aboutus, NULL},

    {"SSID",        lv_create_wifi_keyboard, NULL},//8
    {"待机时间",    lv_create_display_standby, NULL},
};

const char * standby_list_name[]={"15秒", "5分钟", "1小时", "永久"};

static uint8_t sprite_focus_page_goto(uint8_t sprite)
{
	if(sprite^sprite_focus_page){
		sprite_focus_page = sprite;
		for(int i= 0; i< SPRITE_SET_PAGE_MAX; i++){
			if(sprite_create_list[i].sprite_parent){
				lv_obj_del_async(sprite_create_list[i].sprite_parent);
				sprite_create_list[i].sprite_parent = NULL;
			}
		}
		sprite_create_list[sprite].sprite_parent = lv_btn_create(set_layer.lv_obj_layer);
		sprite_create_list[sprite].sprite_create_func(sprite_create_list[sprite].sprite_parent);
	}

    return sprite_focus_page;

}

static void home_list_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int item_select = (int)lv_event_get_user_data(e);

    if(code == LV_EVENT_SHORT_CLICKED) {
        //app_sound_play();
        LV_LOG_USER("LV_EVENT_SHORT_CLICKED, item_select:%d->%d", sprite_focus_page, item_select);
        sprite_focus_page_goto(item_select);
    }
}

static void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = (lv_obj_t * )lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
        LV_LOG_USER("FOCUSED:%d", code);
    }

    if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        LV_LOG_USER("DEFOCUSED:%d", code);
    }

    //lv_keyboard_def_event_cb();
    if(code == LV_EVENT_READY) {
        LV_LOG_USER("LV_EVENT_READY:%s,%s", \
        scan_info_result.ap_info[ssid_list_select].ssid, lv_textarea_get_text(ta));

        sys_param_t * sys_param = settings_get_parameter();

        sys_param->ssid_len = strlen((char *)scan_info_result.ap_info[ssid_list_select].ssid);
        memset(sys_param->ssid, 0, sizeof(sys_param->ssid));
        memcpy(sys_param->ssid, scan_info_result.ap_info[ssid_list_select].ssid, sys_param->ssid_len);

        sys_param->password_len = strlen(lv_textarea_get_text(ta));
        memset(sys_param->password, 0, sizeof(sys_param->password));
        memcpy(sys_param->password, lv_textarea_get_text(ta), sys_param->password_len);

        settings_write_parameter_to_nvs();
        send_network_event(NET_EVENT_RECONNECT);
        sprite_focus_page_goto(SPRITE_SET_PAGE_WiFi);
    }

    if(code == LV_EVENT_CANCEL) {
        LV_LOG_USER("LV_EVENT_CANCEL");
    }
}

static void wifi_list_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int item_select = (int)lv_event_get_user_data(e);

    if(code == LV_EVENT_SHORT_CLICKED) {
        ssid_list_select = item_select;
        //app_sound_play();
        LV_LOG_USER("LV_EVENT_SHORT_CLICKED:%d,%s", item_select, scan_info_result.ap_info[ssid_list_select].ssid);
        sprite_focus_page_goto(SPRITE_SET_WIFI_KEYBOARD);
    }
}

static void display_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int item_select = (int)lv_event_get_user_data(e);

    if(code == LV_EVENT_SHORT_CLICKED) {
        //app_sound_play();
        LV_LOG_USER("LV_EVENT_SHORT_CLICKED:%d", item_select);
        if(1 == item_select){
            sprite_focus_page_goto(SPRITE_SET_DISPLAY_STANDBY);
        }
    }
}

static void btn_back_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int sprite = (int)lv_event_get_user_data(e);

    if(code == LV_EVENT_SHORT_CLICKED) {
        //app_sound_play();

        switch(sprite)
        {
            case SPRITE_SET_PAGE_HOME:
            lv_func_goto_layer(&main_Layer);
            break;
            case SPRITE_SET_WIFI_KEYBOARD:
            sprite_focus_page_goto(SPRITE_SET_PAGE_WiFi);
            break;
            case SPRITE_SET_DISPLAY_STANDBY:
            sprite_focus_page_goto(SPRITE_SET_PAGE_BRIGHT);
            break;
            default:
            sprite_focus_page_goto(SPRITE_SET_PAGE_HOME);
            break;
        }
    }
}

static void slider_show_event_cb(lv_event_t * e)
{
    feed_clock_time();
    int volume_set = 0;
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_VALUE_CHANGED){
        lv_obj_t * slider = lv_event_get_target(e);
        volume_set = (int)lv_slider_get_value(slider);
        sys_param_t * sys_param = settings_get_parameter();
        sys_param->brightness = volume_set;
        LV_LOG_USER("brightness:%d", volume_set);
    }
}

static void volume_event_handler(lv_event_t * e)
{
    int * active_id = (int *)lv_event_get_user_data(e);
    lv_obj_t * cont = lv_event_get_current_target(e);
    lv_obj_t * act_cb = lv_event_get_target(e);
    lv_obj_t * old_cb = lv_obj_get_child(cont, *active_id);

    /*Do nothing if the container was clicked*/
    if(act_cb == cont){
        LV_LOG_USER("act_cb == cont");
        return;
    }

    lv_obj_clear_state(old_cb, LV_STATE_CHECKED);   /*Uncheck the previous radio button*/
    lv_obj_add_state(act_cb, LV_STATE_CHECKED);     /*Uncheck the current radio button*/

    *active_id = lv_obj_get_index(act_cb);
    LV_LOG_USER("Selected radio buttons: %d", *active_id);

    sys_param_t * sys_param = settings_get_parameter();
    if(sys_param->volume^(*active_id)){
        sys_param->volume = *active_id;
        //settings_write_parameter_to_nvs();
        LV_LOG_USER("save volume:%d", sys_param->volume);
    }
    //es8311_codec_set_voice_volume(60 + (*active_id)*4);
    //app_sound_play();
}

static void standby_event_handler(lv_event_t * e)
{
    bool valid_btn = false;
    lv_obj_t ** checkboxList = (lv_obj_t **)lv_event_get_user_data(e);
    lv_obj_t * act_cb = lv_event_get_target(e);//checkbox

    for(int i = 0; i < 4; i++){
        if(*(checkboxList + i) == act_cb){
            valid_btn = true;
        }
    }

    if(valid_btn){
        for(int i = 0; i < 4; i++){
            if(*(checkboxList + i) != act_cb){
                lv_obj_clear_state(*(checkboxList + i), LV_STATE_CHECKED);
            }
            else{
                lv_obj_add_state(*(checkboxList + i), LV_STATE_CHECKED);
                sys_param_t * sys_param = settings_get_parameter();
                sys_param->standby_time = i;
            }
        }
    }
    //app_sound_play();
}

static void mute_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("State: %s", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
        if(lv_obj_has_state(obj, LV_STATE_CHECKED)){
            sys_param_t * sys_param = settings_get_parameter();
            //es8311_codec_set_voice_volume(60 + (sys_param->volume)*4);
            LV_LOG_USER("recover volume:%d", (60 + (sys_param->volume)*4));
            //es8311_set_voice_mute(false);
            //app_sound_play();
        }
        else{
            //es8311_set_voice_mute(true);
        }
    }
}

void set_button_list_style(lv_obj_t * parent)
{
    lv_obj_set_size(parent, LV_PCT(95), 70);
    lv_obj_set_style_bg_color(parent, lv_color_hex(COLOUR_BLACK), LV_PART_MAIN);
    lv_obj_set_style_text_color(parent, lv_color_hex(COLOUR_WHITE), 0);
    lv_obj_set_style_border_color(parent, lv_color_hex(COLOUR_GREY_4F), 0);
    //lv_obj_set_style_transform_width(btn, LV_PCT(90), 0);
    lv_obj_set_style_border_width(parent, 1, 0);
    lv_obj_set_style_border_side(parent, LV_BORDER_SIDE_BOTTOM, 0);
}

lv_obj_t * create_button_list_page(lv_obj_t * parent)
{
    lv_obj_t * list_select = lv_list_create(parent);
    lv_obj_set_size(list_select, LV_PCT(95), LV_PCT(90));
    lv_obj_set_style_border_width(list_select, 0, 0);
    lv_obj_set_style_radius(list_select, 0, 0);
    lv_obj_set_style_text_font(list_select, font20.font, 0);
    lv_obj_align(list_select, LV_ALIGN_TOP_MID, 0, 70);
    lv_obj_set_style_bg_color(list_select, lv_color_hex(COLOUR_BLACK), LV_PART_MAIN);

    lv_obj_set_style_bg_opa(list_select, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(list_select, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);

    return list_select;
}

void lv_create_set_title(lv_obj_t * parent, const char * title_name, SPRITE_SET_PAGE_LIST page)
{
    lv_obj_set_size(parent, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_radius(parent, 0, 0);
    lv_obj_set_style_bg_color(parent, lv_color_hex(COLOUR_BLACK), LV_PART_MAIN);

    lv_obj_t * btn_set_tittle = lv_btn_create(parent);
    lv_obj_remove_style_all(btn_set_tittle);
    lv_obj_align(btn_set_tittle, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_size(btn_set_tittle, 350, 60);

    lv_obj_t * label_title = lv_label_create(btn_set_tittle);
    lv_obj_set_style_text_color(label_title, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_title, font20.font, 0);
    lv_label_set_text(label_title, title_name);
    lv_obj_center(label_title);

    lv_obj_t * btn_set_back = lv_btn_create(parent);
    //lv_obj_remove_style_all(btn_set_back);
    lv_obj_align(btn_set_back, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(btn_set_back, 100, 60);
    lv_obj_set_style_bg_color(btn_set_back, lv_color_hex(COLOUR_BLACK), LV_PART_MAIN);
    lv_obj_add_event_cb(btn_set_back, btn_back_event_cb, LV_EVENT_SHORT_CLICKED, (void * )page);

    lv_obj_t * labelReturn = lv_label_create(btn_set_back);
    lv_obj_remove_style_all(labelReturn);
    lv_obj_set_style_text_color(labelReturn, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelReturn, &lv_font_montserrat_16, 0);
    lv_label_set_text(labelReturn, LV_SYMBOL_LEFT);
    lv_obj_center(labelReturn);
}

static void lv_create_set_home(lv_obj_t * set_background)
{
    uint8_t focused = SPRITE_SET_PAGE_HOME;
    lv_create_set_title(set_background, sprite_create_list[focused].set_list_name, focused);/*0-60*/

    /*Add buttons to the list*/
    lv_obj_t * btn, * label_enter, *label_item;

    lv_obj_t * list_select = create_button_list_page(set_background);/*LV_PCT(90) 70 Top*/

    for(int i = SPRITE_SET_PAGE_WiFi; i<= SPRITE_SET_PAGE_ABOUTUS + 1; i++){

        btn = lv_list_add_btn(list_select, NULL, NULL);
        lv_obj_remove_style_all(btn);
        set_button_list_style(btn);
        if(i > SPRITE_SET_PAGE_ABOUTUS){
            lv_obj_set_style_border_side(btn, LV_BORDER_SIDE_NONE, 0);
            continue;
        }
        lv_obj_add_event_cb(btn, home_list_event_handler, LV_EVENT_SHORT_CLICKED, (void *)i);

        label_item = lv_label_create(btn);
        lv_obj_set_style_text_font(label_item, font20.font, 0);
        lv_label_set_text(label_item, sprite_create_list[i].set_list_name);
        lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

        label_enter = lv_label_create(btn);
        lv_obj_set_style_text_font(label_enter, &lv_font_montserrat_16, 0);
        lv_label_set_text(label_enter, LV_SYMBOL_RIGHT);
        lv_obj_align(label_enter, LV_ALIGN_RIGHT_MID, 0, 0);
    }
    return;
}

static void lv_create_wifi_keyboard(lv_obj_t * set_background)
{
    lv_create_set_title(set_background, (char *)scan_info_result.ap_info[ssid_list_select].ssid, SPRITE_SET_WIFI_KEYBOARD);

    /*Create a keyboard to use it with an of the text areas*/
    lv_obj_t * kb = lv_keyboard_create(set_background);
    lv_obj_set_width(kb, LV_PCT(95));

    /*Create a text area. The keyboard will write here*/
    lv_obj_t * ta;
    ta = lv_textarea_create(set_background);
    lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 120);
    lv_obj_set_size(ta, LV_PCT(95), LV_PCT(10));
    lv_obj_set_style_text_font(ta, font20.font, 0);

    lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);

    sys_param_t * sys_param = settings_get_parameter();
    if(0 == strcmp((char *)scan_info_result.ap_info[ssid_list_select].ssid, (char *)sys_param->ssid)){
        lv_textarea_set_placeholder_text(ta, (const char *)sys_param->password);
    }
    else{
        lv_textarea_set_placeholder_text(ta, "请输入密码");
    }
    lv_keyboard_set_textarea(kb, ta);
}

static void lv_create_display_standby(lv_obj_t * set_background)
{
    uint8_t focused = SPRITE_SET_DISPLAY_STANDBY;
    lv_create_set_title(set_background, sprite_create_list[focused].set_list_name, focused);/*0-60*/
    /*Add buttons to the list*/
    lv_obj_t * btn, *label_item;

    lv_obj_t * list_select = create_button_list_page(set_background);/*LV_PCT(90) 70 Top*/

    //static uint32_t active_index_1;
    static lv_obj_t * btn_checkbox[4];

    lv_obj_add_event_cb(list_select, standby_event_handler, LV_EVENT_CLICKED, &btn_checkbox[0]);

    for(int i = 0; i< sizeof(standby_list_name)/sizeof(standby_list_name[0]); i++){

        btn = lv_list_add_btn(list_select, NULL, NULL);
        lv_obj_remove_style_all(btn);
        set_button_list_style(btn);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_EVENT_BUBBLE);

        label_item = lv_label_create(btn);
        lv_obj_set_style_text_color(label_item, lv_color_hex(COLOUR_WHITE), 0);
        lv_obj_set_style_text_font(label_item, font20.font, 0);
        lv_label_set_text(label_item, standby_list_name[i]);
        lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

        lv_obj_t * obj = lv_checkbox_create(btn);
        lv_checkbox_set_text(obj, " ");
        lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
        // lv_obj_add_style(obj, &style_radio, LV_PART_INDICATOR);
        // lv_obj_add_style(obj, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_align(obj, LV_ALIGN_RIGHT_MID, 0, 0);

        btn_checkbox[i] = obj;
    }
    sys_param_t * sys_param = settings_get_parameter();
    lv_obj_add_state(btn_checkbox[sys_param->standby_time], LV_STATE_CHECKED);

    return;
}

static void lv_create_set_wifi(lv_obj_t * set_background)
{
    uint8_t focused = SPRITE_SET_PAGE_WiFi;
    lv_create_set_title(set_background, sprite_create_list[focused].set_list_name, focused);/*0-60*/

    lv_obj_t * btn, * label_enter, *label_item;

    lv_obj_t * list_select = create_button_list_page(set_background);

    sys_param_t * sys_param = settings_get_parameter();
    for(int i =0; i< scan_info_result.ap_count; i++){
        if((0 == strcmp((char *)sys_param->ssid, (char *)scan_info_result.ap_info[i].ssid)) && (i > 0)){
            uint8_t replace_ssid[32] ={0};
            memcpy(replace_ssid, scan_info_result.ap_info[0].ssid, sizeof(replace_ssid));
            memcpy(scan_info_result.ap_info[0].ssid, \
                    scan_info_result.ap_info[i].ssid, sizeof(replace_ssid));
            memcpy(scan_info_result.ap_info[i].ssid, replace_ssid, sizeof(replace_ssid));

            LV_LOG_USER("replace[%d->0:%s]", i, scan_info_result.ap_info[i].ssid);
        }
    }

    for(int i = 0; i< scan_info_result.ap_count +1 ; i++){

        btn = lv_list_add_btn(list_select, NULL, NULL);
        lv_obj_remove_style_all(btn);
        set_button_list_style(btn);
        lv_obj_set_size(btn, LV_PCT(95), 60);

        if(i >= scan_info_result.ap_count){
            lv_obj_set_style_border_side(btn, LV_BORDER_SIDE_NONE, 0);
            continue;
        }
        lv_obj_add_event_cb(btn, wifi_list_event_handler, LV_EVENT_SHORT_CLICKED, (void *)i);

        label_item = lv_label_create(btn);
        lv_obj_set_style_text_font(label_item, font20.font, 0);

        if(0 == strcmp((char *)scan_info_result.ap_info[i].ssid, (char *)sys_param->ssid)){

            char ssid_conected[32 + 10];
            memset(ssid_conected, 0, sizeof(ssid_conected));
            sprintf(ssid_conected, "%s %s",  scan_info_result.ap_info[i].ssid, \
                                ((true == wifi_connected_already()) ? "已连接":"未连接"));

            lv_label_set_text(label_item, ssid_conected);
            lv_obj_set_style_text_color(label_item, lv_color_hex(COLOUR_YELLOW), 0);
        }
        else{
            lv_label_set_text(label_item, (char *)scan_info_result.ap_info[i].ssid);
        }
        lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

        /*
        label_rssi = lv_label_create(btn);
        lv_obj_set_style_text_font(label_enter, font20.font, 0);
        uint8_t rssi_text[20];
        memset(rssi_text,0, sizeof(rssi_text));
        sprintf(rssi_text, "%d", scan_info_result.ap_info[i].rssi);
        lv_label_set_text(label_rssi, rssi_text);
        lv_obj_align(label_rssi, LV_ALIGN_RIGHT_MID, -40, 0);
        */

        label_enter = lv_label_create(btn);
        lv_obj_set_style_text_font(label_enter, &lv_font_montserrat_16, 0);
        lv_label_set_text(label_enter, LV_SYMBOL_RIGHT);
        lv_obj_align(label_enter, LV_ALIGN_RIGHT_MID, 0, 0);
    }
}

void create_sound_volume(lv_obj_t * parent)
{
    static uint32_t active_index_1 = 0;

    /* The idea is to enable `LV_OBJ_FLAG_EVENT_BUBBLE` on checkboxes and process the
     * `LV_EVENT_CLICKED` on the container.
     * A variable is passed as event user data where the index of the active
     * radiobutton is saved */

    uint32_t i;

    lv_obj_t * cont_volume = lv_obj_create(parent);
    lv_obj_set_flex_flow(cont_volume, LV_FLEX_FLOW_ROW);
    lv_obj_set_size(cont_volume, lv_pct(75), lv_pct(100));
    lv_obj_add_event_cb(cont_volume, volume_event_handler, LV_EVENT_CLICKED, &active_index_1);

    lv_obj_center(cont_volume);
    lv_obj_set_style_bg_color(cont_volume, lv_color_hex(COLOUR_BLACK), LV_PART_MAIN);
    lv_obj_set_style_border_width(cont_volume, 0, 0);
    lv_obj_set_style_bg_opa(cont_volume, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cont_volume, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_clear_flag(cont_volume, LV_OBJ_FLAG_SCROLLABLE);

    for(i = 0; i < 5; i++) {
        lv_obj_t * obj = lv_checkbox_create(cont_volume);
        lv_checkbox_set_text(obj, " ");
        lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
        // lv_obj_add_style(obj, &style_radio, LV_PART_INDICATOR);
        // lv_obj_add_style(obj, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
    }
    /*Make the first checkbox checked*/
    sys_param_t * sys_param = settings_get_parameter();
    if((sys_param->volume < 6) && (sys_param->volume >0)){
        lv_obj_add_state(lv_obj_get_child(cont_volume, sys_param->volume -1), LV_STATE_CHECKED);
        active_index_1 = sys_param->volume -1;
    }
    else{
        lv_obj_add_state(lv_obj_get_child(cont_volume, 5 - 1), LV_STATE_CHECKED);
        active_index_1 = 4;
    }
}

void create_bright_slider(lv_obj_t * parent)
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
    lv_style_set_bg_color(&style_main, lv_color_hex(COLOUR_GREY_8F));
    lv_style_set_radius(&style_main, LV_RADIUS_CIRCLE);
    lv_style_set_pad_ver(&style_main, -2); /*Makes the indicator larger*/

    lv_style_init(&style_indicator);//click
    lv_style_set_bg_opa(&style_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indicator, lv_color_hex(COLOUR_WHITE));
    lv_style_set_radius(&style_indicator, LV_RADIUS_CIRCLE);
    lv_style_set_transition(&style_indicator, &transition_dsc);

    lv_style_init(&style_pressed_color);//slider
    lv_style_set_bg_color(&style_pressed_color, lv_color_hex(COLOUR_WHITE));

    /*Create a slider and add the style*/
    lv_obj_t * slider = lv_slider_create(parent);
    lv_obj_remove_style_all(slider);        /*Remove the styles coming from the theme*/
    lv_obj_set_size(slider, lv_pct(80), 10);
    lv_obj_center(slider);

    lv_obj_add_style(slider, &style_main, LV_PART_MAIN);
    lv_obj_add_style(slider, &style_indicator, LV_PART_INDICATOR);//click
    lv_obj_add_style(slider, &style_pressed_color, LV_PART_INDICATOR | LV_STATE_PRESSED);
    lv_obj_add_style(slider, &style_pressed_color, LV_PART_KNOB | LV_STATE_PRESSED);

    lv_obj_add_event_cb(slider, slider_show_event_cb, LV_EVENT_VALUE_CHANGED,NULL);
    sys_param_t * sys_param = settings_get_parameter();
    lv_slider_set_value(slider, sys_param->brightness, LV_ANIM_OFF);
    //lv_slider_set_range(slider,5,100);
}

static void lv_create_set_voice(lv_obj_t * set_background)
{
    uint8_t focused = SPRITE_SET_PAGE_VOICE;
    lv_create_set_title(set_background, sprite_create_list[focused].set_list_name, focused);/*0-60*/

    /*Add buttons to the list*/
    lv_obj_t * btn, *label_item;

    lv_obj_t * list_select = create_button_list_page(set_background);
    lv_obj_set_size(list_select, LV_PCT(95), LV_PCT(90));
    /*
    Title
    */
    btn = lv_list_add_btn(list_select, NULL, NULL);
    lv_obj_remove_style_all(btn);
    set_button_list_style(btn);

    label_item = lv_label_create(btn);
    lv_obj_set_style_text_font(label_item, font20.font, 0);
    lv_label_set_text(label_item, "静音");
    lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *sw = lv_switch_create(btn);
    lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_align(sw, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_event_cb(sw, mute_event_handler, LV_EVENT_ALL, NULL);
    /*
    slider
    */
    btn = lv_list_add_btn(list_select, NULL, NULL);
    lv_obj_remove_style_all(btn);
    set_button_list_style(btn);

    lv_obj_t *label_left = lv_label_create(btn);
    lv_obj_set_style_text_font(label_left, &lv_font_montserrat_16, 0);
    lv_label_set_text(label_left, LV_SYMBOL_VOLUME_MID);
    lv_obj_align(label_left, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *label_right = lv_label_create(btn);
    lv_obj_set_style_text_font(label_right, &lv_font_montserrat_16, 0);
    lv_label_set_text(label_right, LV_SYMBOL_VOLUME_MAX);
    lv_obj_align(label_right, LV_ALIGN_RIGHT_MID, 0, 0);

    create_sound_volume(btn);
}

static void lv_create_set_bright(lv_obj_t * set_background)
{
    uint8_t focused = SPRITE_SET_PAGE_BRIGHT;
    lv_create_set_title(set_background, sprite_create_list[focused].set_list_name, focused);/*0-60*/

    /*Add buttons to the list*/
    lv_obj_t * btn, * label_enter, *label_item;

    lv_obj_t * list_select = create_button_list_page(set_background);
    lv_obj_set_size(list_select, LV_PCT(95), LV_PCT(90));
    /*
    Title
    */
    btn = lv_list_add_btn(list_select, NULL, NULL);
    lv_obj_remove_style_all(btn);
    set_button_list_style(btn);

    label_item = lv_label_create(btn);
    lv_obj_set_style_text_font(label_item, font20.font, 0);
    lv_label_set_text(label_item, "亮度调节");
    lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

    /*
    slider
    */
    btn = lv_list_add_btn(list_select, NULL, NULL);
    lv_obj_remove_style_all(btn);
    set_button_list_style(btn);

    lv_obj_t *label_left = lv_label_create(btn);
    lv_obj_set_style_text_font(label_left, &lv_font_montserrat_16, 0);
    lv_label_set_text(label_left, LV_SYMBOL_MINUS);
    lv_obj_align(label_left, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *label_right = lv_label_create(btn);
    lv_obj_set_style_text_font(label_right, &lv_font_montserrat_16, 0);
    lv_label_set_text(label_right, LV_SYMBOL_PLUS);
    lv_obj_align(label_right, LV_ALIGN_RIGHT_MID, 0, 0);

    create_bright_slider(btn);
    /*
    Empty
    */
    btn = lv_list_add_btn(list_select, NULL, NULL);
    lv_obj_remove_style_all(btn);
    //set_button_list_style(btn);
    lv_obj_set_size(btn, LV_PCT(100), 40);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(COLOUR_GREY_BF), LV_PART_MAIN);

    /*
    待机首页
    */
    btn = lv_list_add_btn(list_select, NULL, NULL);
    lv_obj_remove_style_all(btn);
    set_button_list_style(btn);
    lv_obj_add_event_cb(btn, display_event_handler, LV_EVENT_SHORT_CLICKED, 0);

    label_item = lv_label_create(btn);
    lv_obj_set_style_text_font(label_item, font20.font, 0);
    lv_label_set_text(label_item, "屏幕首页");
    lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

    label_enter = lv_label_create(btn);
    lv_obj_set_style_text_font(label_enter, font20.font, 0);
    lv_obj_set_style_text_color(label_enter, lv_color_hex(COLOUR_GREY_8F), LV_PART_MAIN);
    lv_label_set_text(label_enter, "页1");
    lv_obj_align(label_enter, LV_ALIGN_LEFT_MID, 300, 0);

    label_enter = lv_label_create(btn);
    lv_obj_set_style_text_font(label_enter, &lv_font_montserrat_16, 0);
    lv_label_set_text(label_enter, LV_SYMBOL_RIGHT);
    lv_obj_align(label_enter, LV_ALIGN_RIGHT_MID, 0, 0);

    /*
    待机时间
    */
    btn = lv_list_add_btn(list_select, NULL, NULL);
    lv_obj_remove_style_all(btn);
    set_button_list_style(btn);
    lv_obj_add_event_cb(btn, display_event_handler, LV_EVENT_SHORT_CLICKED, (void *)1);

    label_item = lv_label_create(btn);
    lv_obj_set_style_text_font(label_item, font20.font, 0);
    lv_label_set_text(label_item, "待机时间");
    lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

    label_enter = lv_label_create(btn);
    lv_obj_set_style_text_font(label_enter, font20.font, 0);
    lv_obj_set_style_text_color(label_enter, lv_color_hex(COLOUR_GREY_8F), LV_PART_MAIN);
    sys_param_t * sys_param = settings_get_parameter();
    lv_label_set_text(label_enter, standby_list_name[sys_param->standby_time]);
    lv_obj_align(label_enter, LV_ALIGN_LEFT_MID, 300, 0);

    label_enter = lv_label_create(btn);
    lv_obj_set_style_text_font(label_enter, &lv_font_montserrat_16, 0);
    lv_label_set_text(label_enter, LV_SYMBOL_RIGHT);
    lv_obj_align(label_enter, LV_ALIGN_RIGHT_MID, 0, 0);
}

static void lv_create_set_updata(lv_obj_t * set_background)
{
    uint8_t focused = SPRITE_SET_PAGE_UPDATA;
    lv_create_set_title(set_background, sprite_create_list[focused].set_list_name, focused);/*0-60*/

    /*Add buttons to the list*/
    lv_obj_t * btn, * label_enter, *label_item;

    lv_obj_t * list_select = create_button_list_page(set_background);
    lv_obj_set_size(list_select, LV_PCT(95), LV_PCT(90));

    /*
    当前版本
    */
    btn = lv_list_add_btn(list_select, NULL, NULL);
    lv_obj_remove_style_all(btn);
    set_button_list_style(btn);
    lv_obj_set_size(btn, LV_PCT(95), 200);
    lv_obj_add_event_cb(btn, NULL, LV_EVENT_SHORT_CLICKED, 0);

    label_item = lv_label_create(btn);
    lv_obj_set_style_text_font(label_item, font20.font, 0);
    char sys_version[50];
    sprintf(sys_version, "当前版本:%d.%d.%d_%s", LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR, LVGL_VERSION_PATCH, __TIME__);

    lv_label_set_text(label_item, sys_version);
    lv_obj_align(label_item, LV_ALIGN_CENTER, 0, 0);

    /*
    版本信息
    */
    btn = lv_list_add_btn(list_select, NULL, NULL);
    lv_obj_remove_style_all(btn);
    set_button_list_style(btn);
    lv_obj_add_event_cb(btn, NULL, LV_EVENT_SHORT_CLICKED, 0);

    label_item = lv_label_create(btn);
    lv_obj_set_style_text_font(label_item, &lv_font_montserrat_16, 0);
    lv_label_set_text(label_item, LV_SYMBOL_REFRESH);
    lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

    label_enter = lv_label_create(btn);
    lv_obj_set_style_text_font(label_enter, font20.font, 0);
    lv_obj_set_style_text_color(label_enter, lv_color_hex(COLOUR_GREY_8F), LV_PART_MAIN);
    lv_label_set_text(label_enter, "已是最新版本");
    lv_obj_align(label_enter, LV_ALIGN_RIGHT_MID, 0, 0);
}

static void lv_create_set_aboutus(lv_obj_t * set_background)
{
    const char * aboutus_list_name[]={
    "设备名称",
    "ESPRESSIF",

    "设备ID",
    "7864E6010203",

    "设备型号",
    "86demo emoji",

    "序列号",
    "45455645412",

    "WLAN地址",
    "78:64:E6:01:02:03",
    };

    uint8_t focused = SPRITE_SET_PAGE_ABOUTUS;
    lv_create_set_title(set_background, sprite_create_list[focused].set_list_name, focused);/*0-60*/
    /*Add buttons to the list*/
    lv_obj_t * btn, * label_enter, *label_item;

    lv_obj_t * list_select = create_button_list_page(set_background);/*LV_PCT(90) 70 Top*/

    uint8_t eth_mac[6];
    char service_name[20];
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);

    for(int i = 0; i< sizeof(aboutus_list_name)/sizeof(aboutus_list_name[0]);){

        btn = lv_list_add_btn(list_select, NULL, NULL);
        lv_obj_remove_style_all(btn);
        set_button_list_style(btn);

        label_item = lv_label_create(btn);
        lv_obj_set_style_text_color(label_item, lv_color_hex(COLOUR_WHITE), 0);
        lv_obj_set_style_text_font(label_item, font20.font, 0);
        lv_label_set_text(label_item, aboutus_list_name[i + 0]);
        lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

        label_enter = lv_label_create(btn);
        lv_obj_set_style_text_color(label_enter, lv_color_hex(0xAFAFAF), 0);
        lv_obj_set_style_text_font(label_enter, font20.font, 0);
        if(1 == i/2){
            memset(service_name, 0, sizeof(service_name));
            snprintf(service_name, sizeof(service_name), "%02x%02x%02x%02x%02x%02x",\
                eth_mac[0], eth_mac[1], eth_mac[2],\
                eth_mac[3], eth_mac[4], eth_mac[5]);
            lv_label_set_text(label_enter, service_name);
        }
        else if(4 == i/2){
            memset(service_name, 0, sizeof(service_name));
            snprintf(service_name, sizeof(service_name), "%02x:%02x:%02x:%02x:%02x:%02x",\
                eth_mac[0], eth_mac[1], eth_mac[2],\
                eth_mac[3], eth_mac[4], eth_mac[5]);
            lv_label_set_text(label_enter, service_name);
        }
        else{
            lv_label_set_text(label_enter, aboutus_list_name[i + 1]);
        }
        lv_obj_align(label_enter, LV_ALIGN_RIGHT_MID, 0, 0);
        i +=2;
    }
    return;
}

static bool set_layer_enter_cb(void *create_layer)
{
	bool ret = false;

    lv_layer_t *layer = create_layer;
	if(NULL == layer->lv_obj_layer){
		ret = true;
		layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(layer->lv_obj_layer);
        lv_obj_set_size(layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);

        sprite_focus_page_goto(SPRITE_SET_PAGE_HOME);
	}

	return ret;
}

static bool set_layer_exit_cb(void *create_layer)
{
    sprite_focus_page = SPRITE_SET_PAGE_MAX;
    for(int i= 0; i< SPRITE_SET_PAGE_MAX; i++){
        if(sprite_create_list[i].sprite_parent){
            sprite_create_list[i].sprite_parent = NULL;
		}
    }
    return true;
}

static void set_layer_timer_cb(lv_timer_t * tmr)
{
}
