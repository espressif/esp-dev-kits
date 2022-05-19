//#include "../lv_examples.h"
#include "lv_demo.h"
#include "./lv_example_func.h"
#include "./lv_example_image.h"

#if LV_BUILD_EXAMPLES

static bool weather_layer_enter_cb(struct lv_layer_t * layer);
static bool weather_layer_exit_cb(struct lv_layer_t * layer);
static void weather_layer_timer_cb(lv_timer_t * tmr);

lv_layer_t weather_Layer ={
    .lv_obj_name    = "weather_Layer",
    .lv_obj_layer   = NULL,
    .lv_show_layer  = &show_set_layer,
    //.lv_show_layer  = NULL,
    .enter_cb       = weather_layer_enter_cb,
    .exit_cb        = weather_layer_exit_cb,
    .timer_cb       = weather_layer_timer_cb,
};

uint32_t idle_period_Test;
static uint32_t period_weather_next, list;
static lv_obj_t * labelLocation, * labelTemp, * labelTempRang;
static lv_obj_t * setpage;
static lv_obj_t * album_img_obj;

extern lv_obj_t *  lv_create_weatherPage(lv_obj_t * parent);

static lv_obj_t * album_img_create(lv_obj_t * parent)
{
    LV_IMG_DECLARE(img_lv_demo_music_cover_1);
    LV_IMG_DECLARE(img_lv_demo_music_cover_2);
    LV_IMG_DECLARE(img_lv_demo_music_cover_3);

    lv_obj_t * img;
    img = lv_img_create(parent);
    //lv_obj_set_size(img, 200, 200);

    list++;

    LV_LOG_USER("##### list:%d, child:%d", list, lv_obj_get_child_cnt(parent));
    switch(list%3) {
    case 2:
        lv_img_set_src(img, &weather_icon_rain_home);
        //spectrum = spectrum_3;
        //spectrum_len = sizeof(spectrum_3) / sizeof(spectrum_3[0]);
        break;
    case 1:
        lv_img_set_src(img, &weather_icon_cloudy_home);
        //spectrum = spectrum_2;
        //spectrum_len = sizeof(spectrum_2) / sizeof(spectrum_2[0]);
        break;
    case 0:
        lv_img_set_src(img, &weather_icon_summer_home);
        //spectrum = spectrum_1;
        //spectrum_len = sizeof(spectrum_1) / sizeof(spectrum_1[0]);
        break;
    }
    lv_img_set_antialias(img, false);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    //lv_obj_align(img, LV_ALIGN_TOP_RIGHT, 200, 200);
    //lv_obj_add_event_cb(img, album_gesture_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(img, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t * labelReturn = lv_label_create(img);
    lv_obj_remove_style_all(labelReturn);
    lv_obj_set_style_text_color(labelReturn, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelReturn, &font_lanting24, 0);
    lv_label_set_text(labelReturn, "测试");
    lv_obj_center(labelReturn);

    return img;

}
void lv_anim_weather_next()
{
#if 1    
    lv_obj_fade_out(album_img_obj, 500, 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, album_img_obj);
    lv_anim_set_time(&a, 500);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
#if LV_DEMO_MUSIC_LANDSCAPE
    if(next) {
        lv_anim_set_values(&a, 0, - LV_HOR_RES / 7);
    } else {
        lv_anim_set_values(&a, 0, LV_HOR_RES / 7);
    }
#else
    if(1) {
        lv_anim_set_values(&a, 0, - LV_HOR_RES / 2);
    } else {
        lv_anim_set_values(&a, 0, LV_HOR_RES / 2);
    }
#endif
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_x);
    lv_anim_set_ready_cb(&a, lv_obj_del_anim_ready_cb);
    lv_anim_start(&a);

    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_set_var(&a, album_img_obj);
    lv_anim_set_time(&a, 500);
    lv_anim_set_values(&a, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE / 2);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_img_set_zoom);
    lv_anim_set_ready_cb(&a, NULL);
    lv_anim_start(&a);

    //album_img_obj = album_img_create(setpage);   
    album_img_obj = lv_create_weatherPage(setpage);//album_img_create(weather_Layer.lv_obj_layer);
    lv_obj_fade_in(album_img_obj, 500, 100);

    lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
    lv_anim_set_var(&a, album_img_obj);
    lv_anim_set_time(&a, 500);
    lv_anim_set_delay(&a, 100);
    lv_anim_set_values(&a, LV_IMG_ZOOM_NONE / 4, LV_IMG_ZOOM_NONE);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_img_set_zoom);
    lv_anim_set_ready_cb(&a, NULL);
    lv_anim_start(&a);
#endif
}
static void return_btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * button = lv_event_get_target(e);
    //uint8_t coverflowPage = lv_event_get_user_data(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("SHORT_CLICKED %d,coverflowPage:%X, buttton:%X,%d", code, 0, button, button->state);
        //lv_obj_add_flag(lv_tv_mainmenu, LV_OBJ_FLAG_HIDDEN);
        main_Layer.lv_obj_user = &weather_Layer;
        lv_func_goto_layer(&weather_Layer, &main_Layer);
        //lv_anim_weather_next();
    }
    else if(LV_EVENT_DRAW_POST_END == code){
    }
}

static lv_coord_t grid_2_col_dsc[] = {100, LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
static lv_coord_t grid_2_row_dsc[] = {
                80,  /*Title*/
                80,  /*Box title*/
                80,  /*Box title*/
                80,  /*Title*/
                LV_GRID_TEMPLATE_LAST
};

lv_obj_t *  lv_create_weatherPage(lv_obj_t * parent)
{
    int i, temp, weatherTabviewnum;
    lv_obj_t * temp_unit, * btn_return, * labelReturn, *weatherInfo;
    uint8_t locationTemp[30];

    list++;
    weatherTabviewnum = sizeof(weather_icon_list)/sizeof(weather_icon_list[0]);
    temp = (list)%(weatherTabviewnum);

    LV_LOG_USER("#####period_weather_next %d-%d, %d, child:%d", temp, weatherTabviewnum, list, lv_obj_get_child_cnt(parent));

    #if 0
    setpage = lv_img_create(parent);
    if(list%2){
        lv_img_set_src(setpage, &weather_rain);
    }
    else{
        lv_img_set_src(setpage, &weather_rain);
    }
    lv_obj_align(setpage, LV_ALIGN_TOP_LEFT, 0, 0);
    weatherInfo = setpage;
    #else
    weatherInfo = lv_img_create(parent);
    lv_obj_align(weatherInfo, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(weatherInfo, 480, 480);
    lv_obj_set_style_radius(weatherInfo, 0, 0);
    //lv_obj_set_style_border_width(weatherInfo, 0, 0);
    lv_obj_set_style_bg_color(weatherInfo, lv_color_hex(0x0000), 0);//lv_color_hex(0x1F1F1F)
    #endif

    lv_obj_set_grid_dsc_array(weatherInfo, grid_2_col_dsc, grid_2_row_dsc);

    btn_return = lv_btn_create(weatherInfo);
    lv_obj_remove_style_all(btn_return);
    lv_obj_set_size(btn_return, 100, 60);
    lv_obj_add_event_cb(btn_return, return_btn_event_cb, LV_EVENT_CLICKED, weatherInfo);
    lv_obj_set_grid_cell(btn_return, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    labelReturn = lv_label_create(btn_return);
    lv_obj_remove_style_all(labelReturn);
    lv_obj_set_style_text_color(labelReturn, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelReturn, &font_lanting24, 0);
    lv_label_set_text(labelReturn, "返回");
    lv_obj_center(labelReturn);

    labelLocation = lv_label_create(weatherInfo);
    lv_obj_remove_style_all(labelLocation);
    lv_obj_set_style_text_color(labelLocation, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelLocation, &font_lanting36, 0);

    sprintf(locationTemp, "%s %s",weather_icon_list[temp].location, weather_icon_list[temp].status);
    lv_label_set_text(labelLocation, locationTemp);
    lv_obj_center(labelLocation);
    //lv_obj_align(labelLocation, LV_ALIGN_CENTER, 50, 0);
    lv_obj_set_grid_cell(labelLocation, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    labelTemp = lv_label_create(weatherInfo);
    lv_obj_remove_style_all(labelTemp);
    lv_obj_set_style_text_color(labelTemp, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelTemp, &font_num50, 0);

    memset(locationTemp, 0, sizeof(locationTemp));
    sprintf(locationTemp, "%d",weather_icon_list[temp].temperature);
    lv_label_set_text(labelTemp, locationTemp);
    lv_obj_center(labelTemp);
    lv_obj_set_grid_cell(labelTemp, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 2,1);
    lv_obj_set_size(labelTemp, 200, 60);

    temp_unit = lv_label_create(labelTemp);
    lv_obj_remove_style_all(temp_unit);
    lv_obj_set_style_text_color(temp_unit, lv_color_hex(COLOUR_GREY), 0);
    lv_obj_set_style_text_font(temp_unit, &font_lantingMid18, 0);

    lv_label_set_text(temp_unit, "摄氏度");
    lv_obj_align(temp_unit, LV_ALIGN_OUT_TOP_RIGHT, 80, 0);

    labelTempRang = lv_label_create(weatherInfo);
    lv_obj_remove_style_all(labelTempRang);
    lv_obj_set_style_text_color(labelTempRang, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelTempRang, &font_num36, 0);

    lv_label_set_text(labelTempRang, weather_icon_list[temp].tempRange);
    lv_obj_center(labelTempRang);
    lv_obj_set_grid_cell(labelTempRang, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    lv_obj_set_size(labelTempRang, 200, 60);

    temp_unit = lv_label_create(labelTempRang);
    lv_obj_remove_style_all(temp_unit);
    lv_obj_set_style_text_color(temp_unit, lv_color_hex(COLOUR_GREY), 0);
    lv_obj_set_style_text_font(temp_unit, &font_lantingMid18, 0);

    lv_label_set_text(temp_unit, "摄氏度");
    lv_obj_align(temp_unit, LV_ALIGN_OUT_TOP_RIGHT, 100, 0);

    return weatherInfo;
}

static bool weather_layer_enter_cb(struct lv_layer_t * layer)
{
	bool ret = false;

	if(NULL == layer->lv_obj_layer){
		ret = true;
		layer->lv_obj_layer = lv_tileview_create(lv_scr_act());

        #if 1
        setpage = lv_img_create(layer->lv_obj_layer);
		lv_img_set_src(setpage, &weather_rain);
		lv_obj_align(setpage, LV_ALIGN_TOP_LEFT, 0, 0);
        #else
        lv_obj_set_style_bg_color(layer->lv_obj_layer, lv_color_hex(COLOUR_BG1_GREY), 0);
        #endif

        album_img_obj = lv_create_weatherPage(setpage);
        //album_img_obj = lv_create_weatherPage(weather_Layer.lv_obj_layer);
        //album_img_obj = album_img_create(setpage);

        /*
        lv_obj_t * btn_return = lv_btn_create(weather_Layer.lv_obj_layer);
        lv_obj_remove_style_all(btn_return);
        lv_obj_set_size(btn_return, 100, 60);
        lv_obj_add_event_cb(btn_return, return_btn_event_cb, LV_EVENT_CLICKED, setpage);

        lv_obj_t * labelReturn = lv_label_create(btn_return);
        lv_obj_remove_style_all(labelReturn);
        lv_obj_set_style_text_color(labelReturn, lv_color_white(), 0);
        lv_obj_set_style_text_font(labelReturn, &font_lanting24, 0);
        lv_label_set_text(labelReturn, "返回");
        lv_obj_center(labelReturn);
        */

        period_weather_next = lv_tick_get();
        reload_screenOff_timer();
        LV_LOG_USER("do period_weather_next %d", period_weather_next);
        list = 0;
	}

	return ret;
}

static bool weather_layer_exit_cb(struct lv_layer_t * layer)
{
    LV_LOG_USER("");
}

static void weather_layer_timer_cb(lv_timer_t * tmr)
{
    //uint32_t idle_period_Test;
    int32_t isTmOut, weatherTabviewnum, temp;
    uint8_t locationTemp[30];

    isTmOut = (lv_tick_get() - (idle_period_Test + 500));
    if(isTmOut > 0){
        idle_period_Test = lv_tick_get();
        //lv_func_goto_layer(&weather_Layer, &main_Layer);
    }

    isTmOut = (lv_tick_get() - (period_weather_next + 2000));
    if(isTmOut > 0){
        period_weather_next = lv_tick_get();
        lv_anim_weather_next();
        /*
        list++;
        weatherTabviewnum = sizeof(weather_icon_list)/sizeof(weather_icon_list[0]);
        temp = (list)%(weatherTabviewnum);

        memset(locationTemp, 0, sizeof(locationTemp));
        sprintf(locationTemp, "%s %s",weather_icon_list[temp].location, weather_icon_list[temp].status);
        lv_label_set_text(labelLocation, locationTemp);
        LV_LOG_USER("->%d,%s", temp, locationTemp);

        memset(locationTemp, 0, sizeof(locationTemp));
        sprintf(locationTemp, "%d",weather_icon_list[temp].temperature);
        lv_label_set_text(labelTemp, locationTemp);

        lv_label_set_text(labelTempRang, weather_icon_list[temp].tempRange);
    */
    }
    trigger_screenOff_timer(&weather_Layer);
}

#endif

