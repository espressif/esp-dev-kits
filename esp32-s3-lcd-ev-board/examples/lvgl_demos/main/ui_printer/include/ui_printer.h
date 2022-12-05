#ifndef UI_DEMO1_H
#define UI_DEMO1_H

#ifdef __cplusplus
extern "C" {
#endif

#if __has_include("lvgl.h")
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

extern lv_obj_t * ui_Screen_1_Print;
extern lv_obj_t * ui_Background;
extern lv_obj_t * ui_Panel_Header;
extern lv_obj_t * ui_Label_Header;
extern lv_obj_t * ui_IMG_Wifi;
extern lv_obj_t * ui_IMG_PC;
extern lv_obj_t * ui_IMG_USB;
extern lv_obj_t * ui_BTN_Menu_Print_S1;
extern lv_obj_t * ui_BTN_Menu_Move_S1;
extern lv_obj_t * ui_BTN_Menu_Setting_S1;
extern lv_obj_t * ui_S1_Content_Panel;
extern lv_obj_t * ui_Slider_Print_View;
extern lv_obj_t * ui_Number_Print;
extern lv_obj_t * ui_Display_Time_S1;
extern lv_obj_t * ui_DT1;
extern lv_obj_t * ui_Label_Printing_Time_1;
extern lv_obj_t * ui_IMG_Tine_1;
extern lv_obj_t * ui_Label_Time_1;
extern lv_obj_t * ui_Panel3;
extern lv_obj_t * ui_DT2;
extern lv_obj_t * ui_Label_Printing_Time_2;
extern lv_obj_t * ui_IMG_Tine_2;
extern lv_obj_t * ui_Label_Time_2;
extern lv_obj_t * ui_Display_Heat_S1;
extern lv_obj_t * ui_DT3;
extern lv_obj_t * ui_Label_Printing_Head_Temp;
extern lv_obj_t * ui_IMG_Head;
extern lv_obj_t * ui_Label_Head_Temp;
extern lv_obj_t * ui_Label_Printing_Head_Temp_2;
extern lv_obj_t * ui_Panel1;
extern lv_obj_t * ui_DT3_copy;
extern lv_obj_t * ui_Label_Printing_Bed_Temp;
extern lv_obj_t * ui_IMG_Head1;
extern lv_obj_t * ui_Label_Bed_Temp;
extern lv_obj_t * ui_Label_Printing_Bed_Temp1;
extern lv_obj_t * ui_Panel_Buttons_S1;
extern lv_obj_t * ui_BTN_Pause;
extern lv_obj_t * ui_BTN_Pause_Top;
extern lv_obj_t * ui_Image_Pause;
extern lv_obj_t * ui_BTN_Cancel;
extern lv_obj_t * ui_BTN_Cancel_Top;
extern lv_obj_t * ui_Image_Cancel;
extern lv_obj_t * ui_Screen_2_Move;
extern lv_obj_t * ui_Background1;
extern lv_obj_t * ui_Panel_Header1;
extern lv_obj_t * ui_Label_Header1;
extern lv_obj_t * ui_IMG_Wifi1;
extern lv_obj_t * ui_IMG_PC1;
extern lv_obj_t * ui_IMG_USB1;
extern lv_obj_t * ui_BTN_Menu_Print_S2;
extern lv_obj_t * ui_BTN_Menu_Move_S2;
extern lv_obj_t * ui_BTN_Menu_Setting_S2;
extern lv_obj_t * ui_S1_Content_Panel1;
extern lv_obj_t * ui_Display_Pos_XY;
extern lv_obj_t * ui_DP1;
extern lv_obj_t * ui_Label_X_Position;
extern lv_obj_t * ui_Label_X_Position_Number;
extern lv_obj_t * ui_Panel2;
extern lv_obj_t * ui_DP2;
extern lv_obj_t * ui_Label_Y_Position;
extern lv_obj_t * ui_Label_Time_3;
extern lv_obj_t * ui_Display_Pos_y;
extern lv_obj_t * ui_DP3;
extern lv_obj_t * ui_Label_Z_Position;
extern lv_obj_t * ui_Label_Z_Position_Number;
extern lv_obj_t * ui_BTN_Move_Z;
extern lv_obj_t * ui_GLOW_1;
extern lv_obj_t * ui_GLOW_2;
extern lv_obj_t * ui_BTN_Image_Top;
extern lv_obj_t * ui_Label_Z_Position_2;
extern lv_obj_t * ui_Arrow_Z_up;
extern lv_obj_t * ui_Arrow_Z_Down;
extern lv_obj_t * ui_BTN_Move_XY;
extern lv_obj_t * ui_GLOW_3;
extern lv_obj_t * ui_GLOW_4;
extern lv_obj_t * ui_GLOW_5;
extern lv_obj_t * ui_GLOW_6;
extern lv_obj_t * ui_BTN_Move_XY_Top;
extern lv_obj_t * ui_Label_XY_Position;
extern lv_obj_t * ui_Arrow_Up_2;
extern lv_obj_t * ui_Arrow_Down_2;
extern lv_obj_t * ui_Arrow_Left_2;
extern lv_obj_t * ui_Arrow_Right_2;
extern lv_obj_t * ui_Roller_Number;
extern lv_obj_t * ui_Image8;
extern lv_obj_t * ui_Roller1;
extern lv_obj_t * ui_Label_Steps;
extern lv_obj_t * ui_BTN_Reset;
extern lv_obj_t * ui_Label_Reset;
extern lv_obj_t * ui_BTN_Heat;
extern lv_obj_t * ui_Label_Reset1;
extern lv_obj_t * ui_Screen_3_Setting;
extern lv_obj_t * ui_Background2;
extern lv_obj_t * ui_Panel_Header2;
extern lv_obj_t * ui_Label_Header2;
extern lv_obj_t * ui_IMG_Wifi2;
extern lv_obj_t * ui_IMG_PC2;
extern lv_obj_t * ui_IMG_USB2;
extern lv_obj_t * ui_BTN_Menu_Print_S3;
extern lv_obj_t * ui_BTN_Menu_Move_S3;
extern lv_obj_t * ui_BTN_Menu_Setting_S3;
extern lv_obj_t * ui_S1_Content_Panel2;
extern lv_obj_t * ui_Roller_Material;
extern lv_obj_t * ui_Image3;
extern lv_obj_t * ui_Roller4;
extern lv_obj_t * ui_Label_Material;
extern lv_obj_t * ui_Roller_Bed_Temp;
extern lv_obj_t * ui_Image2;
extern lv_obj_t * ui_Roller3;
extern lv_obj_t * ui_Label_Bed_Temp1;
extern lv_obj_t * ui_Roller_Head_Temp;
extern lv_obj_t * ui_Image1;
extern lv_obj_t * ui_Roller2;
extern lv_obj_t * ui_Label_Head_Temp1;
extern lv_obj_t * ui_BTN_Heat_Head;
extern lv_obj_t * ui_Label_Reset22;
extern lv_obj_t * ui_BTN_Heat_Bed;
extern lv_obj_t * ui_Label_Heat_Bed;
extern lv_obj_t * ui_BTN_Remove_Filament;
extern lv_obj_t * ui_Label_Remove_Filament;
extern lv_obj_t * ui_Panel_Slider;
extern lv_obj_t * ui_Slider_Print_Speed;
extern lv_obj_t * ui_Label_Print_Speed;
extern lv_obj_t * ui_Label_Print_Speed_Number;
extern lv_obj_t * ui_Swich_Group;
extern lv_obj_t * ui_Label_Light;
extern lv_obj_t * ui_Switch_Light;
extern lv_obj_t * ui_Label_Fan_1;
extern lv_obj_t * ui_Switch_Fan_1;
extern lv_obj_t * ui_Label_Fan_2;
extern lv_obj_t * ui_Switch_Fan_2;
extern lv_obj_t * ui_Label_Fan_3;
extern lv_obj_t * ui_Switch_Fan_3;


LV_IMG_DECLARE(ui_img_3d_printer_bg_png);    // assets\3d_printer_bg.png
LV_IMG_DECLARE(ui_img_icn_wifi_png);    // assets\icn_wifi.png
LV_IMG_DECLARE(ui_img_icn_pc_png);    // assets\icn_pc.png
LV_IMG_DECLARE(ui_img_icn_usb_png);    // assets\icn_usb.png
LV_IMG_DECLARE(ui_img_btn_print_png);    // assets\btn_print.png
LV_IMG_DECLARE(ui_img_btn_move_png);    // assets\btn_move.png
LV_IMG_DECLARE(ui_img_btn_setting_png);    // assets\btn_setting.png
LV_IMG_DECLARE(ui_img_print_view_bg_png);    // assets\print_view_bg.png
LV_IMG_DECLARE(ui_img_print_view_front_png);    // assets\print_view_front.png
LV_IMG_DECLARE(ui_img_icn_time_1_png);    // assets\icn_time_1.png
LV_IMG_DECLARE(ui_img_icn_time_2_png);    // assets\icn_time_2.png
LV_IMG_DECLARE(ui_img_icn_head_png);    // assets\icn_head.png
LV_IMG_DECLARE(ui_img_icn_bed_png);    // assets\icn_bed.png
LV_IMG_DECLARE(ui_img_btn_print_down_png);    // assets\btn_print_down.png
LV_IMG_DECLARE(ui_img_btn_print_top_off_png);    // assets\btn_print_top_off.png
LV_IMG_DECLARE(ui_img_icn_pause_png);    // assets\icn_pause.png
LV_IMG_DECLARE(ui_img_icn_stop_png);    // assets\icn_stop.png
LV_IMG_DECLARE(ui_img_btn_z_botton_png);    // assets\btn_z_botton.png
LV_IMG_DECLARE(ui_img_btn_z_top_png);    // assets\btn_z_top.png
LV_IMG_DECLARE(ui_img_arrow_up_png);    // assets\arrow_up.png
LV_IMG_DECLARE(ui_img_arrow_down_png);    // assets\arrow_down.png
LV_IMG_DECLARE(ui_img_btn_pad_bottom_png);    // assets\btn_pad_bottom.png
LV_IMG_DECLARE(ui_img_btn_pad_top_png);    // assets\btn_pad_top.png
LV_IMG_DECLARE(ui_img_arrow_left_png);    // assets\arrow_left.png
LV_IMG_DECLARE(ui_img_arrow_right_png);    // assets\arrow_right.png
LV_IMG_DECLARE(ui_img_roller_bg_png);    // assets\roller_bg.png
LV_IMG_DECLARE(ui_img_btn_main_off_png);    // assets\btn_main_off.png
LV_IMG_DECLARE(ui_img_btn_main_on_png);    // assets\btn_main_on.png


LV_FONT_DECLARE(ui_font_Small_Font);


void ui_printer_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
