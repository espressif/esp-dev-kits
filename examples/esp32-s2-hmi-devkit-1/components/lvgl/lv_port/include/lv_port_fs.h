/**
 * @file lv_port_fs_templ.h
 *
 */

 /*Copy this file as "lv_port_fs.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_FS_TEMPL_H
#define LV_PORT_FS_TEMPL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include "esp_err.h"
#include "lvgl/lvgl.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**
 * @brief Init LVGL file system.
 * 
 * @return esp_err_t Result of state.
 */
esp_err_t lv_port_fs_init(void);

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_PORT_FS_TEMPL_H*/

#endif /*Disable/Enable content*/
