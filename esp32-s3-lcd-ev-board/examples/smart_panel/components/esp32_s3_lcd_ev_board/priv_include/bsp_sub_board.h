/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef BSP_SUB_BOARD_H
#define BSP_SUB_BOARD_H

#include "bsp/esp32_s3_lcd_ev_board.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SUB_BOARD_TYPE_UNKNOW = 0,
    SUB_BOARD_TYPE_2_480_480,
    SUB_BOARD_TYPE_3_800_480,
} bsp_sub_board_type_t;

/**
 * @brief Get sub-board type
 *
 * @return
 *      - SUB_BOARD_TYPE_UNKNOW: Unknow sub-board
 *      - SUB_BOARD_TYPE_2_480_480: Sub-board 2 with 480x480 LCD (GC9503), Touch (FT5x06)
 *      - SUB_BOARD_TYPE_3_800_480: Sub-board 3 with 800x480 LCD (ST7262), Touch (GT1151)
 */
bsp_sub_board_type_t bsp_sub_board_get_type(void);

#ifdef __cplusplus
}
#endif

#endif
