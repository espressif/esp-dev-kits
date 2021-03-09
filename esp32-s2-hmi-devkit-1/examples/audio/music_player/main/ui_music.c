/**
 * @file ui_music.c
 * @brief Music example UI
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

#include "ui_music.h"

#define COLOR_BAR   lv_color_make(86, 94, 102)
#define COLOR_THEME lv_color_make(252, 199, 0)
#define COLOR_DEEP  lv_color_make(246, 174, 61)
#define COLOR_TEXT  lv_color_make(56, 56, 56)
#define COLOR_BG    lv_color_make(238, 241, 245)

static lv_obj_t *img_album = NULL;
static lv_obj_t *label_music_name = NULL;
static lv_obj_t *btn_prev = NULL;
static lv_obj_t *btn_play_pause = NULL;
static lv_obj_t *btn_next = NULL;
static lv_obj_t *label_music_time = NULL;
static lv_obj_t *label_music_length = NULL;
static lv_obj_t *btn_play_mode = NULL;
static lv_obj_t *btn_list = NULL;
static lv_obj_t *roller_lyric = NULL;
static lv_obj_t *slider_volume = NULL;
static lv_obj_t *obj_lyric = NULL;
static lv_obj_t *obj_control = NULL;
static lv_obj_t *slider_progress = NULL;
static lv_obj_t *img_mask = NULL;
static const char * music_lyric[] = {
    "Poker Face - Lady Gaga\n"
    "Lyrics by:Lady GaGa/Red One\n"
    "Composed by:Lady GaGa/Red One\n"
    "Mum mum mum mah\n"
    "Mum mum mum mah\n"
    "I wanna hold 'em like they do in Texas plays\n"
    "Fold 'em let 'em hit me raise it baby stay with me\n"
    "I love it\n"
    "Luck and intuition play the cards with spades to start\n"
    "And after he's been hooked I'll play the one that's on his heart\n"
    "Oh oh oh oh ohhhh ohh oh e ohh oh oh\n"
    "I'll get him hot show him what I've got\n"
    "Oh oh oh oh ohhhh ohh oh e ohh oh oh\n"
    "I'll get him hot show him what I've got\n"
    "Can't read my\n"
    "Can't read my\n"
    "No he can't read my poker face\n"
    "She's got to love nobody\n"
    "Can't read my\n"
    "Can't read my\n"
    "No he can't read my poker face\n"
    "She's got to love nobody\n"
    "P-p-p-poker face p-p-poker face\n"
    "Mum mum mum mah\n"
    "P-p-p-poker face p-p-poker face\n"
    "Mum mum mum mah\n"
    "I wanna roll with him a hard pair we will be\n"
    "A little gambling is fun when you're with me I love it\n"
    "Russian roulette is not the same without a gun\n"
    "And baby when it's love if its not rough it isn't fun fun\n"
    "Oh oh oh oh ohhhh ohh oh e ohh oh oh\n"
    "I'll get him hot show him what I've got\n"
    "Oh oh oh oh ohhhh ohh oh e ohh oh oh\n"
    "I'll get him hot show him what I've got\n"
    "Can't read my\n"
    "Can't read my\n"
    "No he can't read my poker face\n"
    "She's got to love nobody\n"
    "Can't read my\n"
    "Can't read my\n"
    "No he can't read my poker face\n"
    "She's got to love nobody\n"
    "P-p-p-poker face p-p-poker face\n"
    "Mum mum mum mah\n"
    "P-p-p-poker face p-p-poker face\n"
    "Mum mum mum mah\n"
    "I won't tell you that I love you\n"
    "Kiss or hug you\n"
    "Cause I'm bluffing with my muffin\n"
    "I'm not lying I'm just stunning with my love glue gunning\n"
    "Just like a chick in the casino\n"
    "Take your bank before I pay you out\n"
    "I promise this promise this\n"
    "Check this hand cause I'm marvelous\n"
    "Can't read my\n"
    "Can't read my\n"
    "No he can't read my poker face\n"
    "She's got to love nobody\n"
    "Can't read my\n"
    "Can't read my\n"
    "No he can't read my poker face\n"
    "She's got to love nobody\n"
    "Can't read my\n"
    "Can't read my\n"
    "No he can't read my poker face\n"
    "She's got to love nobody\n"
    "Can't read my\n"
    "Can't read my\n"
    "No he can't read my poker face\n"
    "She's got to love nobody\n"
    "Can't read my\n"
    "Can't read my\n"
    "No he can't read my poker face\n"
    "She's got to love nobody\n"
    "Can't read my\n"
    "Can't read my\n"
    "No he can't read my poker face\n"
    "She's got to love nobody\n"
    "P-p-p-poker face p-p-poker face\n"
    "P-p-p-poker face p-p-poker face\n"
    "P-p-p-poker face p-p-poker face\n"
    "Mum mum mum mah\n"
    "P-p-p-poker face p-p-poker face\n"
    "Mum mum mum mah\n"
    "P-p-p-poker face p-p-poker face\n"
    "Mum mum mum mah\n"
    "P-p-p-poker face p-p-poker face\n"
    "Mum mum mum mah",
    
    "Outside - Ellie Goulding / Calvin Harris\n"
    "Lyrics by : Calvin Harris\n"
    "Composed by : Calvin Harris\n"
    "Look at what you've done\n"
    "Stand still falling away from me\n"
    "When it takes so long\n"
    "Fire's out what do you want to be\n"
    "Now I'm holding on\n"
    "Myself was never enough for me\n"
    "Gotta be so strong\n"
    "There's a power in what you do\n"
    "Now every other day\n"
    "I'll be watching you\n"
    "I'll show you what it feels like\n"
    "Now I'm on the outside\n"
    "We did everything right\n"
    "Now I'm on the outside\n"
    "I'll show you what it feels like\n"
    "Now I'm on the outside\n"
    "We did everything right\n"
    "Now I'm on the outside\n"
    "So you give me no reason\n"
    "For me to stay close to you\n"
    "Tell me what lovers do\n"
    "How are we still breathing\n"
    "It's never for us to choose\n"
    "I'll be the strength in you\n"
    "Now I'm holding on\n"
    "I'm holding on\n"
    "Myself was never enough for me\n"
    "Gotta be so strong\n"
    "There's a power in what you do\n"
    "Now every other day\n"
    "I'll be watching you\n"
    "I'll show you what it feels like\n"
    "Now I'm on the outside\n"
    "We did everything right\n"
    "Now I'm on the outside\n"
    "I'll show you what it feels like\n"
    "Now I'm on the outside\n"
    "We did everything right\n"
    "Now I'm on the outside\n"
    "I'll show you what it feels like\n"
    "Now I'm on the outside\n"
    "I'll show you what it feels like\n"
    "I'll show you what it feels like\n"
    "Now I'm on the outside\n"
    "We did everything right\n"
    "Now I'm on the outside",
};

static void slider_volume_cb(lv_obj_t *obj, lv_event_t event);
static void btn_play_pause_cb(lv_obj_t *obj, lv_event_t event);

static bool is_playing = false;

LV_IMG_DECLARE(music_album)

void ui_music(void)
{
    img_mask = lv_objmask_create(lv_scr_act(), NULL);
    lv_obj_set_size(img_mask, 256, 256);
    lv_obj_set_style_local_radius(img_mask, LV_OBJMASK_PART_MAIN, LV_STATE_DEFAULT, 90);
    lv_obj_set_pos(img_mask, 25, 50);

    do {
        lv_area_t a;
        lv_draw_mask_radius_param_t r1;
        a.x1 = 0;
        a.y1 = 0;
        a.x2 = 255;
        a.y2 = 255;
        lv_draw_mask_radius_init(&r1, &a, 10, false);
        lv_objmask_add_mask(img_mask, &r1);
    } while (0);

    img_album = lv_img_create(img_mask, NULL);
    lv_img_set_src(img_album, &music_album);
    lv_obj_set_style_local_radius(img_album, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_align(img_album, NULL, LV_ALIGN_CENTER, 0, 0);

    obj_lyric = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(obj_lyric, 465, 305);
    lv_obj_set_style_local_radius(obj_lyric, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
    lv_obj_set_style_local_border_width(obj_lyric, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(obj_lyric, img_mask, LV_ALIGN_OUT_RIGHT_TOP, 25, 0);

    slider_volume = lv_slider_create(lv_scr_act(), NULL);
    lv_obj_set_size(slider_volume, 200, 4);
    lv_slider_set_range(slider_volume, 0, 100);
    lv_slider_set_value(slider_volume, 30, LV_ANIM_ON);
    lv_obj_set_style_local_radius(slider_volume, LV_BAR_PART_BG, LV_STATE_DEFAULT, 15);
    lv_obj_align(slider_volume, img_mask, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 30);
    lv_obj_set_style_local_value_str(slider_volume, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_SYMBOL_VOLUME_MID);
    lv_obj_set_style_local_value_align(slider_volume, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_LEFT_MID);
    lv_obj_set_style_local_value_font(slider_volume, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, &lv_font_montserrat_32);
    lv_obj_set_style_local_value_ofs_x(slider_volume, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, -20);
    lv_obj_set_style_local_value_color(slider_volume, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_bg_color(slider_volume, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_event_cb(slider_volume, slider_volume_cb);

    obj_control = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_click(obj_control, false);
    lv_obj_set_style_local_border_width(obj_control, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_size(obj_control, 750, 80);
    lv_obj_align(obj_control, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -25);

    btn_prev = lv_btn_create(obj_control, NULL);
    lv_obj_set_size(btn_prev, 60, 60);
    lv_obj_set_style_local_radius(btn_prev, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 30);
    lv_obj_set_style_local_value_str(btn_prev, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PREV);
    lv_obj_set_style_local_border_width(btn_prev, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_width(btn_prev, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, 0);
    lv_obj_set_style_local_outline_width(btn_prev, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_outline_width(btn_prev, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, 0);
    
    lv_obj_set_style_local_bg_color(btn_prev, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_value_font(btn_prev, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_32);
    lv_obj_align(btn_prev, NULL, LV_ALIGN_IN_LEFT_MID, 25, 0);

    btn_play_pause = lv_btn_create(obj_control, btn_prev);
    lv_obj_set_style_local_value_str(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PLAY);
    lv_obj_set_style_local_value_ofs_x(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_border_width(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 2);
    lv_obj_set_style_local_border_width(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, 2);
    lv_obj_set_style_local_border_color(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_border_color(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, COLOR_THEME);
    lv_obj_set_style_local_border_color(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_value_align(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_align(btn_play_pause, btn_prev, LV_ALIGN_OUT_RIGHT_MID, 25, 0);
    lv_obj_set_event_cb(btn_play_pause, btn_play_pause_cb);

    btn_next = lv_btn_create(obj_control, btn_prev);
    lv_obj_set_style_local_value_str(btn_next, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_NEXT);
    lv_obj_align(btn_next, btn_play_pause, LV_ALIGN_OUT_RIGHT_MID, 25, 0);

    slider_progress = lv_slider_create(obj_control, NULL);
    lv_obj_set_size(slider_progress , 250, 4);
    lv_obj_set_click(slider_progress, false);
    lv_obj_set_style_local_bg_color(slider_progress, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_align(slider_progress, btn_next, LV_ALIGN_OUT_RIGHT_MID, 35, 0);

    label_music_time = lv_label_create(obj_control, NULL);
    lv_label_set_text(label_music_time, "00:00");
    lv_obj_set_style_local_text_font(label_music_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    lv_obj_align(label_music_time, slider_progress, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);

    label_music_length = lv_label_create(obj_control, NULL);
    lv_label_set_text(label_music_length, "00:00");
    lv_obj_set_style_local_text_font(label_music_length, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    lv_obj_align(label_music_length, slider_progress, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 6);
    
    btn_play_mode = lv_btn_create(obj_control, btn_prev);
    lv_obj_set_style_local_value_str(btn_play_mode, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_REFRESH);
    lv_obj_set_style_local_value_color(btn_play_mode, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_value_color(btn_play_mode, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_obj_align(btn_play_mode, slider_progress, LV_ALIGN_OUT_RIGHT_MID, 35, 0);

    btn_list = lv_btn_create(obj_control, btn_prev);
    lv_obj_set_style_local_value_str(btn_list, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_LIST);
    lv_obj_set_style_local_value_color(btn_list, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_value_color(btn_list, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_obj_align(btn_list, btn_play_mode, LV_ALIGN_OUT_RIGHT_MID, 25, 0);
    // lv_obj_set_event_cb(btn_list, btn_list_cb);

    label_music_name = lv_label_create(obj_lyric, NULL);
    lv_label_set_text(label_music_name, "Poker Face");
    lv_obj_set_style_local_text_font(label_music_name, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_48);
    lv_obj_align(label_music_name, NULL, LV_ALIGN_IN_TOP_MID, 0, 40);

    roller_lyric = lv_roller_create(lv_scr_act(), NULL);
    lv_roller_set_options(roller_lyric,
                        music_lyric[0],
                        LV_ROLLER_MODE_NORMAL);

    lv_roller_set_visible_row_count(roller_lyric, 3);
    lv_obj_set_style_local_text_font(roller_lyric, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, &font_en_20);
    lv_obj_set_style_local_text_font(roller_lyric, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, &font_en_20);

    lv_obj_set_style_local_bg_color(roller_lyric, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(roller_lyric, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    
    lv_obj_set_style_local_text_color(roller_lyric, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_text_color(roller_lyric, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, COLOR_THEME);

    lv_obj_set_style_local_bg_opa(roller_lyric, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, 0);
    
    lv_roller_set_auto_fit(roller_lyric, false);
    lv_obj_set_style_local_border_width(roller_lyric, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_width(roller_lyric, 450);
    lv_obj_align(roller_lyric, label_music_name, LV_ALIGN_OUT_BOTTOM_MID, 0, 60);
}

static void slider_volume_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_VALUE_CHANGED == event) {
        tpl0401_set_resistance(lv_slider_get_value(obj));
    }
}

static void btn_play_pause_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        if (is_playing) {
            lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PLAY);
            audio_hal_pause();
        } else {
            lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PAUSE);
            audio_hal_start();
        }
        is_playing = !is_playing;
    }
}

