/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <time.h>
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "ui.h"
#include "ui_clock.h"

#include "lv_example_pub.h"
#include "lv_example_func.h"
#include "lv_example_image.h"

static const char *TAG = "MENU_CLOCK";

static bool clock_screen_layer_enter_cb(struct lv_layer_t *layer);
static bool clock_screen_layer_exit_cb(struct lv_layer_t *layer);
static void clock_screen_layer_timer_cb(lv_timer_t *tmr);

lv_layer_t clock_screen_layer = {
    .lv_obj_name    = "clock_screen_layer",
    .lv_obj_layer   = NULL,
    .enter_cb       = clock_screen_layer_enter_cb,
    .exit_cb        = clock_screen_layer_exit_cb,
    .timer_cb       = clock_screen_layer_timer_cb,
};

static uint16_t flash_sub_step = 0;
static uint8_t flash_main_step = 0;

static lv_obj_t *page;
static lv_obj_t *img_face, *img_eye_bg, *img_eye, * img_mouth, *img_eye_fade;
static lv_obj_t *img_eye_left, * img_eye_right;

static time_out_count time_50ms, time_screen_off_1min;
static time_out_count time_enter_clock = {
    .timeOut = 0,
    .time_base = 0,
};

void reload_screenOff_timer()
{
    reload_time_out(&time_enter_clock);
}

void enter_screenOff_immediate()
{
    time_enter_clock.time_base = 0;
    LV_LOG_USER("");
}

void trigger_screenOff_timer(lv_layer_t *src_layer)
{
    if (0 == time_enter_clock.time_base == time_enter_clock.timeOut) {
        set_time_out(&time_enter_clock, TIME_ENTER_CLOCK_2MIN);
        LV_LOG_USER("time_enter_clock init");
    }

    if (is_time_out(&time_enter_clock)) {
        reload_time_out(&time_screen_off_1min);
        lv_func_goto_layer(&clock_screen_layer);
    }
}

static void wakeup_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (LV_EVENT_FOCUSED == code) {
        lv_group_set_editing(lv_group_get_default(), true);
    } else if ((LV_EVENT_LONG_PRESSED == code) || (LV_EVENT_CLICKED == code) || (LV_EVENT_KEY == code)) {
        lv_indev_wait_release(lv_indev_get_next(NULL));
        ui_remove_all_objs_from_encoder_group();
        lv_func_goto_layer(&main_Layer);
    }
}

static void set_mouth_zoom(void *img, int32_t v)
{
    if (2 == flash_main_step) {
        lv_img_set_zoom(img, v);
    }
}

void ui_flash_face_init(lv_obj_t *parent)
{
    page = lv_obj_create(parent);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(page);

    img_face = lv_img_create(page);
    lv_img_set_src(img_face, &standby_face);
    lv_obj_align(img_face, LV_ALIGN_CENTER, 0, 0);

    img_eye_bg = lv_img_create(page);
    lv_img_set_src(img_eye_bg, &standby_eye_close);
    lv_obj_align(img_eye_bg, LV_ALIGN_CENTER, 0, 0);

    img_eye_fade = lv_img_create(page);
    lv_img_set_src(img_eye_fade, &standby_eye_1_fade);
    lv_obj_align(img_eye_fade, LV_ALIGN_CENTER, 0, 40);

    img_eye = lv_img_create(page);
    lv_img_set_src(img_eye, &standby_eye_3);
    lv_obj_align(img_eye, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(img_eye, LV_OBJ_FLAG_HIDDEN);

    img_eye_left = lv_img_create(page);
    lv_img_set_src(img_eye_left, &standby_eye_left);
    lv_obj_align(img_eye_left, LV_ALIGN_TOP_LEFT, 70, 105);

    img_eye_right = lv_img_create(page);
    lv_img_set_src(img_eye_right, &standby_eye_right);
    lv_obj_align(img_eye_right, LV_ALIGN_TOP_RIGHT, 163, 105);

    img_mouth = lv_img_create(page);
    lv_img_set_src(img_mouth, &standby_mouth_2);
    lv_obj_align(img_mouth, LV_ALIGN_TOP_MID, 0, 163);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, img_mouth);
    lv_anim_set_time(&a, 2000);
    lv_anim_set_exec_cb(&a, set_mouth_zoom);
    lv_anim_set_values(&a, 128, 256);
    lv_anim_set_playback_time(&a, 1000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    lv_obj_add_event_cb(page, wakeup_event_cb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(page, wakeup_event_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(page, wakeup_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(page, wakeup_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    ui_add_obj_to_encoder_group(page);
}

static bool clock_screen_layer_enter_cb(struct lv_layer_t *layer)
{
    bool ret = false;

    if (NULL == layer->lv_obj_layer) {
        ret = true;

        layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(layer->lv_obj_layer);
        lv_obj_set_size(layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);

        ui_flash_face_init(layer->lv_obj_layer);
        set_time_out(&time_50ms, 50);

        flash_sub_step = 0;
        flash_main_step = 0;
    }
    return ret;
}

static bool clock_screen_layer_exit_cb(struct lv_layer_t *layer)
{
    LV_LOG_USER("");
}

static void set_anim_left_eye(void *obj, int32_t v)
{
    if (0 == flash_main_step) {
        lv_obj_set_x(img_eye_left, v);
    }
}

static void set_anim_right_eye(void *obj, int32_t v)
{
    if (0 == flash_main_step) {
        lv_obj_set_x(img_eye_right, v);
    }
}

static void clock_screen_layer_timer_cb(lv_timer_t *tmr)
{
    int32_t isTmOut;
    static lv_anim_t anim_eye;

    if (is_time_out(&time_50ms)) {

        switch (flash_main_step) {
        case 0:
            if (0 == flash_sub_step) {
                lv_img_set_src(img_eye_bg, &standby_eye_open);
                lv_obj_align(img_eye_bg, LV_ALIGN_CENTER, 0, 0);

                lv_obj_align(img_eye_left, LV_ALIGN_TOP_LEFT, 75, 100);
                lv_obj_align(img_eye_right, LV_ALIGN_TOP_LEFT, 115, 100);

                lv_obj_clear_flag(img_eye_left, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(img_eye_right, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(img_eye, LV_OBJ_FLAG_HIDDEN);

                lv_anim_init(&anim_eye);
                lv_anim_set_var(&anim_eye, img_eye_left);
                lv_anim_set_delay(&anim_eye, 0);
                lv_anim_set_path_cb(&anim_eye, lv_anim_path_ease_in_out);
                lv_anim_set_time(&anim_eye, 2000);
                lv_anim_set_playback_time(&anim_eye, 1000);
                lv_anim_set_repeat_count(&anim_eye, LV_ANIM_REPEAT_INFINITE);

                lv_anim_set_values(&anim_eye, lv_obj_get_x_aligned(img_eye_left) + 0, lv_obj_get_x_aligned(img_eye_left) - 50);
                lv_anim_set_exec_cb(&anim_eye, set_anim_left_eye);
                lv_anim_start(&anim_eye);

                lv_anim_set_values(&anim_eye, lv_obj_get_x_aligned(img_eye_right) + 0, lv_obj_get_x_aligned(img_eye_right) + 50);
                lv_anim_set_exec_cb(&anim_eye, set_anim_right_eye);
                lv_anim_start(&anim_eye);
                flash_sub_step += 1;
            } else {
                if (flash_sub_step++ < 80) { //0-400
                    if ((flash_sub_step / 50) % 2) {
                        lv_obj_clear_flag(img_eye_fade, LV_OBJ_FLAG_HIDDEN);
                    } else {
                        lv_obj_add_flag(img_eye_fade, LV_OBJ_FLAG_HIDDEN);
                    }
                } else {
                    flash_sub_step = 0;
                    flash_main_step += 1;
                }
            }
            break;
        case 1:
            if (0 == flash_sub_step) {
                lv_img_set_src(img_eye_bg, &standby_eye_open);
                lv_img_set_src(img_eye, &standby_eye_2);
                lv_obj_align(img_eye_bg, LV_ALIGN_CENTER, 0, 0);
                lv_obj_align(img_eye, LV_ALIGN_CENTER, 0, 0);

                lv_obj_add_flag(img_eye_fade, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(img_eye_left, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(img_eye_right, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(img_eye, LV_OBJ_FLAG_HIDDEN);
            }
            if (flash_sub_step++ > 40) { //0-4000
                flash_sub_step = 0;
                flash_main_step += 1;
            }
            break;
        case 2:
            if (0 == flash_sub_step) {
                lv_img_set_src(img_eye_bg, &standby_eye_close);
                lv_img_set_src(img_eye, &standby_eye_3);
                lv_obj_align(img_eye_bg, LV_ALIGN_CENTER, 0, 0);
                lv_obj_align(img_eye, LV_ALIGN_CENTER, 0, 0 + 5);
            }
            if (flash_sub_step++ > 80 * 2) { //0-8000
                flash_sub_step = 0;
                flash_main_step = 0;
            }
            break;
        }
    }

    if (is_time_out(&time_screen_off_1min)) {
    }
}
