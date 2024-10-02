/**
 * @file lv_demo_music.h
 *
 */

#ifndef APP_DEMO_MUSIC_H
#define APP_DEMO_MUSIC_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "bsp_board_extra.h"

#define APP_DEMO_MUSIC_ENABLE       1
#define APP_DEMO_MUSIC_LARGE        0

#if APP_DEMO_MUSIC_ENABLE

/*********************
 *      DEFINES
 *********************/

#if APP_DEMO_MUSIC_LARGE
#  define APP_DEMO_MUSIC_HANDLE_SIZE  40
#else
#  define APP_DEMO_MUSIC_HANDLE_SIZE  20
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_demo_music(lv_obj_t *parent, file_iterator_instance_t *file_iterator);
void lv_demo_music_close(void);

const char * _lv_demo_music_get_title(uint32_t track_id);
const char * _lv_demo_music_get_artist(uint32_t track_id);
const char * _lv_demo_music_get_genre(uint32_t track_id);
uint32_t _lv_demo_music_get_track_length(uint32_t track_id);

/**********************
 *      MACROS
 **********************/

#endif /*APP_DEMO_MUSIC_ENABLE*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*APP_DEMO_MUSIC_H*/
