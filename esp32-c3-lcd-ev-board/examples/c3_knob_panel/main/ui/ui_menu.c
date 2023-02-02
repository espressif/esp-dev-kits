/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "lvgl.h"
#include <stdio.h>
#include "esp_system.h"
#ifdef ESP_IDF_VERSION
#include "esp_log.h"
#endif
#include "ui.h"
#include "ui_clock.h"
#include "ui_light.h"
#include "ui_player.h"
#include "ui_weather.h"
#include "ui_fan.h"
#include "ui_washing.h"

#include "lv_example_pub.h"
#include "lv_example_func.h"
#include "lv_example_image.h"

#if !USE_NEW_MENU
static bool main_layer_enter_cb(struct lv_layer_t *layer);
static bool main_layer_exit_cb(struct lv_layer_t *layer);
static void main_layer_timer_cb(lv_timer_t *tmr);

lv_layer_t main_Layer = {
    .lv_obj_name    = "main_menu_Layer",
    .lv_obj_parent  = NULL,
    .lv_obj_layer   = NULL,
    .lv_show_layer  = NULL,
    .enter_cb       = main_layer_enter_cb,
    .exit_cb        = main_layer_exit_cb,
    .timer_cb       = main_layer_timer_cb,
};

typedef struct {
    const char *name;
    const lv_img_dsc_t *icon;
    struct lv_layer_t *layer;
} ui_menu_app_t;

LV_IMG_DECLARE(icon_clock);
LV_IMG_DECLARE(icon_fans);
LV_IMG_DECLARE(icon_light);
LV_IMG_DECLARE(icon_player);
LV_IMG_DECLARE(icon_weather);
LV_IMG_DECLARE(icon_washing);

static const ui_menu_app_t menu[] = {
    {"clock",   &icon_clock,    &clock_Layer},
    {"washing", &icon_washing,  &washing_Layer},
    {"fans",    &icon_fans,     &thermostat_Layer},
    {"light",   &icon_light,    &light_2color_Layer},
    {"player",  &icon_player,   &player_Layer},
    {"weather", &icon_weather,  &weather_Layer},
};

#define APP_NUM 4//(sizeof(menu) / sizeof(ui_menu_app_t))
#define APP_ICON_GAP_PIXEL (80)
#define ICONS_SHOW_NUM 3

static uint8_t app_index = 0;
static lv_obj_t *page;
static lv_obj_t *image_bg;
static bool anim_flag = false;
static lv_obj_t *icons[ICONS_SHOW_NUM + 1];
static lv_coord_t old_y[ICONS_SHOW_NUM + 1];
static uint8_t visible_index[ICONS_SHOW_NUM];
static uint8_t invisable_index;

static time_out_count time_500ms;

static void menu_anim_ready_cb(lv_anim_t *a);

static uint32_t get_num_offset(uint32_t num, int32_t max, int32_t offset)
{
    if (num >= max) {
        printf("[ERROR] num should less than max\n");
        return num;
    }

    uint32_t i;
    if (offset >= 0) {
        i = (num + offset) % max;
    } else {
        offset = max + (offset % max);
        i = (num + offset) % max;
    }

    return i;
}

static uint32_t get_app_index(int8_t offset)
{
    return get_num_offset(app_index, APP_NUM, offset);
}

static void app_return_cb(void *args)
{
    ui_add_obj_to_encoder_group(image_bg);
}

static void menu_anim_exec_cb(void *args, int32_t v)
{
    int8_t extra_icon_index = (int8_t)args;

    for (int i = 0; i < ICONS_SHOW_NUM + 1; i++) {
        if (extra_icon_index > 0) {
            lv_obj_set_y(icons[i], old_y[i] - v);
        } else {
            lv_obj_set_y(icons[i], old_y[i] + v);
        }
        int32_t abs_y = LV_ABS(lv_obj_get_y_aligned(icons[i]));
        if (abs_y < 130) {
            lv_img_set_zoom(icons[i], 256 * (130 - abs_y) / 100);
        }
    }
}

static void menu_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    printf("evt=%d\n", code);
    if (LV_EVENT_FOCUSED == code) {
        lv_group_set_editing(lv_group_get_default(), true);
    } else if (LV_EVENT_KEY == code && false == anim_flag) {

        uint32_t key = lv_event_get_key(e);
        int8_t  extra_icon_index = 0;
        if (LV_KEY_RIGHT == key) {
            extra_icon_index = (ICONS_SHOW_NUM / 2) + 1;
        } else if (LV_KEY_LEFT == key) {
            extra_icon_index = -(ICONS_SHOW_NUM / 2) - 1;
        }
        printf("evt=%s\n", (LV_KEY_RIGHT == key) ? "LV_KEY_RIGHT" : "LV_KEY_LEFT");

        lv_img_set_src(icons[invisable_index], menu[get_app_index(extra_icon_index)].icon);
        lv_obj_align(icons[invisable_index], LV_ALIGN_CENTER, 0, (extra_icon_index)* APP_ICON_GAP_PIXEL);
        lv_img_set_zoom(icons[invisable_index], 1);

        for (size_t i = 0; i < ICONS_SHOW_NUM + 1; i++) {
            old_y[i] = lv_obj_get_y_aligned(icons[i]);
        }

        anim_flag = true;
        lv_anim_t a1;
        lv_anim_init(&a1);
        lv_anim_set_var(&a1, (void *)extra_icon_index);
        lv_anim_set_values(&a1, 0, APP_ICON_GAP_PIXEL);
        lv_anim_set_exec_cb(&a1, menu_anim_exec_cb);
        lv_anim_set_path_cb(&a1, lv_anim_path_ease_in_out);
        lv_anim_set_ready_cb(&a1, menu_anim_ready_cb);
        lv_anim_set_time(&a1, 200);
        lv_anim_set_user_data(&a1, (void *)extra_icon_index);
        lv_anim_start(&a1);
        reload_screenOff_timer();
    } else if (LV_EVENT_CLICKED == code) {
        printf("evt=%s\n", "LV_EVENT_CLICKED");
        if (menu[get_app_index(0)].layer) {
            lv_group_set_editing(lv_group_get_default(), false);
            ui_remove_all_objs_from_encoder_group();

            //submode_item_focus = 0;
            lv_func_goto_layer(menu[get_app_index(0)].layer);
        } else {
            printf("Not supported\n");
        }
        reload_screenOff_timer();
    }
}

static void menu_anim_ready_cb(lv_anim_t *a)
{
    int8_t extra_icon_index = (int8_t)lv_anim_get_user_data(a);
    int8_t dir = extra_icon_index > 0 ? 1 : -1;
    app_index = get_app_index(dir);
    invisable_index = get_num_offset(invisable_index, ICONS_SHOW_NUM + 1, dir);
    for (size_t i = 0; i < ICONS_SHOW_NUM; i++) {
        visible_index[i] = get_num_offset(visible_index[i], ICONS_SHOW_NUM + 1, dir);
    }
    anim_flag = false;
    printf("dir=%d, app_index=%d, invisable_index=%d\n", dir, app_index, invisable_index);
}

void ui_menu_init(lv_obj_t *parent)
{
    page = lv_obj_create(parent);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES);
    //lv_obj_set_size(page, lv_obj_get_width(lv_obj_get_parent(page)), lv_obj_get_height(lv_obj_get_parent(page)));

    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(page);
    lv_obj_refr_size(page);
    lv_obj_set_style_bg_color(page, lv_color_make(0x0, 0x0, 0x0), 0);

    /*
    image_bg = lv_img_create(page);
    LV_IMG_DECLARE(img_bg);
    lv_img_set_src(image_bg, &img_bg);
    lv_obj_align(image_bg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_img_opa(image_bg, LV_OPA_60, 0);
    */
    image_bg = page;

    for (int i = 0; i < ICONS_SHOW_NUM; i++) {
        visible_index[i] = i;
        icons[visible_index[i]] = lv_img_create(image_bg);
        lv_img_set_src(icons[visible_index[i]], menu[get_app_index(i - (ICONS_SHOW_NUM / 2))].icon);
        lv_obj_align(icons[visible_index[i]], LV_ALIGN_CENTER, 0, (i - (ICONS_SHOW_NUM / 2)) * APP_ICON_GAP_PIXEL);
        int32_t abs_y = LV_ABS(lv_obj_get_y_aligned(icons[visible_index[i]]));
        if (abs_y < 130) {
            lv_img_set_zoom(icons[visible_index[i]], 256 * (130 - abs_y) / 100);
        } else {
            lv_img_set_zoom(icons[visible_index[i]], 1);
        }
    }
    invisable_index = ICONS_SHOW_NUM;
    icons[invisable_index] = lv_img_create(image_bg);
    lv_obj_set_size(icons[invisable_index], LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(icons[invisable_index], LV_ALIGN_CENTER, 0, lv_obj_get_height(lv_obj_get_parent(image_bg)));


    lv_obj_add_event_cb(image_bg, menu_event_cb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(image_bg, menu_event_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(image_bg, menu_event_cb, LV_EVENT_CLICKED, NULL);
    ui_add_obj_to_encoder_group(image_bg);
}



static bool main_layer_enter_cb(struct lv_layer_t *layer)
{
    bool ret = false;
    uint32_t i;
    LV_LOG_USER("lv_obj_name:%s, screen:[%d, %d]", layer->lv_obj_name, LV_HOR_RES, LV_VER_RES);
    if (NULL == layer->lv_obj_layer) {
        ret = true;
        layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_set_size(layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_border_width(layer->lv_obj_layer, 0, 0);
        lv_obj_set_style_pad_all(layer->lv_obj_layer, 0, 0);

        ui_menu_init(layer->lv_obj_layer);
    }
    set_time_out(&time_500ms, 500);
    reload_screenOff_timer();

    return ret;
}


static bool main_layer_exit_cb(struct lv_layer_t *layer)
{
    LV_LOG_USER("");
    return true;
}

static void main_layer_timer_cb(lv_timer_t *tmr)
{
    trigger_screenOff_timer(&main_Layer);

    if (is_time_out(&time_500ms)) {
        //update_time(NULL);
    }

    return;
}
#endif