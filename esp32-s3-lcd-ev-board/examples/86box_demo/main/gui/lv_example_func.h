/**
 * @file lv_example_scroll.h
 *
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

#ifdef __cplusplus
} /*extern "C"*/
#endif

#define USER_MAINMENU_MAX       3
#define USER_CTRLBTN_MAX        7

#define TIME_ENTER_CLOCK_2MIN    (2*60*1000)/1//2min (2*60*1000)/(50 ms)


#define COLOUR_BLACK            0xFFFFFF
#define COLOUR_GREY             0xBFBFBF
#define COLOUR_WHITE            0xFFFFFF
#define COLOUR_YELLOW           0xE9BD85
#define COLOUR_BG_GREY          0x3F3F3F
#define COLOUR_BG1_GREY         0x2F2F2F

typedef bool (*lv_layer_enter_cb)(struct lv_layer_t * layer);
typedef bool (*lv_layer_exit_cb)(struct lv_layer_t * layer);

typedef struct lv_layer_t{
    uint8_t * lv_obj_name;
    uint8_t * lv_obj_user;
    lv_obj_t * lv_obj_layer;
    struct lv_layer_t *lv_show_layer;
    lv_layer_enter_cb enter_cb;
    lv_layer_exit_cb exit_cb;
    lv_timer_cb_t timer_cb;
    lv_timer_t * timer_handle;
}lv_layer_t;

typedef struct{
    const lv_img_dsc_t * addr_weather_icon;
    const uint8_t * status;
    const uint8_t * location;
    const uint8_t temperature;
    const uint8_t *tempRange;
}weather_info_list_t;

typedef struct{
    const lv_img_dsc_t * addr_open;
    const lv_img_dsc_t * addr_close;
    const uint8_t *btn_name;
}warm_fan_info_list_t;

typedef struct{
    const lv_img_dsc_t * addr_open;
    const lv_img_dsc_t * addr_close;
    const uint8_t *btn_name;
}water_heat_info_list_t;

extern lv_layer_t show_set_layer;
extern lv_layer_t test_layer;
extern lv_layer_t weather_Layer;
extern lv_layer_t main_Layer;
extern lv_layer_t ctrl_Layer;
extern lv_layer_t clock_screen_layer; 
extern lv_layer_t show_music_layer;

extern const warm_fan_info_list_t warm_openclose_icon[7];
extern const weather_info_list_t weather_icon_list[8];

extern void lv_func_goto_layer(lv_layer_t * src_layer, lv_layer_t * dst_layer);

extern uint32_t sys_clock_getSecond();

extern void reload_screenOff_timer();

extern void trigger_screenOff_timer(lv_layer_t * src_layer);

#endif /*LV_EXAMPLE_FUNC_H*/
