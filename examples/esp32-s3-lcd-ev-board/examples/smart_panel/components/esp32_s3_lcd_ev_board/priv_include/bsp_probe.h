/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MODULE_TYPE_UNKNOW = 0,
    MODULE_TYPE_R8,     /*!< ESP32-S3-WROOM-1-N16R8 */
    MODULE_TYPE_R16,    /*!< ESP32-S3-WROOM-1-N16R16V */
} bsp_module_type_t;

typedef enum {
    SUB_BOARD_TYPE_UNKNOW = 0,
    SUB_BOARD_TYPE_2_480_480,   /*!< Sub-board 2 with 480x480 LCD (GC9503), Touch (FT5x06) */
    SUB_BOARD_TYPE_3_800_480,   /*!< Sub-board 3 with 800x480 LCD (ST7262), Touch (GT1151) */
} bsp_sub_board_type_t;

/**
 * @brief Get module type
 *
 * @return
 *      - MODULE_TYPE_UNKNOW: Unknow module
 *      - MODULE_TYPE_R8: ESP32-S3-WROOM-1-N16R8
 *      - MODULE_TYPE_R16: ESP32-S3-WROOM-1-N16R16V
 */
bsp_module_type_t bsp_probe_module_type(void);

/**
 * @brief Get sub-board type
 *
 * @return
 *      - SUB_BOARD_TYPE_UNKNOW: Unknow sub-board
 *      - SUB_BOARD_TYPE_2_480_480: Sub-board 2 with 480x480 LCD (GC9503), Touch (FT5x06)
 *      - SUB_BOARD_TYPE_3_800_480: Sub-board 3 with 800x480 LCD (ST7262), Touch (GT1151)
 */
bsp_sub_board_type_t bsp_probe_sub_board_type(void);

#ifdef __cplusplus
}
#endif
