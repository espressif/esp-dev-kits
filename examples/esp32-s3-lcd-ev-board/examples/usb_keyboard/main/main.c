/*
 * SPDX-FileCopyrightText: 2021-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "bsp/esp-bsp.h"

extern void usb_keyboard_init(void);

void app_main(void)
{
    bsp_i2c_init();
    bsp_display_start();

    usb_keyboard_init();
}
