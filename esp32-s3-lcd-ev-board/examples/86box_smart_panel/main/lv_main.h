/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#ifndef __LV_MAIN_H__
#define __LV_MAIN_H__

#include "app_sr.h"

#define SR_FUNC_ENABLED             1
#define WIFI_FUNC_ENABLED           0
#define WEATHER_FUNC_ENABLED        0

extern void sr_sem_take(void);

extern void sr_sem_give(void);

extern void monitor_print_once(uint8_t *str, uint32_t len);

#endif
