/**
 * @file lv_demo_music_list.h
 *
 */

#ifndef APP_DEMO_MUSIC_LIST_H
#define APP_DEMO_MUSIC_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lv_demo_music.h"
#if APP_DEMO_MUSIC_ENABLE

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
lv_obj_t * _lv_demo_music_list_create(lv_obj_t * parent);
void _lv_demo_music_list_close(void);

void _lv_demo_music_list_btn_check(uint32_t track_id, bool state);

/**********************
 *      MACROS
 **********************/

#endif /*APP_DEMO_MUSIC_ENABLE*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*APP_DEMO_MUSIC_LIST_H*/
