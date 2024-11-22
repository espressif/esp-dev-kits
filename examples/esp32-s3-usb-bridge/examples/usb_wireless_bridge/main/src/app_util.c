/* SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <esp_system.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "app_util.h"
#include "sdkconfig.h"
#include "app_io.h"
#include "ws2812_led.h"
#include "app_indicator.h"
#include "app_mode.h"

void __attribute__((noreturn)) eub_abort(void)
{
    app_indicator_update_mode(MODE_ERROR, BLINK_EUB_ABORT, true);
    abort();
}