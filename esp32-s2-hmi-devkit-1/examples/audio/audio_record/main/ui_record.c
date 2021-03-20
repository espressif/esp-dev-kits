/**
 * @file ui_record.c
 * @brief Audio record example ui
 * @version 0.1
 * @date 2021-03-04
 * 
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 * 
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "ui_record.h"

#define COLOR_BAR   lv_color_make(86, 94, 102)
#define COLOR_THEME lv_color_make(252, 199, 0)
#define COLOR_DEEP  lv_color_make(246, 174, 61)
#define COLOR_TEXT  lv_color_make(56, 56, 56)
#define COLOR_BG    lv_color_make(238, 241, 245)

static void btn_cb(lv_obj_t *obj, lv_event_t event);
static void slider_volume_cb(lv_obj_t *obj, lv_event_t event);

void ui_record(void)
{
    lv_port_sem_take();

    lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_size(btn, 100, 100);
    lv_obj_align(btn, NULL, LV_ALIGN_CENTER, 0, -100);
    lv_obj_set_event_cb(btn, btn_cb);

    lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text_static(label, "Press to record, release to play.");
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_28);
    lv_obj_align(label, btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);

    lv_obj_t *volume_slider = lv_slider_create(lv_scr_act(), NULL);
    lv_obj_set_width(volume_slider, 250);
    lv_slider_set_range(volume_slider, 0, 100);
    lv_slider_set_value(volume_slider, 30, LV_ANIM_ON);
    lv_obj_align(volume_slider, NULL, LV_ALIGN_CENTER, 0, 100);
    lv_obj_set_event_cb(volume_slider, slider_volume_cb);

    lv_obj_set_style_local_value_font(volume_slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_style_local_value_str(volume_slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, "Volume : 30");
    lv_obj_set_style_local_value_ofs_y(volume_slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, 30);

    lv_port_sem_give();
}

static void btn_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED == event) {
        audio_record_start();
    } else if (LV_EVENT_RELEASED == event) {
        audio_play_start();
    }
}

static void slider_volume_cb(lv_obj_t *obj, lv_event_t event)
{
    static char fmt_text[16];

    if (LV_EVENT_VALUE_CHANGED == event) {
        int16_t value = lv_slider_get_value(obj);
        tpl0401_set_resistance(value * 255 / 100);
        sprintf(fmt_text, "Volume : %d", value);
        lv_obj_set_style_local_value_str(obj, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, fmt_text);
    }
}
