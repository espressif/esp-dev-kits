#ifndef BSP_LCD_H
#define BSP_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_LCD_VSYNC_GPIO            (3)
#define BSP_LCD_HSYNC_GPIO            (46)
#define BSP_LCD_DE_GPIO               (17)
#define BSP_LCD_PCLK_GPIO             (9)

#define BSP_LCD_DATA0_GPIO            (10)  // B0
#define BSP_LCD_DATA1_GPIO            (11)  // B1
#define BSP_LCD_DATA2_GPIO            (12)  // B2
#define BSP_LCD_DATA3_GPIO            (13)  // B3
#define BSP_LCD_DATA4_GPIO            (14)  // B4

#define BSP_LCD_DATA5_GPIO            (21)  // G0
#define BSP_LCD_DATA6_GPIO            (47)  // G1
#define BSP_LCD_DATA7_GPIO            (48)  // G2
#define BSP_LCD_DATA8_GPIO            (45)  // G3
#define BSP_LCD_DATA9_GPIO            (38)  // G4
#define BSP_LCD_DATA10_GPIO           (39)  // G5

#define BSP_LCD_DATA11_GPIO           (40) // R0
#define BSP_LCD_DATA12_GPIO           (41) // R1
#define BSP_LCD_DATA13_GPIO           (42) // R2
#define BSP_LCD_DATA14_GPIO           (2)  // R3
#define BSP_LCD_DATA15_GPIO           (1)  // R4
#define BSP_LCD_DISP_EN_GPIO          (-1)

#define BSP_LCD_WIDTH                 (800)
#define BSP_LCD_HEIGHT                (480)
#define BSP_LCD_PCLK                  (18000000)
#define BSP_LCD_HSYC_BACK_PORCH       (40)
#define BSP_LCD_HSYC_FRONT_PORCH      (48)
#define BSP_LCD_HSYC_PLUS_WIDTH       (40)
#define BSP_LCD_VSYC_BACK_PORCH       (32)
#define BSP_LCD_VSYC_FRONT_PORCH      (13)
#define BSP_LCD_VSYC_PLUS_WIDTH       (23)
#define BSP_LCD_PCLK_ACTIVE_NEG       (1)

typedef bool (*bsp_lcd_frame_callback_t)(void);

void  bsp_lcd_init(void);

void *bsp_lcd_get_panel_handle(void);

/**
 * @return Whether a high priority task has been waken up by this function
 */
void  bsp_lcd_frame_cb_register(bsp_lcd_frame_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif