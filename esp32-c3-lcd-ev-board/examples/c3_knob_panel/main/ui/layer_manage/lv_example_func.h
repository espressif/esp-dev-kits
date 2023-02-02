/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#ifndef LV_EXAMPLE_FUNC_H
#define LV_EXAMPLE_FUNC_H

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


/**********************
 *      MACROS
 **********************/

#define USER_MAINMENU_MAX       3

#define TIME_ENTER_CLOCK_2MIN    (2*60*1000)/6///2min (2*60*1000)/(50 ms)
//#define TIME_ENTER_CLOCK_2MIN    (0x88*60*1000)/1//2min (2*60*1000)/(50 ms)

#define COLOUR_BLACK            0x000000
#define COLOUR_WHITE            0xFFFFFF
#define COLOUR_YELLOW           0xE9BD85
#define COLOUR_GREY_1F          0x1F1F1F
#define COLOUR_GREY_2F          0x2F2F2F

#define COLOUR_GREY_BF          0xBFBFBF
#define COLOUR_GREY_8F          0x8F8F8F
#define COLOUR_GREY_4F          0x4F4F4F

typedef void (* lv_obj_sprite_create)(lv_obj_t *parent);

typedef bool (*lv_layer_enter_cb)(struct lv_layer_t *layer);

typedef bool (*lv_layer_exit_cb)(struct lv_layer_t *layer);

typedef struct lv_layer_t {
    char *lv_obj_name;
    uint8_t *lv_obj_parent;
    lv_obj_t *lv_obj_layer;
    struct lv_layer_t *lv_show_layer;
    lv_layer_enter_cb enter_cb;
    lv_layer_exit_cb exit_cb;
    lv_timer_cb_t timer_cb;
    lv_timer_t *timer_handle;
} lv_layer_t;

typedef struct {
    const uint8_t *set_list_name;
    lv_obj_sprite_create sprite_create_func;
    lv_obj_t *sprite_parent;
} sprite_create_func_t;

extern lv_layer_t boot_Layer;
extern lv_layer_t main_Layer;
extern lv_layer_t weather_Layer;
extern lv_layer_t clock_Layer;
extern lv_layer_t fan_Layer;
extern lv_layer_t light_Layer;
extern lv_layer_t player_Layer;
extern lv_layer_t washing_Layer;
extern lv_layer_t light_2color_Layer;
extern lv_layer_t thermostat_Layer;

extern void lv_func_goto_layer(lv_layer_t *dst_layer);

extern void enter_screenOff_immediate();

extern void reload_screenOff_timer();

extern void trigger_screenOff_timer(lv_layer_t *src_layer);

extern bool freetype_font_load_init(void);

#endif /*LV_EXAMPLE_FUNC_H*/
