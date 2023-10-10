/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "lv_example_pub.h"
#include "lv_example_image.h"

#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"

#include "audio_player.h"
#include "file_iterator.h"
#include "lv_demo_music.h"

static file_iterator_instance_t *file_iterator;
extern QueueHandle_t audio_adv_event_queue;

static bool show_music_layer_enter_cb(void *layer);
static bool show_music_layer_exit_cb(void *layer);
static void show_music_layer_timer_cb(lv_timer_t *tmr);

lv_layer_t show_music_layer = {
    .lv_obj_name    = "show_music_layer",
    .lv_obj_layer   = NULL,
    .lv_show_layer  = NULL,
    .enter_cb       = show_music_layer_enter_cb,
    .exit_cb        = show_music_layer_exit_cb,
    .timer_cb       = show_music_layer_timer_cb,
};

static bool show_music_layer_enter_cb(void *layer)
{
    bool ret = false;
    lv_layer_t *create_layer = layer;
    LV_LOG_USER("Enter:%s", create_layer->lv_obj_name);

    if (NULL == create_layer->lv_obj_layer) {
        ret = true;

        create_layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(create_layer->lv_obj_layer);
        lv_obj_set_size(create_layer->lv_obj_layer, BSP_LCD_H_RES, BSP_LCD_V_RES);

        file_iterator = bsp_extra_get_file_instance();
        assert(file_iterator != NULL);
        lv_demo_music(create_layer->lv_obj_layer);
    }

    return ret;
}

static bool show_music_layer_exit_cb(void *layer)
{
    return true;
}

static void show_music_layer_timer_cb(lv_timer_t *tmr)
{
    feed_clock_time();
    //adv_event_t adv_event = ADV_EVENT_NONE;

    // if (pdPASS == xQueueReceive(audio_adv_event_queue, &adv_event, 0)) {
    // if (ADV_EVENT_NEXT == adv_event) {
    //      LV_LOG_USER("#######ADV_EVENT_NEXT");
    //      _lv_demo_music_album_passive_next();
    //      LV_LOG_USER("#######ADV_EVENT_NEXT");
    // }
    // else if(ADV_EVENT_FILE_SCAN_DONE == adv_event){
    //     LV_LOG_USER("#######ADV_EVENT_FILE_SCAN_DONE");
    // }
    // }
}
