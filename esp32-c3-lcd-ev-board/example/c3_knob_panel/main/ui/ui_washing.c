
#include <stdio.h>
#include <time.h>
#include "lvgl.h"
#include "ui.h"
#include "ui_washing.h"
#include "src/misc/lv_math.h"

#include "lv_example_pub.h"
#include "lv_example_func.h"
#include "lv_example_image.h"

static lv_obj_t  *page;
static lv_obj_t *img_bg_wash;
static lv_obj_t *img_wave1, *img_wave2;
static lv_coord_t img_wave1_x, img_wave2_x;

static time_out_count time_20ms;

static bool washing_Layer_enter_cb(struct lv_layer_t * layer);
static bool washing_Layer_exit_cb(struct lv_layer_t * layer);
static void washing_Layer_timer_cb(lv_timer_t * tmr);

lv_layer_t washing_Layer ={
    .lv_obj_name    = "washing_Layer",
    .lv_obj_parent  = NULL,
    .lv_obj_layer   = NULL,
    .lv_show_layer  = NULL,
    .enter_cb       = washing_Layer_enter_cb,
    .exit_cb        = washing_Layer_exit_cb,
    .timer_cb       = washing_Layer_timer_cb,
};

LV_IMG_DECLARE(img_washing_bg);
LV_IMG_DECLARE(img_washing_wave1);
LV_IMG_DECLARE(img_washing_wave2);
LV_IMG_DECLARE(img_washing_bubble1);
LV_IMG_DECLARE(img_washing_bubble2);
LV_IMG_DECLARE(img_washing_stand);
LV_IMG_DECLARE(img_washing_shirt);
LV_IMG_DECLARE(img_washing_underwear);

#define FUNC_NUM 3
static const lv_img_dsc_t *wash_funcs[FUNC_NUM] = {
    &img_washing_stand,
    &img_washing_shirt,
    &img_washing_underwear,
};

static const uint8_t wash_time[FUNC_NUM] = {
    58,
    68,
    78,
};

static lv_obj_t *label_wash_time;
static lv_obj_t *img_funcs[FUNC_NUM];

static lv_coord_t cycle_init_y_axis[FUNC_NUM];
static lv_coord_t cycle_init_x_axis[FUNC_NUM];

static uint8_t item_central = 0;

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

    printf("new [%d] -> [%d] -> [%d] \r\n",item_top, item_central, item_bottom);

    for(int i = 0; i < FUNC_NUM; i++){
        if(i == item_top){
            abs_t = 33;
            x_axis = 20;
            y_axis = (0 - 80);
        }
        else if(i == item_central){
            abs_t = 0;
            x_axis = 60;
            y_axis = (0);
        }
        else if(i == item_bottom){
            abs_t = 33;
            x_axis = 20;
            y_axis = (0 + 80);
        }

        lv_img_set_zoom(img_funcs[i], 256 * (100 - abs_t) / 70);
        lv_obj_set_style_img_recolor_opa(img_funcs[i], LV_OPA_COVER, 0);
        lv_obj_set_style_img_recolor(img_funcs[i], lv_color_hsv_to_rgb(200, (40-abs_t)*60/40, 100), 0);
        lv_obj_align(img_funcs[i], LV_ALIGN_CENTER, x_axis, y_axis);
        lv_label_set_text_fmt(label_wash_time, "- %02d min -", wash_time[item_central]);
    }
}

static void func_anim_cb(void *args, int32_t v)
{
    int dir = (int)args;
    uint8_t follow_up = 0;
    int32_t x_axis, y_axis;

    follow_up = get_next_cycle_position(dir);
    for (int i = 0; i < FUNC_NUM; i++) {
        
        y_axis = cycle_init_y_axis[i] - (dir)*v;
        if(i == item_central){
            x_axis = cycle_init_x_axis[i] - v/2;
        }
        else if(i == follow_up){
            x_axis = cycle_init_x_axis[i] + v/2;
        }
        else{
            /*40 - 80*/
            if(v > 40){
                x_axis = 0 + v/4;
                y_axis = dir*(120 - v/2);
            }
            /*0 - 40*/
            else{
                x_axis = cycle_init_x_axis[i] - v/2;
            }
            //printf("X,Y:[%02d,%02d]\r\n", x_axis, y_axis);
        }

        lv_obj_set_x(img_funcs[i], x_axis);
        lv_obj_set_y(img_funcs[i], y_axis);

        int32_t abs_t = LV_ABS(lv_obj_get_y_aligned(img_funcs[i]));
        if (abs_t <= 40) {
            lv_img_set_zoom(img_funcs[i], 256 * (100 - abs_t) / 70);
            lv_obj_set_style_img_recolor_opa(img_funcs[i], LV_OPA_COVER, 0);
            lv_obj_set_style_img_recolor(img_funcs[i], lv_color_hsv_to_rgb(200, (40-abs_t)*60/40, 100), 0);
        }
    }
}

static void func_anim_ready_cb(lv_anim_t *a)
{
    int8_t extra_icon_index = (int8_t)lv_anim_get_user_data(a);
    int8_t dir = extra_icon_index > 0 ? 1 : -1;

    item_central = get_next_cycle_position(dir);
    menu_position_reset();
}

static void washing_event_cb(lv_event_t *e)
{
    int16_t offset = 0;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

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
        if(lv_obj_get_y_aligned(img_funcs[item_central]) != 0){
            printf("runniing:%d, path left:%d\r\n", cycle_init_y_axis[item_central], \
                                                80- LV_ABS(cycle_init_y_axis[item_central]));
            changed = 0;
        }
        else{
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
        lv_indev_wait_release(lv_indev_get_next(NULL));
        ui_remove_all_objs_from_encoder_group();
        lv_func_goto_layer(&main_Layer);
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

static void mask_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

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

void ui_washing_init(lv_obj_t * parent)
{
    page = lv_obj_create(parent);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES);
    //lv_obj_set_size(page, lv_obj_get_width(lv_obj_get_parent(page)), lv_obj_get_height(lv_obj_get_parent(page)));

    lv_obj_set_style_border_width(page, 5, 0);
    lv_obj_set_style_radius(page, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(page, lv_color_make(20, 20, 20), 0);
    lv_obj_set_style_border_color(page, lv_palette_main(LV_PALETTE_LIGHT_BLUE), 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(page);
    lv_obj_refr_size(page);
    img_bg_wash = lv_img_create(page);
    lv_img_set_src(img_bg_wash, &img_washing_bg);
    lv_obj_align(img_bg_wash, LV_ALIGN_LEFT_MID, -7, 0);
    img_wave1 = lv_img_create(img_bg_wash);
    lv_img_set_src(img_wave1, &img_washing_wave1);
    lv_obj_align(img_wave1, LV_ALIGN_BOTTOM_MID, -15, 10);
    img_wave2 = lv_img_create(img_bg_wash);
    lv_img_set_src(img_wave2, &img_washing_wave2);
    lv_obj_align(img_wave2, LV_ALIGN_BOTTOM_MID, 20, 10);
    lv_obj_t *img_bub1 = lv_img_create(img_bg_wash);
    lv_img_set_src(img_bub1, &img_washing_bubble1);
    lv_obj_center(img_bub1);
    lv_obj_t *img_bub2 = lv_img_create(img_bg_wash);
    lv_img_set_src(img_bub2, &img_washing_bubble2);
    lv_obj_center(img_bub2);

    lv_obj_add_event_cb(img_wave1, mask_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(img_wave2, mask_event_cb, LV_EVENT_ALL, NULL);

    label_wash_time = lv_label_create(page);
    lv_obj_set_style_text_font(label_wash_time, &lv_font_montserrat_16, 0);
    lv_label_set_text_fmt(label_wash_time, "- %02d min -", wash_time[item_central]);
    lv_obj_set_style_text_opa(label_wash_time, LV_OPA_70, 0);
    lv_obj_set_width(label_wash_time, 150);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_style_text_align(label_wash_time, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label_wash_time, LV_ALIGN_CENTER, 60, 27);

    int16_t x, y;
    for (size_t i = 0; i < FUNC_NUM; i++) {
        //arc_path_by_theta(i * 45, &x, &y);
        img_funcs[i] = lv_img_create(page);
        lv_img_set_src(img_funcs[i], wash_funcs[i]);
        x = 40;
        y = (i -1)*40;
        lv_obj_align(img_funcs[i], LV_ALIGN_CENTER, x, y);
    }
    item_central = 0;
    menu_position_reset();

    lv_anim_t a1;
    lv_anim_init(&a1);
    lv_anim_set_var(&a1, img_bub1);
    lv_anim_set_delay(&a1, 0);
    lv_anim_set_values(&a1, lv_obj_get_y_aligned(img_bub1), lv_obj_get_y_aligned(img_bub1) - 90);
    lv_anim_set_exec_cb(&a1, bub1_anim_cb);
    lv_anim_set_path_cb(&a1, lv_anim_path_ease_in_out);
    lv_anim_set_time(&a1, lv_rand(1800, 2300));
    lv_anim_set_repeat_count(&a1, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a1);

    lv_anim_t a2;
    lv_anim_init(&a2);
    lv_anim_set_var(&a2, img_bub2);
    lv_anim_set_delay(&a2, 0);
    lv_anim_set_values(&a2, lv_obj_get_y_aligned(img_bub2), lv_obj_get_y_aligned(img_bub2) - 90);
    lv_anim_set_exec_cb(&a2, bub1_anim_cb);
    lv_anim_set_path_cb(&a2, lv_anim_path_ease_in_out);
    lv_anim_set_time(&a2, lv_rand(2000, 2800));
    lv_anim_set_repeat_count(&a2, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a2);

    img_wave1_x = lv_obj_get_x_aligned(img_wave1);
    img_wave2_x = lv_obj_get_x_aligned(img_wave2);
    lv_anim_t a3;
    lv_anim_init(&a3);
    lv_anim_set_var(&a3, img_wave1);
    lv_anim_set_delay(&a3, 0);
    lv_anim_set_values(&a3, -40, 40);
    lv_anim_set_exec_cb(&a3, wave_anim_cb);
    lv_anim_set_path_cb(&a3, lv_anim_path_ease_in_out);
    lv_anim_set_time(&a3, lv_rand(3200, 4000));
    lv_anim_set_repeat_count(&a3, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a3);

    lv_obj_add_event_cb(page, washing_event_cb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(page, washing_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    lv_obj_add_event_cb(page, washing_event_cb, LV_EVENT_KEY, NULL);
    ui_add_obj_to_encoder_group(page);
}

static bool washing_Layer_enter_cb(struct lv_layer_t * layer)
{
    bool ret = false;

	if(NULL == layer->lv_obj_layer){
		ret = true;
		layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(layer->lv_obj_layer);
        lv_obj_set_size(layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);

        ui_washing_init(layer->lv_obj_layer);
        set_time_out(&time_20ms, 20);
	}

	return ret;
}

static bool washing_Layer_exit_cb(struct lv_layer_t * layer)
{
    LV_LOG_USER("");
    return true;
}

static void washing_Layer_timer_cb(lv_timer_t * tmr)
{
    if(is_time_out(&time_20ms)){
        //washing_Lhandler();
    }
}
