/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "bsp/esp-bsp.h"
#include <time.h>
#include <sys/time.h>
#include "settings.h"

#include "lv_example_pub.h"
#include "lv_example_image.h"

static const char *TAG = "GUIDE";

typedef enum {
    HINT_PAGE_LANGUAGE,
    HINT_PAGE_DEMO,
    HINT_PAGE_SR_ENABLE,
    HINT_PAGE_SR_GUIDE,
    HINT_PAGE_SR_COMMAND,
    HINT_PAGE_NETWORK
} HINT_PAGE_STEP;

static bool user_guide_layer_enter_cb(void *create_layer);
static bool user_guide_layer_exit_cb(void *create_layer);
static void user_guide_layer_timer_cb(lv_timer_t *tmr);

void ui_hint_page_language(lv_obj_t *parent);
void ui_hint_page_voice(lv_obj_t *parent);
void ui_hint_page_voice_guide(lv_obj_t *parent);
void ui_hint_page_voice_cmd(lv_obj_t *parent);
void ui_hint_page_wifi(lv_obj_t *parent);
void ui_hint_page_demo_select(lv_obj_t *parent);

lv_layer_t user_guide_layer = {
    .lv_obj_name    = "user_guide_layer",
    .lv_obj_layer   = NULL,
    .enter_cb       = user_guide_layer_enter_cb,
    .exit_cb        = user_guide_layer_exit_cb,
    .timer_cb       = user_guide_layer_timer_cb,
};

static time_out_count time_500ms;
static lv_obj_t *hint_tabview;
static lv_obj_t *img_point_sr, *img_point_ui;

static void language_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int item_select = (int)lv_event_get_user_data(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        ESP_LOGI(TAG, "language select:%d", item_select);
        sys_set->sr_lang = item_select;

        ui_hint_page_demo_select(hint_tabview);
        ui_hint_page_voice(hint_tabview);
        ui_hint_page_voice_guide(hint_tabview);
        ui_hint_page_voice_cmd(hint_tabview);
        ui_hint_page_wifi(hint_tabview);

        lv_tabview_set_act(hint_tabview, HINT_PAGE_DEMO, LV_ANIM_OFF);
    }
}

static void voice_enable_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int item_select = (int)lv_event_get_user_data(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        ESP_LOGI(TAG, "voice select:%d", item_select);
        sys_set->sr_enable = item_select;
        if (sys_set->sr_enable) {
            lv_tabview_set_act(hint_tabview, HINT_PAGE_SR_GUIDE, LV_ANIM_OFF);
        } else {
            lv_tabview_set_act(hint_tabview, HINT_PAGE_NETWORK, LV_ANIM_OFF);
        }
    }
}

static void voice_guide_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED) {
        ESP_LOGI(TAG, "SR guide next.");
        lv_tabview_set_act(hint_tabview, HINT_PAGE_SR_COMMAND, LV_ANIM_OFF);
    }
}

static void voice_cmd_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED) {
        ESP_LOGI(TAG, "SR cmd next.");
        if (sys_set->demo_gui) {
            lv_tabview_set_act(hint_tabview, HINT_PAGE_NETWORK, LV_ANIM_OFF);
        } else{
            lv_func_goto_layer(&sr_layer);
        }
    }
}

static void network_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int item_select = (int)lv_event_get_user_data(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        ESP_LOGI(TAG, "newtwork select:%d", item_select);
        if (item_select) {
            set_layer.user_data = SPRITE_SET_PAGE_WiFi;
            lv_func_goto_layer(&set_layer);
        } else {
            lv_func_goto_layer(&main_Layer);
        }
    }
}

static void demo_select_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int item_select = (int)lv_event_get_user_data(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        sys_set->demo_gui = item_select;
        if (sys_set->demo_gui) {
            sys_set->sr_enable = false;
            lv_tabview_set_act(hint_tabview, HINT_PAGE_NETWORK, LV_ANIM_OFF);
        } else {
            sys_set->sr_enable = true;
            lv_tabview_set_act(hint_tabview, HINT_PAGE_SR_GUIDE, LV_ANIM_OFF);
        }
        ESP_LOGI(TAG, "demo select:%s, SR Enable:%d", sys_set->demo_gui ? "UI":"SR", sys_set->sr_enable);
    }
}

void ui_hint_page_language(lv_obj_t *parent)
{
    lv_obj_t *label_tips;

    lv_obj_t *hint_page = lv_tabview_add_tab(parent, "Language");
    lv_obj_set_scrollbar_mode(hint_page, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *btn_labguage_en = lv_btn_create(hint_page);
    lv_obj_set_size(btn_labguage_en, 100, 50);
    lv_obj_align(btn_labguage_en, LV_ALIGN_CENTER, 0, -100);
    lv_obj_add_event_cb(btn_labguage_en, language_event_handler, LV_EVENT_SHORT_CLICKED, (void *)SR_LANG_EN);

    label_tips = lv_label_create(btn_labguage_en);
    lv_obj_align(label_tips, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_style_text_color(label_tips, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_tips, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_tips, "EN");

    lv_obj_t *btn_labguage_cn = lv_btn_create(hint_page);
    lv_obj_set_size(btn_labguage_cn, 100, 50);
    lv_obj_align(btn_labguage_cn, LV_ALIGN_CENTER, 0, 100);
    lv_obj_add_event_cb(btn_labguage_cn, language_event_handler, LV_EVENT_SHORT_CLICKED, (void *)SR_LANG_CN);

    label_tips = lv_label_create(btn_labguage_cn);
    lv_obj_align(label_tips, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_style_text_color(label_tips, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_tips, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_tips, "中文");
}

void ui_hint_page_voice(lv_obj_t *parent)
{
    lv_obj_t *hint_page = lv_tabview_add_tab(parent, "SR");
    lv_obj_set_scrollbar_mode(hint_page, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *label_tips = lv_label_create(hint_page);
    lv_obj_align(label_tips, LV_ALIGN_CENTER, 0, -80);
    lv_obj_set_style_text_color(label_tips, lv_color_hex(COLOUR_GREY_BF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_tips, &SourceHanSansCN_Normal_20, 0);
    if (SR_LANG_CN == sys_set->sr_lang) {
        lv_label_set_text(label_tips, "是否开启离线语音助手?");
    } else {
        lv_label_set_text(label_tips, "Whether to enable offline voice assistant?");
    }

    lv_obj_t *btn_cancel = lv_btn_create(hint_page);
    lv_obj_set_size(btn_cancel, 100, 50);
    lv_obj_align(btn_cancel, LV_ALIGN_CENTER, -100, 60);
    lv_obj_add_event_cb(btn_cancel, voice_enable_event_handler, LV_EVENT_SHORT_CLICKED, (void *)0);

    label_tips = lv_label_create(btn_cancel);
    lv_obj_align(label_tips, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_style_text_color(label_tips, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_tips, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_tips, (SR_LANG_CN == sys_set->sr_lang) ? "否" : "No");

    lv_obj_t *btn_confirm = lv_btn_create(hint_page);
    lv_obj_set_size(btn_confirm, 100, 50);
    lv_obj_align(btn_confirm, LV_ALIGN_CENTER, 100, 60);
    lv_obj_add_event_cb(btn_confirm, voice_enable_event_handler, LV_EVENT_SHORT_CLICKED, (void *)1);

    label_tips = lv_label_create(btn_confirm);
    lv_obj_align(label_tips, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_style_text_color(label_tips, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_tips, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_tips, (SR_LANG_CN == sys_set->sr_lang) ? "是" : "Yes");
}


void ui_hint_page_voice_guide(lv_obj_t *parent)
{
    lv_obj_t *btn_next, *label;

    lv_obj_t *hint_page = lv_tabview_add_tab(parent, "STEP");
    lv_obj_set_scrollbar_mode(hint_page, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *label_title = lv_label_create(hint_page);
    lv_obj_set_style_text_color(label_title, lv_color_hex(COLOUR_GREY_BF), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label_title, &SourceHanSansCN_Normal_20, LV_STATE_DEFAULT);
    lv_label_set_recolor(label_title, true);
    if (SR_LANG_EN == sys_set->sr_lang) {
        lv_label_set_text_static(label_title, "Steps for Voice Assistant");
    } else {
        lv_label_set_text_static(label_title, "语音助手操作步骤");
    }
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 50);

    lv_obj_t *label_hint = lv_label_create(hint_page);
    lv_obj_set_style_text_line_space(label_hint, 6, LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(label_hint, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_style_text_color(label_hint, lv_color_hex(COLOUR_GREY_BF), LV_STATE_DEFAULT);

    if (SR_LANG_EN == sys_set->sr_lang) {
        lv_obj_set_style_text_font(label_hint, &SourceHanSansCN_Normal_20, LV_STATE_DEFAULT);
        static const char msg[] = "1: Say \"Hi E. S. P.\" to wake-up the device.\n"
                                  "2: Wait for the \"Hi ESP\" shows on screen.\n"
                                  "3: Say command, like \"turn on the light\".";
        lv_label_set_text_static(label_hint, msg);
    } else {
        lv_obj_set_style_text_font(label_hint, &SourceHanSansCN_Normal_20, LV_STATE_DEFAULT);
        static const char msg[] = "1: 请说 \"Hi 乐鑫\" 唤醒设备.\n"
                                  "2: 等待屏幕显示 \"请说\".\n"
                                  "3: 说出命令, 例如 \"打开电灯\".";
        lv_label_set_text_static(label_hint, msg);
    }
    lv_obj_align_to(label_hint, label_title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    btn_next = lv_btn_create(hint_page);
    lv_obj_set_size(btn_next, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_text_align(btn_next, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(btn_next, LV_ALIGN_BOTTOM_RIGHT, -10, 0);
    label = lv_label_create(btn_next);
    lv_obj_set_style_text_font(label, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label, (SR_LANG_EN == sys_set->sr_lang) ? "Next": "下一步");
    lv_obj_center(label);
    lv_obj_add_event_cb(btn_next, voice_guide_event_handler, LV_EVENT_SHORT_CLICKED, hint_tabview);
}


void ui_hint_page_voice_cmd(lv_obj_t *parent)
{
    lv_obj_t *btn_next, *label;

    lv_obj_t *hint_page = lv_tabview_add_tab(parent, "Cmd");
    lv_obj_set_scrollbar_mode(hint_page, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *label_title = lv_label_create(hint_page);
    lv_obj_set_style_text_color(label_title, lv_color_hex(COLOUR_GREY_BF), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label_title, &SourceHanSansCN_Normal_20, LV_STATE_DEFAULT);
    lv_label_set_recolor(label_title, true);
    if (SR_LANG_EN == sys_set->sr_lang) {
        lv_label_set_text_static(label_title, "Default Command Words:");
    } else {
        lv_label_set_text_static(label_title, "默认命令词");
    }
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 50);

    lv_obj_t *label_cmd_hint = lv_label_create(hint_page);
    lv_label_set_recolor(label_cmd_hint, true);
    if (SR_LANG_EN == sys_set->sr_lang) {
        lv_obj_set_style_text_font(label_cmd_hint, &SourceHanSansCN_Normal_20, LV_STATE_DEFAULT);
        static const char cmd_msg[] = "\"Switch On / Open the air conditioner\"\n"
                                      "\"Switch Off / Close the air conditioner\"\n"
                                      "\"Raise / Reduce the temperature\"\n"
                                      "\"Switch On / Turn On the Light\"\n"
                                      "\"Switch Off / Turn Off the Light\"";
        lv_label_set_text_static(label_cmd_hint, cmd_msg);
    } else {
        lv_obj_set_style_text_font(label_cmd_hint, &SourceHanSansCN_Normal_20, LV_STATE_DEFAULT);
        static const char cmd_msg[] = "\"打开空调\", \"关闭空调\"\n"
                                      "\"升高温度\", \"降低温度\"\n"
                                      "\"打开电灯\", \"关闭电灯\"";
        lv_label_set_text_static(label_cmd_hint, cmd_msg);
    }
    lv_obj_set_style_text_line_space(label_cmd_hint, 6, LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(label_cmd_hint, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_style_text_color(label_cmd_hint, lv_color_hex(COLOUR_GREY_BF), LV_STATE_DEFAULT);
    lv_obj_align_to(label_cmd_hint, label_title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    btn_next = lv_btn_create(hint_page);
    lv_obj_set_size(btn_next, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_text_align(btn_next, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(btn_next, LV_ALIGN_BOTTOM_RIGHT, -10, 0);
    label = lv_label_create(btn_next);
    lv_obj_set_style_text_font(label, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label, (SR_LANG_EN == sys_set->sr_lang) ? "Next": "下一步");
    lv_obj_center(label);
    lv_obj_add_event_cb(btn_next, voice_cmd_event_handler, LV_EVENT_SHORT_CLICKED, hint_tabview);
}

void ui_hint_page_wifi(lv_obj_t *parent)
{
    lv_obj_t *hint_page = lv_tabview_add_tab(parent, "WiFi");
    lv_obj_set_scrollbar_mode(hint_page, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *label_tips = lv_label_create(hint_page);
    lv_obj_align(label_tips, LV_ALIGN_CENTER, 0, -80);
    lv_obj_set_style_text_color(label_tips, lv_color_hex(COLOUR_GREY_BF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_tips, &SourceHanSansCN_Normal_20, 0);
    if (SR_LANG_CN == sys_set->sr_lang) {
        lv_label_set_text(label_tips, "是否连接网络?");
    } else {
        lv_label_set_text(label_tips, "Whether to connect to the network?");
    }

    lv_obj_t *btn_cancel = lv_btn_create(hint_page);
    lv_obj_set_size(btn_cancel, 100, 50);
    lv_obj_align(btn_cancel, LV_ALIGN_CENTER, -100, 60);
    lv_obj_add_event_cb(btn_cancel, network_event_handler, LV_EVENT_SHORT_CLICKED, (void *)0);

    label_tips = lv_label_create(btn_cancel);
    lv_obj_align(label_tips, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_style_text_color(label_tips, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_tips, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_tips, (SR_LANG_CN == sys_set->sr_lang) ? "否" : "No");

    lv_obj_t *btn_confirm = lv_btn_create(hint_page);
    lv_obj_set_size(btn_confirm, 100, 50);
    lv_obj_align(btn_confirm, LV_ALIGN_CENTER, 100, 60);
    lv_obj_add_event_cb(btn_confirm, network_event_handler, LV_EVENT_SHORT_CLICKED, (void *)1);

    label_tips = lv_label_create(btn_confirm);
    lv_obj_align(label_tips, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_style_text_color(label_tips, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_tips, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_tips, (SR_LANG_CN == sys_set->sr_lang) ? "是" : "Yes");
}

void ui_hint_page_demo_select(lv_obj_t *parent)
{
    lv_obj_t *hint_page = lv_tabview_add_tab(parent, "demo");
    lv_obj_set_scrollbar_mode(hint_page, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *label_tips = lv_label_create(hint_page);
    lv_obj_align(label_tips, LV_ALIGN_CENTER, 0, -120);
    lv_obj_set_style_text_color(label_tips, lv_color_hex(COLOUR_GREY_BF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_tips, &SourceHanSansCN_Normal_20, 0);
    if (SR_LANG_CN == sys_set->sr_lang) {
        lv_label_set_text(label_tips, "本出厂固件提供了两个演示: 精美的UI 和语音控制，\n请选择进入您想体验的演示");
    } else {
        lv_label_set_text(label_tips, "The firmware offers two demos: \nfancy UI and voice control. \nPlease choose the one that suits your preferences.");
    }

    lv_obj_t *btn_SR = lv_btn_create(hint_page);
    lv_obj_remove_style_all(btn_SR);
    lv_obj_set_size(btn_SR, 100, 75);
    lv_obj_align(btn_SR, LV_ALIGN_CENTER, -100, 60);
    lv_obj_add_event_cb(btn_SR, demo_select_event_handler, LV_EVENT_SHORT_CLICKED, (void *)0);

    lv_obj_t *img_ui = lv_img_create(btn_SR);
    lv_img_set_src(img_ui, &icon_voice);
    lv_obj_align(img_ui, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *btn_UI = lv_btn_create(hint_page);
    lv_obj_remove_style_all(btn_UI);
    lv_obj_set_size(btn_UI, 100, 75);
    lv_obj_align(btn_UI, LV_ALIGN_CENTER, 100, 60);
    lv_obj_add_event_cb(btn_UI, demo_select_event_handler, LV_EVENT_SHORT_CLICKED, (void *)1);

    lv_obj_t *img_UI = lv_img_create(btn_UI);
    lv_img_set_src(img_UI, &icon_display);
    lv_obj_align(img_UI, LV_ALIGN_CENTER, 0, 0);

    img_point_sr = lv_img_create(hint_page);
    lv_img_set_src(img_point_sr, &hand_down);
    lv_obj_set_style_img_recolor(img_point_sr, lv_color_hex(COLOUR_WHITE), 0);
    lv_obj_set_style_img_recolor_opa(img_point_sr, 100, 0);
    lv_obj_align_to(img_point_sr, btn_SR, LV_ALIGN_OUT_TOP_MID, 0, -10);

    img_point_ui = lv_img_create(hint_page);
    lv_img_set_src(img_point_ui, &hand_down);
    lv_obj_set_style_img_recolor(img_point_ui, lv_color_hex(COLOUR_WHITE), 0);
    lv_obj_set_style_img_recolor_opa(img_point_ui, 100, 0);
    lv_obj_align_to(img_point_ui, btn_UI, LV_ALIGN_OUT_TOP_MID, 0, -10);

    lv_obj_clear_flag(img_point_sr, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(img_point_ui, LV_OBJ_FLAG_HIDDEN);
}

void ui_hint_start(lv_obj_t *parent)
{
    /*Create a Tab view object*/
    hint_tabview = lv_tabview_create(parent, LV_DIR_TOP, 0);

    lv_obj_set_size(hint_tabview, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_radius(hint_tabview, 0, 0);
    lv_obj_clear_flag(lv_tabview_get_content(hint_tabview), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(hint_tabview, lv_color_hex(COLOUR_GREY_2F), LV_PART_MAIN);

    ui_hint_page_language(hint_tabview);
}

static bool user_guide_layer_enter_cb(void *create_layer)
{
    bool ret = false;

    lv_layer_t *layer = create_layer;
    ESP_LOGI(TAG, "Enter User Guide");

    if (NULL == layer->lv_obj_layer) {
        ret = true;

        layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_set_size(layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_border_width(layer->lv_obj_layer, 0, 0);
        lv_obj_set_style_pad_all(layer->lv_obj_layer, 0, 0);
        lv_obj_set_style_bg_color(layer->lv_obj_layer, lv_color_hex(COLOUR_GREY_BF), LV_PART_MAIN);

        ui_hint_start(layer->lv_obj_layer);
        set_time_out(&time_500ms, 1000);
    }
    return ret;
}

static bool user_guide_layer_exit_cb(void *create_layer)
{
    sys_set->need_hint = false;
    settings_write_parameter_to_nvs();
    ESP_LOGI(TAG, "exit");
    return true;
}

static void user_guide_layer_timer_cb(lv_timer_t *tmr)
{
    feed_clock_time();

    if (is_time_out(&time_500ms)) {

        if(img_point_sr && img_point_ui){
            if(lv_obj_has_flag(img_point_sr, LV_OBJ_FLAG_HIDDEN)){
                lv_obj_clear_flag(img_point_sr, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(img_point_ui, LV_OBJ_FLAG_HIDDEN);
            }else{
                lv_obj_clear_flag(img_point_ui, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(img_point_sr, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}
