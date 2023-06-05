/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "lv_example_pub.h"
#include "lv_example_image.h"
#include "settings.h"

static const char *TAG = "pullpage";

#define LV_HANDLE_SIZE  20

static bool show_set_layer_enter_cb(void *layer);
static bool show_set_layer_exit_cb(void *layer);
static void show_set_layer_timer_cb(lv_timer_t *tmr);

lv_layer_t show_set_layer = {
    .lv_obj_name    = "show_set_layer",
    .lv_obj_layer   = NULL,
    .enter_cb       = show_set_layer_enter_cb,
    .exit_cb        = show_set_layer_exit_cb,
    .timer_cb       = show_set_layer_timer_cb,
};

typedef enum {
    SET_FUNC_SCREENOFF,
    SET_FUNC_WIFI,
    SET_FUNC_BLE,
    SET_FUNC_SET,
} set_fun_t;

typedef struct {
    const lv_img_dsc_t *img_checked;
    const lv_img_dsc_t *img_unchecked;
    const char *btn_name[2];
} warm_fan_info_list_t;

const warm_fan_info_list_t title_set_info[4] = {
    {&set_icon_screen_y,    &set_icon_screen_B,     {"ScreenOff", "关闭屏幕"}},
    {&set_icon_WIFI_y,      &set_icon_WIFI_B,       {"WiFi", "WiFi"}},
    {&set_icon_BLE_y,       &set_icon_BLE_B,        {"BLE", "蓝牙"}},
    {&set_icon_set_y,       &set_icon_set_B,        {"Setting", "设置"}},
};

static time_out_count time_1000ms;

static lv_obj_t *main_cont, * setting_page;

static void ctrlBtn_set_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *button = lv_event_get_target(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        int btn = (int)lv_event_get_user_data(e);
        feed_clock_time();
        //app_sound_play();

        lv_obj_t *btnchild = lv_obj_get_child(button, 0);

        if (LV_STATE_CHECKED == (button->state & LV_STATE_CHECKED)) {
            lv_obj_clear_state(button, LV_STATE_CHECKED);
            lv_img_set_src(btnchild, title_set_info[btn].img_checked);
            ESP_LOGI(TAG, "[%d] checked:%X", btn, button->state & LV_STATE_CHECKED);
        } else {
            lv_obj_add_state(button, LV_STATE_CHECKED);
            lv_img_set_src(btnchild, title_set_info[btn].img_unchecked);
            ESP_LOGI(TAG, "[%d] checked:%X", btn, button->state & LV_STATE_CHECKED);
        }

        switch (btn) {
        case SET_FUNC_SCREENOFF:
            enter_clock_time();
            break;
        case SET_FUNC_WIFI:
            set_layer.user_data = SPRITE_SET_PAGE_WiFi;
            lv_func_goto_layer(&set_layer);
            break;
        case SET_FUNC_BLE:
            break;
        case SET_FUNC_SET:
            set_layer.user_data = SPRITE_SET_PAGE_HOME;
            lv_func_goto_layer(&set_layer);
            break;
        }
    }
}

static void top_pulldown_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_point_t scroll_end;

    lv_obj_get_scroll_end(main_cont, &scroll_end);

    if (code == LV_EVENT_SCROLL_END) {

        feed_clock_time();
        if ((480 == lv_obj_get_scroll_y(main_cont))) {
            lv_obj_set_style_opa(setting_page, LV_OPA_0, 0);
        } else if (0 == lv_obj_get_scroll_y(main_cont)) {
        }
    } else if (code == LV_EVENT_SCROLL_BEGIN) {
        lv_obj_set_style_opa(setting_page, LV_OPA_100, 0);
        // lv_obj_set_style_opa(setting_page, LV_OPA_80, 0);
    } else if (code == LV_EVENT_SCROLL) {
    }
}

static void lv_create_set_pulldown(lv_obj_t *parent)
{
    int i;

    static lv_coord_t grid_set_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_set_row_dsc[] = {
        50,  /*power key*/
        50,  /*current temp*/
        50,  /*Title "current temp*/
        LV_GRID_TEMPLATE_LAST
    };

    lv_obj_t *bg_setpage, *set_btn, * bg_set_btn, * label_set;

    bg_setpage = lv_list_create(parent);
    lv_obj_set_style_opa(bg_setpage, LV_OPA_100, 0);
    lv_obj_set_size(bg_setpage, LV_HOR_RES, LV_PCT(45));
    lv_obj_align(bg_setpage, LV_ALIGN_TOP_LEFT, 0, 15);

    lv_obj_set_style_bg_color(bg_setpage, (thream_set ? lv_color_hex(COLOUR_WARM_SET) : lv_color_hex(COLOUR_COOL_SET)), LV_PART_MAIN);
    lv_obj_set_style_border_width(bg_setpage, 0, 0);
    lv_obj_set_style_radius(bg_setpage, 0, 10);

    lv_obj_set_grid_dsc_array(bg_setpage, grid_set_col_dsc, grid_set_row_dsc);

    for (i = 0; i < sizeof(title_set_info) / sizeof(title_set_info[0]); i++) {
        set_btn = lv_btn_create(bg_setpage);
        lv_obj_remove_style_all(set_btn);
        lv_obj_set_size(set_btn, 70, 70);
        lv_obj_add_event_cb(set_btn, ctrlBtn_set_event_cb, LV_EVENT_SHORT_CLICKED, (void *)i);
        lv_obj_clear_state(set_btn, LV_STATE_CHECKED);
        lv_obj_set_grid_cell(set_btn, LV_GRID_ALIGN_CENTER, i, 1, LV_GRID_ALIGN_CENTER, 1, 1);

        bg_set_btn = lv_img_create(set_btn);
        lv_img_set_src(bg_set_btn, title_set_info[i].img_checked);
        lv_obj_align(bg_set_btn, LV_ALIGN_CENTER, 0, 0);

        label_set = lv_label_create(bg_setpage);
        lv_obj_remove_style_all(label_set);
        lv_obj_set_style_text_color(label_set, lv_color_white(), 0);
        lv_obj_set_style_text_font(label_set, &SourceHanSansCN_Normal_20, 0);

        lv_label_set_text(label_set, title_set_info[i].btn_name[sys_set->sr_lang]);
        lv_obj_set_grid_cell(label_set, LV_GRID_ALIGN_CENTER, i, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    }
}

void lv_create_pulldown_bg(lv_obj_t *parent)
{
    main_cont = lv_obj_create(parent);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_remove_style_all(main_cont);
    lv_obj_set_size(main_cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_scroll_snap_y(main_cont, LV_SCROLL_SNAP_CENTER);

    setting_page = lv_obj_create(main_cont);
    lv_obj_set_size(setting_page, LV_HOR_RES, LV_VER_RES + LV_HANDLE_SIZE * 2);
    lv_obj_set_y(setting_page, - LV_HANDLE_SIZE);
    lv_obj_set_style_opa(setting_page, LV_OPA_0, 0);
    lv_obj_set_style_bg_color(setting_page, lv_color_hex(COLOUR_BLACK), 0);
    lv_obj_set_style_border_width(setting_page, 0, 0);
    lv_obj_set_style_pad_all(setting_page, 0, 0);
    lv_obj_set_scroll_dir(setting_page, LV_DIR_VER);

    lv_obj_t *placeholder = lv_obj_create(main_cont);
    lv_obj_remove_style_all(placeholder);
    lv_obj_set_size(placeholder, LV_HOR_RES, LV_VER_RES - 2 *  LV_HANDLE_SIZE);
    lv_obj_set_y(placeholder, LV_VER_RES + LV_HANDLE_SIZE);
    lv_obj_clear_flag(placeholder, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_update_layout(main_cont);
    lv_obj_add_event_cb(main_cont, top_pulldown_event_cb, LV_EVENT_ALL, 0);
    lv_create_set_pulldown(setting_page);

    lv_obj_scroll_by(main_cont, 0, 0 - (LV_VER_RES - 0 *  LV_HANDLE_SIZE), LV_ANIM_OFF);
    return;
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

static bool show_set_layer_exit_cb(void *layer)
{
    LV_LOG_USER("exit");
    return true;
}

static void show_set_layer_timer_cb(lv_timer_t *tmr)
{
    if (is_time_out(&time_1000ms)) {
    }
}
