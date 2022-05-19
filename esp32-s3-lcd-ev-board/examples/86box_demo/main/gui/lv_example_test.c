//#include "../lv_examples.h"
#include "lv_demo.h"
#include "./lv_example_func.h"
#include "./lv_example_image.h"

#if LV_BUILD_EXAMPLES

static bool test_layer_enter_cb(struct lv_layer_t * layer);
static bool test_layer_exit_cb(struct lv_layer_t * layer);
static void test_layer_timer_cb(lv_timer_t * tmr);

lv_layer_t test_layer ={
    .lv_obj_name    = "test_layer",
    .lv_obj_layer   = NULL,
    .enter_cb       = test_layer_enter_cb,
    .exit_cb        = test_layer_exit_cb,
    .timer_cb       = test_layer_timer_cb,
};

static uint32_t period_clock_next;
static lv_obj_t * btn_changed;

static void btn_exit_clock_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * button = lv_event_get_target(e);

    if(code == LV_EVENT_SHORT_CLICKED) {
        LV_LOG_USER("SHORT_CLICKED %d, test_layer -> main_Layer");
        //main_Layer.lv_obj_user = &test_layer;
        //lv_func_goto_layer(&test_layer, &main_Layer);
    }
}

static bool test_layer_enter_cb(struct lv_layer_t * layer)
{
	bool ret = false;

	if(NULL == layer->lv_obj_layer){
		ret = true;
		layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(layer->lv_obj_layer);
        lv_obj_set_size(layer->lv_obj_layer, 480, 480);

        btn_changed = lv_btn_create(layer->lv_obj_layer);
        lv_obj_set_size(btn_changed, 480, 480);
        lv_obj_set_style_radius(btn_changed, 0, 0);
        lv_obj_set_style_bg_color(btn_changed, lv_color_hex(0x0000), 0);//lv_color_hex(0x1F1F1F)
        lv_obj_add_event_cb(btn_changed, btn_exit_clock_event_cb, LV_EVENT_ALL, btn_changed);

        period_clock_next = lv_tick_get();
	}

	return ret;
}

static bool test_layer_exit_cb(struct lv_layer_t * layer)
{
    LV_LOG_USER("");
}

static void test_layer_timer_cb(lv_timer_t * tmr)
{
    //uint32_t idle_period_Test;
    int32_t isTmOut;
    static uint8_t list = 0;

    isTmOut = (lv_tick_get() - (period_clock_next + 1500));
    if(isTmOut > 0){
        period_clock_next = lv_tick_get();
        list++;
        LV_LOG_USER("new, %d", list%5);

        switch(list%5)
        {
            case 0:
                lv_obj_set_style_bg_color(btn_changed, lv_color_hex(0xFF0000), 0);
            break;
             case 1:
                lv_obj_set_style_bg_color(btn_changed, lv_color_hex(0x00FF00), 0);
            break;
             case 2:
                lv_obj_set_style_bg_color(btn_changed, lv_color_hex(0x0000FF), 0);
            break;
            case 3:
                lv_obj_set_style_bg_color(btn_changed, lv_color_hex(0x000000), 0);
            break;
            case 4:
                lv_obj_set_style_bg_color(btn_changed, lv_color_hex(0xFFFFFF), 0);
            break;
        }
    }
}

#endif
