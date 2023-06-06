/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <time.h>
#include "lvgl.h"
#include "ui_washing.h"
#include "src/misc/lv_math.h"

#include "settings.h"
#include "app_audio.h"
#include "lv_example_pub.h"
#include "lv_example_image.h"

static bool washing_layer_enter_cb(void *layer);
static bool washing_layer_exit_cb(void *layer);
static void washing_layer_timer_cb(lv_timer_t *tmr);

lv_layer_t washing_Layer = {
    .lv_obj_name    = "washing_Layer",
    .lv_obj_parent  = NULL,
    .lv_obj_layer   = NULL,
    .lv_show_layer  = NULL,
    .enter_cb       = washing_layer_enter_cb,
    .exit_cb        = washing_layer_exit_cb,
    .timer_cb       = washing_layer_timer_cb,
};

#define FUNC_NUM 3
typedef struct {
    const lv_img_dsc_t *wash_funcs_CN;
    const lv_img_dsc_t *wash_funcs_EN;
    uint8_t wash_time;
} wash_cycle_t;

typedef enum {
    WASH_MODE_STANDBY,
    WASH_MODE_RUN,
    WASH_MODE_PAUSE,
    WASH_MODE_EOC,
    WASH_MODE_MAX,
} WASH_MODE_T;

static const wash_cycle_t wash_cycle[FUNC_NUM] = {
    {&img_washing_stand, &wash_basic, 58},
    {&img_washing_shirt, &wash_blouse, 68},
    {&img_washing_underwear, &wash_briefs, 28},
};

static lv_coord_t cycle_init_y_axis[FUNC_NUM];
static lv_coord_t cycle_init_x_axis[FUNC_NUM];

static lv_anim_t anmi_run_wave;
static lv_obj_t *label_wash_time;
static lv_obj_t *img_funcs[FUNC_NUM];

static lv_obj_t *page_background, *page_standby, *page_run;
static lv_obj_t *img_bg_wash;
static lv_obj_t *img_wave1, *img_wave2;
static lv_obj_t *img_run_wave1, *img_run_wave2;
static lv_coord_t img_wave1_x, img_wave2_x, img_run_wave1_x, img_run_wave2_x;
static lv_obj_t *img_anmi_shirt, *img_anmi_underwear1, *img_anmi_underwear2;
static lv_obj_t *label_leftTimeH, *label_leftTimeL, *label_leftTime_unit;

static WASH_MODE_T wash_mode, wash_mode_xor;
static uint8_t item_central;
static uint32_t wash_time_left, wash_demo_left;
static time_out_count time_1000ms;

static uint32_t get_cycle_position(uint32_t num, int32_t max, int32_t offset)
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

static uint32_t get_next_cycle_position(int8_t offset)
{
    return get_cycle_position(item_central, 3, offset);
}

static void menu_position_reset()
{
    int32_t abs_t, x_axis, y_axis;

    uint8_t item_top = get_next_cycle_position(-1);
    uint8_t item_bottom = get_next_cycle_position(1);

    abs_t = x_axis = y_axis = 0;
    for (int i = 0; i < FUNC_NUM; i++) {
        if (i == item_top) {
            abs_t = 33;
            x_axis = 20;
            y_axis = (0 - 80);
        } else if (i == item_central) {
            abs_t = 0;
            x_axis = 60;
            y_axis = (0);
        } else if (i == item_bottom) {
            abs_t = 33;
            x_axis = 20;
            y_axis = (0 + 80);
        }

        lv_img_set_zoom(img_funcs[i], 256 * (100 - abs_t) / 70);
        lv_obj_set_style_img_recolor_opa(img_funcs[i], LV_OPA_COVER, 0);
        lv_obj_set_style_img_recolor(img_funcs[i], lv_color_hsv_to_rgb(200, (40 - abs_t) * 60 / 40, 100), 0);
        lv_obj_align(img_funcs[i], LV_ALIGN_CENTER, x_axis, y_axis);
        lv_label_set_text_fmt(label_wash_time, "- %02d min -", wash_cycle[item_central].wash_time);

        lv_obj_add_flag(img_anmi_shirt, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(img_anmi_underwear1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(img_anmi_underwear2, LV_OBJ_FLAG_HIDDEN);

        if (2 == item_central) {
            lv_obj_clear_flag(img_anmi_underwear1, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(img_anmi_underwear2, LV_OBJ_FLAG_HIDDEN);
        } else if (1 == item_central) {
            lv_obj_clear_flag(img_anmi_shirt, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void func_anim_cb(void *args, int32_t v)
{
    int dir = (int)args;
    uint8_t follow_up = 0;
    int32_t x_axis, y_axis;

    follow_up = get_next_cycle_position(dir);
    for (int i = 0; i < FUNC_NUM; i++) {

        y_axis = cycle_init_y_axis[i] - (dir) * v;
        if (i == item_central) {
            x_axis = cycle_init_x_axis[i] - v / 2;
        } else if (i == follow_up) {
            x_axis = cycle_init_x_axis[i] + v / 2;
        } else {
            /*40 - 80*/
            if (v > 40) {
                x_axis = 0 + v / 4;
                y_axis = dir * (120 - v / 2);
            }
            /*0 - 40*/
            else {
                x_axis = cycle_init_x_axis[i] - v / 2;
            }
            //printf("X,Y:[%02d,%02d]\r\n", x_axis, y_axis);
        }

        lv_obj_set_x(img_funcs[i], x_axis);
        lv_obj_set_y(img_funcs[i], y_axis);

        int32_t abs_t = LV_ABS(lv_obj_get_y_aligned(img_funcs[i]));
        if (abs_t <= 40) {
            lv_img_set_zoom(img_funcs[i], 256 * (100 - abs_t) / 70);
            lv_obj_set_style_img_recolor_opa(img_funcs[i], LV_OPA_COVER, 0);
            lv_obj_set_style_img_recolor(img_funcs[i], lv_color_hsv_to_rgb(200, (40 - abs_t) * 60 / 40, 100), 0);
        }
    }
}

static void func_anim_ready_cb(lv_anim_t *a)
{
    int extra_icon_index = (int)lv_anim_get_user_data(a);
    int8_t dir = extra_icon_index > 0 ? 1 : -1;

    item_central = get_next_cycle_position(dir);
    menu_position_reset();
}

static void washing_event_cb(lv_event_t *e)
{
    static uint8_t forbidden_sec_trigger = false;

    lv_event_code_t code = lv_event_get_code(e);

    if (LV_EVENT_FOCUSED == code) {
        lv_group_set_editing(lv_group_get_default(), true);
    } else if (LV_EVENT_KEY == code) {
        uint32_t key = lv_event_get_key(e);
        int changed = 0;
        if (LV_KEY_LEFT == key) {
            changed = 1;
        } else if (LV_KEY_RIGHT == key) {
            changed = -1;
        }
        if (lv_obj_get_y_aligned(img_funcs[item_central]) != 0) {
            changed = 0;
        } else {
            for (size_t i = 0; i < FUNC_NUM; i++) {
                cycle_init_y_axis[i] = lv_obj_get_y_aligned(img_funcs[i]);
                cycle_init_x_axis[i] = lv_obj_get_x_aligned(img_funcs[i]);
            }
        }

        if (changed) {
            lv_anim_t a1;
            lv_anim_init(&a1);
            lv_anim_set_var(&a1, (void *)changed);
            lv_anim_set_delay(&a1, 0);
            //lv_anim_set_values(&a1, 0, 45);
            lv_anim_set_values(&a1, 0, 80 - LV_ABS(cycle_init_y_axis[item_central]));
            lv_anim_set_exec_cb(&a1, func_anim_cb);
            lv_anim_set_path_cb(&a1, lv_anim_path_ease_in_out);
            lv_anim_set_ready_cb(&a1, func_anim_ready_cb);
            lv_anim_set_user_data(&a1, (void *)changed);
            lv_anim_set_time(&a1, 350);
            lv_anim_start(&a1);
        }

    } else if (LV_EVENT_LONG_PRESSED == code) {
        forbidden_sec_trigger = true;
        if (WASH_MODE_STANDBY == wash_mode) {
            lv_indev_wait_release(lv_indev_get_next(NULL));
            ui_remove_all_objs_from_encoder_group();
            lv_func_goto_layer(&menu_layer);
        } else if ((WASH_MODE_RUN == wash_mode) || (WASH_MODE_PAUSE == wash_mode)) {
            wash_mode = WASH_MODE_EOC;
        } else if (WASH_MODE_EOC == wash_mode) {
            wash_mode = WASH_MODE_STANDBY;
        }
    } else if (LV_EVENT_CLICKED == code) {
        if(false == forbidden_sec_trigger) {
            if (WASH_MODE_STANDBY == wash_mode) {
                wash_mode = WASH_MODE_RUN;
            } else if (WASH_MODE_RUN == wash_mode) {
                wash_mode = WASH_MODE_PAUSE;
            } else if (WASH_MODE_PAUSE == wash_mode) {
                wash_mode = WASH_MODE_RUN;
            }
        } else {
            forbidden_sec_trigger = false;
        }
    }
}

static void bub1_anim_cb(void *args, int32_t v)
{
    lv_obj_t *img_bub = (lv_obj_t *)args;
    lv_coord_t y = lv_obj_get_y_aligned(img_bub);
    int opa = 0;
    if (y > -60) {
        opa = 255 * (60 + y) / 60;
    }
    lv_obj_set_style_img_opa(img_bub, opa, 0);
    lv_obj_set_y(img_bub, v);
}

static void wave_anim_cb(void *args, int32_t v)
{
    lv_obj_set_x(img_wave1, img_wave1_x + LV_ABS(v));
    lv_obj_set_x(img_wave2, img_wave2_x - LV_ABS(v));
}

static void wave_run_anim_cb(void *args, int32_t v)
{
    if (WASH_MODE_RUN == wash_mode) {
        lv_obj_set_x(img_run_wave1, img_run_wave1_x + LV_ABS(v));
        lv_obj_set_x(img_run_wave2, img_run_wave2_x - LV_ABS(v));
    }
}

static void shirt_anim_cb(void *args, int32_t v)
{
    lv_obj_t *img_shirt = (lv_obj_t *)args;
    lv_img_set_pivot(img_shirt, 58 / 2, 2 * 58 / 3);
    lv_img_set_angle(img_shirt, v);
}

static void underwear_anim_cb(void *args, int32_t v)
{
    lv_img_set_angle(img_anmi_underwear1, v);
    lv_img_set_angle(img_anmi_underwear2, -v);
}

static void mask_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    static int16_t mask_id = -1;

    if (code == LV_EVENT_COVER_CHECK) {
        lv_event_set_cover_res(e, LV_COVER_RES_MASKED);
    } else if (code == LV_EVENT_DRAW_MAIN_BEGIN) {
        /* add mask */
        lv_area_t win_area;
        lv_obj_get_coords(img_bg_wash, &win_area);
        lv_draw_mask_radius_param_t *mask_out_param = lv_mem_buf_get(sizeof(lv_draw_mask_radius_param_t));
        lv_draw_mask_radius_init(mask_out_param, &win_area, LV_RADIUS_CIRCLE, 0);
        mask_id = lv_draw_mask_add(mask_out_param, NULL);
    } else if (code == LV_EVENT_DRAW_POST_END) {
        lv_draw_mask_radius_param_t *mask = lv_draw_mask_remove_id(mask_id);
        lv_draw_mask_free_param(mask);
        lv_mem_buf_release(mask);
        mask_id = -1;
    }
}

void ui_washing_init(lv_obj_t *parent)
{
    sys_param_t *param = settings_get_parameter();

    page_background = lv_obj_create(parent);
    lv_obj_set_size(page_background, LV_HOR_RES, LV_VER_RES);

    lv_obj_set_style_border_width(page_background, 5, 0);
    lv_obj_set_style_radius(page_background, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(page_background, lv_color_make(20, 20, 20), 0);
    lv_obj_set_style_border_color(page_background, lv_palette_main(LV_PALETTE_LIGHT_BLUE), 0);
    lv_obj_clear_flag(page_background, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(page_background);
    lv_obj_refr_size(page_background);

    page_standby = lv_obj_create(page_background);
    lv_obj_remove_style_all(page_standby);
    lv_obj_set_size(page_standby, LV_HOR_RES - 10, LV_VER_RES - 10);
    lv_obj_align(page_standby, LV_ALIGN_CENTER, 0, 0);

    page_run = lv_obj_create(page_background);
    lv_obj_remove_style_all(page_run);
    // lv_obj_set_size(page_run, LV_HOR_RES -10, LV_VER_RES -10);
    lv_obj_set_size(page_run, LV_HOR_RES, LV_VER_RES);
    lv_obj_align(page_run, LV_ALIGN_CENTER, 0, 0);

    /*
     * create run page
     */
    label_leftTimeH = lv_label_create(page_run);
    lv_obj_set_style_text_font(label_leftTimeH, &HelveticaNeue_Regular_48, 0);
    lv_label_set_text(label_leftTimeH, "12");
    lv_obj_align(label_leftTimeH, LV_ALIGN_CENTER, -30, -20);

    label_leftTimeL = lv_label_create(page_run);
    lv_obj_set_style_text_font(label_leftTimeL, &HelveticaNeue_Regular_48, 0);
    lv_label_set_text(label_leftTimeL, "04");
    lv_obj_align(label_leftTimeL, LV_ALIGN_CENTER, 30, -20);

    label_leftTime_unit = lv_label_create(page_run);
    lv_obj_set_style_text_font(label_leftTime_unit, &HelveticaNeue_Regular_48, 0);
    lv_label_set_text(label_leftTime_unit, ":");
    lv_obj_align(label_leftTime_unit, LV_ALIGN_CENTER, -8, -23);

    lv_obj_t *label_info = lv_label_create(page_run);
    lv_obj_set_style_text_color(label_info, lv_color_hex(COLOUR_GREY_4F), 0);
    if (LANGUAGE_CN == param->language) {
        lv_obj_set_style_text_font(label_info, &font_SourceHanSansCN_20, 0);
        lv_label_set_text(label_info, "长按结束");
    } else {
        lv_obj_set_style_text_font(label_info, &HelveticaNeue_Regular_20, 0);
        lv_label_set_text(label_info, "long press to end");
    }
    lv_obj_align(label_info, LV_ALIGN_CENTER, 0, 40);

    img_run_wave1 = lv_img_create(page_run);
    lv_img_set_src(img_run_wave1, &img_washing_wave1);
    lv_obj_align(img_run_wave1, LV_ALIGN_BOTTOM_MID, -15, 10);
    lv_img_set_zoom(img_run_wave1, 256 * (240 - 0) / 162);
    img_run_wave2 = lv_img_create(page_run);
    lv_img_set_src(img_run_wave2, &img_washing_wave2);
    lv_obj_align(img_run_wave2, LV_ALIGN_BOTTOM_MID, 20, 10);
    lv_img_set_zoom(img_run_wave2, 256 * (240 - 0) / 162);
    // lv_obj_add_event_cb(img_run_wave1, mask_event_cb, LV_EVENT_ALL, NULL);
    // lv_obj_add_event_cb(img_run_wave2, mask_event_cb, LV_EVENT_ALL, NULL);

    img_run_wave1_x = lv_obj_get_x_aligned(img_run_wave1);
    img_run_wave2_x = lv_obj_get_x_aligned(img_run_wave2);
    lv_anim_init(&anmi_run_wave);
    lv_anim_set_var(&anmi_run_wave, img_run_wave1);
    lv_anim_set_delay(&anmi_run_wave, 0);
    lv_anim_set_values(&anmi_run_wave, -40 * 240 / 160, 40 * 240 / 160);
    lv_anim_set_exec_cb(&anmi_run_wave, wave_run_anim_cb);
    lv_anim_set_path_cb(&anmi_run_wave, lv_anim_path_ease_in_out);
    lv_anim_set_time(&anmi_run_wave, lv_rand(3200, 4000));
    lv_anim_set_repeat_count(&anmi_run_wave, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&anmi_run_wave);

    lv_obj_add_flag(page_run, LV_OBJ_FLAG_HIDDEN);

    /*
     * create standby page
     */
    img_bg_wash = lv_img_create(page_standby);
    lv_img_set_src(img_bg_wash, &img_washing_bg);
    lv_obj_align(img_bg_wash, LV_ALIGN_LEFT_MID, 7, 0);

    img_wave1 = lv_img_create(img_bg_wash);
    lv_img_set_src(img_wave1, &img_washing_wave1);
    lv_obj_align(img_wave1, LV_ALIGN_BOTTOM_MID, -15, 10);
    img_wave2 = lv_img_create(img_bg_wash);
    lv_img_set_src(img_wave2, &img_washing_wave2);
    lv_obj_align(img_wave2, LV_ALIGN_BOTTOM_MID, 20, 10);
    lv_obj_add_event_cb(img_wave1, mask_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(img_wave2, mask_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *img_bub1 = lv_img_create(img_bg_wash);
    lv_img_set_src(img_bub1, &img_washing_bubble1);
    lv_obj_center(img_bub1);
    lv_obj_t *img_bub2 = lv_img_create(img_bg_wash);
    lv_img_set_src(img_bub2, &img_washing_bubble2);
    lv_obj_center(img_bub2);

    img_anmi_shirt = lv_img_create(img_bg_wash);
    lv_img_set_src(img_anmi_shirt, &wash_shirt);
    lv_obj_align(img_anmi_shirt, LV_ALIGN_TOP_MID, 0, 20);
    img_anmi_underwear1 = lv_img_create(img_bg_wash);
    lv_img_set_src(img_anmi_underwear1, &wash_underwear1);
    lv_obj_align(img_anmi_underwear1, LV_ALIGN_TOP_MID, 0, 15);
    img_anmi_underwear2 = lv_img_create(img_bg_wash);
    lv_img_set_src(img_anmi_underwear2, &wash_underwear2);
    lv_obj_align(img_anmi_underwear2, LV_ALIGN_TOP_MID, 0, 15 + 28 + 8);

    label_wash_time = lv_label_create(page_standby);
    lv_obj_set_style_text_font(label_wash_time, &lv_font_montserrat_16, 0);
    lv_label_set_text_fmt(label_wash_time, "- %02d min -", wash_cycle[item_central].wash_time);
    lv_obj_set_style_text_opa(label_wash_time, LV_OPA_70, 0);
    lv_obj_set_width(label_wash_time, 150);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_style_text_align(label_wash_time, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label_wash_time, LV_ALIGN_CENTER, 60, 27);

    int16_t x, y;
    for (size_t i = 0; i < FUNC_NUM; i++) {
        //arc_path_by_theta(i * 45, &x, &y);
        img_funcs[i] = lv_img_create(page_standby);
        if (LANGUAGE_CN == param->language) {
            lv_img_set_src(img_funcs[i], wash_cycle[i].wash_funcs_CN);
        } else {
            lv_img_set_src(img_funcs[i], wash_cycle[i].wash_funcs_EN);
        }
        x = 40;
        y = (i - 1) * 40;
        lv_obj_align(img_funcs[i], LV_ALIGN_CENTER, x, y);
    }

    lv_anim_t anmi_bub1;
    lv_anim_init(&anmi_bub1);
    lv_anim_set_var(&anmi_bub1, img_bub1);
    lv_anim_set_delay(&anmi_bub1, 0);
    lv_anim_set_values(&anmi_bub1, lv_obj_get_y_aligned(img_bub1), lv_obj_get_y_aligned(img_bub1) - 90);
    lv_anim_set_exec_cb(&anmi_bub1, bub1_anim_cb);
    lv_anim_set_path_cb(&anmi_bub1, lv_anim_path_ease_in_out);
    lv_anim_set_time(&anmi_bub1, lv_rand(1800, 2300));
    lv_anim_set_repeat_count(&anmi_bub1, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&anmi_bub1);

    lv_anim_t anmi_bub2;
    lv_anim_init(&anmi_bub2);
    lv_anim_set_var(&anmi_bub2, img_bub2);
    lv_anim_set_delay(&anmi_bub2, 0);
    lv_anim_set_values(&anmi_bub2, lv_obj_get_y_aligned(img_bub2), lv_obj_get_y_aligned(img_bub2) - 90);
    lv_anim_set_exec_cb(&anmi_bub2, bub1_anim_cb);
    lv_anim_set_path_cb(&anmi_bub2, lv_anim_path_ease_in_out);
    lv_anim_set_time(&anmi_bub2, lv_rand(2000, 2800));
    lv_anim_set_repeat_count(&anmi_bub2, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&anmi_bub2);

    img_wave1_x = lv_obj_get_x_aligned(img_wave1);
    img_wave2_x = lv_obj_get_x_aligned(img_wave2);
    lv_anim_t anmi_wave;
    lv_anim_init(&anmi_wave);
    lv_anim_set_var(&anmi_wave, img_wave1);
    lv_anim_set_delay(&anmi_wave, 0);
    lv_anim_set_values(&anmi_wave, -40, 40);
    lv_anim_set_exec_cb(&anmi_wave, wave_anim_cb);
    lv_anim_set_path_cb(&anmi_wave, lv_anim_path_ease_in_out);
    lv_anim_set_time(&anmi_wave, lv_rand(3200, 4000));
    lv_anim_set_repeat_count(&anmi_wave, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&anmi_wave);

    lv_anim_t anmi_shirt;
    lv_anim_init(&anmi_shirt);
    lv_anim_set_var(&anmi_shirt, img_anmi_shirt);
    lv_anim_set_delay(&anmi_shirt, 0);
    lv_anim_set_values(&anmi_shirt, -450, 450);
    lv_anim_set_exec_cb(&anmi_shirt, shirt_anim_cb);
    lv_anim_set_path_cb(&anmi_shirt, lv_anim_path_ease_in_out);
    lv_anim_set_time(&anmi_shirt, lv_rand(3200, 4000));
    lv_anim_set_repeat_count(&anmi_shirt, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_playback_time(&anmi_shirt, lv_rand(3200, 4000));
    lv_anim_start(&anmi_shirt);

    lv_anim_t anmi_underwear;
    lv_anim_init(&anmi_underwear);
    lv_anim_set_var(&anmi_underwear, img_anmi_underwear1);
    lv_anim_set_delay(&anmi_underwear, 0);
    lv_anim_set_values(&anmi_underwear, -150, 150);
    lv_anim_set_exec_cb(&anmi_underwear, underwear_anim_cb);
    lv_anim_set_path_cb(&anmi_underwear, lv_anim_path_ease_in_out);
    lv_anim_set_time(&anmi_underwear, lv_rand(2200, 3000));
    lv_anim_set_repeat_count(&anmi_underwear, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_playback_time(&anmi_underwear, lv_rand(2200, 3000));
    lv_anim_start(&anmi_underwear);

    lv_obj_add_event_cb(page_background, washing_event_cb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(page_background, washing_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    lv_obj_add_event_cb(page_background, washing_event_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(page_background, washing_event_cb, LV_EVENT_CLICKED, NULL);
    ui_add_obj_to_encoder_group(page_background);

    item_central = 0;
    wash_mode = WASH_MODE_STANDBY;
    wash_mode_xor = WASH_MODE_MAX;
    menu_position_reset();
}

static bool washing_layer_enter_cb(void *layer)
{
    bool ret = false;

    LV_LOG_USER("");
    lv_layer_t *create_layer = layer;
    if (NULL == create_layer->lv_obj_layer) {
        ret = true;
        create_layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(create_layer->lv_obj_layer);
        lv_obj_set_size(create_layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);

        ui_washing_init(create_layer->lv_obj_layer);
        set_time_out(&time_1000ms, 500);
    }

    return ret;
}

static bool washing_layer_exit_cb(void *layer)
{
    LV_LOG_USER("");
    return true;
}

static void washing_layer_timer_cb(lv_timer_t *tmr)
{
    feed_clock_time();
    sys_param_t *param = settings_get_parameter();

    if (wash_mode_xor ^ wash_mode) {
        switch (wash_mode) {
        case WASH_MODE_STANDBY: {
            lv_obj_add_flag(page_run, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(page_standby, LV_OBJ_FLAG_HIDDEN);
        }
        break;
        case WASH_MODE_RUN: {
            if (WASH_MODE_STANDBY == wash_mode_xor) {
                wash_demo_left = 8;
                wash_time_left = 60 * wash_cycle[item_central].wash_time;
                lv_label_set_text_fmt(label_leftTimeH, "%d", (wash_time_left + 59) / 3600);
                lv_label_set_text_fmt(label_leftTimeL, "%02d", ((wash_time_left + 59) % 3600) / 60);
            }
            lv_obj_clear_flag(page_run, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(page_standby, LV_OBJ_FLAG_HIDDEN);
        }
        break;
        case WASH_MODE_PAUSE: {
            lv_obj_clear_flag(label_leftTime_unit, LV_OBJ_FLAG_HIDDEN);
        }
        break;
        case WASH_MODE_EOC: {
            lv_obj_add_flag(label_leftTime_unit, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(label_leftTimeH, "-");
            lv_label_set_text(label_leftTimeL, "-");
            audio_handle_info((LANGUAGE_CN == param->language) ? SOUND_TYPE_WASH_END_CN : SOUND_TYPE_WASH_END_EN);
        }
        break;
        default:
            break;
        }
        wash_mode_xor = wash_mode;
    }

    if (is_time_out(&time_1000ms) && (WASH_MODE_RUN == wash_mode)) {
        if (wash_demo_left) {
            if (wash_time_left % 2) {
                lv_obj_add_flag(label_leftTime_unit, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_clear_flag(label_leftTime_unit, LV_OBJ_FLAG_HIDDEN);
            }
            lv_label_set_text_fmt(label_leftTimeH, "%d", (wash_time_left + 59) / 3600);
            lv_label_set_text_fmt(label_leftTimeL, "%02d", ((wash_time_left + 59) % 3600) / 60);
            wash_time_left--;
            wash_demo_left--;
        } else {
            wash_mode = WASH_MODE_EOC;
        }
    }
}
