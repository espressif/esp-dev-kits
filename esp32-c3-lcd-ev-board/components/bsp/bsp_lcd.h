#ifndef BSP_LCD_H
#define BSP_LCD_H

#include "esp_lcd_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LCD_H_RES               (240)
#define LCD_V_RES               (240)

typedef bool (*bsp_lcd_trans_done_cb_t)(void);

esp_lcd_panel_handle_t bsp_lcd_init(void);

void  bsp_lcd_trans_done_cb_register(bsp_lcd_trans_done_cb_t callback);

void bsp_lcd_set_brightness(uint8_t percent);

#ifdef __cplusplus
}
#endif

#endif