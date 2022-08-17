//#include "../lv_examples.h"
#include "lv_demo.h"
#include "./lv_example_func.h"
#include "./lv_example_image.h"

#if LV_BUILD_EXAMPLES

static bool clock_screen_layer_enter_cb(struct lv_layer_t * layer);
static bool clock_screen_layer_exit_cb(struct lv_layer_t * layer);
static void clock_screen_layer_timer_cb(lv_timer_t * tmr);

lv_layer_t clock_screen_layer ={
    .lv_obj_name    = "clock_screen_layer",
    .lv_obj_layer   = NULL,
    .enter_cb       = clock_screen_layer_enter_cb,
    .exit_cb        = clock_screen_layer_exit_cb,
    .timer_cb       = clock_screen_layer_timer_cb,
};

static uint32_t period_clock_next, period_clock_tick;
static lv_obj_t * label_clock_show;

static void btn_exit_clock_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * button = lv_event_get_target(e);

    if(code == LV_EVENT_SHORT_CLICKED) {
        LV_LOG_USER("SHORT_CLICKED %d, clock_screen_layer -> main_Layer");
        //lv_obj_add_flag(lv_tv_mainmenu, LV_OBJ_FLAG_HIDDEN);
        main_Layer.lv_obj_user = &clock_screen_layer;
        lv_func_goto_layer(&clock_screen_layer, &main_Layer);
    }
}

static void btn_press_clock_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * button = lv_event_get_target(e);

    if(code == LV_EVENT_SHORT_CLICKED) {
        LV_LOG_USER("SHORT_CLICKED %d, skip");
        period_clock_next = 0;
        //lv_obj_add_flag(lv_tv_mainmenu, LV_OBJ_FLAG_HIDDEN);
        //lv_func_goto_layer(&clock_screen_layer, &main_Layer);
    }
    if(code == LV_EVENT_PRESSED) {
        LV_LOG_USER("LV_EVENT_PRESSED %d, skip");
        lv_obj_set_style_text_color(label_clock_show, lv_color_hex(COLOUR_GREY), 0);
    }
}

static bool clock_screen_layer_enter_cb(struct lv_layer_t * layer)
{
	bool ret = false;
    uint8_t timeBuf[20];
    uint32_t temp;

	if(NULL == layer->lv_obj_layer){
		ret = true;
		layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(layer->lv_obj_layer);
        lv_obj_set_size(layer->lv_obj_layer, 480, 480);

        lv_obj_t * btn_clock_exit = lv_btn_create(layer->lv_obj_layer);
        lv_obj_set_size(btn_clock_exit, 480, 480);
        lv_obj_set_style_radius(btn_clock_exit, 0, 0);
        lv_obj_set_style_bg_color(btn_clock_exit, lv_color_hex(0x0000), 0);//lv_color_hex(0x1F1F1F)
        lv_obj_add_event_cb(btn_clock_exit, btn_exit_clock_event_cb, LV_EVENT_ALL, btn_clock_exit);
        
        label_clock_show = lv_label_create(layer->lv_obj_layer);
        lv_obj_remove_style_all(label_clock_show);
        lv_obj_set_style_text_color(label_clock_show, lv_color_white(), 0);
        lv_obj_set_style_text_font(label_clock_show, &font_num50, 0);

        temp = sys_clock_getSecond();
        sprintf(timeBuf, "%02d:%02d", (temp/60)%24,temp%60);
        lv_label_set_text(label_clock_show, timeBuf);
        lv_obj_align(label_clock_show, LV_ALIGN_CENTER, 0, 0);

        lv_obj_t * btn_clock_run = lv_btn_create(label_clock_show);
        lv_obj_set_size(btn_clock_run, 140, 80);
        lv_obj_set_style_radius(btn_clock_run, 0, 0);
        lv_obj_set_style_opa(btn_clock_run, LV_OPA_TRANSP, 0);
        //lv_obj_set_style_bg_color(btn_clock_run, lv_color_hex(0x001F1F), 0);//lv_color_hex(0x1F1F1F)
        lv_obj_add_event_cb(btn_clock_run, btn_press_clock_event_cb, LV_EVENT_ALL, btn_clock_run);

        period_clock_next = lv_tick_get();
        period_clock_tick = lv_tick_get();
	}

	return ret;
}

static bool clock_screen_layer_exit_cb(struct lv_layer_t * layer)
{
    LV_LOG_USER("");
}

static void clock_screen_layer_timer_cb(lv_timer_t * tmr)
{
    //uint32_t idle_period_Test;
    int32_t isTmOut;
    uint8_t timeBuf[20];
    uint32_t temp;

    isTmOut = (lv_tick_get() - (period_clock_next + 9500));
    if(isTmOut > 0){
        period_clock_next = lv_tick_get();
        lv_obj_set_style_text_color(label_clock_show, lv_color_white(), 0);
        lv_obj_align(label_clock_show, LV_ALIGN_TOP_LEFT, lv_rand(0, 350), lv_rand(0, 400));
        LV_LOG_USER("new location");
    }

    isTmOut = (lv_tick_get() - (period_clock_tick + 1000));
    if(isTmOut > 0){
        period_clock_tick = lv_tick_get();
        temp = sys_clock_getSecond();
        sprintf(timeBuf, "%02d:%02d", (temp/60)%24,temp%60);
        lv_label_set_text(label_clock_show, timeBuf);
    }
}

#endif
