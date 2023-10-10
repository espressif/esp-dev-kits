/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
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
#include "lvgl.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/* Initialize your Storage device and File system. */
void fs_init(void);

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
