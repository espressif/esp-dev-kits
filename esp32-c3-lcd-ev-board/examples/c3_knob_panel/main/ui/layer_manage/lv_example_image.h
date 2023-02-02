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
LV_IMG_DECLARE(icon_clock);
LV_IMG_DECLARE(icon_fans);
LV_IMG_DECLARE(icon_light);
LV_IMG_DECLARE(icon_player);
LV_IMG_DECLARE(icon_weather);
LV_IMG_DECLARE(icon_washing);
LV_IMG_DECLARE(icon_thermostat);
LV_IMG_DECLARE(icon_washing_ns);
LV_IMG_DECLARE(icon_thermostat_ns);
LV_IMG_DECLARE(icon_light_ns);

LV_IMG_DECLARE(img_weather);
LV_IMG_DECLARE(img_bg);
LV_IMG_DECLARE(espressif_logo);

LV_IMG_DECLARE(light_close_bg)
LV_IMG_DECLARE(light_close_pwm)
LV_IMG_DECLARE(light_close_status)
LV_IMG_DECLARE(light_cool_100)
LV_IMG_DECLARE(light_cool_25)
LV_IMG_DECLARE(light_cool_50)
LV_IMG_DECLARE(light_cool_75)
LV_IMG_DECLARE(light_cool_bg)
LV_IMG_DECLARE(light_warm_100)
LV_IMG_DECLARE(light_warm_25)
LV_IMG_DECLARE(light_warm_50)
LV_IMG_DECLARE(light_warm_75)
LV_IMG_DECLARE(light_warm_bg)

LV_IMG_DECLARE(light_pwm_00)
LV_IMG_DECLARE(light_pwm_25)
LV_IMG_DECLARE(light_pwm_50)
LV_IMG_DECLARE(light_pwm_75)
LV_IMG_DECLARE(light_pwm_100)

LV_IMG_DECLARE(img_washing_bg);
LV_IMG_DECLARE(img_washing_wave1);
LV_IMG_DECLARE(img_washing_wave2);
LV_IMG_DECLARE(img_washing_bubble1);
LV_IMG_DECLARE(img_washing_bubble2);
LV_IMG_DECLARE(img_washing_stand);
LV_IMG_DECLARE(img_washing_shirt);
LV_IMG_DECLARE(img_washing_underwear);
LV_IMG_DECLARE(wash_underwear1)
LV_IMG_DECLARE(wash_underwear2)
LV_IMG_DECLARE(wash_shirt)

LV_IMG_DECLARE(AC_BG)
LV_IMG_DECLARE(AC_temper)
LV_IMG_DECLARE(AC_unit)

LV_IMG_DECLARE(standby_eye_left)
LV_IMG_DECLARE(standby_eye_right)
LV_IMG_DECLARE(standby_eye_1)
LV_IMG_DECLARE(standby_eye_2)
LV_IMG_DECLARE(standby_eye_close)
LV_IMG_DECLARE(standby_face)
LV_IMG_DECLARE(standby_mouth_2)
LV_IMG_DECLARE(standby_eye_1_fade)
LV_IMG_DECLARE(standby_eye_3)
LV_IMG_DECLARE(standby_eye_open)
LV_IMG_DECLARE(standby_mouth_1)


/********************************
 * font
********************************/
LV_FONT_DECLARE(font_cn_12);
LV_FONT_DECLARE(font_cn_32);
LV_FONT_DECLARE(font_cn_48);

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

extern lv_style_t style_radio;
extern lv_style_t style_radio_chk;

extern uint8_t submode_item_focus;
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_EXAMPLE_IMAGE_H*/