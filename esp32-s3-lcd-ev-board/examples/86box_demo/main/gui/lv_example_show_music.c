//#include "../lv_examples.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "lv_demo.h"
#include "./lv_example_func.h"
#include "./lv_example_image.h"

#include "audio.h"

#if LV_BUILD_EXAMPLES


extern QueueHandle_t audio_adv_event_queue;

static bool show_music_layer_enter_cb(struct lv_layer_t * layer);
static bool show_music_layer_exit_cb(struct lv_layer_t * layer);
static void show_music_layer_timer_cb(lv_timer_t * tmr);

lv_layer_t show_music_layer ={
    .lv_obj_name    = "show_music_layer",
    .lv_obj_layer   = NULL,
    .lv_show_layer  = NULL,
    .enter_cb       = show_music_layer_enter_cb,
    .exit_cb        = show_music_layer_exit_cb,
    .timer_cb       = show_music_layer_timer_cb,
};


static uint32_t period_clock_next;

static bool show_music_layer_enter_cb(struct lv_layer_t * layer)
{
	bool ret = false;
    LV_LOG_USER("Enter:%s", layer->lv_obj_name);
	if(NULL == layer->lv_obj_layer){
		ret = true;

        layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(layer->lv_obj_layer);
        lv_obj_set_size(layer->lv_obj_layer, 480, 480);

        lv_demo_music(layer->lv_obj_layer);
        period_clock_next = lv_tick_get();
	}

	return ret;
}

extern lv_timer_t * sec_counter_timer;
extern lv_timer_t * timer;
extern bool start_anim;

static bool show_music_layer_exit_cb(struct lv_layer_t * layer)
{   
#if 1
    lv_anim_del_all();
    //_lv_demo_music_pause();
    audio_pause();

    if(sec_counter_timer){
        lv_timer_del(sec_counter_timer);
    }

    if(timer){
        lv_timer_del(timer);
    }
    LV_LOG_USER("lv_timer_del, %X,%X", timer, sec_counter_timer);
#else
    lv_timer_pause(sec_counter_timer);
    lv_timer_pause(timer);
    LV_LOG_USER("lv_timer_pause");
#endif
}

static void show_music_layer_timer_cb(lv_timer_t * tmr)
{
    int32_t isTmOut;
    adv_event_t adv_event = ADV_EVENT_NONE;

    isTmOut = (lv_tick_get() - (period_clock_next + 500));
    if(isTmOut > 0){
        period_clock_next = lv_tick_get(); 
    }

    if (pdPASS == xQueueReceive(audio_adv_event_queue, &adv_event, 0)) {
        if (ADV_EVENT_NEXT == adv_event) {
             LV_LOG_USER("#######ADV_EVENT_NEXT");
             _lv_demo_music_album_passive_next();
             LV_LOG_USER("#######ADV_EVENT_NEXT");
        }
        else if(ADV_EVENT_FILE_SCAN_DONE == adv_event){
            LV_LOG_USER("#######ADV_EVENT_FILE_SCAN_DONE");
        }
    }
}

#endif
