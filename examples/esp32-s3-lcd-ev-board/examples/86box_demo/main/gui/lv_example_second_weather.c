/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "bsp/esp-bsp.h"
#include "lv_example_pub.h"
#include "lv_example_image.h"

static bool weather_layer_enter_cb(void *layer);
static bool weather_layer_exit_cb(void *layer);
static void weather_layer_timer_cb(lv_timer_t *tmr);

static void _img_set_zoom_anim_cb(void *obj, int32_t zoom);
static void album_fade_anim_cb(void *var, int32_t v);
static void _obj_set_x_anim_cb(void *obj, int32_t x);

lv_layer_t weather_Layer = {
    .lv_obj_name    = "weather_Layer",
    .lv_obj_layer   = NULL,
    .lv_show_layer  = &show_set_layer,
    //.lv_show_layer  = NULL,
    .enter_cb       = weather_layer_enter_cb,
    .exit_cb        = weather_layer_exit_cb,
    .timer_cb       = weather_layer_timer_cb,
};

static uint32_t weather_list;
static lv_obj_t *weather_info_bg;
static lv_obj_t *weather_img_obj;

static time_out_count time_2000ms;

extern lv_obj_t *lv_create_weatherPage(lv_obj_t *parent);

void lv_anim_weather_next()
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, weather_img_obj);
    lv_anim_set_values(&a, lv_obj_get_style_img_opa(weather_img_obj, 0), LV_OPA_TRANSP);
    lv_anim_set_exec_cb(&a, album_fade_anim_cb);
    lv_anim_set_time(&a, 500);
    lv_anim_start(&a);

    lv_anim_init(&a);
    lv_anim_set_var(&a, weather_img_obj);
    lv_anim_set_time(&a, 500);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_values(&a, 0, - LV_HOR_RES / 2);//next

    lv_anim_set_exec_cb(&a, _obj_set_x_anim_cb);
    lv_anim_set_ready_cb(&a, lv_obj_del_anim_ready_cb);
    lv_anim_start(&a);

    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_set_var(&a, weather_img_obj);
    lv_anim_set_time(&a, 500);
    lv_anim_set_values(&a, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE / 2);
    lv_anim_set_exec_cb(&a, _img_set_zoom_anim_cb);
    lv_anim_set_ready_cb(&a, NULL);
    lv_anim_start(&a);

    weather_img_obj = lv_create_weatherPage(weather_info_bg);

    lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
    lv_anim_set_var(&a, weather_img_obj);
    lv_anim_set_time(&a, 500);
    lv_anim_set_delay(&a, 100);
    lv_anim_set_values(&a, LV_IMG_ZOOM_NONE / 4, LV_IMG_ZOOM_NONE);
    lv_anim_set_exec_cb(&a, _img_set_zoom_anim_cb);
    lv_anim_set_ready_cb(&a, NULL);
    lv_anim_start(&a);

    lv_anim_init(&a);
    lv_anim_set_var(&a, weather_img_obj);
    lv_anim_set_values(&a, 0, LV_OPA_COVER);
    lv_anim_set_exec_cb(&a, album_fade_anim_cb);
    lv_anim_set_time(&a, 500);
    lv_anim_set_delay(&a, 100);
    lv_anim_start(&a);
}

static void return_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *button = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        LV_LOG_USER("SHORT_CLICKED %d buttton:%p, %d", code, button, button->state);
        lv_func_goto_layer(&main_Layer);
        //lv_anim_weather_next();
    } else if (LV_EVENT_DRAW_POST_END == code) {
    }
}

lv_obj_t *lv_create_weatherPage(lv_obj_t *parent)
{
    char temp_buf[30];
    int temp = (weather_list++) % (sizeof(weather_icon_list) / sizeof(weather_icon_list[0]));

    static lv_coord_t grid_weather_col_dsc[] = {100, 120, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_weather_row_dsc[] = {120, 80, 80, 80, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *weatherInfo = lv_img_create(parent);
    lv_obj_align(weatherInfo, LV_ALIGN_TOP_LEFT, 0, 0);
    //lv_obj_set_size(weatherInfo, lv_obj_get_content_width(parent), lv_obj_get_content_height(parent));
    lv_obj_set_size(weatherInfo, BSP_LCD_H_RES * 3 / 4, BSP_LCD_V_RES);
    lv_obj_set_style_radius(weatherInfo, 0, 0);
    lv_obj_set_style_bg_color(weatherInfo, lv_color_hex(0x0000), 0);//lv_color_hex(0x1F1F1F)
    lv_obj_set_grid_dsc_array(weatherInfo, grid_weather_col_dsc, grid_weather_row_dsc);

    /*
     * City info
     */
    lv_obj_t *label_location = lv_label_create(weatherInfo);
    lv_obj_remove_style_all(label_location);
    lv_obj_set_style_text_color(label_location, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_location, &font_lanting36, 0);

    lv_label_set_text(label_location, weather_icon_list[temp].location);
    lv_obj_set_grid_cell(label_location, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_t *label_location_status = lv_label_create(weatherInfo);
    lv_obj_remove_style_all(label_location_status);
    lv_obj_set_style_text_color(label_location_status, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_location_status, &font_lanting36, 0);

    lv_label_set_text(label_location_status, weather_icon_list[temp].status);
    lv_obj_set_grid_cell(label_location_status, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    /*
     * temp
     */
    lv_obj_t *label_temp = lv_label_create(weatherInfo);
    lv_obj_remove_style_all(label_temp);
    lv_obj_set_style_text_color(label_temp, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_temp, &font_num50, 0);

    memset(temp_buf, 0, sizeof(temp_buf));
    sprintf(temp_buf, "%d", weather_icon_list[temp].temperature);
    lv_label_set_text(label_temp, temp_buf);
    lv_obj_center(label_temp);
    lv_obj_set_grid_cell(label_temp, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    lv_obj_set_size(label_temp, 200, 60);

    lv_obj_t *temp_unit = lv_label_create(weatherInfo);
    lv_obj_remove_style_all(temp_unit);
    lv_obj_set_style_text_color(temp_unit, lv_color_hex(COLOUR_GREY), 0);
    lv_obj_set_style_text_font(temp_unit, &font_lantingMid18, 0);
    lv_label_set_text(temp_unit, "摄氏度");
    lv_obj_set_grid_cell(temp_unit, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_START, 2, 1);

    /*
     * temp range
     */
    lv_obj_t *label_temp_rang = lv_label_create(weatherInfo);
    lv_obj_remove_style_all(label_temp_rang);
    lv_obj_set_style_text_color(label_temp_rang, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_temp_rang, &font_num36, 0);

    lv_label_set_text(label_temp_rang, weather_icon_list[temp].tempRange);
    lv_obj_center(label_temp_rang);
    lv_obj_set_grid_cell(label_temp_rang, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    lv_obj_set_size(label_temp_rang, 200, 60);

    temp_unit = lv_label_create(weatherInfo);
    lv_obj_remove_style_all(temp_unit);
    lv_obj_set_style_text_color(temp_unit, lv_color_hex(COLOUR_GREY), 0);
    lv_obj_set_style_text_font(temp_unit, &font_lantingMid18, 0);
    lv_label_set_text(temp_unit, "摄氏度");
    lv_obj_set_grid_cell(temp_unit, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_START, 3, 1);

    return weatherInfo;
}

/*
 * Callback adapter function to convert parameter types to avoid compile-time
 * warning.
 */
static void _img_set_zoom_anim_cb(void *obj, int32_t zoom)
{
    lv_img_set_zoom((lv_obj_t *)obj, (uint16_t)zoom);
}

/*
 * Callback adapter function to convert parameter types to avoid compile-time
 * warning.
 */
static void _obj_set_x_anim_cb(void *obj, int32_t x)
{
    lv_obj_set_x((lv_obj_t *)obj, (lv_coord_t)x);
}

static void album_fade_anim_cb(void *var, int32_t v)
{
    lv_obj_set_style_img_opa(var, v, 0);
}

static bool weather_layer_enter_cb(void *layer)
{
    bool ret = false;
    lv_layer_t *create_layer = layer;

    if (NULL == create_layer->lv_obj_layer) {
        ret = true;
        create_layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(create_layer->lv_obj_layer);
        lv_obj_set_size(create_layer->lv_obj_layer, BSP_LCD_H_RES, BSP_LCD_V_RES);

        lv_obj_t *img_weather_bg = lv_img_create(create_layer->lv_obj_layer);
        lv_img_set_src(img_weather_bg, &weather_rain);
        lv_obj_align(img_weather_bg, LV_ALIGN_TOP_LEFT, 0, 0);

        weather_info_bg = lv_obj_create(img_weather_bg);
        lv_obj_remove_style_all(weather_info_bg);
        lv_obj_set_size(weather_info_bg, BSP_LCD_H_RES * 3 / 4, BSP_LCD_V_RES);
        lv_obj_align(weather_info_bg, LV_ALIGN_TOP_MID, 0, 0);

        weather_list = 0;
        weather_img_obj = lv_create_weatherPage(weather_info_bg);

        lv_obj_t *btn_return = lv_btn_create(img_weather_bg);
        lv_obj_remove_style_all(btn_return);
        lv_obj_set_size(btn_return, 100, 60);
        lv_obj_align(btn_return, LV_ALIGN_TOP_LEFT, 30, 20);
        lv_obj_add_event_cb(btn_return, return_btn_event_cb, LV_EVENT_CLICKED, img_weather_bg);

        lv_obj_t *label_return = lv_label_create(btn_return);
        lv_obj_remove_style_all(label_return);
        lv_obj_set_style_text_color(label_return, lv_color_white(), 0);
        lv_obj_set_style_text_font(label_return, &font_lanting24, 0);
        lv_label_set_text(label_return, "返回");
        lv_obj_center(label_return);
    }
    feed_clock_time();
    set_time_out(&time_2000ms, 2000);

    return ret;
}

static bool weather_layer_exit_cb(void *layer)
{
    LV_LOG_USER("exit");
    return true;
}

static void weather_layer_timer_cb(lv_timer_t *tmr)
{
    if (is_time_out(&time_2000ms)) {
        lv_anim_weather_next();
        // lv_func_goto_layer(&main_Layer);
    }
}
