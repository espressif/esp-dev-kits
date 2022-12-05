#ifndef UI_DEMO2_H
#define UI_DEMO2_H

#ifdef __cplusplus
extern "C" {
#endif

#if __has_include("lvgl.h")
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

extern lv_obj_t * ui_Audio_Player;
extern lv_obj_t * ui_Footer;
extern lv_obj_t * ui_Slider_AB;
extern lv_obj_t * ui_A;
extern lv_obj_t * ui_B;
extern lv_obj_t * ui_Background;
extern lv_obj_t * ui_Content_group;
extern lv_obj_t * ui_Display;
extern lv_obj_t * ui_Label_artist;
extern lv_obj_t * ui_Label_song;
extern lv_obj_t * ui_Audio_Wave;
extern lv_obj_t * ui_Label_Time;
extern lv_obj_t * ui_BTN_EQ;
extern lv_obj_t * ui_Left_group;
extern lv_obj_t * ui_Slider_group_left;
extern lv_obj_t * ui_Max;
extern lv_obj_t * ui_Min;
extern lv_obj_t * ui_Slider2;
extern lv_obj_t * ui_Cut_1;
extern lv_obj_t * ui_BTN_Cut_Left;
extern lv_obj_t * ui_Label_Cut_1;
extern lv_obj_t * ui_Right_group;
extern lv_obj_t * ui_Cut_3;
extern lv_obj_t * ui_BTN_Cut_Left1;
extern lv_obj_t * ui_Label_Cut_3;
extern lv_obj_t * ui_Slider_group_right;
extern lv_obj_t * ui_Max1;
extern lv_obj_t * ui_Min2;
extern lv_obj_t * ui_Slider3;
extern lv_obj_t * ui_Center_group;
extern lv_obj_t * ui_Play;
extern lv_obj_t * ui_BTN_Play;
extern lv_obj_t * ui_Indicator_group;
extern lv_obj_t * ui_Label_Levels;
extern lv_obj_t * ui_Min1;
extern lv_obj_t * ui_Indicator_Left;
extern lv_obj_t * ui_Indicator_Right;
extern lv_obj_t * ui_EQ_group;
extern lv_obj_t * ui_EQ1;
extern lv_obj_t * ui_Image2;
extern lv_obj_t * ui_Arc1;
extern lv_obj_t * ui_Label_Low;
extern lv_obj_t * ui_EQ2;
extern lv_obj_t * ui_Image3;
extern lv_obj_t * ui_Arc2;
extern lv_obj_t * ui_Label_Mid;
extern lv_obj_t * ui_EQ3;
extern lv_obj_t * ui_Image4;
extern lv_obj_t * ui_Arc3;
extern lv_obj_t * ui_Label_High;
extern lv_obj_t * ui_Tempo_group;
extern lv_obj_t * ui_Slider_AB_2;
extern lv_obj_t * ui_Label_Tempo;


LV_IMG_DECLARE(ui_img_pot_hor_line_png);    // assets\pot_hor_line.png
LV_IMG_DECLARE(ui_img_pot_hor_knob_png);    // assets\pot_hor_knob.png
LV_IMG_DECLARE(ui_img_audio_wave_1_png);    // assets\audio_wave_1.png
LV_IMG_DECLARE(ui_img_btn_eq_inact_png);    // assets\btn_eq_inact.png
LV_IMG_DECLARE(ui_img_btn_eq_act_png);    // assets\btn_eq_act.png
LV_IMG_DECLARE(ui_img_pot_ver_line_png);    // assets\pot_ver_line.png
LV_IMG_DECLARE(ui_img_pot_ver_knob_png);    // assets\pot_ver_knob.png
LV_IMG_DECLARE(ui_img_btn_1_inact_png);    // assets\btn_1_inact.png
LV_IMG_DECLARE(ui_img_btn_1_act_png);    // assets\btn_1_act.png
LV_IMG_DECLARE(ui_img_btn_play_inact_png);    // assets\btn_play_inact.png
LV_IMG_DECLARE(ui_img_btn_play_act_png);    // assets\btn_play_act.png
LV_IMG_DECLARE(ui_img_indicator_ver_png);    // assets\indicator_ver.png
LV_IMG_DECLARE(ui_img_btn_knob_png);    // assets\btn_knob.png
LV_IMG_DECLARE(ui_img_pot_hor_line_tempo_png);    // assets\pot_hor_line_tempo.png


LV_FONT_DECLARE(ui_font_14_bold);
LV_FONT_DECLARE(ui_font_mos_semibold_16);
LV_FONT_DECLARE(ui_font_number);


void ui_tuner_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
