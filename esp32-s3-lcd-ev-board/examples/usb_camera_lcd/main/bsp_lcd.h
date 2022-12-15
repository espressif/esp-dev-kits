/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#pragma once

#include "esp_lcd_types.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_LCD_VSYNC           (GPIO_NUM_3)
#define BSP_LCD_HSYNC           (GPIO_NUM_46)
#define BSP_LCD_DE              (GPIO_NUM_17)
#define BSP_LCD_PCLK            (GPIO_NUM_9)
#define BSP_LCD_DATA0           (GPIO_NUM_10)   // B3
#define BSP_LCD_DATA1           (GPIO_NUM_11)   // B4
#define BSP_LCD_DATA2           (GPIO_NUM_12)   // B5
#define BSP_LCD_DATA3           (GPIO_NUM_13)   // B6
#define BSP_LCD_DATA4           (GPIO_NUM_14)   // B7
#define BSP_LCD_DATA5           (GPIO_NUM_21)   // G2
#define BSP_LCD_DATA6           (GPIO_NUM_47)   // G3
#define BSP_LCD_DATA7           (GPIO_NUM_48)   // G4
#define BSP_LCD_DATA8           (GPIO_NUM_45)   // G5
#define BSP_LCD_DATA9           (GPIO_NUM_38)   // G6
#define BSP_LCD_DATA10          (GPIO_NUM_39)   // G7
#define BSP_LCD_DATA11          (GPIO_NUM_40)   // R3
#define BSP_LCD_DATA12          (GPIO_NUM_41)   // R4
#define BSP_LCD_DATA13          (GPIO_NUM_42)   // R5
#define BSP_LCD_DATA14          (GPIO_NUM_2)    // R6
#define BSP_LCD_DATA15          (GPIO_NUM_1)    // R7

#define BSP_LCD_H_RES                   (800)
#define BSP_LCD_V_RES                   (480)
#define BSP_LCD_PIXEL_CLOCK_HZ          (18 * 1000 * 1000)
#define BSP_LCD_HSYNC_BACK_PORCH        (40)
#define BSP_LCD_HSYNC_FRONT_PORCH       (48)
#define BSP_LCD_HSYNC_PULSE_WIDTH       (40)
#define BSP_LCD_VSYNC_BACK_PORCH        (32)
#define BSP_LCD_VSYNC_FRONT_PORCH       (13)
#define BSP_LCD_VSYNC_PULSE_WIDTH       (23)
#define BSP_LCD_PCLK_ACTIVE_NEG         (true)

esp_lcd_panel_handle_t bsp_lcd_init(void);

#ifdef __cplusplus
}
#endif
