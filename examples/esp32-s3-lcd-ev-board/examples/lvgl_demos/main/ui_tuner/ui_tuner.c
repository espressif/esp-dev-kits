// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.0.5
// LVGL VERSION: 8.2
// PROJECT: audio_mixer

#include "ui_tuner.h"
#include "ui_helpers.h"

///////////////////// VARIABLES ////////////////////
lv_obj_t * ui_Audio_Player;
lv_obj_t * ui_Footer;
lv_obj_t * ui_Slider_AB;
lv_obj_t * ui_A;
lv_obj_t * ui_B;
lv_obj_t * ui_Background;
lv_obj_t * ui_Content_group;
lv_obj_t * ui_Display;
lv_obj_t * ui_Label_artist;
lv_obj_t * ui_Label_song;
lv_obj_t * ui_Audio_Wave;
lv_obj_t * ui_Label_Time;
lv_obj_t * ui_BTN_EQ;
lv_obj_t * ui_Left_group;
lv_obj_t * ui_Slider_group_left;
lv_obj_t * ui_Max;
lv_obj_t * ui_Min;
lv_obj_t * ui_Slider2;
lv_obj_t * ui_Cut_1;
lv_obj_t * ui_BTN_Cut_Left;
lv_obj_t * ui_Label_Cut_1;
lv_obj_t * ui_Right_group;
lv_obj_t * ui_Cut_3;
lv_obj_t * ui_BTN_Cut_Left1;
lv_obj_t * ui_Label_Cut_3;
lv_obj_t * ui_Slider_group_right;
lv_obj_t * ui_Max1;
lv_obj_t * ui_Min2;
lv_obj_t * ui_Slider3;
lv_obj_t * ui_Center_group;
lv_obj_t * ui_Play;
lv_obj_t * ui_BTN_Play;
lv_obj_t * ui_Indicator_group;
lv_obj_t * ui_Label_Levels;
lv_obj_t * ui_Min1;
lv_obj_t * ui_Indicator_Left;
lv_obj_t * ui_Indicator_Right;
lv_obj_t * ui_EQ_group;
lv_obj_t * ui_EQ1;
lv_obj_t * ui_Image2;
lv_obj_t * ui_Arc1;
lv_obj_t * ui_Label_Low;
lv_obj_t * ui_EQ2;
lv_obj_t * ui_Image3;
lv_obj_t * ui_Arc2;
lv_obj_t * ui_Label_Mid;
lv_obj_t * ui_EQ3;
lv_obj_t * ui_Image4;
lv_obj_t * ui_Arc3;
lv_obj_t * ui_Label_High;
lv_obj_t * ui_Tempo_group;
lv_obj_t * ui_Slider_AB_2;
lv_obj_t * ui_Label_Tempo;

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
    #error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP !=0
    #error "#error LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////

//
// FUNCTION HEADER
void Fadeoff_Animation(lv_obj_t * TargetObject, int delay);

// FUNCTION
void Fadeoff_Animation(lv_obj_t * TargetObject, int delay)
{

    //
    lv_anim_t PropertyAnimation_0;
    lv_anim_init(&PropertyAnimation_0);
    lv_anim_set_time(&PropertyAnimation_0, 100);
    lv_anim_set_user_data(&PropertyAnimation_0, TargetObject);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_opacity);
    lv_anim_set_values(&PropertyAnimation_0, 255, 0);
    lv_anim_set_path_cb(&PropertyAnimation_0, lv_anim_path_linear);
    lv_anim_set_delay(&PropertyAnimation_0, delay + 0);
    lv_anim_set_early_apply(&PropertyAnimation_0, true);
    lv_anim_start(&PropertyAnimation_0);

}

//
// FUNCTION HEADER
void Fadeon_Animation(lv_obj_t * TargetObject, int delay);

// FUNCTION
void Fadeon_Animation(lv_obj_t * TargetObject, int delay)
{

    //
    lv_anim_t PropertyAnimation_0;
    lv_anim_init(&PropertyAnimation_0);
    lv_anim_set_time(&PropertyAnimation_0, 100);
    lv_anim_set_user_data(&PropertyAnimation_0, TargetObject);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_opacity);
    lv_anim_set_values(&PropertyAnimation_0, 0, 255);
    lv_anim_set_path_cb(&PropertyAnimation_0, lv_anim_path_linear);
    lv_anim_set_delay(&PropertyAnimation_0, delay + 0);
    lv_anim_set_early_apply(&PropertyAnimation_0, true);
    lv_anim_start(&PropertyAnimation_0);

}

///////////////////// FUNCTIONS ////////////////////
static void ui_event_BTN_EQ(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_VALUE_CHANGED &&  lv_obj_has_state(ta, LV_STATE_CHECKED)) {
        Fadeoff_Animation(ui_Slider_group_left, 0);
        Fadeoff_Animation(ui_Slider_group_right, 200);
        Fadeoff_Animation(ui_Indicator_group, 100);
        Fadeon_Animation(ui_EQ_group, 200);
        Fadeon_Animation(ui_Tempo_group, 300);
        _ui_flag_modify(ui_EQ_group, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
        _ui_flag_modify(ui_Tempo_group, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
    }
    if(event == LV_EVENT_VALUE_CHANGED &&  !lv_obj_has_state(ta, LV_STATE_CHECKED)) {
        Fadeon_Animation(ui_Slider_group_left, 0);
        Fadeon_Animation(ui_Indicator_group, 100);
        Fadeon_Animation(ui_Slider_group_right, 200);
        Fadeoff_Animation(ui_Tempo_group, 100);
        Fadeoff_Animation(ui_EQ_group, 0);
        _ui_flag_modify(ui_Tempo_group, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
        _ui_flag_modify(ui_EQ_group, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
    }
}
static void ui_event_Label_Cut_1(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_CLICKED) {
        _ui_flag_modify(ui_Audio_Player, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
    }
}
static void ui_event_Label_Cut_3(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_CLICKED) {
        _ui_flag_modify(ui_Audio_Player, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
    }
}

///////////////////// SCREENS ////////////////////
void ui_Audio_Player_screen_init(void)
{

    // ui_Audio_Player

    ui_Audio_Player = lv_obj_create(NULL);

    lv_obj_clear_flag(ui_Audio_Player, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Audio_Player, lv_color_hex(0x2D2F36), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Audio_Player, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Footer

    ui_Footer = lv_obj_create(ui_Audio_Player);

    lv_obj_set_height(ui_Footer, 128);
    lv_obj_set_width(ui_Footer, lv_pct(100));

    lv_obj_set_x(ui_Footer, 0);
    lv_obj_set_y(ui_Footer, 0);

    lv_obj_set_align(ui_Footer, LV_ALIGN_BOTTOM_MID);

    lv_obj_set_style_bg_color(ui_Footer, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Footer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Footer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Footer, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Footer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Footer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Footer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Footer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Footer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Slider_AB

    ui_Slider_AB = lv_slider_create(ui_Footer);
    lv_slider_set_range(ui_Slider_AB, -100, 100);
    lv_slider_set_mode(ui_Slider_AB, LV_SLIDER_MODE_SYMMETRICAL);
    lv_slider_set_value(ui_Slider_AB, 80, LV_ANIM_OFF);
    if(lv_slider_get_mode(ui_Slider_AB) == LV_SLIDER_MODE_RANGE) lv_slider_set_left_value(ui_Slider_AB, 0, LV_ANIM_OFF);

    lv_obj_set_width(ui_Slider_AB, 276);
    lv_obj_set_height(ui_Slider_AB, 79);

    lv_obj_set_x(ui_Slider_AB, 0);
    lv_obj_set_y(ui_Slider_AB, 0);

    lv_obj_set_align(ui_Slider_AB, LV_ALIGN_CENTER);

    lv_obj_set_style_bg_color(ui_Slider_AB, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider_AB, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_Slider_AB, &ui_img_pot_hor_line_png, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Slider_AB, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Slider_AB, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Slider_AB, 35, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Slider_AB, 35, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_Slider_AB, lv_color_hex(0x50FF7D), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider_AB, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Slider_AB, lv_color_hex(0x50FF7D), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_Slider_AB, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Slider_AB, 50, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_Slider_AB, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(ui_Slider_AB, 12, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Slider_AB, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Slider_AB, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_Slider_AB, &ui_img_pot_hor_knob_png, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Slider_AB, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_Slider_AB, 100, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Slider_AB, 30, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_Slider_AB, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui_Slider_AB, 5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui_Slider_AB, 5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Slider_AB, -1, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Slider_AB, -1, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Slider_AB, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Slider_AB, -4, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Slider_AB, lv_color_hex(0x50FF7D), LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(ui_Slider_AB, 255, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_color(ui_Slider_AB, lv_color_hex(0x50FF7D), LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_opa(ui_Slider_AB, 255, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(ui_Slider_AB, 40, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_spread(ui_Slider_AB, 0, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_ofs_x(ui_Slider_AB, 0, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_ofs_y(ui_Slider_AB, 0, LV_PART_KNOB | LV_STATE_PRESSED);

    // ui_A

    ui_A = lv_label_create(ui_Footer);

    lv_obj_set_width(ui_A, 18);
    lv_obj_set_height(ui_A, 24);

    lv_obj_set_x(ui_A, 40);
    lv_obj_set_y(ui_A, 0);

    lv_obj_set_align(ui_A, LV_ALIGN_LEFT_MID);

    lv_label_set_text(ui_A, "A");

    lv_obj_set_style_text_color(ui_A, lv_color_hex(0x9395A1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_A, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_A, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_B

    ui_B = lv_label_create(ui_Footer);

    lv_obj_set_width(ui_B, 18);
    lv_obj_set_height(ui_B, 24);

    lv_obj_set_x(ui_B, -40);
    lv_obj_set_y(ui_B, 0);

    lv_obj_set_align(ui_B, LV_ALIGN_RIGHT_MID);

    lv_label_set_text(ui_B, "B\n");

    lv_obj_set_style_text_color(ui_B, lv_color_hex(0x9395A1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_B, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_B, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Background

    ui_Background = lv_obj_create(ui_Audio_Player);

    lv_obj_set_width(ui_Background, lv_pct(100));
    lv_obj_set_height(ui_Background, lv_pct(88));

    lv_obj_set_x(ui_Background, 0);
    lv_obj_set_y(ui_Background, -28);

    lv_obj_set_align(ui_Background, LV_ALIGN_TOP_MID);

    lv_obj_clear_flag(ui_Background, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Background, lv_color_hex(0x4C4E5B), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Background, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui_Background, lv_color_hex(0x393B46), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Background, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Background, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_Background, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Background, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Background, lv_color_hex(0x191B20), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_Background, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Background, 140, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_Background, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui_Background, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui_Background, 4, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Content_group

    ui_Content_group = lv_obj_create(ui_Background);

    lv_obj_set_width(ui_Content_group, lv_pct(100));
    lv_obj_set_height(ui_Content_group, lv_pct(98));

    lv_obj_set_x(ui_Content_group, 0);
    lv_obj_set_y(ui_Content_group, 20);

    lv_obj_set_align(ui_Content_group, LV_ALIGN_TOP_MID);

    lv_obj_clear_flag(ui_Content_group, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Content_group, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Content_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Content_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Content_group, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Content_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Content_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Content_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Content_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Content_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Display

    ui_Display = lv_obj_create(ui_Content_group);

    lv_obj_set_width(ui_Display, lv_pct(95));
    lv_obj_set_height(ui_Display, lv_pct(20));

    lv_obj_set_x(ui_Display, 0);
    lv_obj_set_y(ui_Display, 15);

    lv_obj_set_align(ui_Display, LV_ALIGN_TOP_MID);

    lv_obj_clear_flag(ui_Display, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Display, lv_color_hex(0x272A33), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Display, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Display, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Display, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Display, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_Display, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Display, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_artist

    ui_Label_artist = lv_label_create(ui_Display);

    lv_obj_set_width(ui_Label_artist, 151);
    lv_obj_set_height(ui_Label_artist, 17);

    lv_obj_set_x(ui_Label_artist, 0);
    lv_obj_set_y(ui_Label_artist, 0);

    lv_label_set_long_mode(ui_Label_artist, LV_LABEL_LONG_DOT);
    lv_label_set_text(ui_Label_artist, "The Roboto Tech Band");

    lv_obj_set_style_text_font(ui_Label_artist, &ui_font_mos_semibold_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_song

    ui_Label_song = lv_label_create(ui_Display);

    lv_obj_set_width(ui_Label_song, 152);
    lv_obj_set_height(ui_Label_song, 20);

    lv_obj_set_x(ui_Label_song, 0);
    lv_obj_set_y(ui_Label_song, 22);

    lv_label_set_long_mode(ui_Label_song, LV_LABEL_LONG_DOT);
    lv_label_set_text(ui_Label_song, "Hard Techno");

    lv_obj_set_style_text_color(ui_Label_song, lv_color_hex(0x7D7F8B), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_song, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_song, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Audio_Wave

    ui_Audio_Wave = lv_slider_create(ui_Display);
    lv_slider_set_range(ui_Audio_Wave, 0, 100);
    lv_slider_set_value(ui_Audio_Wave, 50, LV_ANIM_OFF);
    if(lv_slider_get_mode(ui_Audio_Wave) == LV_SLIDER_MODE_RANGE) lv_slider_set_left_value(ui_Audio_Wave, 0, LV_ANIM_OFF);

    lv_obj_set_width(ui_Audio_Wave, 289);
    lv_obj_set_height(ui_Audio_Wave, 38);

    lv_obj_set_x(ui_Audio_Wave, 0);
    lv_obj_set_y(ui_Audio_Wave, 0);

    lv_obj_set_align(ui_Audio_Wave, LV_ALIGN_BOTTOM_LEFT);

    lv_obj_set_style_bg_color(ui_Audio_Wave, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Audio_Wave, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Audio_Wave, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Audio_Wave, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_Audio_Wave, &ui_img_audio_wave_1_png, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui_Audio_Wave, 50, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_Audio_Wave, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Audio_Wave, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Audio_Wave, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Audio_Wave, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_Audio_Wave, &ui_img_audio_wave_1_png, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_Audio_Wave, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Audio_Wave, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    // ui_Label_Time

    ui_Label_Time = lv_label_create(ui_Display);

    lv_obj_set_width(ui_Label_Time, 123);
    lv_obj_set_height(ui_Label_Time, 39);

    lv_obj_set_x(ui_Label_Time, 175);
    lv_obj_set_y(ui_Label_Time, -1);

    lv_label_set_text(ui_Label_Time, "04:52");

    lv_obj_set_style_text_font(ui_Label_Time, &ui_font_number, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_EQ

    ui_BTN_EQ = lv_imgbtn_create(ui_Display);
    lv_imgbtn_set_src(ui_BTN_EQ, LV_IMGBTN_STATE_RELEASED, NULL, &ui_img_btn_eq_inact_png, NULL);
    lv_imgbtn_set_src(ui_BTN_EQ, LV_IMGBTN_STATE_PRESSED, NULL, &ui_img_btn_eq_act_png, NULL);
    lv_imgbtn_set_src(ui_BTN_EQ, LV_IMGBTN_STATE_DISABLED, NULL, &ui_img_btn_eq_inact_png, NULL);
    lv_imgbtn_set_src(ui_BTN_EQ, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &ui_img_btn_eq_act_png, NULL);
    lv_imgbtn_set_src(ui_BTN_EQ, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &ui_img_btn_eq_act_png, NULL);
    lv_imgbtn_set_src(ui_BTN_EQ, LV_IMGBTN_STATE_CHECKED_DISABLED, NULL, &ui_img_btn_eq_inact_png, NULL);

    lv_obj_set_width(ui_BTN_EQ, 105);
    lv_obj_set_height(ui_BTN_EQ, 105);

    lv_obj_set_x(ui_BTN_EQ, 8);
    lv_obj_set_y(ui_BTN_EQ, 4);

    lv_obj_set_align(ui_BTN_EQ, LV_ALIGN_BOTTOM_RIGHT);

    lv_obj_add_flag(ui_BTN_EQ, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_add_event_cb(ui_BTN_EQ, ui_event_BTN_EQ, LV_EVENT_ALL, NULL);

    // ui_Left_group

    ui_Left_group = lv_obj_create(ui_Content_group);

    lv_obj_set_width(ui_Left_group, lv_pct(28));
    lv_obj_set_height(ui_Left_group, lv_pct(76));

    lv_obj_set_x(ui_Left_group, 0);
    lv_obj_set_y(ui_Left_group, 0);

    lv_obj_set_align(ui_Left_group, LV_ALIGN_BOTTOM_LEFT);

    lv_obj_clear_flag(ui_Left_group, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Left_group, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Left_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Left_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Left_group, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Left_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Left_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Left_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Left_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Left_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Slider_group_left

    ui_Slider_group_left = lv_obj_create(ui_Left_group);

    lv_obj_set_height(ui_Slider_group_left, 369);
    lv_obj_set_width(ui_Slider_group_left, lv_pct(100));

    lv_obj_set_x(ui_Slider_group_left, 0);
    lv_obj_set_y(ui_Slider_group_left, 20);

    lv_obj_set_align(ui_Slider_group_left, LV_ALIGN_TOP_MID);

    lv_obj_clear_flag(ui_Slider_group_left, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Slider_group_left, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider_group_left, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Slider_group_left, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Slider_group_left, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Slider_group_left, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Max

    ui_Max = lv_label_create(ui_Slider_group_left);

    lv_obj_set_width(ui_Max, 38);
    lv_obj_set_height(ui_Max, 19);

    lv_obj_set_x(ui_Max, 0);
    lv_obj_set_y(ui_Max, -20);

    lv_obj_set_align(ui_Max, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Max, "MAX");

    lv_obj_set_style_text_color(ui_Max, lv_color_hex(0x9395A1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Max, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Max, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Max, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Min

    ui_Min = lv_label_create(ui_Slider_group_left);

    lv_obj_set_width(ui_Min, 35);
    lv_obj_set_height(ui_Min, 19);

    lv_obj_set_x(ui_Min, 0);
    lv_obj_set_y(ui_Min, 20);

    lv_obj_set_align(ui_Min, LV_ALIGN_BOTTOM_MID);

    lv_label_set_text(ui_Min, "MIN");

    lv_obj_set_style_text_color(ui_Min, lv_color_hex(0x9395A1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Min, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Min, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Min, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Slider2

    ui_Slider2 = lv_slider_create(ui_Slider_group_left);
    lv_slider_set_range(ui_Slider2, 0, 100);
    lv_slider_set_value(ui_Slider2, 30, LV_ANIM_OFF);
    if(lv_slider_get_mode(ui_Slider2) == LV_SLIDER_MODE_RANGE) lv_slider_set_left_value(ui_Slider2, 0, LV_ANIM_OFF);

    lv_obj_set_width(ui_Slider2, 65);
    lv_obj_set_height(ui_Slider2, 289);

    lv_obj_set_x(ui_Slider2, 0);
    lv_obj_set_y(ui_Slider2, 0);

    lv_obj_set_align(ui_Slider2, LV_ALIGN_CENTER);

    lv_obj_set_style_bg_color(ui_Slider2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Slider2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Slider2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_Slider2, &ui_img_pot_ver_line_png, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Slider2, 28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Slider2, 28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Slider2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Slider2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_Slider2, lv_color_hex(0x50FF7D), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider2, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Slider2, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Slider2, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Slider2, lv_color_hex(0x50FF7D), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_Slider2, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Slider2, 50, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_Slider2, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(ui_Slider2, 12, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Slider2, lv_color_hex(0x50FF7D), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider2, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Slider2, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Slider2, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_Slider2, &ui_img_pot_ver_knob_png, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Slider2, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_Slider2, 100, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Slider2, 30, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_Slider2, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui_Slider2, 5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui_Slider2, 5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Slider2, 3, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Slider2, 2, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Slider2, 6, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Slider2, 6, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_Slider2, 12, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(ui_Slider2, lv_color_hex(0x50FF7D), LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(ui_Slider2, 255, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_color(ui_Slider2, lv_color_hex(0x50FF7D), LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_opa(ui_Slider2, 255, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(ui_Slider2, 40, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_spread(ui_Slider2, 0, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_ofs_x(ui_Slider2, 0, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_ofs_y(ui_Slider2, 0, LV_PART_KNOB | LV_STATE_PRESSED);

    // ui_Cut_1

    ui_Cut_1 = lv_obj_create(ui_Left_group);

    lv_obj_set_height(ui_Cut_1, 94);
    lv_obj_set_width(ui_Cut_1, lv_pct(100));

    lv_obj_set_x(ui_Cut_1, 0);
    lv_obj_set_y(ui_Cut_1, 0);

    lv_obj_set_align(ui_Cut_1, LV_ALIGN_BOTTOM_MID);

    lv_obj_clear_flag(ui_Cut_1, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Cut_1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Cut_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Cut_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Cut_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Cut_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Cut_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Cut_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Cut_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Cut_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_Cut_Left

    ui_BTN_Cut_Left = lv_imgbtn_create(ui_Cut_1);
    lv_imgbtn_set_src(ui_BTN_Cut_Left, LV_IMGBTN_STATE_RELEASED, NULL, &ui_img_btn_1_inact_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Cut_Left, LV_IMGBTN_STATE_PRESSED, NULL, &ui_img_btn_1_act_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Cut_Left, LV_IMGBTN_STATE_DISABLED, NULL, &ui_img_btn_1_inact_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Cut_Left, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &ui_img_btn_1_act_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Cut_Left, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &ui_img_btn_1_act_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Cut_Left, LV_IMGBTN_STATE_CHECKED_DISABLED, NULL, &ui_img_btn_1_inact_png, NULL);

    lv_obj_set_width(ui_BTN_Cut_Left, 125);
    lv_obj_set_height(ui_BTN_Cut_Left, 94);

    lv_obj_set_x(ui_BTN_Cut_Left, 0);
    lv_obj_set_y(ui_BTN_Cut_Left, 0);

    lv_obj_set_align(ui_BTN_Cut_Left, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_BTN_Cut_Left, LV_OBJ_FLAG_CHECKABLE);

    // ui_Label_Cut_1

    ui_Label_Cut_1 = lv_label_create(ui_BTN_Cut_Left);

    lv_obj_set_width(ui_Label_Cut_1, 35);
    lv_obj_set_height(ui_Label_Cut_1, 19);

    lv_obj_set_x(ui_Label_Cut_1, 0);
    lv_obj_set_y(ui_Label_Cut_1, 0);

    lv_obj_set_align(ui_Label_Cut_1, LV_ALIGN_CENTER);

    lv_label_set_text(ui_Label_Cut_1, "CUT");

    lv_obj_add_event_cb(ui_Label_Cut_1, ui_event_Label_Cut_1, LV_EVENT_ALL, NULL);
    lv_obj_set_style_text_color(ui_Label_Cut_1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Cut_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_Cut_1, LV_TEXT_ALIGN_AUTO, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Cut_1, &ui_font_mos_semibold_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Right_group

    ui_Right_group = lv_obj_create(ui_Content_group);

    lv_obj_set_width(ui_Right_group, lv_pct(28));
    lv_obj_set_height(ui_Right_group, lv_pct(76));

    lv_obj_set_x(ui_Right_group, 0);
    lv_obj_set_y(ui_Right_group, 0);

    lv_obj_set_align(ui_Right_group, LV_ALIGN_BOTTOM_RIGHT);

    lv_obj_clear_flag(ui_Right_group, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Right_group, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Right_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Right_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Right_group, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Right_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Right_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Right_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Right_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Right_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Cut_3

    ui_Cut_3 = lv_obj_create(ui_Right_group);

    lv_obj_set_height(ui_Cut_3, 94);
    lv_obj_set_width(ui_Cut_3, lv_pct(100));

    lv_obj_set_x(ui_Cut_3, 0);
    lv_obj_set_y(ui_Cut_3, 0);

    lv_obj_set_align(ui_Cut_3, LV_ALIGN_BOTTOM_MID);

    lv_obj_clear_flag(ui_Cut_3, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Cut_3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Cut_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Cut_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Cut_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Cut_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Cut_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Cut_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Cut_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Cut_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_Cut_Left1

    ui_BTN_Cut_Left1 = lv_imgbtn_create(ui_Cut_3);
    lv_imgbtn_set_src(ui_BTN_Cut_Left1, LV_IMGBTN_STATE_RELEASED, NULL, &ui_img_btn_1_inact_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Cut_Left1, LV_IMGBTN_STATE_PRESSED, NULL, &ui_img_btn_1_act_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Cut_Left1, LV_IMGBTN_STATE_DISABLED, NULL, &ui_img_btn_1_inact_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Cut_Left1, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &ui_img_btn_1_act_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Cut_Left1, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &ui_img_btn_1_act_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Cut_Left1, LV_IMGBTN_STATE_CHECKED_DISABLED, NULL, &ui_img_btn_1_inact_png, NULL);

    lv_obj_set_width(ui_BTN_Cut_Left1, 125);
    lv_obj_set_height(ui_BTN_Cut_Left1, 94);

    lv_obj_set_x(ui_BTN_Cut_Left1, 0);
    lv_obj_set_y(ui_BTN_Cut_Left1, 0);

    lv_obj_set_align(ui_BTN_Cut_Left1, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_BTN_Cut_Left1, LV_OBJ_FLAG_CHECKABLE);

    // ui_Label_Cut_3

    ui_Label_Cut_3 = lv_label_create(ui_BTN_Cut_Left1);

    lv_obj_set_width(ui_Label_Cut_3, 35);
    lv_obj_set_height(ui_Label_Cut_3, 19);

    lv_obj_set_x(ui_Label_Cut_3, 0);
    lv_obj_set_y(ui_Label_Cut_3, 0);

    lv_obj_set_align(ui_Label_Cut_3, LV_ALIGN_CENTER);

    lv_label_set_text(ui_Label_Cut_3, "CUT");

    lv_obj_add_event_cb(ui_Label_Cut_3, ui_event_Label_Cut_3, LV_EVENT_ALL, NULL);
    lv_obj_set_style_text_color(ui_Label_Cut_3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Cut_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_Cut_3, LV_TEXT_ALIGN_AUTO, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Cut_3, &ui_font_mos_semibold_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Slider_group_right

    ui_Slider_group_right = lv_obj_create(ui_Right_group);

    lv_obj_set_height(ui_Slider_group_right, 369);
    lv_obj_set_width(ui_Slider_group_right, lv_pct(100));

    lv_obj_set_x(ui_Slider_group_right, 0);
    lv_obj_set_y(ui_Slider_group_right, 20);

    lv_obj_set_align(ui_Slider_group_right, LV_ALIGN_TOP_MID);

    lv_obj_clear_flag(ui_Slider_group_right, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Slider_group_right, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider_group_right, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Slider_group_right, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Slider_group_right, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Slider_group_right, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Max1

    ui_Max1 = lv_label_create(ui_Slider_group_right);

    lv_obj_set_width(ui_Max1, 38);
    lv_obj_set_height(ui_Max1, 19);

    lv_obj_set_x(ui_Max1, 0);
    lv_obj_set_y(ui_Max1, -20);

    lv_obj_set_align(ui_Max1, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Max1, "MAX");

    lv_obj_set_style_text_color(ui_Max1, lv_color_hex(0x9395A1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Max1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Max1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Max1, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Min2

    ui_Min2 = lv_label_create(ui_Slider_group_right);

    lv_obj_set_width(ui_Min2, 35);
    lv_obj_set_height(ui_Min2, 19);

    lv_obj_set_x(ui_Min2, 0);
    lv_obj_set_y(ui_Min2, 20);

    lv_obj_set_align(ui_Min2, LV_ALIGN_BOTTOM_MID);

    lv_label_set_text(ui_Min2, "MIN");

    lv_obj_set_style_text_color(ui_Min2, lv_color_hex(0x9395A1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Min2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Min2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Min2, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Slider3

    ui_Slider3 = lv_slider_create(ui_Slider_group_right);
    lv_slider_set_range(ui_Slider3, 0, 100);
    lv_slider_set_value(ui_Slider3, 80, LV_ANIM_OFF);
    if(lv_slider_get_mode(ui_Slider3) == LV_SLIDER_MODE_RANGE) lv_slider_set_left_value(ui_Slider3, 0, LV_ANIM_OFF);

    lv_obj_set_width(ui_Slider3, 65);
    lv_obj_set_height(ui_Slider3, 289);

    lv_obj_set_x(ui_Slider3, 0);
    lv_obj_set_y(ui_Slider3, 0);

    lv_obj_set_align(ui_Slider3, LV_ALIGN_CENTER);

    lv_obj_set_style_bg_color(ui_Slider3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Slider3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Slider3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_Slider3, &ui_img_pot_ver_line_png, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Slider3, 28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Slider3, 28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Slider3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Slider3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_Slider3, lv_color_hex(0x50FF7D), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider3, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Slider3, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Slider3, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Slider3, lv_color_hex(0x50FF7D), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_Slider3, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Slider3, 50, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_Slider3, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(ui_Slider3, 12, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Slider3, lv_color_hex(0x50FF7D), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider3, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Slider3, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Slider3, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_Slider3, &ui_img_pot_ver_knob_png, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Slider3, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_Slider3, 100, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Slider3, 30, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_Slider3, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui_Slider3, 5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui_Slider3, 5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Slider3, 3, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Slider3, 2, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Slider3, 6, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Slider3, 6, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_Slider3, 12, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(ui_Slider3, lv_color_hex(0x50FF7D), LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(ui_Slider3, 255, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_color(ui_Slider3, lv_color_hex(0x50FF7D), LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_opa(ui_Slider3, 255, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(ui_Slider3, 40, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_spread(ui_Slider3, 0, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_ofs_x(ui_Slider3, 0, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_ofs_y(ui_Slider3, 0, LV_PART_KNOB | LV_STATE_PRESSED);

    // ui_Center_group

    ui_Center_group = lv_obj_create(ui_Content_group);

    lv_obj_set_width(ui_Center_group, lv_pct(35));
    lv_obj_set_height(ui_Center_group, lv_pct(76));

    lv_obj_set_x(ui_Center_group, 0);
    lv_obj_set_y(ui_Center_group, 0);

    lv_obj_set_align(ui_Center_group, LV_ALIGN_BOTTOM_MID);

    lv_obj_clear_flag(ui_Center_group, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Center_group, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Center_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Center_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Center_group, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Center_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Center_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Center_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Center_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Center_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Play

    ui_Play = lv_obj_create(ui_Center_group);

    lv_obj_set_width(ui_Play, lv_pct(100));
    lv_obj_set_height(ui_Play, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Play, 0);
    lv_obj_set_y(ui_Play, 0);

    lv_obj_set_align(ui_Play, LV_ALIGN_BOTTOM_MID);

    lv_obj_clear_flag(ui_Play, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Play, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Play, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Play, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Play, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Play, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Play, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Play, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Play, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Play, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_Play

    ui_BTN_Play = lv_imgbtn_create(ui_Play);
    lv_imgbtn_set_src(ui_BTN_Play, LV_IMGBTN_STATE_RELEASED, NULL, &ui_img_btn_play_inact_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Play, LV_IMGBTN_STATE_PRESSED, NULL, &ui_img_btn_play_act_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Play, LV_IMGBTN_STATE_DISABLED, NULL, &ui_img_btn_play_inact_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Play, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &ui_img_btn_play_act_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Play, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &ui_img_btn_play_act_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Play, LV_IMGBTN_STATE_CHECKED_DISABLED, NULL, &ui_img_btn_play_inact_png, NULL);

    lv_obj_set_width(ui_BTN_Play, 145);
    lv_obj_set_height(ui_BTN_Play, 145);

    lv_obj_set_x(ui_BTN_Play, 0);
    lv_obj_set_y(ui_BTN_Play, 0);

    lv_obj_set_align(ui_BTN_Play, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_BTN_Play, LV_OBJ_FLAG_CHECKABLE);

    // ui_Indicator_group

    ui_Indicator_group = lv_obj_create(ui_Center_group);

    lv_obj_set_height(ui_Indicator_group, 369);
    lv_obj_set_width(ui_Indicator_group, lv_pct(100));

    lv_obj_set_x(ui_Indicator_group, 0);
    lv_obj_set_y(ui_Indicator_group, 20);

    lv_obj_set_align(ui_Indicator_group, LV_ALIGN_TOP_MID);

    lv_obj_clear_flag(ui_Indicator_group, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Indicator_group, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Indicator_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Indicator_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Indicator_group, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Indicator_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_Levels

    ui_Label_Levels = lv_label_create(ui_Indicator_group);

    lv_obj_set_width(ui_Label_Levels, 77);
    lv_obj_set_height(ui_Label_Levels, 17);

    lv_obj_set_x(ui_Label_Levels, 0);
    lv_obj_set_y(ui_Label_Levels, -18);

    lv_obj_set_align(ui_Label_Levels, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Label_Levels, "LEVEL");

    lv_obj_set_style_text_color(ui_Label_Levels, lv_color_hex(0x9395A1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Levels, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_Levels, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Levels, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Min1

    ui_Min1 = lv_label_create(ui_Indicator_group);

    lv_obj_set_width(ui_Min1, 24);
    lv_obj_set_height(ui_Min1, 293);

    lv_obj_set_x(ui_Min1, 0);
    lv_obj_set_y(ui_Min1, 24);

    lv_obj_set_align(ui_Min1, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Min1, "00\n-02\n-04\n-06\n-08\n-10\n-12\n-14\n-18");

    lv_obj_set_style_text_color(ui_Min1, lv_color_hex(0x9395A1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Min1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui_Min1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui_Min1, 19, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Min1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Min1, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Indicator_Left

    ui_Indicator_Left = lv_slider_create(ui_Indicator_group);
    lv_slider_set_range(ui_Indicator_Left, 0, 29);
    lv_slider_set_value(ui_Indicator_Left, 10, LV_ANIM_OFF);
    if(lv_slider_get_mode(ui_Indicator_Left) == LV_SLIDER_MODE_RANGE) lv_slider_set_left_value(ui_Indicator_Left, 0,
                                                                                                   LV_ANIM_OFF);

    lv_obj_set_width(ui_Indicator_Left, 33);
    lv_obj_set_height(ui_Indicator_Left, 289);

    lv_obj_set_x(ui_Indicator_Left, 0);
    lv_obj_set_y(ui_Indicator_Left, 0);

    lv_obj_set_align(ui_Indicator_Left, LV_ALIGN_LEFT_MID);

    lv_obj_set_style_radius(ui_Indicator_Left, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Indicator_Left, lv_color_hex(0x272A33), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Indicator_Left, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Indicator_Left, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Indicator_Left, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Indicator_Left, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Indicator_Left, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Indicator_Left, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Indicator_Left, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Indicator_Left, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(ui_Indicator_Left, 4, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Indicator_Left, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Indicator_Left, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Indicator_Left, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Indicator_Left, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_Indicator_Left, &ui_img_indicator_ver_png, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_Indicator_Left, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Indicator_Left, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    // ui_Indicator_Right

    ui_Indicator_Right = lv_slider_create(ui_Indicator_group);
    lv_slider_set_range(ui_Indicator_Right, 0, 29);
    lv_slider_set_value(ui_Indicator_Right, 25, LV_ANIM_OFF);
    if(lv_slider_get_mode(ui_Indicator_Right) == LV_SLIDER_MODE_RANGE) lv_slider_set_left_value(ui_Indicator_Right, 0,
                                                                                                    LV_ANIM_OFF);

    lv_obj_set_width(ui_Indicator_Right, 33);
    lv_obj_set_height(ui_Indicator_Right, 289);

    lv_obj_set_x(ui_Indicator_Right, 0);
    lv_obj_set_y(ui_Indicator_Right, 0);

    lv_obj_set_align(ui_Indicator_Right, LV_ALIGN_RIGHT_MID);

    lv_obj_set_style_radius(ui_Indicator_Right, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Indicator_Right, lv_color_hex(0x272A33), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Indicator_Right, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Indicator_Right, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Indicator_Right, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Indicator_Right, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Indicator_Right, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Indicator_Right, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Indicator_Right, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Indicator_Right, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(ui_Indicator_Right, 4, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Indicator_Right, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Indicator_Right, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Indicator_Right, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Indicator_Right, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_Indicator_Right, &ui_img_indicator_ver_png, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_Indicator_Right, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Indicator_Right, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    // ui_EQ_group

    ui_EQ_group = lv_obj_create(ui_Content_group);

    lv_obj_set_height(ui_EQ_group, 164);
    lv_obj_set_width(ui_EQ_group, lv_pct(95));

    lv_obj_set_x(ui_EQ_group, 0);
    lv_obj_set_y(ui_EQ_group, -70);

    lv_obj_set_align(ui_EQ_group, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_EQ_group, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_EQ_group, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_EQ_group, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_EQ_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_EQ_group, lv_color_hex(0x747682), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_EQ_group, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_EQ_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_EQ_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_EQ_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_EQ_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_EQ1

    ui_EQ1 = lv_obj_create(ui_EQ_group);

    lv_obj_set_height(ui_EQ1, 164);
    lv_obj_set_width(ui_EQ1, lv_pct(33));

    lv_obj_set_x(ui_EQ1, 0);
    lv_obj_set_y(ui_EQ1, 0);

    lv_obj_set_align(ui_EQ1, LV_ALIGN_LEFT_MID);

    lv_obj_set_style_bg_color(ui_EQ1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_EQ1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_EQ1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_EQ1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_EQ1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Image2

    ui_Image2 = lv_img_create(ui_EQ1);
    lv_img_set_src(ui_Image2, &ui_img_btn_knob_png);

    lv_obj_set_width(ui_Image2, 104);
    lv_obj_set_height(ui_Image2, 105);

    lv_obj_set_x(ui_Image2, 0);
    lv_obj_set_y(ui_Image2, -12);

    lv_obj_set_align(ui_Image2, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_Image2, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_Arc1

    ui_Arc1 = lv_arc_create(ui_EQ1);

    lv_obj_set_width(ui_Arc1, 95);
    lv_obj_set_height(ui_Arc1, 97);

    lv_obj_set_x(ui_Arc1, 0);
    lv_obj_set_y(ui_Arc1, -12);

    lv_obj_set_align(ui_Arc1, LV_ALIGN_CENTER);

    lv_arc_set_range(ui_Arc1, 10, 1000);
    lv_arc_set_value(ui_Arc1, 50);
    lv_arc_set_bg_angles(ui_Arc1, 129, 51);

    lv_obj_set_style_pad_left(ui_Arc1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Arc1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Arc1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Arc1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(ui_Arc1, lv_color_hex(0x4040FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_Arc1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_Arc1, 20, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(ui_Arc1, lv_color_hex(0x50FF7D), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_Arc1, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_Arc1, 2, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(ui_Arc1, true, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_Arc1, lv_color_hex(0x50FF7D), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Arc1, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    // ui_Label_Low

    ui_Label_Low = lv_label_create(ui_EQ1);

    lv_obj_set_width(ui_Label_Low, 68);
    lv_obj_set_height(ui_Label_Low, 17);

    lv_obj_set_x(ui_Label_Low, 0);
    lv_obj_set_y(ui_Label_Low, 0);

    lv_obj_set_align(ui_Label_Low, LV_ALIGN_BOTTOM_MID);

    lv_label_set_text(ui_Label_Low, "LOW");

    lv_obj_set_style_text_color(ui_Label_Low, lv_color_hex(0x9395A1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Low, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_Low, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Low, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_EQ2

    ui_EQ2 = lv_obj_create(ui_EQ_group);

    lv_obj_set_height(ui_EQ2, 164);
    lv_obj_set_width(ui_EQ2, lv_pct(33));

    lv_obj_set_x(ui_EQ2, 0);
    lv_obj_set_y(ui_EQ2, 0);

    lv_obj_set_align(ui_EQ2, LV_ALIGN_CENTER);

    lv_obj_set_style_bg_color(ui_EQ2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_EQ2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_EQ2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_EQ2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_EQ2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Image3

    ui_Image3 = lv_img_create(ui_EQ2);
    lv_img_set_src(ui_Image3, &ui_img_btn_knob_png);

    lv_obj_set_width(ui_Image3, 104);
    lv_obj_set_height(ui_Image3, 105);

    lv_obj_set_x(ui_Image3, 0);
    lv_obj_set_y(ui_Image3, -12);

    lv_obj_set_align(ui_Image3, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_Image3, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_Arc2

    ui_Arc2 = lv_arc_create(ui_EQ2);

    lv_obj_set_width(ui_Arc2, 95);
    lv_obj_set_height(ui_Arc2, 97);

    lv_obj_set_x(ui_Arc2, 0);
    lv_obj_set_y(ui_Arc2, -12);

    lv_obj_set_align(ui_Arc2, LV_ALIGN_CENTER);

    lv_arc_set_range(ui_Arc2, 10, 1000);
    lv_arc_set_value(ui_Arc2, 600);
    lv_arc_set_bg_angles(ui_Arc2, 129, 51);

    lv_obj_set_style_pad_left(ui_Arc2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Arc2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Arc2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Arc2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(ui_Arc2, lv_color_hex(0x4040FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_Arc2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_Arc2, 20, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(ui_Arc2, lv_color_hex(0x50FF7D), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_Arc2, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_Arc2, 2, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(ui_Arc2, true, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_Arc2, lv_color_hex(0x50FF7D), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Arc2, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    // ui_Label_Mid

    ui_Label_Mid = lv_label_create(ui_EQ2);

    lv_obj_set_width(ui_Label_Mid, 68);
    lv_obj_set_height(ui_Label_Mid, 17);

    lv_obj_set_x(ui_Label_Mid, 0);
    lv_obj_set_y(ui_Label_Mid, 0);

    lv_obj_set_align(ui_Label_Mid, LV_ALIGN_BOTTOM_MID);

    lv_label_set_text(ui_Label_Mid, "MID");

    lv_obj_set_style_text_color(ui_Label_Mid, lv_color_hex(0x9395A1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Mid, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_Mid, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Mid, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_EQ3

    ui_EQ3 = lv_obj_create(ui_EQ_group);

    lv_obj_set_height(ui_EQ3, 164);
    lv_obj_set_width(ui_EQ3, lv_pct(33));

    lv_obj_set_x(ui_EQ3, 0);
    lv_obj_set_y(ui_EQ3, 0);

    lv_obj_set_align(ui_EQ3, LV_ALIGN_RIGHT_MID);

    lv_obj_set_style_bg_color(ui_EQ3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_EQ3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_EQ3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_EQ3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_EQ3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Image4

    ui_Image4 = lv_img_create(ui_EQ3);
    lv_img_set_src(ui_Image4, &ui_img_btn_knob_png);

    lv_obj_set_width(ui_Image4, 104);
    lv_obj_set_height(ui_Image4, 105);

    lv_obj_set_x(ui_Image4, 0);
    lv_obj_set_y(ui_Image4, -12);

    lv_obj_set_align(ui_Image4, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_Image4, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_Arc3

    ui_Arc3 = lv_arc_create(ui_EQ3);

    lv_obj_set_width(ui_Arc3, 95);
    lv_obj_set_height(ui_Arc3, 97);

    lv_obj_set_x(ui_Arc3, 0);
    lv_obj_set_y(ui_Arc3, -12);

    lv_obj_set_align(ui_Arc3, LV_ALIGN_CENTER);

    lv_arc_set_range(ui_Arc3, 10, 1000);
    lv_arc_set_value(ui_Arc3, 900);
    lv_arc_set_bg_angles(ui_Arc3, 129, 51);

    lv_obj_set_style_pad_left(ui_Arc3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Arc3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Arc3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Arc3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(ui_Arc3, lv_color_hex(0x4040FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_Arc3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_Arc3, 20, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(ui_Arc3, lv_color_hex(0x50FF7D), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_Arc3, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_Arc3, 2, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(ui_Arc3, true, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_Arc3, lv_color_hex(0x50FF7D), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Arc3, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    // ui_Label_High

    ui_Label_High = lv_label_create(ui_EQ3);

    lv_obj_set_width(ui_Label_High, 68);
    lv_obj_set_height(ui_Label_High, 17);

    lv_obj_set_x(ui_Label_High, 0);
    lv_obj_set_y(ui_Label_High, 0);

    lv_obj_set_align(ui_Label_High, LV_ALIGN_BOTTOM_MID);

    lv_label_set_text(ui_Label_High, "HIGH");

    lv_obj_set_style_text_color(ui_Label_High, lv_color_hex(0x9395A1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_High, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_High, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_High, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Tempo_group

    ui_Tempo_group = lv_obj_create(ui_Content_group);

    lv_obj_set_height(ui_Tempo_group, 164);
    lv_obj_set_width(ui_Tempo_group, lv_pct(95));

    lv_obj_set_x(ui_Tempo_group, 0);
    lv_obj_set_y(ui_Tempo_group, -140);

    lv_obj_set_align(ui_Tempo_group, LV_ALIGN_BOTTOM_MID);

    lv_obj_add_flag(ui_Tempo_group, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_Tempo_group, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Tempo_group, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Tempo_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Tempo_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Tempo_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Tempo_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Tempo_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Tempo_group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Slider_AB_2

    ui_Slider_AB_2 = lv_slider_create(ui_Tempo_group);
    lv_slider_set_range(ui_Slider_AB_2, -100, 100);
    lv_slider_set_mode(ui_Slider_AB_2, LV_SLIDER_MODE_SYMMETRICAL);
    lv_slider_set_value(ui_Slider_AB_2, 100, LV_ANIM_OFF);
    if(lv_slider_get_mode(ui_Slider_AB_2) == LV_SLIDER_MODE_RANGE) lv_slider_set_left_value(ui_Slider_AB_2, 0, LV_ANIM_OFF);

    lv_obj_set_width(ui_Slider_AB_2, 315);
    lv_obj_set_height(ui_Slider_AB_2, 79);

    lv_obj_set_x(ui_Slider_AB_2, 0);
    lv_obj_set_y(ui_Slider_AB_2, -12);

    lv_obj_set_align(ui_Slider_AB_2, LV_ALIGN_CENTER);

    lv_obj_set_style_bg_color(ui_Slider_AB_2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider_AB_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_Slider_AB_2, &ui_img_pot_hor_line_tempo_png, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Slider_AB_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Slider_AB_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Slider_AB_2, 35, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Slider_AB_2, 35, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_Slider_AB_2, lv_color_hex(0x50FF7D), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider_AB_2, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Slider_AB_2, lv_color_hex(0x50FF7D), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_Slider_AB_2, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Slider_AB_2, 50, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_Slider_AB_2, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(ui_Slider_AB_2, 12, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Slider_AB_2, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider_AB_2, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Slider_AB_2, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Slider_AB_2, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_Slider_AB_2, &ui_img_pot_hor_knob_png, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Slider_AB_2, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_Slider_AB_2, 100, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Slider_AB_2, 30, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_Slider_AB_2, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui_Slider_AB_2, 5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui_Slider_AB_2, 5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Slider_AB_2, -1, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Slider_AB_2, -1, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Slider_AB_2, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Slider_AB_2, -4, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_Slider_AB_2, 12, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(ui_Slider_AB_2, lv_color_hex(0x50FF7D), LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(ui_Slider_AB_2, 255, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_color(ui_Slider_AB_2, lv_color_hex(0x50FF7D), LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_opa(ui_Slider_AB_2, 255, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(ui_Slider_AB_2, 40, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_spread(ui_Slider_AB_2, 0, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_ofs_x(ui_Slider_AB_2, 0, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_ofs_y(ui_Slider_AB_2, 0, LV_PART_KNOB | LV_STATE_PRESSED);

    // ui_Label_Tempo

    ui_Label_Tempo = lv_label_create(ui_Tempo_group);

    lv_obj_set_width(ui_Label_Tempo, 68);
    lv_obj_set_height(ui_Label_Tempo, 17);

    lv_obj_set_x(ui_Label_Tempo, 0);
    lv_obj_set_y(ui_Label_Tempo, -8);

    lv_obj_set_align(ui_Label_Tempo, LV_ALIGN_BOTTOM_MID);

    lv_label_set_text(ui_Label_Tempo, "TEMPO");

    lv_obj_set_style_text_color(ui_Label_Tempo, lv_color_hex(0x9395A1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Tempo, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_Tempo, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Tempo, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

}

void ui_tuner_init(void)
{
    lv_disp_t * dispp = lv_disp_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    ui_Audio_Player_screen_init();
    lv_disp_load_scr(ui_Audio_Player);
}

