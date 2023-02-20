/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#ifndef LV_EXAMPLE_IMAGE_H
#define LV_EXAMPLE_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
LV_IMG_DECLARE(img_lv_demo_music_btn_list_play);
LV_IMG_DECLARE(img_lv_demo_music_btn_list_pause);

LV_IMG_DECLARE(img_lv_demo_music_cover_1);
LV_IMG_DECLARE(img_lv_demo_music_cover_2);
LV_IMG_DECLARE(img_lv_demo_music_cover_3);

LV_IMG_DECLARE(weather_icon_rain_home);
LV_IMG_DECLARE(weather_icon_cloudy_home);
LV_IMG_DECLARE(weather_icon_smog_home);
LV_IMG_DECLARE(weather_icon_floatingdust_home);
LV_IMG_DECLARE(weather_icon_snow_home);
LV_IMG_DECLARE(weather_icon_forst_home);
LV_IMG_DECLARE(weather_icon_summer_home);
LV_IMG_DECLARE(weather_icon_overcast_home);
LV_IMG_DECLARE(home_btn);
LV_IMG_DECLARE(img_player2);
LV_IMG_DECLARE(img_return2);

LV_IMG_DECLARE(weather_summer);
LV_IMG_DECLARE(weather_rain);

LV_IMG_DECLARE(global_bg1);
LV_IMG_DECLARE(global_bg2);
LV_IMG_DECLARE(global_bg3);

LV_IMG_DECLARE(wallpaper);
LV_IMG_DECLARE(bg);
LV_IMG_DECLARE(bg1);
LV_IMG_DECLARE(heatingDetaiBg);

LV_IMG_DECLARE(icon_title_bg);
LV_IMG_DECLARE(icon_title_bg1);

LV_IMG_DECLARE(set_icon_BLE_B);
LV_IMG_DECLARE(set_icon_BLE_y);
LV_IMG_DECLARE(set_icon_WIFI_B);
LV_IMG_DECLARE(set_icon_WIFI_y);
LV_IMG_DECLARE(set_icon_screen_B);
LV_IMG_DECLARE(set_icon_screen_y);
LV_IMG_DECLARE(set_icon_set_B);
LV_IMG_DECLARE(set_icon_set_y);
LV_IMG_DECLARE(set_icon_temp_B);
LV_IMG_DECLARE(set_icon_temp_y);
LV_IMG_DECLARE(set_pulldown_bg);

LV_IMG_DECLARE(warmIcon_open);
LV_IMG_DECLARE(warmIcon_close);
LV_IMG_DECLARE(warmIcon1_open);
LV_IMG_DECLARE(warmIcon1_close);
LV_IMG_DECLARE(warmIcon2_open);
LV_IMG_DECLARE(warmIcon2_close);
LV_IMG_DECLARE(warmIcon3_open);
LV_IMG_DECLARE(warmIcon3_close);
LV_IMG_DECLARE(warmIcon4_open);
LV_IMG_DECLARE(warmIcon4_close);
LV_IMG_DECLARE(warmIcon5_open);
LV_IMG_DECLARE(warmIcon5_close);
LV_IMG_DECLARE(warmIcon6_open);
LV_IMG_DECLARE(warmIcon6_close);

LV_IMG_DECLARE(detail_add);
LV_IMG_DECLARE(detail_dec);
LV_IMG_DECLARE(detail_fun_1_B);
LV_IMG_DECLARE(detail_fun_1_y);
LV_IMG_DECLARE(detail_fun_2_B);
LV_IMG_DECLARE(detail_fun_2_y);
LV_IMG_DECLARE(detail_fun_3_B);
LV_IMG_DECLARE(detail_fun_3_y);
LV_IMG_DECLARE(detail_fun_4_B);
LV_IMG_DECLARE(detail_fun_4_y);
LV_IMG_DECLARE(detail_fun_5_B);
LV_IMG_DECLARE(detail_fun_5_y);
LV_IMG_DECLARE(detail_fun_6_B);
LV_IMG_DECLARE(detail_fun_6_y);
LV_IMG_DECLARE(detail_powerBig_B);
LV_IMG_DECLARE(detail_powerBig_Y);

LV_IMG_DECLARE(icon_player_yellow);
LV_IMG_DECLARE(icon_player_black);

/********************************
 * font
********************************/
LV_FONT_DECLARE(font_lanting16);
LV_FONT_DECLARE(font_lantingMid18);
LV_FONT_DECLARE(font_lantingMid20);
LV_FONT_DECLARE(font_lanting24);
LV_FONT_DECLARE(font_lanting36);
LV_FONT_DECLARE(font_num50);
LV_FONT_DECLARE(font_num36);
LV_FONT_DECLARE(font_num24);

extern lv_style_t style_btn;            //按钮1按下前的样式变量
extern lv_style_t style_btn_pressed;    //按钮按下时的样式变量

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_EXAMPLE_IMAGE_H*/
