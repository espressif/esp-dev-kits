/* SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "ui.h"
#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"
#include "app_msc.h"

void app_main()
{
    /* Initialize I2C (for touch and audio) */
    bsp_i2c_init();

    /* Initialize display and LVGL */
    bsp_display_start();

    /* Initialize LVGL widgets */
    ui_init();

    /* Initialize USB MSC */
    msc_init();

    /* Initialize audio */
    bsp_extra_codec_init();
    bsp_extra_player_init(NULL);

}
