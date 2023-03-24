/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
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

LV_IMG_DECLARE(watch_bg);
LV_IMG_DECLARE(hour);
LV_IMG_DECLARE(minute);
LV_IMG_DECLARE(second);

LV_IMG_DECLARE(new_watch_bg);
LV_IMG_DECLARE(watch_hour);
LV_IMG_DECLARE(watch_min);
LV_IMG_DECLARE(watch_sec);

LV_IMG_DECLARE(Icon_little_AC);
LV_IMG_DECLARE(Icon_little_music);
LV_IMG_DECLARE(Icon_little_home);
LV_IMG_DECLARE(Icon_little_game);

LV_IMG_DECLARE(Icon_blue_1_1);
LV_IMG_DECLARE(Icon_white_1_1);
LV_IMG_DECLARE(Icon_yellow_1_1);
LV_IMG_DECLARE(Icon_red_1_2);
LV_IMG_DECLARE(Icon_black_2_2);
LV_IMG_DECLARE(Icon_black_2_2_new);

LV_IMG_DECLARE(test_gif);
LV_IMG_DECLARE(mic_logo);
/*
Light
*/
LV_IMG_DECLARE(livingRoom_Light_on_yellow);
LV_IMG_DECLARE(livingRoom_Light_on_red);
LV_IMG_DECLARE(livingRoom_Light_on_white);
LV_IMG_DECLARE(livingRoom_Light_on_blue);
LV_IMG_DECLARE(livingRoom_Light_off);

LV_IMG_DECLARE(livingRoom_power_on);
LV_IMG_DECLARE(livingRoom_power_off);

LV_IMG_DECLARE(livingRoom_light_red);
LV_IMG_DECLARE(livingRoom_light_white);
LV_IMG_DECLARE(livingRoom_light_blue);
LV_IMG_DECLARE(livingRoom_light_yellow);

LV_IMG_DECLARE(livingRoom_slider_icon);
LV_IMG_DECLARE(livingRoom_slider_bg);

/*
AC
*/
LV_IMG_DECLARE(ac_background);
LV_IMG_DECLARE(ac_power_off);
LV_IMG_DECLARE(ac_power_on);
LV_IMG_DECLARE(ac_temp_add_off);
LV_IMG_DECLARE(ac_temp_add_on);
LV_IMG_DECLARE(ac_temp_dec_off);
LV_IMG_DECLARE(ac_temp_dec_on);

LV_IMG_DECLARE(icon_160_light);
LV_IMG_DECLARE(icon_160_home);
LV_IMG_DECLARE(icon_160_theam);

LV_IMG_DECLARE(icon_320_AC);
LV_IMG_DECLARE(icon_320_weather);

LV_IMG_DECLARE(icon_child_ac_small);
LV_IMG_DECLARE(icon_child_weather);
LV_IMG_DECLARE(icon_child_ac);
LV_IMG_DECLARE(icon_child_ac_bg);
LV_IMG_DECLARE(icon_child_ac_add1);
LV_IMG_DECLARE(icon_child_ac_add2);
LV_IMG_DECLARE(icon_child_ac_dec1);
LV_IMG_DECLARE(icon_child_ac_dec2);
LV_IMG_DECLARE(icon_child_ac_pow1);
LV_IMG_DECLARE(icon_child_ac_pow2);
LV_IMG_DECLARE(icon_child_home);
LV_IMG_DECLARE(icon_child_theam);
LV_IMG_DECLARE(icon_child_light);

LV_IMG_DECLARE(icon_child_ac_add1_warm);
LV_IMG_DECLARE(icon_child_ac_dec2_warm);
LV_IMG_DECLARE(icon_child_ac_warm);
LV_IMG_DECLARE(icon_child_ac_add2_warm);
LV_IMG_DECLARE(icon_child_ac_pow1_warm);
LV_IMG_DECLARE(icon_child_weather_warm);
LV_IMG_DECLARE(icon_child_ac_bg_warm);
LV_IMG_DECLARE(icon_child_ac_pow2_warm);
LV_IMG_DECLARE(icon_child_ac_dec1_warm);
LV_IMG_DECLARE(icon_child_ac_small);

LV_IMG_DECLARE(icon_light_change);
LV_IMG_DECLARE(icon_light_color);
LV_IMG_DECLARE(icon_light_light);
LV_IMG_DECLARE(icon_light_normal);
LV_IMG_DECLARE(icon_light_shaw);

LV_IMG_DECLARE(icon_light_color_warm);
LV_IMG_DECLARE(icon_light_light_warm);
LV_IMG_DECLARE(icon_light_normal_warm);

/********************************
 * font
********************************/
LV_FONT_DECLARE(font_KaiTi20);
LV_FONT_DECLARE(font_KaiTi60);
LV_FONT_DECLARE(font_KaiTi96);

LV_FONT_DECLARE(siyuan20);
LV_FONT_DECLARE(siyuan36);

LV_FONT_DECLARE(pingfang_18);
LV_FONT_DECLARE(pingfang_20);

LV_FONT_DECLARE(helveticaneue_32);
LV_FONT_DECLARE(helveticaneue_36);

LV_FONT_DECLARE(Montserrat_Bold_116);

LV_FONT_DECLARE(my_font_64);

//LV_FONT_DECLARE(lv_font_montserrat_20);

#if (0 == LV_USE_FREETYPE)
typedef struct {
    lv_font_t *font;    /* point to lvgl font */
} lv_ft_info_t;
#endif

extern lv_ft_info_t font30;
extern lv_ft_info_t font20;
extern lv_ft_info_t font60;
extern lv_ft_info_t font36;
extern lv_ft_info_t font120;

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_EXAMPLE_IMAGE_H*/
