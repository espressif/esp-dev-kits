// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.0.5
// LVGL VERSION: 8.2
// PROJECT: 3d_printer

#include "ui_printer.h"
#include "ui_helpers.h"

///////////////////// VARIABLES ////////////////////
lv_obj_t * ui_Screen_1_Print;
lv_obj_t * ui_Background;
lv_obj_t * ui_Panel_Header;
lv_obj_t * ui_Label_Header;
lv_obj_t * ui_IMG_Wifi;
lv_obj_t * ui_IMG_PC;
lv_obj_t * ui_IMG_USB;
lv_obj_t * ui_BTN_Menu_Print_S1;
lv_obj_t * ui_BTN_Menu_Move_S1;
lv_obj_t * ui_BTN_Menu_Setting_S1;
lv_obj_t * ui_S1_Content_Panel;
lv_obj_t * ui_Slider_Print_View;
lv_obj_t * ui_Number_Print;
lv_obj_t * ui_Display_Time_S1;
lv_obj_t * ui_DT1;
lv_obj_t * ui_Label_Printing_Time_1;
lv_obj_t * ui_IMG_Tine_1;
lv_obj_t * ui_Label_Time_1;
lv_obj_t * ui_Panel3;
lv_obj_t * ui_DT2;
lv_obj_t * ui_Label_Printing_Time_2;
lv_obj_t * ui_IMG_Tine_2;
lv_obj_t * ui_Label_Time_2;
lv_obj_t * ui_Display_Heat_S1;
lv_obj_t * ui_DT3;
lv_obj_t * ui_Label_Printing_Head_Temp;
lv_obj_t * ui_IMG_Head;
lv_obj_t * ui_Label_Head_Temp;
lv_obj_t * ui_Label_Printing_Head_Temp_2;
lv_obj_t * ui_Panel1;
lv_obj_t * ui_DT3_copy;
lv_obj_t * ui_Label_Printing_Bed_Temp;
lv_obj_t * ui_IMG_Head1;
lv_obj_t * ui_Label_Bed_Temp;
lv_obj_t * ui_Label_Printing_Bed_Temp1;
lv_obj_t * ui_Panel_Buttons_S1;
lv_obj_t * ui_BTN_Pause;
lv_obj_t * ui_BTN_Pause_Top;
lv_obj_t * ui_Image_Pause;
lv_obj_t * ui_BTN_Cancel;
lv_obj_t * ui_BTN_Cancel_Top;
lv_obj_t * ui_Image_Cancel;
lv_obj_t * ui_Screen_2_Move;
lv_obj_t * ui_Background1;
lv_obj_t * ui_Panel_Header1;
lv_obj_t * ui_Label_Header1;
lv_obj_t * ui_IMG_Wifi1;
lv_obj_t * ui_IMG_PC1;
lv_obj_t * ui_IMG_USB1;
lv_obj_t * ui_BTN_Menu_Print_S2;
lv_obj_t * ui_BTN_Menu_Move_S2;
lv_obj_t * ui_BTN_Menu_Setting_S2;
lv_obj_t * ui_S1_Content_Panel1;
lv_obj_t * ui_Display_Pos_XY;
lv_obj_t * ui_DP1;
lv_obj_t * ui_Label_X_Position;
lv_obj_t * ui_Label_X_Position_Number;
lv_obj_t * ui_Panel2;
lv_obj_t * ui_DP2;
lv_obj_t * ui_Label_Y_Position;
lv_obj_t * ui_Label_Time_3;
lv_obj_t * ui_Display_Pos_y;
lv_obj_t * ui_DP3;
lv_obj_t * ui_Label_Z_Position;
lv_obj_t * ui_Label_Z_Position_Number;
lv_obj_t * ui_BTN_Move_Z;
lv_obj_t * ui_GLOW_1;
lv_obj_t * ui_GLOW_2;
lv_obj_t * ui_BTN_Image_Top;
lv_obj_t * ui_Label_Z_Position_2;
lv_obj_t * ui_Arrow_Z_up;
lv_obj_t * ui_Arrow_Z_Down;
lv_obj_t * ui_BTN_Move_XY;
lv_obj_t * ui_GLOW_3;
lv_obj_t * ui_GLOW_4;
lv_obj_t * ui_GLOW_5;
lv_obj_t * ui_GLOW_6;
lv_obj_t * ui_BTN_Move_XY_Top;
lv_obj_t * ui_Label_XY_Position;
lv_obj_t * ui_Arrow_Up_2;
lv_obj_t * ui_Arrow_Down_2;
lv_obj_t * ui_Arrow_Left_2;
lv_obj_t * ui_Arrow_Right_2;
lv_obj_t * ui_Roller_Number;
lv_obj_t * ui_Image8;
lv_obj_t * ui_Roller1;
lv_obj_t * ui_Label_Steps;
lv_obj_t * ui_BTN_Reset;
lv_obj_t * ui_Label_Reset;
lv_obj_t * ui_BTN_Heat;
lv_obj_t * ui_Label_Reset1;
lv_obj_t * ui_Screen_3_Setting;
lv_obj_t * ui_Background2;
lv_obj_t * ui_Panel_Header2;
lv_obj_t * ui_Label_Header2;
lv_obj_t * ui_IMG_Wifi2;
lv_obj_t * ui_IMG_PC2;
lv_obj_t * ui_IMG_USB2;
lv_obj_t * ui_BTN_Menu_Print_S3;
lv_obj_t * ui_BTN_Menu_Move_S3;
lv_obj_t * ui_BTN_Menu_Setting_S3;
lv_obj_t * ui_S1_Content_Panel2;
lv_obj_t * ui_Roller_Material;
lv_obj_t * ui_Image3;
lv_obj_t * ui_Roller4;
lv_obj_t * ui_Label_Material;
lv_obj_t * ui_Roller_Bed_Temp;
lv_obj_t * ui_Image2;
lv_obj_t * ui_Roller3;
lv_obj_t * ui_Label_Bed_Temp1;
lv_obj_t * ui_Roller_Head_Temp;
lv_obj_t * ui_Image1;
lv_obj_t * ui_Roller2;
lv_obj_t * ui_Label_Head_Temp1;
lv_obj_t * ui_BTN_Heat_Head;
lv_obj_t * ui_Label_Reset22;
lv_obj_t * ui_BTN_Heat_Bed;
lv_obj_t * ui_Label_Heat_Bed;
lv_obj_t * ui_BTN_Remove_Filament;
lv_obj_t * ui_Label_Remove_Filament;
lv_obj_t * ui_Panel_Slider;
lv_obj_t * ui_Slider_Print_Speed;
lv_obj_t * ui_Label_Print_Speed;
lv_obj_t * ui_Label_Print_Speed_Number;
lv_obj_t * ui_Swich_Group;
lv_obj_t * ui_Label_Light;
lv_obj_t * ui_Switch_Light;
lv_obj_t * ui_Label_Fan_1;
lv_obj_t * ui_Switch_Fan_1;
lv_obj_t * ui_Label_Fan_2;
lv_obj_t * ui_Switch_Fan_2;
lv_obj_t * ui_Label_Fan_3;
lv_obj_t * ui_Switch_Fan_3;

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
    #error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP !=0
    #error "#error LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////

//
// FUNCTION HEADER
void OpaOn_Animation(lv_obj_t * TargetObject, int delay);

// FUNCTION
void OpaOn_Animation(lv_obj_t * TargetObject, int delay)
{

    //
    lv_anim_t PropertyAnimation_0;
    lv_anim_init(&PropertyAnimation_0);
    lv_anim_set_time(&PropertyAnimation_0, 10);
    lv_anim_set_user_data(&PropertyAnimation_0, TargetObject);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_opacity);
    lv_anim_set_values(&PropertyAnimation_0, 0, 255);
    lv_anim_set_path_cb(&PropertyAnimation_0, lv_anim_path_linear);
    lv_anim_set_delay(&PropertyAnimation_0, delay + 0);
    lv_anim_set_early_apply(&PropertyAnimation_0, false);
    lv_anim_start(&PropertyAnimation_0);

}

//
// FUNCTION HEADER
void OpaOff_Animation(lv_obj_t * TargetObject, int delay);

// FUNCTION
void OpaOff_Animation(lv_obj_t * TargetObject, int delay)
{

    //
    lv_anim_t PropertyAnimation_0;
    lv_anim_init(&PropertyAnimation_0);
    lv_anim_set_time(&PropertyAnimation_0, 200);
    lv_anim_set_user_data(&PropertyAnimation_0, TargetObject);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_opacity);
    lv_anim_set_values(&PropertyAnimation_0, 255, 0);
    lv_anim_set_path_cb(&PropertyAnimation_0, lv_anim_path_linear);
    lv_anim_set_delay(&PropertyAnimation_0, delay + 0);
    lv_anim_set_early_apply(&PropertyAnimation_0, false);
    lv_anim_start(&PropertyAnimation_0);

}

//
// FUNCTION HEADER
void ScreenIn_Animation(lv_obj_t * TargetObject, int delay);

// FUNCTION
void ScreenIn_Animation(lv_obj_t * TargetObject, int delay)
{

    //
    lv_anim_t PropertyAnimation_0;
    lv_anim_init(&PropertyAnimation_0);
    lv_anim_set_time(&PropertyAnimation_0, 300);
    lv_anim_set_user_data(&PropertyAnimation_0, TargetObject);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_opacity);
    lv_anim_set_values(&PropertyAnimation_0, 0, 255);
    lv_anim_set_path_cb(&PropertyAnimation_0, lv_anim_path_linear);
    lv_anim_set_delay(&PropertyAnimation_0, delay + 0);
    lv_anim_set_early_apply(&PropertyAnimation_0, true);
    lv_anim_start(&PropertyAnimation_0);

}

//
// FUNCTION HEADER
void MenuIn_Animation(lv_obj_t * TargetObject, int delay);

// FUNCTION
void MenuIn_Animation(lv_obj_t * TargetObject, int delay)
{

    //
    lv_anim_t PropertyAnimation_0;
    lv_anim_init(&PropertyAnimation_0);
    lv_anim_set_time(&PropertyAnimation_0, 500);
    lv_anim_set_user_data(&PropertyAnimation_0, TargetObject);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_opacity);
    lv_anim_set_values(&PropertyAnimation_0, 0, 255);
    lv_anim_set_path_cb(&PropertyAnimation_0, lv_anim_path_ease_out);
    lv_anim_set_delay(&PropertyAnimation_0, delay + 0);
    lv_anim_set_early_apply(&PropertyAnimation_0, true);
    lv_anim_start(&PropertyAnimation_0);

}

//
// FUNCTION HEADER
void MenuOut_Animation(lv_obj_t * TargetObject, int delay);

// FUNCTION
void MenuOut_Animation(lv_obj_t * TargetObject, int delay)
{

    //
    lv_anim_t PropertyAnimation_0;
    lv_anim_init(&PropertyAnimation_0);
    lv_anim_set_time(&PropertyAnimation_0, 150);
    lv_anim_set_user_data(&PropertyAnimation_0, TargetObject);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_opacity);
    lv_anim_set_values(&PropertyAnimation_0, 255, 0);
    lv_anim_set_path_cb(&PropertyAnimation_0, lv_anim_path_ease_in);
    lv_anim_set_delay(&PropertyAnimation_0, delay + 0);
    lv_anim_set_early_apply(&PropertyAnimation_0, false);
    lv_anim_start(&PropertyAnimation_0);

}

//
// FUNCTION HEADER
void ScreenOut_Animation(lv_obj_t * TargetObject, int delay);

// FUNCTION
void ScreenOut_Animation(lv_obj_t * TargetObject, int delay)
{

    //
    lv_anim_t PropertyAnimation_0;
    lv_anim_init(&PropertyAnimation_0);
    lv_anim_set_time(&PropertyAnimation_0, 200);
    lv_anim_set_user_data(&PropertyAnimation_0, TargetObject);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_opacity);
    lv_anim_set_values(&PropertyAnimation_0, 255, 0);
    lv_anim_set_path_cb(&PropertyAnimation_0, lv_anim_path_linear);
    lv_anim_set_delay(&PropertyAnimation_0, delay + 0);
    lv_anim_set_early_apply(&PropertyAnimation_0, false);
    lv_anim_start(&PropertyAnimation_0);

}

///////////////////// FUNCTIONS ////////////////////
static void ui_event_BTN_Menu_Move_S1(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_CLICKED) {
        _ui_screen_change(ui_Screen_2_Move, LV_SCR_LOAD_ANIM_FADE_ON, 10, 0);
    }
}
static void ui_event_BTN_Menu_Setting_S1(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_CLICKED) {
        _ui_screen_change(ui_Screen_3_Setting, LV_SCR_LOAD_ANIM_FADE_ON, 10, 0);
    }
}
static void ui_event_Slider_Print_View(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_VALUE_CHANGED) {
        _ui_slider_set_text_value(ui_Number_Print, ta, "", "%");
    }
}
static void ui_event_BTN_Cancel_Top(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_CLICKED) {
        _ui_state_modify(ui_BTN_Pause_Top, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
    }
}
static void ui_event_BTN_Menu_Print_S2(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_CLICKED) {
        _ui_screen_change(ui_Screen_1_Print, LV_SCR_LOAD_ANIM_FADE_ON, 10, 0);
    }
}
static void ui_event_BTN_Menu_Setting_S2(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_CLICKED) {
        _ui_screen_change(ui_Screen_3_Setting, LV_SCR_LOAD_ANIM_FADE_ON, 10, 0);
    }
}
static void ui_event_GLOW_1(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_PRESSED) {
        OpaOn_Animation(ui_GLOW_1, 0);
        _ui_state_modify(ui_Arrow_Z_up, LV_STATE_CHECKED, _UI_MODIFY_STATE_ADD);
    }
    if(event == LV_EVENT_RELEASED) {
        OpaOff_Animation(ui_GLOW_1, 0);
        _ui_state_modify(ui_Arrow_Z_up, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
    }
    if(event == LV_EVENT_PRESS_LOST) {
        OpaOff_Animation(ui_GLOW_1, 0);
        _ui_state_modify(ui_Arrow_Z_up, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
    }
}
static void ui_event_GLOW_2(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_PRESSED) {
        OpaOn_Animation(ui_GLOW_2, 0);
        _ui_state_modify(ui_Arrow_Z_Down, LV_STATE_CHECKED, _UI_MODIFY_STATE_ADD);
    }
    if(event == LV_EVENT_RELEASED) {
        OpaOff_Animation(ui_GLOW_2, 0);
        _ui_state_modify(ui_Arrow_Z_Down, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
    }
    if(event == LV_EVENT_PRESS_LOST) {
        OpaOff_Animation(ui_GLOW_2, 0);
        _ui_state_modify(ui_Arrow_Z_Down, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
    }
}
static void ui_event_GLOW_3(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_PRESSED) {
        OpaOn_Animation(ui_GLOW_3, 0);
        _ui_state_modify(ui_Arrow_Right_2, LV_STATE_CHECKED, _UI_MODIFY_STATE_ADD);
    }
    if(event == LV_EVENT_RELEASED) {
        OpaOff_Animation(ui_GLOW_3, 0);
        _ui_state_modify(ui_Arrow_Right_2, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
    }
    if(event == LV_EVENT_PRESS_LOST) {
        OpaOff_Animation(ui_GLOW_3, 0);
        _ui_state_modify(ui_Arrow_Right_2, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
    }
}
static void ui_event_GLOW_4(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_PRESSED) {
        OpaOn_Animation(ui_GLOW_4, 0);
        _ui_state_modify(ui_Arrow_Up_2, LV_STATE_CHECKED, _UI_MODIFY_STATE_ADD);
    }
    if(event == LV_EVENT_RELEASED) {
        OpaOff_Animation(ui_GLOW_4, 0);
        _ui_state_modify(ui_Arrow_Up_2, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
    }
    if(event == LV_EVENT_PRESS_LOST) {
        OpaOff_Animation(ui_GLOW_4, 0);
        _ui_state_modify(ui_Arrow_Up_2, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
    }
}
static void ui_event_GLOW_5(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_PRESSED) {
        OpaOn_Animation(ui_GLOW_5, 0);
        _ui_state_modify(ui_Arrow_Down_2, LV_STATE_CHECKED, _UI_MODIFY_STATE_ADD);
    }
    if(event == LV_EVENT_RELEASED) {
        OpaOff_Animation(ui_GLOW_5, 0);
        _ui_state_modify(ui_Arrow_Down_2, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
    }
    if(event == LV_EVENT_PRESS_LOST) {
        OpaOff_Animation(ui_GLOW_5, 0);
        _ui_state_modify(ui_Arrow_Down_2, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
    }
}
static void ui_event_GLOW_6(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_PRESSED) {
        OpaOn_Animation(ui_GLOW_6, 0);
        _ui_state_modify(ui_Arrow_Left_2, LV_STATE_CHECKED, _UI_MODIFY_STATE_ADD);
    }
    if(event == LV_EVENT_RELEASED) {
        OpaOff_Animation(ui_GLOW_6, 0);
        _ui_state_modify(ui_Arrow_Left_2, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
    }
    if(event == LV_EVENT_PRESS_LOST) {
        OpaOff_Animation(ui_GLOW_6, 0);
        _ui_state_modify(ui_Arrow_Left_2, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
    }
}
static void ui_event_BTN_Menu_Print_S3(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_CLICKED) {
        _ui_screen_change(ui_Screen_1_Print, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0);
    }
}
static void ui_event_BTN_Menu_Move_S3(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_CLICKED) {
        _ui_screen_change(ui_Screen_2_Move, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0);
    }
}
static void ui_event_Slider_Print_Speed(lv_event_t * e)
{
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event == LV_EVENT_VALUE_CHANGED) {
        _ui_slider_set_text_value(ui_Label_Print_Speed_Number, ta, "", "%");
    }
}

///////////////////// SCREENS ////////////////////
void ui_Screen_1_Print_screen_init(void)
{

    // ui_Screen_1_Print

    ui_Screen_1_Print = lv_obj_create(NULL);

    lv_obj_set_style_bg_color(ui_Screen_1_Print, lv_color_hex(0x191D26), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Screen_1_Print, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Background

    ui_Background = lv_img_create(ui_Screen_1_Print);
    lv_img_set_src(ui_Background, &ui_img_3d_printer_bg_png);

    lv_obj_set_width(ui_Background, 800);
    lv_obj_set_height(ui_Background, 480);

    lv_obj_set_x(ui_Background, 0);
    lv_obj_set_y(ui_Background, 0);

    lv_obj_set_align(ui_Background, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_Background, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_Panel_Header

    ui_Panel_Header = lv_obj_create(ui_Screen_1_Print);

    lv_obj_set_width(ui_Panel_Header, 665);
    lv_obj_set_height(ui_Panel_Header, 39);

    lv_obj_set_x(ui_Panel_Header, -30);
    lv_obj_set_y(ui_Panel_Header, 0);

    lv_obj_set_align(ui_Panel_Header, LV_ALIGN_TOP_RIGHT);

    lv_obj_clear_flag(ui_Panel_Header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Panel_Header, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Panel_Header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Panel_Header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Panel_Header, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Panel_Header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_Header

    ui_Label_Header = lv_label_create(ui_Panel_Header);

    lv_obj_set_width(ui_Label_Header, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Header, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Header, 0);
    lv_obj_set_y(ui_Label_Header, 0);

    lv_obj_set_align(ui_Label_Header, LV_ALIGN_LEFT_MID);

    lv_label_set_text(ui_Label_Header, "The #ffffff squareline.gcode# word file is being printed");
    lv_label_set_recolor(ui_Label_Header, "true");

    lv_obj_set_style_text_color(ui_Label_Header, lv_color_hex(0x9098AA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Header, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Header, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_IMG_Wifi

    ui_IMG_Wifi = lv_img_create(ui_Panel_Header);
    lv_img_set_src(ui_IMG_Wifi, &ui_img_icn_wifi_png);

    lv_obj_set_width(ui_IMG_Wifi, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_IMG_Wifi, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_IMG_Wifi, 0);
    lv_obj_set_y(ui_IMG_Wifi, 0);

    lv_obj_set_align(ui_IMG_Wifi, LV_ALIGN_RIGHT_MID);

    lv_obj_add_flag(ui_IMG_Wifi, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_IMG_PC

    ui_IMG_PC = lv_img_create(ui_Panel_Header);
    lv_img_set_src(ui_IMG_PC, &ui_img_icn_pc_png);

    lv_obj_set_width(ui_IMG_PC, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_IMG_PC, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_IMG_PC, -42);
    lv_obj_set_y(ui_IMG_PC, 0);

    lv_obj_set_align(ui_IMG_PC, LV_ALIGN_RIGHT_MID);

    lv_obj_add_flag(ui_IMG_PC, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_IMG_USB

    ui_IMG_USB = lv_img_create(ui_Panel_Header);
    lv_img_set_src(ui_IMG_USB, &ui_img_icn_usb_png);

    lv_obj_set_width(ui_IMG_USB, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_IMG_USB, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_IMG_USB, -84);
    lv_obj_set_y(ui_IMG_USB, 0);

    lv_obj_set_align(ui_IMG_USB, LV_ALIGN_RIGHT_MID);

    lv_obj_add_flag(ui_IMG_USB, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_BTN_Menu_Print_S1

    ui_BTN_Menu_Print_S1 = lv_img_create(ui_Screen_1_Print);
    lv_img_set_src(ui_BTN_Menu_Print_S1, &ui_img_btn_print_png);

    lv_obj_set_width(ui_BTN_Menu_Print_S1, 79);
    lv_obj_set_height(ui_BTN_Menu_Print_S1, 160);

    lv_obj_set_x(ui_BTN_Menu_Print_S1, 0);
    lv_obj_set_y(ui_BTN_Menu_Print_S1, 0);

    lv_obj_add_flag(ui_BTN_Menu_Print_S1, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(ui_BTN_Menu_Print_S1, LV_OBJ_FLAG_SCROLLABLE);

    // ui_BTN_Menu_Move_S1

    ui_BTN_Menu_Move_S1 = lv_img_create(ui_Screen_1_Print);
    lv_img_set_src(ui_BTN_Menu_Move_S1, &ui_img_btn_move_png);

    lv_obj_set_width(ui_BTN_Menu_Move_S1, 79);
    lv_obj_set_height(ui_BTN_Menu_Move_S1, 162);

    lv_obj_set_x(ui_BTN_Menu_Move_S1, 0);
    lv_obj_set_y(ui_BTN_Menu_Move_S1, 0);

    lv_obj_set_align(ui_BTN_Menu_Move_S1, LV_ALIGN_LEFT_MID);

    lv_obj_add_flag(ui_BTN_Menu_Move_S1, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(ui_BTN_Menu_Move_S1, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_event_cb(ui_BTN_Menu_Move_S1, ui_event_BTN_Menu_Move_S1, LV_EVENT_ALL, NULL);
    lv_obj_set_style_opa(ui_BTN_Menu_Move_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_Menu_Setting_S1

    ui_BTN_Menu_Setting_S1 = lv_img_create(ui_Screen_1_Print);
    lv_img_set_src(ui_BTN_Menu_Setting_S1, &ui_img_btn_setting_png);

    lv_obj_set_width(ui_BTN_Menu_Setting_S1, 79);
    lv_obj_set_height(ui_BTN_Menu_Setting_S1, 160);

    lv_obj_set_x(ui_BTN_Menu_Setting_S1, 0);
    lv_obj_set_y(ui_BTN_Menu_Setting_S1, 0);

    lv_obj_set_align(ui_BTN_Menu_Setting_S1, LV_ALIGN_BOTTOM_LEFT);

    lv_obj_add_flag(ui_BTN_Menu_Setting_S1, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(ui_BTN_Menu_Setting_S1, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_event_cb(ui_BTN_Menu_Setting_S1, ui_event_BTN_Menu_Setting_S1, LV_EVENT_ALL, NULL);
    lv_obj_set_style_opa(ui_BTN_Menu_Setting_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_S1_Content_Panel

    ui_S1_Content_Panel = lv_obj_create(ui_Screen_1_Print);

    lv_obj_set_width(ui_S1_Content_Panel, 720);
    lv_obj_set_height(ui_S1_Content_Panel, 430);

    lv_obj_set_x(ui_S1_Content_Panel, 0);
    lv_obj_set_y(ui_S1_Content_Panel, 0);

    lv_obj_set_align(ui_S1_Content_Panel, LV_ALIGN_BOTTOM_RIGHT);

    lv_obj_clear_flag(ui_S1_Content_Panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_S1_Content_Panel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_S1_Content_Panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_S1_Content_Panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_S1_Content_Panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_S1_Content_Panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_S1_Content_Panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_S1_Content_Panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_S1_Content_Panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_S1_Content_Panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Slider_Print_View

    ui_Slider_Print_View = lv_slider_create(ui_S1_Content_Panel);
    lv_slider_set_range(ui_Slider_Print_View, 0, 100);
    lv_slider_set_value(ui_Slider_Print_View, 70, LV_ANIM_OFF);
    if(lv_slider_get_mode(ui_Slider_Print_View) == LV_SLIDER_MODE_RANGE) lv_slider_set_left_value(ui_Slider_Print_View, 0,
                                                                                                      LV_ANIM_OFF);

    lv_obj_set_width(ui_Slider_Print_View, 332);
    lv_obj_set_height(ui_Slider_Print_View, 396);

    lv_obj_set_x(ui_Slider_Print_View, 0);
    lv_obj_set_y(ui_Slider_Print_View, -10);

    lv_obj_set_align(ui_Slider_Print_View, LV_ALIGN_LEFT_MID);

    lv_obj_add_event_cb(ui_Slider_Print_View, ui_event_Slider_Print_View, LV_EVENT_ALL, NULL);
    lv_obj_set_style_radius(ui_Slider_Print_View, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Slider_Print_View, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider_Print_View, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Slider_Print_View, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Slider_Print_View, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_Slider_Print_View, &ui_img_print_view_bg_png, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(ui_Slider_Print_View, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Slider_Print_View, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider_Print_View, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Slider_Print_View, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Slider_Print_View, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_Slider_Print_View, &ui_img_print_view_front_png, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_Slider_Print_View, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider_Print_View, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    // ui_Number_Print

    ui_Number_Print = lv_label_create(ui_Slider_Print_View);

    lv_obj_set_width(ui_Number_Print, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Number_Print, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Number_Print, 0);
    lv_obj_set_y(ui_Number_Print, -40);

    lv_obj_set_align(ui_Number_Print, LV_ALIGN_BOTTOM_MID);

    lv_label_set_text(ui_Number_Print, "70%");

    lv_obj_set_style_text_color(ui_Number_Print, lv_color_hex(0x00D2FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Number_Print, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Number_Print, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Display_Time_S1

    ui_Display_Time_S1 = lv_obj_create(ui_S1_Content_Panel);

    lv_obj_set_width(ui_Display_Time_S1, 350);
    lv_obj_set_height(ui_Display_Time_S1, 79);

    lv_obj_set_x(ui_Display_Time_S1, -26);
    lv_obj_set_y(ui_Display_Time_S1, 30);

    lv_obj_set_align(ui_Display_Time_S1, LV_ALIGN_TOP_RIGHT);

    lv_obj_clear_flag(ui_Display_Time_S1, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Display_Time_S1, lv_color_hex(0x191D26), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Display_Time_S1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Display_Time_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Display_Time_S1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Display_Time_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_Display_Time_S1, lv_color_hex(0x414B62), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(ui_Display_Time_S1, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_Display_Time_S1, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_pad(ui_Display_Time_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Display_Time_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Display_Time_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Display_Time_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Display_Time_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_DT1

    ui_DT1 = lv_obj_create(ui_Display_Time_S1);

    lv_obj_set_width(ui_DT1, lv_pct(50));
    lv_obj_set_height(ui_DT1, lv_pct(100));

    lv_obj_set_x(ui_DT1, 0);
    lv_obj_set_y(ui_DT1, 0);

    lv_obj_set_align(ui_DT1, LV_ALIGN_LEFT_MID);

    lv_obj_clear_flag(ui_DT1, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_DT1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_DT1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_DT1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_DT1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_DT1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_DT1, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_DT1, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_DT1, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_DT1, 12, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_Printing_Time_1

    ui_Label_Printing_Time_1 = lv_label_create(ui_DT1);

    lv_obj_set_width(ui_Label_Printing_Time_1, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Printing_Time_1, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Printing_Time_1, 0);
    lv_obj_set_y(ui_Label_Printing_Time_1, 0);

    lv_obj_set_align(ui_Label_Printing_Time_1, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Label_Printing_Time_1, "Printing Time");
    lv_label_set_recolor(ui_Label_Printing_Time_1, "true");

    lv_obj_set_style_text_color(ui_Label_Printing_Time_1, lv_color_hex(0x9098AA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Printing_Time_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Printing_Time_1, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_IMG_Tine_1

    ui_IMG_Tine_1 = lv_img_create(ui_DT1);
    lv_img_set_src(ui_IMG_Tine_1, &ui_img_icn_time_1_png);

    lv_obj_set_width(ui_IMG_Tine_1, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_IMG_Tine_1, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_IMG_Tine_1, 20);
    lv_obj_set_y(ui_IMG_Tine_1, 10);

    lv_obj_set_align(ui_IMG_Tine_1, LV_ALIGN_LEFT_MID);

    lv_obj_add_flag(ui_IMG_Tine_1, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_Label_Time_1

    ui_Label_Time_1 = lv_label_create(ui_DT1);

    lv_obj_set_width(ui_Label_Time_1, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Time_1, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Time_1, 60);
    lv_obj_set_y(ui_Label_Time_1, 10);

    lv_obj_set_align(ui_Label_Time_1, LV_ALIGN_LEFT_MID);

    lv_label_set_text(ui_Label_Time_1, "10:52");

    lv_obj_set_style_text_color(ui_Label_Time_1, lv_color_hex(0xDBE6FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Time_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Time_1, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Panel3

    ui_Panel3 = lv_obj_create(ui_Display_Time_S1);

    lv_obj_set_width(ui_Panel3, 2);
    lv_obj_set_height(ui_Panel3, 50);

    lv_obj_set_x(ui_Panel3, 0);
    lv_obj_set_y(ui_Panel3, 0);

    lv_obj_set_align(ui_Panel3, LV_ALIGN_CENTER);

    lv_obj_set_style_bg_color(ui_Panel3, lv_color_hex(0x3A3F4B), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Panel3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Panel3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Panel3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Panel3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_DT2

    ui_DT2 = lv_obj_create(ui_Display_Time_S1);

    lv_obj_set_width(ui_DT2, lv_pct(50));
    lv_obj_set_height(ui_DT2, lv_pct(100));

    lv_obj_set_x(ui_DT2, 0);
    lv_obj_set_y(ui_DT2, 0);

    lv_obj_set_align(ui_DT2, LV_ALIGN_RIGHT_MID);

    lv_obj_clear_flag(ui_DT2, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_DT2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_DT2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_DT2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_DT2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_DT2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_DT2, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_DT2, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_DT2, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_DT2, 12, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_Printing_Time_2

    ui_Label_Printing_Time_2 = lv_label_create(ui_DT2);

    lv_obj_set_width(ui_Label_Printing_Time_2, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Printing_Time_2, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Printing_Time_2, 0);
    lv_obj_set_y(ui_Label_Printing_Time_2, 0);

    lv_obj_set_align(ui_Label_Printing_Time_2, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Label_Printing_Time_2, "Printing Time");
    lv_label_set_recolor(ui_Label_Printing_Time_2, "true");

    lv_obj_set_style_text_color(ui_Label_Printing_Time_2, lv_color_hex(0x9098AA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Printing_Time_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Printing_Time_2, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_IMG_Tine_2

    ui_IMG_Tine_2 = lv_img_create(ui_DT2);
    lv_img_set_src(ui_IMG_Tine_2, &ui_img_icn_time_2_png);

    lv_obj_set_width(ui_IMG_Tine_2, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_IMG_Tine_2, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_IMG_Tine_2, 20);
    lv_obj_set_y(ui_IMG_Tine_2, 10);

    lv_obj_set_align(ui_IMG_Tine_2, LV_ALIGN_LEFT_MID);

    lv_obj_add_flag(ui_IMG_Tine_2, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_Label_Time_2

    ui_Label_Time_2 = lv_label_create(ui_DT2);

    lv_obj_set_width(ui_Label_Time_2, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Time_2, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Time_2, 60);
    lv_obj_set_y(ui_Label_Time_2, 10);

    lv_obj_set_align(ui_Label_Time_2, LV_ALIGN_LEFT_MID);

    lv_label_set_text(ui_Label_Time_2, "08:25");

    lv_obj_set_style_text_color(ui_Label_Time_2, lv_color_hex(0xDBE6FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Time_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Time_2, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Display_Heat_S1

    ui_Display_Heat_S1 = lv_obj_create(ui_S1_Content_Panel);

    lv_obj_set_width(ui_Display_Heat_S1, 350);
    lv_obj_set_height(ui_Display_Heat_S1, 79);

    lv_obj_set_x(ui_Display_Heat_S1, -26);
    lv_obj_set_y(ui_Display_Heat_S1, 130);

    lv_obj_set_align(ui_Display_Heat_S1, LV_ALIGN_TOP_RIGHT);

    lv_obj_clear_flag(ui_Display_Heat_S1, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Display_Heat_S1, lv_color_hex(0x191D26), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Display_Heat_S1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Display_Heat_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Display_Heat_S1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Display_Heat_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_Display_Heat_S1, lv_color_hex(0x414B62), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(ui_Display_Heat_S1, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_Display_Heat_S1, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_pad(ui_Display_Heat_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Display_Heat_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Display_Heat_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Display_Heat_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Display_Heat_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_DT3

    ui_DT3 = lv_obj_create(ui_Display_Heat_S1);

    lv_obj_set_width(ui_DT3, lv_pct(50));
    lv_obj_set_height(ui_DT3, lv_pct(100));

    lv_obj_set_x(ui_DT3, 0);
    lv_obj_set_y(ui_DT3, 0);

    lv_obj_set_align(ui_DT3, LV_ALIGN_LEFT_MID);

    lv_obj_clear_flag(ui_DT3, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_DT3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_DT3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_DT3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_DT3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_DT3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_DT3, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_DT3, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_DT3, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_DT3, 12, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_Printing_Head_Temp

    ui_Label_Printing_Head_Temp = lv_label_create(ui_DT3);

    lv_obj_set_width(ui_Label_Printing_Head_Temp, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Printing_Head_Temp, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Printing_Head_Temp, 0);
    lv_obj_set_y(ui_Label_Printing_Head_Temp, 0);

    lv_obj_set_align(ui_Label_Printing_Head_Temp, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Label_Printing_Head_Temp, "Head Temp.");
    lv_label_set_recolor(ui_Label_Printing_Head_Temp, "true");

    lv_obj_set_style_text_color(ui_Label_Printing_Head_Temp, lv_color_hex(0x9098AA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Printing_Head_Temp, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Printing_Head_Temp, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_IMG_Head

    ui_IMG_Head = lv_img_create(ui_DT3);
    lv_img_set_src(ui_IMG_Head, &ui_img_icn_head_png);

    lv_obj_set_width(ui_IMG_Head, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_IMG_Head, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_IMG_Head, 15);
    lv_obj_set_y(ui_IMG_Head, 10);

    lv_obj_set_align(ui_IMG_Head, LV_ALIGN_LEFT_MID);

    lv_obj_add_flag(ui_IMG_Head, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_Label_Head_Temp

    ui_Label_Head_Temp = lv_label_create(ui_DT3);

    lv_obj_set_width(ui_Label_Head_Temp, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Head_Temp, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Head_Temp, 40);
    lv_obj_set_y(ui_Label_Head_Temp, 10);

    lv_obj_set_align(ui_Label_Head_Temp, LV_ALIGN_LEFT_MID);

    lv_label_set_text(ui_Label_Head_Temp, "195°");

    lv_obj_set_style_text_color(ui_Label_Head_Temp, lv_color_hex(0xDBE6FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Head_Temp, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Head_Temp, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_Printing_Head_Temp_2

    ui_Label_Printing_Head_Temp_2 = lv_label_create(ui_DT3);

    lv_obj_set_width(ui_Label_Printing_Head_Temp_2, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Printing_Head_Temp_2, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Printing_Head_Temp_2, 95);
    lv_obj_set_y(ui_Label_Printing_Head_Temp_2, 13);

    lv_obj_set_align(ui_Label_Printing_Head_Temp_2, LV_ALIGN_LEFT_MID);

    lv_label_set_text(ui_Label_Printing_Head_Temp_2, "/ 195°");
    lv_label_set_recolor(ui_Label_Printing_Head_Temp_2, "true");

    lv_obj_set_style_text_color(ui_Label_Printing_Head_Temp_2, lv_color_hex(0x9098AA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Printing_Head_Temp_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Printing_Head_Temp_2, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Panel1

    ui_Panel1 = lv_obj_create(ui_Display_Heat_S1);

    lv_obj_set_width(ui_Panel1, 2);
    lv_obj_set_height(ui_Panel1, 50);

    lv_obj_set_x(ui_Panel1, 0);
    lv_obj_set_y(ui_Panel1, 0);

    lv_obj_set_align(ui_Panel1, LV_ALIGN_CENTER);

    lv_obj_set_style_bg_color(ui_Panel1, lv_color_hex(0x3A3F4B), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Panel1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Panel1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Panel1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Panel1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_DT3_copy

    ui_DT3_copy = lv_obj_create(ui_Display_Heat_S1);

    lv_obj_set_width(ui_DT3_copy, lv_pct(50));
    lv_obj_set_height(ui_DT3_copy, lv_pct(100));

    lv_obj_set_x(ui_DT3_copy, 0);
    lv_obj_set_y(ui_DT3_copy, 0);

    lv_obj_set_align(ui_DT3_copy, LV_ALIGN_RIGHT_MID);

    lv_obj_clear_flag(ui_DT3_copy, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_DT3_copy, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_DT3_copy, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_DT3_copy, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_DT3_copy, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_DT3_copy, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_DT3_copy, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_DT3_copy, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_DT3_copy, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_DT3_copy, 12, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_Printing_Bed_Temp

    ui_Label_Printing_Bed_Temp = lv_label_create(ui_DT3_copy);

    lv_obj_set_width(ui_Label_Printing_Bed_Temp, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Printing_Bed_Temp, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Printing_Bed_Temp, 0);
    lv_obj_set_y(ui_Label_Printing_Bed_Temp, 0);

    lv_obj_set_align(ui_Label_Printing_Bed_Temp, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Label_Printing_Bed_Temp, "Bed Temp.");
    lv_label_set_recolor(ui_Label_Printing_Bed_Temp, "true");

    lv_obj_set_style_text_color(ui_Label_Printing_Bed_Temp, lv_color_hex(0x9098AA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Printing_Bed_Temp, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Printing_Bed_Temp, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_IMG_Head1

    ui_IMG_Head1 = lv_img_create(ui_DT3_copy);
    lv_img_set_src(ui_IMG_Head1, &ui_img_icn_bed_png);

    lv_obj_set_width(ui_IMG_Head1, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_IMG_Head1, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_IMG_Head1, 20);
    lv_obj_set_y(ui_IMG_Head1, 10);

    lv_obj_set_align(ui_IMG_Head1, LV_ALIGN_LEFT_MID);

    lv_obj_add_flag(ui_IMG_Head1, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_Label_Bed_Temp

    ui_Label_Bed_Temp = lv_label_create(ui_DT3_copy);

    lv_obj_set_width(ui_Label_Bed_Temp, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Bed_Temp, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Bed_Temp, 55);
    lv_obj_set_y(ui_Label_Bed_Temp, 10);

    lv_obj_set_align(ui_Label_Bed_Temp, LV_ALIGN_LEFT_MID);

    lv_label_set_text(ui_Label_Bed_Temp, "65°");

    lv_obj_set_style_text_color(ui_Label_Bed_Temp, lv_color_hex(0xDBE6FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Bed_Temp, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Bed_Temp, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_Printing_Bed_Temp1

    ui_Label_Printing_Bed_Temp1 = lv_label_create(ui_DT3_copy);

    lv_obj_set_width(ui_Label_Printing_Bed_Temp1, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Printing_Bed_Temp1, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Printing_Bed_Temp1, 100);
    lv_obj_set_y(ui_Label_Printing_Bed_Temp1, 13);

    lv_obj_set_align(ui_Label_Printing_Bed_Temp1, LV_ALIGN_LEFT_MID);

    lv_label_set_text(ui_Label_Printing_Bed_Temp1, "/ 65°");
    lv_label_set_recolor(ui_Label_Printing_Bed_Temp1, "true");

    lv_obj_set_style_text_color(ui_Label_Printing_Bed_Temp1, lv_color_hex(0x9098AA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Printing_Bed_Temp1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Printing_Bed_Temp1, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Panel_Buttons_S1

    ui_Panel_Buttons_S1 = lv_obj_create(ui_S1_Content_Panel);

    lv_obj_set_width(ui_Panel_Buttons_S1, 380);
    lv_obj_set_height(ui_Panel_Buttons_S1, 200);

    lv_obj_set_x(ui_Panel_Buttons_S1, 0);
    lv_obj_set_y(ui_Panel_Buttons_S1, 0);

    lv_obj_set_align(ui_Panel_Buttons_S1, LV_ALIGN_BOTTOM_RIGHT);

    lv_obj_clear_flag(ui_Panel_Buttons_S1, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Panel_Buttons_S1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Panel_Buttons_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Panel_Buttons_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Panel_Buttons_S1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Panel_Buttons_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Panel_Buttons_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Panel_Buttons_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Panel_Buttons_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Panel_Buttons_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_Pause

    ui_BTN_Pause = lv_img_create(ui_Panel_Buttons_S1);
    lv_img_set_src(ui_BTN_Pause, &ui_img_btn_print_down_png);

    lv_obj_set_width(ui_BTN_Pause, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_BTN_Pause, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_BTN_Pause, 10);
    lv_obj_set_y(ui_BTN_Pause, 0);

    lv_obj_set_align(ui_BTN_Pause, LV_ALIGN_LEFT_MID);

    lv_obj_add_flag(ui_BTN_Pause, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_BTN_Pause_Top

    ui_BTN_Pause_Top = lv_img_create(ui_BTN_Pause);
    lv_img_set_src(ui_BTN_Pause_Top, &ui_img_btn_print_top_off_png);

    lv_obj_set_width(ui_BTN_Pause_Top, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_BTN_Pause_Top, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_BTN_Pause_Top, 0);
    lv_obj_set_y(ui_BTN_Pause_Top, 0);

    lv_obj_set_align(ui_BTN_Pause_Top, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_BTN_Pause_Top, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_ADV_HITTEST);

    lv_obj_set_style_radius(ui_BTN_Pause_Top, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_BTN_Pause_Top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_BTN_Pause_Top, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_BTN_Pause_Top, lv_color_hex(0x00D2FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_opa(ui_BTN_Pause_Top, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(ui_BTN_Pause_Top, 9, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_spread(ui_BTN_Pause_Top, 3, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_ofs_x(ui_BTN_Pause_Top, 0, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_ofs_y(ui_BTN_Pause_Top, 0, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_color(ui_BTN_Pause_Top, lv_color_hex(0x00D2FF), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_opa(ui_BTN_Pause_Top, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(ui_BTN_Pause_Top, 5, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_spread(ui_BTN_Pause_Top, 5, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_ofs_x(ui_BTN_Pause_Top, 0, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_ofs_y(ui_BTN_Pause_Top, 0, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_img_recolor(ui_BTN_Pause_Top, lv_color_hex(0x67799B), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_img_recolor_opa(ui_BTN_Pause_Top, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    // ui_Image_Pause

    ui_Image_Pause = lv_img_create(ui_BTN_Pause_Top);
    lv_img_set_src(ui_Image_Pause, &ui_img_icn_pause_png);

    lv_obj_set_width(ui_Image_Pause, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Image_Pause, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Image_Pause, 0);
    lv_obj_set_y(ui_Image_Pause, 0);

    lv_obj_set_align(ui_Image_Pause, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_Image_Pause, LV_OBJ_FLAG_ADV_HITTEST);

    lv_obj_set_style_img_recolor(ui_Image_Pause, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui_Image_Pause, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_Cancel

    ui_BTN_Cancel = lv_img_create(ui_Panel_Buttons_S1);
    lv_img_set_src(ui_BTN_Cancel, &ui_img_btn_print_down_png);

    lv_obj_set_width(ui_BTN_Cancel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_BTN_Cancel, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_BTN_Cancel, -30);
    lv_obj_set_y(ui_BTN_Cancel, 0);

    lv_obj_set_align(ui_BTN_Cancel, LV_ALIGN_RIGHT_MID);

    lv_obj_add_flag(ui_BTN_Cancel, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_BTN_Cancel_Top

    ui_BTN_Cancel_Top = lv_img_create(ui_BTN_Cancel);
    lv_img_set_src(ui_BTN_Cancel_Top, &ui_img_btn_print_top_off_png);

    lv_obj_set_width(ui_BTN_Cancel_Top, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_BTN_Cancel_Top, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_BTN_Cancel_Top, 0);
    lv_obj_set_y(ui_BTN_Cancel_Top, 0);

    lv_obj_set_align(ui_BTN_Cancel_Top, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_BTN_Cancel_Top, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST);

    lv_obj_add_event_cb(ui_BTN_Cancel_Top, ui_event_BTN_Cancel_Top, LV_EVENT_ALL, NULL);
    lv_obj_set_style_radius(ui_BTN_Cancel_Top, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_BTN_Cancel_Top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_BTN_Cancel_Top, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_BTN_Cancel_Top, lv_color_hex(0x00D2FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_opa(ui_BTN_Cancel_Top, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(ui_BTN_Cancel_Top, 9, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_spread(ui_BTN_Cancel_Top, 3, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_ofs_x(ui_BTN_Cancel_Top, 0, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_ofs_y(ui_BTN_Cancel_Top, 0, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_color(ui_BTN_Cancel_Top, lv_color_hex(0x00D2FF), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_opa(ui_BTN_Cancel_Top, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(ui_BTN_Cancel_Top, 5, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_spread(ui_BTN_Cancel_Top, 5, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_img_recolor(ui_BTN_Cancel_Top, lv_color_hex(0x67799B), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_img_recolor_opa(ui_BTN_Cancel_Top, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    // ui_Image_Cancel

    ui_Image_Cancel = lv_img_create(ui_BTN_Cancel_Top);
    lv_img_set_src(ui_Image_Cancel, &ui_img_icn_stop_png);

    lv_obj_set_width(ui_Image_Cancel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Image_Cancel, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Image_Cancel, 0);
    lv_obj_set_y(ui_Image_Cancel, 0);

    lv_obj_set_align(ui_Image_Cancel, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_Image_Cancel, LV_OBJ_FLAG_ADV_HITTEST);

    lv_obj_set_style_img_recolor(ui_Image_Cancel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui_Image_Cancel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

}
void ui_Screen_2_Move_screen_init(void)
{

    // ui_Screen_2_Move

    ui_Screen_2_Move = lv_obj_create(NULL);

    lv_obj_set_style_bg_color(ui_Screen_2_Move, lv_color_hex(0x191D26), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Screen_2_Move, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Background1

    ui_Background1 = lv_img_create(ui_Screen_2_Move);
    lv_img_set_src(ui_Background1, &ui_img_3d_printer_bg_png);

    lv_obj_set_width(ui_Background1, 800);
    lv_obj_set_height(ui_Background1, 480);

    lv_obj_set_x(ui_Background1, 0);
    lv_obj_set_y(ui_Background1, 0);

    lv_obj_set_align(ui_Background1, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_Background1, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_Panel_Header1

    ui_Panel_Header1 = lv_obj_create(ui_Screen_2_Move);

    lv_obj_set_width(ui_Panel_Header1, 665);
    lv_obj_set_height(ui_Panel_Header1, 39);

    lv_obj_set_x(ui_Panel_Header1, -30);
    lv_obj_set_y(ui_Panel_Header1, 0);

    lv_obj_set_align(ui_Panel_Header1, LV_ALIGN_TOP_RIGHT);

    lv_obj_clear_flag(ui_Panel_Header1, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Panel_Header1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Panel_Header1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Panel_Header1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Panel_Header1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Panel_Header1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_Header1

    ui_Label_Header1 = lv_label_create(ui_Panel_Header1);

    lv_obj_set_width(ui_Label_Header1, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Header1, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Header1, 0);
    lv_obj_set_y(ui_Label_Header1, 0);

    lv_obj_set_align(ui_Label_Header1, LV_ALIGN_LEFT_MID);

    lv_label_set_text(ui_Label_Header1, "Press the control buttons to move the tray and head");
    lv_label_set_recolor(ui_Label_Header1, "true");

    lv_obj_set_style_text_color(ui_Label_Header1, lv_color_hex(0x9098AA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Header1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Header1, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_IMG_Wifi1

    ui_IMG_Wifi1 = lv_img_create(ui_Panel_Header1);
    lv_img_set_src(ui_IMG_Wifi1, &ui_img_icn_wifi_png);

    lv_obj_set_width(ui_IMG_Wifi1, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_IMG_Wifi1, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_IMG_Wifi1, 0);
    lv_obj_set_y(ui_IMG_Wifi1, 0);

    lv_obj_set_align(ui_IMG_Wifi1, LV_ALIGN_RIGHT_MID);

    lv_obj_add_flag(ui_IMG_Wifi1, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_IMG_PC1

    ui_IMG_PC1 = lv_img_create(ui_Panel_Header1);
    lv_img_set_src(ui_IMG_PC1, &ui_img_icn_pc_png);

    lv_obj_set_width(ui_IMG_PC1, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_IMG_PC1, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_IMG_PC1, -42);
    lv_obj_set_y(ui_IMG_PC1, 0);

    lv_obj_set_align(ui_IMG_PC1, LV_ALIGN_RIGHT_MID);

    lv_obj_add_flag(ui_IMG_PC1, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_IMG_USB1

    ui_IMG_USB1 = lv_img_create(ui_Panel_Header1);
    lv_img_set_src(ui_IMG_USB1, &ui_img_icn_usb_png);

    lv_obj_set_width(ui_IMG_USB1, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_IMG_USB1, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_IMG_USB1, -84);
    lv_obj_set_y(ui_IMG_USB1, 0);

    lv_obj_set_align(ui_IMG_USB1, LV_ALIGN_RIGHT_MID);

    lv_obj_add_flag(ui_IMG_USB1, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_BTN_Menu_Print_S2

    ui_BTN_Menu_Print_S2 = lv_img_create(ui_Screen_2_Move);
    lv_img_set_src(ui_BTN_Menu_Print_S2, &ui_img_btn_print_png);

    lv_obj_set_width(ui_BTN_Menu_Print_S2, 79);
    lv_obj_set_height(ui_BTN_Menu_Print_S2, 160);

    lv_obj_set_x(ui_BTN_Menu_Print_S2, 0);
    lv_obj_set_y(ui_BTN_Menu_Print_S2, 0);

    lv_obj_add_flag(ui_BTN_Menu_Print_S2, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(ui_BTN_Menu_Print_S2, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_event_cb(ui_BTN_Menu_Print_S2, ui_event_BTN_Menu_Print_S2, LV_EVENT_ALL, NULL);
    lv_obj_set_style_opa(ui_BTN_Menu_Print_S2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_Menu_Move_S2

    ui_BTN_Menu_Move_S2 = lv_img_create(ui_Screen_2_Move);
    lv_img_set_src(ui_BTN_Menu_Move_S2, &ui_img_btn_move_png);

    lv_obj_set_width(ui_BTN_Menu_Move_S2, 79);
    lv_obj_set_height(ui_BTN_Menu_Move_S2, 162);

    lv_obj_set_x(ui_BTN_Menu_Move_S2, 0);
    lv_obj_set_y(ui_BTN_Menu_Move_S2, 0);

    lv_obj_set_align(ui_BTN_Menu_Move_S2, LV_ALIGN_LEFT_MID);

    lv_obj_add_flag(ui_BTN_Menu_Move_S2, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(ui_BTN_Menu_Move_S2, LV_OBJ_FLAG_SCROLLABLE);

    // ui_BTN_Menu_Setting_S2

    ui_BTN_Menu_Setting_S2 = lv_img_create(ui_Screen_2_Move);
    lv_img_set_src(ui_BTN_Menu_Setting_S2, &ui_img_btn_setting_png);

    lv_obj_set_width(ui_BTN_Menu_Setting_S2, 79);
    lv_obj_set_height(ui_BTN_Menu_Setting_S2, 160);

    lv_obj_set_x(ui_BTN_Menu_Setting_S2, 0);
    lv_obj_set_y(ui_BTN_Menu_Setting_S2, 0);

    lv_obj_set_align(ui_BTN_Menu_Setting_S2, LV_ALIGN_BOTTOM_LEFT);

    lv_obj_add_flag(ui_BTN_Menu_Setting_S2, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(ui_BTN_Menu_Setting_S2, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_event_cb(ui_BTN_Menu_Setting_S2, ui_event_BTN_Menu_Setting_S2, LV_EVENT_ALL, NULL);
    lv_obj_set_style_opa(ui_BTN_Menu_Setting_S2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_S1_Content_Panel1

    ui_S1_Content_Panel1 = lv_obj_create(ui_Screen_2_Move);

    lv_obj_set_width(ui_S1_Content_Panel1, 720);
    lv_obj_set_height(ui_S1_Content_Panel1, 430);

    lv_obj_set_x(ui_S1_Content_Panel1, 0);
    lv_obj_set_y(ui_S1_Content_Panel1, 0);

    lv_obj_set_align(ui_S1_Content_Panel1, LV_ALIGN_BOTTOM_RIGHT);

    lv_obj_clear_flag(ui_S1_Content_Panel1, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_S1_Content_Panel1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_S1_Content_Panel1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_S1_Content_Panel1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_S1_Content_Panel1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_S1_Content_Panel1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_S1_Content_Panel1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_S1_Content_Panel1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_S1_Content_Panel1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_S1_Content_Panel1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Display_Pos_XY

    ui_Display_Pos_XY = lv_obj_create(ui_S1_Content_Panel1);

    lv_obj_set_width(ui_Display_Pos_XY, 283);
    lv_obj_set_height(ui_Display_Pos_XY, 79);

    lv_obj_set_x(ui_Display_Pos_XY, 200);
    lv_obj_set_y(ui_Display_Pos_XY, 21);

    lv_obj_clear_flag(ui_Display_Pos_XY, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Display_Pos_XY, lv_color_hex(0x191D26), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Display_Pos_XY, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Display_Pos_XY, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Display_Pos_XY, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Display_Pos_XY, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_Display_Pos_XY, lv_color_hex(0x414B62), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(ui_Display_Pos_XY, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_Display_Pos_XY, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_pad(ui_Display_Pos_XY, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Display_Pos_XY, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Display_Pos_XY, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Display_Pos_XY, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Display_Pos_XY, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_DP1

    ui_DP1 = lv_obj_create(ui_Display_Pos_XY);

    lv_obj_set_width(ui_DP1, lv_pct(50));
    lv_obj_set_height(ui_DP1, lv_pct(100));

    lv_obj_set_x(ui_DP1, 0);
    lv_obj_set_y(ui_DP1, 0);

    lv_obj_set_align(ui_DP1, LV_ALIGN_LEFT_MID);

    lv_obj_clear_flag(ui_DP1, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_DP1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_DP1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_DP1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_DP1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_DP1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_DP1, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_DP1, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_DP1, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_DP1, 12, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_X_Position

    ui_Label_X_Position = lv_label_create(ui_DP1);

    lv_obj_set_width(ui_Label_X_Position, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_X_Position, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_X_Position, 0);
    lv_obj_set_y(ui_Label_X_Position, 0);

    lv_obj_set_align(ui_Label_X_Position, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Label_X_Position, "X Position");
    lv_label_set_recolor(ui_Label_X_Position, "true");

    lv_obj_set_style_text_color(ui_Label_X_Position, lv_color_hex(0x9098AA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_X_Position, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_X_Position, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_X_Position_Number

    ui_Label_X_Position_Number = lv_label_create(ui_DP1);

    lv_obj_set_width(ui_Label_X_Position_Number, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_X_Position_Number, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_X_Position_Number, 0);
    lv_obj_set_y(ui_Label_X_Position_Number, 10);

    lv_obj_set_align(ui_Label_X_Position_Number, LV_ALIGN_CENTER);

    lv_label_set_text(ui_Label_X_Position_Number, "185");

    lv_obj_set_style_text_color(ui_Label_X_Position_Number, lv_color_hex(0xDBE6FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_X_Position_Number, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_X_Position_Number, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Panel2

    ui_Panel2 = lv_obj_create(ui_Display_Pos_XY);

    lv_obj_set_width(ui_Panel2, 2);
    lv_obj_set_height(ui_Panel2, 50);

    lv_obj_set_x(ui_Panel2, 0);
    lv_obj_set_y(ui_Panel2, 0);

    lv_obj_set_align(ui_Panel2, LV_ALIGN_CENTER);

    lv_obj_set_style_bg_color(ui_Panel2, lv_color_hex(0x3A3F4B), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Panel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Panel2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Panel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Panel2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_DP2

    ui_DP2 = lv_obj_create(ui_Display_Pos_XY);

    lv_obj_set_width(ui_DP2, lv_pct(50));
    lv_obj_set_height(ui_DP2, lv_pct(100));

    lv_obj_set_x(ui_DP2, 0);
    lv_obj_set_y(ui_DP2, 0);

    lv_obj_set_align(ui_DP2, LV_ALIGN_RIGHT_MID);

    lv_obj_clear_flag(ui_DP2, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_DP2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_DP2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_DP2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_DP2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_DP2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_DP2, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_DP2, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_DP2, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_DP2, 12, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_Y_Position

    ui_Label_Y_Position = lv_label_create(ui_DP2);

    lv_obj_set_width(ui_Label_Y_Position, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Y_Position, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Y_Position, 0);
    lv_obj_set_y(ui_Label_Y_Position, 0);

    lv_obj_set_align(ui_Label_Y_Position, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Label_Y_Position, "Y Position");
    lv_label_set_recolor(ui_Label_Y_Position, "true");

    lv_obj_set_style_text_color(ui_Label_Y_Position, lv_color_hex(0x9098AA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Y_Position, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Y_Position, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_Time_3

    ui_Label_Time_3 = lv_label_create(ui_DP2);

    lv_obj_set_width(ui_Label_Time_3, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Time_3, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Time_3, 0);
    lv_obj_set_y(ui_Label_Time_3, 10);

    lv_obj_set_align(ui_Label_Time_3, LV_ALIGN_CENTER);

    lv_label_set_text(ui_Label_Time_3, "350");

    lv_obj_set_style_text_color(ui_Label_Time_3, lv_color_hex(0xDBE6FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Time_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Time_3, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Display_Pos_y

    ui_Display_Pos_y = lv_obj_create(ui_S1_Content_Panel1);

    lv_obj_set_width(ui_Display_Pos_y, 133);
    lv_obj_set_height(ui_Display_Pos_y, 79);

    lv_obj_set_x(ui_Display_Pos_y, 20);
    lv_obj_set_y(ui_Display_Pos_y, 21);

    lv_obj_clear_flag(ui_Display_Pos_y, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Display_Pos_y, lv_color_hex(0x191D26), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Display_Pos_y, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Display_Pos_y, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Display_Pos_y, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Display_Pos_y, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_Display_Pos_y, lv_color_hex(0x414B62), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(ui_Display_Pos_y, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_Display_Pos_y, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_pad(ui_Display_Pos_y, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Display_Pos_y, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Display_Pos_y, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Display_Pos_y, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Display_Pos_y, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_DP3

    ui_DP3 = lv_obj_create(ui_Display_Pos_y);

    lv_obj_set_width(ui_DP3, lv_pct(100));
    lv_obj_set_height(ui_DP3, lv_pct(100));

    lv_obj_set_x(ui_DP3, 0);
    lv_obj_set_y(ui_DP3, 0);

    lv_obj_set_align(ui_DP3, LV_ALIGN_RIGHT_MID);

    lv_obj_clear_flag(ui_DP3, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_DP3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_DP3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_DP3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_DP3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_DP3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_DP3, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_DP3, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_DP3, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_DP3, 12, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_Z_Position

    ui_Label_Z_Position = lv_label_create(ui_DP3);

    lv_obj_set_width(ui_Label_Z_Position, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Z_Position, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Z_Position, 0);
    lv_obj_set_y(ui_Label_Z_Position, 0);

    lv_obj_set_align(ui_Label_Z_Position, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Label_Z_Position, "Z Position");
    lv_label_set_recolor(ui_Label_Z_Position, "true");

    lv_obj_set_style_text_color(ui_Label_Z_Position, lv_color_hex(0x9098AA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Z_Position, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Z_Position, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_Z_Position_Number

    ui_Label_Z_Position_Number = lv_label_create(ui_DP3);

    lv_obj_set_width(ui_Label_Z_Position_Number, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Z_Position_Number, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Z_Position_Number, 0);
    lv_obj_set_y(ui_Label_Z_Position_Number, 10);

    lv_obj_set_align(ui_Label_Z_Position_Number, LV_ALIGN_CENTER);

    lv_label_set_text(ui_Label_Z_Position_Number, "350");

    lv_obj_set_style_text_color(ui_Label_Z_Position_Number, lv_color_hex(0xDBE6FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Z_Position_Number, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Z_Position_Number, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_Move_Z

    ui_BTN_Move_Z = lv_img_create(ui_S1_Content_Panel1);
    lv_img_set_src(ui_BTN_Move_Z, &ui_img_btn_z_botton_png);

    lv_obj_set_width(ui_BTN_Move_Z, 140);
    lv_obj_set_height(ui_BTN_Move_Z, 312);

    lv_obj_set_x(ui_BTN_Move_Z, -273);
    lv_obj_set_y(ui_BTN_Move_Z, 45);

    lv_obj_set_align(ui_BTN_Move_Z, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_BTN_Move_Z, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_GLOW_1

    ui_GLOW_1 = lv_obj_create(ui_BTN_Move_Z);

    lv_obj_set_width(ui_GLOW_1, 75);
    lv_obj_set_height(ui_GLOW_1, 70);

    lv_obj_set_x(ui_GLOW_1, 0);
    lv_obj_set_y(ui_GLOW_1, 34);

    lv_obj_set_align(ui_GLOW_1, LV_ALIGN_TOP_MID);

    lv_obj_add_event_cb(ui_GLOW_1, ui_event_GLOW_1, LV_EVENT_ALL, NULL);
    lv_obj_set_style_radius(ui_GLOW_1, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_GLOW_1, lv_color_hex(0x00D2FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_GLOW_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_GLOW_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_GLOW_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_GLOW_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_GLOW_1, lv_color_hex(0x60BEFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_GLOW_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_GLOW_1, 40, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_GLOW_1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_opa(ui_GLOW_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_GLOW_2

    ui_GLOW_2 = lv_obj_create(ui_BTN_Move_Z);

    lv_obj_set_width(ui_GLOW_2, 75);
    lv_obj_set_height(ui_GLOW_2, 70);

    lv_obj_set_x(ui_GLOW_2, 0);
    lv_obj_set_y(ui_GLOW_2, 211);

    lv_obj_set_align(ui_GLOW_2, LV_ALIGN_TOP_MID);

    lv_obj_add_event_cb(ui_GLOW_2, ui_event_GLOW_2, LV_EVENT_ALL, NULL);
    lv_obj_set_style_radius(ui_GLOW_2, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_GLOW_2, lv_color_hex(0x00D2FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_GLOW_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_GLOW_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_GLOW_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_GLOW_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_GLOW_2, lv_color_hex(0x60BEFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_GLOW_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_GLOW_2, 40, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_GLOW_2, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_opa(ui_GLOW_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_Image_Top

    ui_BTN_Image_Top = lv_img_create(ui_BTN_Move_Z);
    lv_img_set_src(ui_BTN_Image_Top, &ui_img_btn_z_top_png);

    lv_obj_set_width(ui_BTN_Image_Top, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_BTN_Image_Top, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_BTN_Image_Top, 0);
    lv_obj_set_y(ui_BTN_Image_Top, 0);

    lv_obj_set_align(ui_BTN_Image_Top, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_BTN_Image_Top, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_Label_Z_Position_2

    ui_Label_Z_Position_2 = lv_label_create(ui_BTN_Image_Top);

    lv_obj_set_width(ui_Label_Z_Position_2, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Z_Position_2, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Z_Position_2, 0);
    lv_obj_set_y(ui_Label_Z_Position_2, 0);

    lv_obj_set_align(ui_Label_Z_Position_2, LV_ALIGN_CENTER);

    lv_label_set_text(ui_Label_Z_Position_2, "MOVE\nZ");
    lv_label_set_recolor(ui_Label_Z_Position_2, "true");

    lv_obj_set_style_text_color(ui_Label_Z_Position_2, lv_color_hex(0xABC1ED), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Z_Position_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_Z_Position_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Z_Position_2, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Arrow_Z_up

    ui_Arrow_Z_up = lv_img_create(ui_BTN_Image_Top);
    lv_img_set_src(ui_Arrow_Z_up, &ui_img_arrow_up_png);

    lv_obj_set_width(ui_Arrow_Z_up, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Arrow_Z_up, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Arrow_Z_up, 0);
    lv_obj_set_y(ui_Arrow_Z_up, 20);

    lv_obj_set_align(ui_Arrow_Z_up, LV_ALIGN_TOP_MID);

    lv_obj_clear_flag(ui_Arrow_Z_up, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
                      LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM |
                      LV_OBJ_FLAG_SCROLL_CHAIN);

    lv_obj_set_style_img_recolor(ui_Arrow_Z_up, lv_color_hex(0xABC1ED), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui_Arrow_Z_up, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor(ui_Arrow_Z_up, lv_color_hex(0x00D1FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_img_recolor_opa(ui_Arrow_Z_up, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_img_recolor(ui_Arrow_Z_up, lv_color_hex(0x00D1FF), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_img_recolor_opa(ui_Arrow_Z_up, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    // ui_Arrow_Z_Down

    ui_Arrow_Z_Down = lv_img_create(ui_BTN_Image_Top);
    lv_img_set_src(ui_Arrow_Z_Down, &ui_img_arrow_down_png);

    lv_obj_set_width(ui_Arrow_Z_Down, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Arrow_Z_Down, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Arrow_Z_Down, 0);
    lv_obj_set_y(ui_Arrow_Z_Down, -20);

    lv_obj_set_align(ui_Arrow_Z_Down, LV_ALIGN_BOTTOM_MID);

    lv_obj_add_flag(ui_Arrow_Z_Down, LV_OBJ_FLAG_ADV_HITTEST);

    lv_obj_set_style_img_recolor(ui_Arrow_Z_Down, lv_color_hex(0xABC1ED), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui_Arrow_Z_Down, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor(ui_Arrow_Z_Down, lv_color_hex(0x00D2FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_img_recolor_opa(ui_Arrow_Z_Down, 255, LV_PART_MAIN | LV_STATE_CHECKED);

    // ui_BTN_Move_XY

    ui_BTN_Move_XY = lv_img_create(ui_S1_Content_Panel1);
    lv_img_set_src(ui_BTN_Move_XY, &ui_img_btn_pad_bottom_png);

    lv_obj_set_width(ui_BTN_Move_XY, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_BTN_Move_XY, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_BTN_Move_XY, -15);
    lv_obj_set_y(ui_BTN_Move_XY, 40);

    lv_obj_set_align(ui_BTN_Move_XY, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_BTN_Move_XY, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_GLOW_3

    ui_GLOW_3 = lv_obj_create(ui_BTN_Move_XY);

    lv_obj_set_width(ui_GLOW_3, 120);
    lv_obj_set_height(ui_GLOW_3, 120);

    lv_obj_set_x(ui_GLOW_3, 64);
    lv_obj_set_y(ui_GLOW_3, 97);

    lv_obj_set_align(ui_GLOW_3, LV_ALIGN_TOP_MID);

    lv_obj_add_event_cb(ui_GLOW_3, ui_event_GLOW_3, LV_EVENT_ALL, NULL);
    lv_obj_set_style_radius(ui_GLOW_3, 60, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_GLOW_3, lv_color_hex(0x00D2FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_GLOW_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_GLOW_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_GLOW_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_GLOW_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_GLOW_3, lv_color_hex(0x60BEFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_GLOW_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_GLOW_3, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_GLOW_3, 14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_opa(ui_GLOW_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_GLOW_4

    ui_GLOW_4 = lv_obj_create(ui_BTN_Move_XY);

    lv_obj_set_width(ui_GLOW_4, 120);
    lv_obj_set_height(ui_GLOW_4, 120);

    lv_obj_set_x(ui_GLOW_4, 1);
    lv_obj_set_y(ui_GLOW_4, 33);

    lv_obj_set_align(ui_GLOW_4, LV_ALIGN_TOP_MID);

    lv_obj_add_event_cb(ui_GLOW_4, ui_event_GLOW_4, LV_EVENT_ALL, NULL);
    lv_obj_set_style_radius(ui_GLOW_4, 60, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_GLOW_4, lv_color_hex(0x00D2FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_GLOW_4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_GLOW_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_GLOW_4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_GLOW_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_GLOW_4, lv_color_hex(0x60BEFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_GLOW_4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_GLOW_4, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_GLOW_4, 14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_opa(ui_GLOW_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_GLOW_5

    ui_GLOW_5 = lv_obj_create(ui_BTN_Move_XY);

    lv_obj_set_width(ui_GLOW_5, 120);
    lv_obj_set_height(ui_GLOW_5, 120);

    lv_obj_set_x(ui_GLOW_5, 0);
    lv_obj_set_y(ui_GLOW_5, 158);

    lv_obj_set_align(ui_GLOW_5, LV_ALIGN_TOP_MID);

    lv_obj_add_event_cb(ui_GLOW_5, ui_event_GLOW_5, LV_EVENT_ALL, NULL);
    lv_obj_set_style_radius(ui_GLOW_5, 60, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_GLOW_5, lv_color_hex(0x00D2FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_GLOW_5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_GLOW_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_GLOW_5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_GLOW_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_GLOW_5, lv_color_hex(0x60BEFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_GLOW_5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_GLOW_5, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_GLOW_5, 14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_opa(ui_GLOW_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_GLOW_6

    ui_GLOW_6 = lv_obj_create(ui_BTN_Move_XY);

    lv_obj_set_width(ui_GLOW_6, 120);
    lv_obj_set_height(ui_GLOW_6, 120);

    lv_obj_set_x(ui_GLOW_6, -64);
    lv_obj_set_y(ui_GLOW_6, 96);

    lv_obj_set_align(ui_GLOW_6, LV_ALIGN_TOP_MID);

    lv_obj_add_event_cb(ui_GLOW_6, ui_event_GLOW_6, LV_EVENT_ALL, NULL);
    lv_obj_set_style_radius(ui_GLOW_6, 60, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_GLOW_6, lv_color_hex(0x00D2FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_GLOW_6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_GLOW_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_GLOW_6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_GLOW_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_GLOW_6, lv_color_hex(0x60BEFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_GLOW_6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_GLOW_6, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_GLOW_6, 14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_opa(ui_GLOW_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_Move_XY_Top

    ui_BTN_Move_XY_Top = lv_img_create(ui_BTN_Move_XY);
    lv_img_set_src(ui_BTN_Move_XY_Top, &ui_img_btn_pad_top_png);

    lv_obj_set_width(ui_BTN_Move_XY_Top, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_BTN_Move_XY_Top, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_BTN_Move_XY_Top, 0);
    lv_obj_set_y(ui_BTN_Move_XY_Top, 0);

    lv_obj_set_align(ui_BTN_Move_XY_Top, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_BTN_Move_XY_Top, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_Label_XY_Position

    ui_Label_XY_Position = lv_label_create(ui_BTN_Move_XY_Top);

    lv_obj_set_width(ui_Label_XY_Position, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_XY_Position, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_XY_Position, 0);
    lv_obj_set_y(ui_Label_XY_Position, 0);

    lv_obj_set_align(ui_Label_XY_Position, LV_ALIGN_CENTER);

    lv_label_set_text(ui_Label_XY_Position, "MOVE\nXY");
    lv_label_set_recolor(ui_Label_XY_Position, "true");

    lv_obj_set_style_text_color(ui_Label_XY_Position, lv_color_hex(0xABC1ED), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_XY_Position, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_XY_Position, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_XY_Position, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Arrow_Up_2

    ui_Arrow_Up_2 = lv_img_create(ui_BTN_Move_XY_Top);
    lv_img_set_src(ui_Arrow_Up_2, &ui_img_arrow_up_png);

    lv_obj_set_width(ui_Arrow_Up_2, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Arrow_Up_2, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Arrow_Up_2, 0);
    lv_obj_set_y(ui_Arrow_Up_2, 25);

    lv_obj_set_align(ui_Arrow_Up_2, LV_ALIGN_TOP_MID);

    lv_obj_add_flag(ui_Arrow_Up_2, LV_OBJ_FLAG_ADV_HITTEST);

    lv_obj_set_style_img_recolor(ui_Arrow_Up_2, lv_color_hex(0xABC1ED), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui_Arrow_Up_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor(ui_Arrow_Up_2, lv_color_hex(0x00D2FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_img_recolor_opa(ui_Arrow_Up_2, 255, LV_PART_MAIN | LV_STATE_CHECKED);

    // ui_Arrow_Down_2

    ui_Arrow_Down_2 = lv_img_create(ui_BTN_Move_XY_Top);
    lv_img_set_src(ui_Arrow_Down_2, &ui_img_arrow_down_png);

    lv_obj_set_width(ui_Arrow_Down_2, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Arrow_Down_2, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Arrow_Down_2, 0);
    lv_obj_set_y(ui_Arrow_Down_2, -25);

    lv_obj_set_align(ui_Arrow_Down_2, LV_ALIGN_BOTTOM_MID);

    lv_obj_add_flag(ui_Arrow_Down_2, LV_OBJ_FLAG_ADV_HITTEST);

    lv_obj_set_style_img_recolor(ui_Arrow_Down_2, lv_color_hex(0xABC1ED), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui_Arrow_Down_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor(ui_Arrow_Down_2, lv_color_hex(0x00D2FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_img_recolor_opa(ui_Arrow_Down_2, 255, LV_PART_MAIN | LV_STATE_CHECKED);

    // ui_Arrow_Left_2

    ui_Arrow_Left_2 = lv_img_create(ui_BTN_Move_XY_Top);
    lv_img_set_src(ui_Arrow_Left_2, &ui_img_arrow_left_png);

    lv_obj_set_width(ui_Arrow_Left_2, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Arrow_Left_2, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Arrow_Left_2, 25);
    lv_obj_set_y(ui_Arrow_Left_2, 0);

    lv_obj_set_align(ui_Arrow_Left_2, LV_ALIGN_LEFT_MID);

    lv_obj_add_flag(ui_Arrow_Left_2, LV_OBJ_FLAG_ADV_HITTEST);

    lv_obj_set_style_img_recolor(ui_Arrow_Left_2, lv_color_hex(0xABC1ED), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui_Arrow_Left_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor(ui_Arrow_Left_2, lv_color_hex(0x00D2FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_img_recolor_opa(ui_Arrow_Left_2, 255, LV_PART_MAIN | LV_STATE_CHECKED);

    // ui_Arrow_Right_2

    ui_Arrow_Right_2 = lv_img_create(ui_BTN_Move_XY_Top);
    lv_img_set_src(ui_Arrow_Right_2, &ui_img_arrow_right_png);

    lv_obj_set_width(ui_Arrow_Right_2, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Arrow_Right_2, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Arrow_Right_2, -25);
    lv_obj_set_y(ui_Arrow_Right_2, 0);

    lv_obj_set_align(ui_Arrow_Right_2, LV_ALIGN_RIGHT_MID);

    lv_obj_add_flag(ui_Arrow_Right_2, LV_OBJ_FLAG_ADV_HITTEST);

    lv_obj_set_style_img_recolor(ui_Arrow_Right_2, lv_color_hex(0xABC1ED), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui_Arrow_Right_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor(ui_Arrow_Right_2, lv_color_hex(0x00D2FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_img_recolor_opa(ui_Arrow_Right_2, 255, LV_PART_MAIN | LV_STATE_CHECKED);

    // ui_Roller_Number

    ui_Roller_Number = lv_obj_create(ui_S1_Content_Panel1);

    lv_obj_set_height(ui_Roller_Number, 215);
    lv_obj_set_width(ui_Roller_Number, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Roller_Number, 238);
    lv_obj_set_y(ui_Roller_Number, 86);

    lv_obj_set_align(ui_Roller_Number, LV_ALIGN_CENTER);

    lv_obj_clear_flag(ui_Roller_Number, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Roller_Number, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Roller_Number, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Roller_Number, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Roller_Number, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Roller_Number, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Roller_Number, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Roller_Number, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Roller_Number, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Roller_Number, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Image8

    ui_Image8 = lv_img_create(ui_Roller_Number);
    lv_img_set_src(ui_Image8, &ui_img_roller_bg_png);

    lv_obj_set_width(ui_Image8, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Image8, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Image8, 0);
    lv_obj_set_y(ui_Image8, 0);

    lv_obj_set_align(ui_Image8, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_Image8, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_Roller1

    ui_Roller1 = lv_roller_create(ui_Roller_Number);
    lv_roller_set_options(ui_Roller1, "1\n5\n10\n15\n25\n50", LV_ROLLER_MODE_INFINITE);

    lv_obj_set_width(ui_Roller1, 142);
    lv_obj_set_height(ui_Roller1, 131);

    lv_obj_set_x(ui_Roller1, 0);
    lv_obj_set_y(ui_Roller1, 0);

    lv_obj_set_align(ui_Roller1, LV_ALIGN_CENTER);

    lv_obj_set_style_text_color(ui_Roller1, lv_color_hex(0x3F475E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Roller1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Roller1, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Roller1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Roller1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Roller1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Roller1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Roller1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_text_color(ui_Roller1, lv_color_hex(0xDBE6FF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Roller1, 255, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Roller1, &lv_font_montserrat_26, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Roller1, lv_color_hex(0x3A445C), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Roller1, 255, LV_PART_SELECTED | LV_STATE_DEFAULT);

    // ui_Label_Steps

    ui_Label_Steps = lv_label_create(ui_Roller_Number);

    lv_obj_set_width(ui_Label_Steps, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Steps, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Steps, 0);
    lv_obj_set_y(ui_Label_Steps, 0);

    lv_obj_set_align(ui_Label_Steps, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Label_Steps, "STEPS SIZE");
    lv_label_set_recolor(ui_Label_Steps, "true");

    lv_obj_set_style_text_color(ui_Label_Steps, lv_color_hex(0xD2DCF2), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Steps, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_Steps, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Steps, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_Reset

    ui_BTN_Reset = lv_imgbtn_create(ui_S1_Content_Panel1);
    lv_imgbtn_set_src(ui_BTN_Reset, LV_IMGBTN_STATE_RELEASED, NULL, &ui_img_btn_main_off_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Reset, LV_IMGBTN_STATE_PRESSED, NULL, &ui_img_btn_main_on_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Reset, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &ui_img_btn_main_on_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Reset, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &ui_img_btn_main_on_png, NULL);

    lv_obj_set_height(ui_BTN_Reset, 66);
    lv_obj_set_width(ui_BTN_Reset, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_BTN_Reset, -40);
    lv_obj_set_y(ui_BTN_Reset, 20);

    lv_obj_set_align(ui_BTN_Reset, LV_ALIGN_TOP_RIGHT);

    lv_obj_set_style_text_color(ui_BTN_Reset, lv_color_hex(0xABC1ED), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_BTN_Reset, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_BTN_Reset, lv_color_hex(0x00B9FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(ui_BTN_Reset, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(ui_BTN_Reset, lv_color_hex(0x06CEFB), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_opa(ui_BTN_Reset, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    // ui_Label_Reset

    ui_Label_Reset = lv_label_create(ui_BTN_Reset);

    lv_obj_set_width(ui_Label_Reset, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Reset, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Reset, 0);
    lv_obj_set_y(ui_Label_Reset, 0);

    lv_obj_set_align(ui_Label_Reset, LV_ALIGN_CENTER);

    lv_label_set_text(ui_Label_Reset, "RESET POS.");
    lv_label_set_recolor(ui_Label_Reset, "true");

    lv_obj_set_style_text_align(ui_Label_Reset, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Reset, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_Heat

    ui_BTN_Heat = lv_imgbtn_create(ui_S1_Content_Panel1);
    lv_imgbtn_set_src(ui_BTN_Heat, LV_IMGBTN_STATE_RELEASED, NULL, &ui_img_btn_main_off_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Heat, LV_IMGBTN_STATE_PRESSED, NULL, &ui_img_btn_main_on_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Heat, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &ui_img_btn_main_on_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Heat, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &ui_img_btn_main_on_png, NULL);

    lv_obj_set_height(ui_BTN_Heat, 66);
    lv_obj_set_width(ui_BTN_Heat, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_BTN_Heat, -40);
    lv_obj_set_y(ui_BTN_Heat, 100);

    lv_obj_set_align(ui_BTN_Heat, LV_ALIGN_TOP_RIGHT);

    lv_obj_add_flag(ui_BTN_Heat, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_set_style_text_color(ui_BTN_Heat, lv_color_hex(0xABC1ED), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_BTN_Heat, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_BTN_Heat, lv_color_hex(0x00D1FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(ui_BTN_Heat, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(ui_BTN_Heat, lv_color_hex(0x00D1FF), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_opa(ui_BTN_Heat, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    // ui_Label_Reset1

    ui_Label_Reset1 = lv_label_create(ui_BTN_Heat);

    lv_obj_set_width(ui_Label_Reset1, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Reset1, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Reset1, 0);
    lv_obj_set_y(ui_Label_Reset1, 0);

    lv_obj_set_align(ui_Label_Reset1, LV_ALIGN_CENTER);

    lv_label_set_text(ui_Label_Reset1, "HEAT");
    lv_label_set_recolor(ui_Label_Reset1, "true");

    lv_obj_set_style_text_align(ui_Label_Reset1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Reset1, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

}
void ui_Screen_3_Setting_screen_init(void)
{

    // ui_Screen_3_Setting

    ui_Screen_3_Setting = lv_obj_create(NULL);

    lv_obj_set_style_bg_color(ui_Screen_3_Setting, lv_color_hex(0x191D26), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Screen_3_Setting, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Background2

    ui_Background2 = lv_img_create(ui_Screen_3_Setting);
    lv_img_set_src(ui_Background2, &ui_img_3d_printer_bg_png);

    lv_obj_set_width(ui_Background2, 800);
    lv_obj_set_height(ui_Background2, 480);

    lv_obj_set_x(ui_Background2, 0);
    lv_obj_set_y(ui_Background2, 0);

    lv_obj_set_align(ui_Background2, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_Background2, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_Panel_Header2

    ui_Panel_Header2 = lv_obj_create(ui_Screen_3_Setting);

    lv_obj_set_width(ui_Panel_Header2, 665);
    lv_obj_set_height(ui_Panel_Header2, 39);

    lv_obj_set_x(ui_Panel_Header2, -30);
    lv_obj_set_y(ui_Panel_Header2, 0);

    lv_obj_set_align(ui_Panel_Header2, LV_ALIGN_TOP_RIGHT);

    lv_obj_clear_flag(ui_Panel_Header2, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Panel_Header2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Panel_Header2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Panel_Header2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Panel_Header2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Panel_Header2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_Header2

    ui_Label_Header2 = lv_label_create(ui_Panel_Header2);

    lv_obj_set_width(ui_Label_Header2, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Header2, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Header2, 0);
    lv_obj_set_y(ui_Label_Header2, 0);

    lv_obj_set_align(ui_Label_Header2, LV_ALIGN_LEFT_MID);

    lv_label_set_text(ui_Label_Header2, "Adjust the print parameters");
    lv_label_set_recolor(ui_Label_Header2, "true");

    lv_obj_set_style_text_color(ui_Label_Header2, lv_color_hex(0x9098AA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Header2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Header2, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_IMG_Wifi2

    ui_IMG_Wifi2 = lv_img_create(ui_Panel_Header2);
    lv_img_set_src(ui_IMG_Wifi2, &ui_img_icn_wifi_png);

    lv_obj_set_width(ui_IMG_Wifi2, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_IMG_Wifi2, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_IMG_Wifi2, 0);
    lv_obj_set_y(ui_IMG_Wifi2, 0);

    lv_obj_set_align(ui_IMG_Wifi2, LV_ALIGN_RIGHT_MID);

    lv_obj_add_flag(ui_IMG_Wifi2, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_IMG_PC2

    ui_IMG_PC2 = lv_img_create(ui_Panel_Header2);
    lv_img_set_src(ui_IMG_PC2, &ui_img_icn_pc_png);

    lv_obj_set_width(ui_IMG_PC2, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_IMG_PC2, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_IMG_PC2, -42);
    lv_obj_set_y(ui_IMG_PC2, 0);

    lv_obj_set_align(ui_IMG_PC2, LV_ALIGN_RIGHT_MID);

    lv_obj_add_flag(ui_IMG_PC2, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_IMG_USB2

    ui_IMG_USB2 = lv_img_create(ui_Panel_Header2);
    lv_img_set_src(ui_IMG_USB2, &ui_img_icn_usb_png);

    lv_obj_set_width(ui_IMG_USB2, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_IMG_USB2, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_IMG_USB2, -84);
    lv_obj_set_y(ui_IMG_USB2, 0);

    lv_obj_set_align(ui_IMG_USB2, LV_ALIGN_RIGHT_MID);

    lv_obj_add_flag(ui_IMG_USB2, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_BTN_Menu_Print_S3

    ui_BTN_Menu_Print_S3 = lv_obj_create(ui_Screen_3_Setting);

    lv_obj_set_width(ui_BTN_Menu_Print_S3, 79);
    lv_obj_set_height(ui_BTN_Menu_Print_S3, 160);

    lv_obj_set_x(ui_BTN_Menu_Print_S3, 0);
    lv_obj_set_y(ui_BTN_Menu_Print_S3, 0);

    lv_obj_clear_flag(ui_BTN_Menu_Print_S3, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_event_cb(ui_BTN_Menu_Print_S3, ui_event_BTN_Menu_Print_S3, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_main_stop(ui_BTN_Menu_Print_S3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_BTN_Menu_Print_S3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_BTN_Menu_Print_S3, &ui_img_btn_print_png, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_BTN_Menu_Print_S3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_opa(ui_BTN_Menu_Print_S3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_Menu_Move_S3

    ui_BTN_Menu_Move_S3 = lv_obj_create(ui_Screen_3_Setting);

    lv_obj_set_width(ui_BTN_Menu_Move_S3, 79);
    lv_obj_set_height(ui_BTN_Menu_Move_S3, 160);

    lv_obj_set_x(ui_BTN_Menu_Move_S3, 0);
    lv_obj_set_y(ui_BTN_Menu_Move_S3, 0);

    lv_obj_set_align(ui_BTN_Menu_Move_S3, LV_ALIGN_LEFT_MID);

    lv_obj_clear_flag(ui_BTN_Menu_Move_S3, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_event_cb(ui_BTN_Menu_Move_S3, ui_event_BTN_Menu_Move_S3, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_main_stop(ui_BTN_Menu_Move_S3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_BTN_Menu_Move_S3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_BTN_Menu_Move_S3, &ui_img_btn_move_png, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_BTN_Menu_Move_S3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_opa(ui_BTN_Menu_Move_S3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_Menu_Setting_S3

    ui_BTN_Menu_Setting_S3 = lv_obj_create(ui_Screen_3_Setting);

    lv_obj_set_width(ui_BTN_Menu_Setting_S3, 79);
    lv_obj_set_height(ui_BTN_Menu_Setting_S3, 160);

    lv_obj_set_x(ui_BTN_Menu_Setting_S3, 0);
    lv_obj_set_y(ui_BTN_Menu_Setting_S3, 0);

    lv_obj_set_align(ui_BTN_Menu_Setting_S3, LV_ALIGN_BOTTOM_LEFT);

    lv_obj_clear_flag(ui_BTN_Menu_Setting_S3, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_main_stop(ui_BTN_Menu_Setting_S3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_BTN_Menu_Setting_S3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_BTN_Menu_Setting_S3, &ui_img_btn_setting_png, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_BTN_Menu_Setting_S3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_S1_Content_Panel2

    ui_S1_Content_Panel2 = lv_obj_create(ui_Screen_3_Setting);

    lv_obj_set_width(ui_S1_Content_Panel2, 720);
    lv_obj_set_height(ui_S1_Content_Panel2, 430);

    lv_obj_set_x(ui_S1_Content_Panel2, 0);
    lv_obj_set_y(ui_S1_Content_Panel2, 0);

    lv_obj_set_align(ui_S1_Content_Panel2, LV_ALIGN_BOTTOM_RIGHT);

    lv_obj_clear_flag(ui_S1_Content_Panel2, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_S1_Content_Panel2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_S1_Content_Panel2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_S1_Content_Panel2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_S1_Content_Panel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_S1_Content_Panel2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_S1_Content_Panel2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_S1_Content_Panel2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_S1_Content_Panel2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_S1_Content_Panel2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Roller_Material

    ui_Roller_Material = lv_obj_create(ui_S1_Content_Panel2);

    lv_obj_set_height(ui_Roller_Material, 215);
    lv_obj_set_width(ui_Roller_Material, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Roller_Material, 350);
    lv_obj_set_y(ui_Roller_Material, 20);

    lv_obj_clear_flag(ui_Roller_Material, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Roller_Material, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Roller_Material, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Roller_Material, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Roller_Material, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Roller_Material, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Roller_Material, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Roller_Material, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Roller_Material, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Roller_Material, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Image3

    ui_Image3 = lv_img_create(ui_Roller_Material);
    lv_img_set_src(ui_Image3, &ui_img_roller_bg_png);

    lv_obj_set_width(ui_Image3, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Image3, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Image3, 0);
    lv_obj_set_y(ui_Image3, 0);

    lv_obj_set_align(ui_Image3, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_Image3, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_Roller4

    ui_Roller4 = lv_roller_create(ui_Roller_Material);
    lv_roller_set_options(ui_Roller4, "ABS\nPLA\nPETC", LV_ROLLER_MODE_INFINITE);

    lv_obj_set_width(ui_Roller4, 142);
    lv_obj_set_height(ui_Roller4, 131);

    lv_obj_set_x(ui_Roller4, 0);
    lv_obj_set_y(ui_Roller4, 0);

    lv_obj_set_align(ui_Roller4, LV_ALIGN_CENTER);

    lv_obj_set_style_text_color(ui_Roller4, lv_color_hex(0x3F475E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Roller4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Roller4, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Roller4, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Roller4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Roller4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Roller4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Roller4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_text_color(ui_Roller4, lv_color_hex(0xDBE6FF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Roller4, 255, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Roller4, &lv_font_montserrat_26, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Roller4, lv_color_hex(0x3A445C), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Roller4, 255, LV_PART_SELECTED | LV_STATE_DEFAULT);

    // ui_Label_Material

    ui_Label_Material = lv_label_create(ui_Roller_Material);

    lv_obj_set_width(ui_Label_Material, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Material, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Material, 0);
    lv_obj_set_y(ui_Label_Material, 0);

    lv_obj_set_align(ui_Label_Material, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Label_Material, "MATERIAL");
    lv_label_set_recolor(ui_Label_Material, "true");

    lv_obj_set_style_text_color(ui_Label_Material, lv_color_hex(0xD2DCF2), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Material, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_Material, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Material, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Roller_Bed_Temp

    ui_Roller_Bed_Temp = lv_obj_create(ui_S1_Content_Panel2);

    lv_obj_set_height(ui_Roller_Bed_Temp, 215);
    lv_obj_set_width(ui_Roller_Bed_Temp, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Roller_Bed_Temp, 175);
    lv_obj_set_y(ui_Roller_Bed_Temp, 20);

    lv_obj_clear_flag(ui_Roller_Bed_Temp, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Roller_Bed_Temp, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Roller_Bed_Temp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Roller_Bed_Temp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Roller_Bed_Temp, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Roller_Bed_Temp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Roller_Bed_Temp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Roller_Bed_Temp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Roller_Bed_Temp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Roller_Bed_Temp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Image2

    ui_Image2 = lv_img_create(ui_Roller_Bed_Temp);
    lv_img_set_src(ui_Image2, &ui_img_roller_bg_png);

    lv_obj_set_width(ui_Image2, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Image2, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Image2, 0);
    lv_obj_set_y(ui_Image2, 0);

    lv_obj_set_align(ui_Image2, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_Image2, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_Roller3

    ui_Roller3 = lv_roller_create(ui_Roller_Bed_Temp);
    lv_roller_set_options(ui_Roller3, "50°\n55°\n60°\n65°\n70°\n75°\n80°\n90°", LV_ROLLER_MODE_INFINITE);

    lv_obj_set_width(ui_Roller3, 142);
    lv_obj_set_height(ui_Roller3, 131);

    lv_obj_set_x(ui_Roller3, 0);
    lv_obj_set_y(ui_Roller3, 0);

    lv_obj_set_align(ui_Roller3, LV_ALIGN_CENTER);

    lv_obj_set_style_text_color(ui_Roller3, lv_color_hex(0x3F475E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Roller3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Roller3, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Roller3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Roller3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Roller3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Roller3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Roller3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_text_color(ui_Roller3, lv_color_hex(0xDBE6FF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Roller3, 255, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Roller3, &lv_font_montserrat_26, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Roller3, lv_color_hex(0x3A445C), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Roller3, 255, LV_PART_SELECTED | LV_STATE_DEFAULT);

    // ui_Label_Bed_Temp1

    ui_Label_Bed_Temp1 = lv_label_create(ui_Roller_Bed_Temp);

    lv_obj_set_width(ui_Label_Bed_Temp1, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Bed_Temp1, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Bed_Temp1, 0);
    lv_obj_set_y(ui_Label_Bed_Temp1, 0);

    lv_obj_set_align(ui_Label_Bed_Temp1, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Label_Bed_Temp1, "BED TEMP.");
    lv_label_set_recolor(ui_Label_Bed_Temp1, "true");

    lv_obj_set_style_text_color(ui_Label_Bed_Temp1, lv_color_hex(0xD2DCF2), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Bed_Temp1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_Bed_Temp1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Bed_Temp1, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Roller_Head_Temp

    ui_Roller_Head_Temp = lv_obj_create(ui_S1_Content_Panel2);

    lv_obj_set_height(ui_Roller_Head_Temp, 215);
    lv_obj_set_width(ui_Roller_Head_Temp, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Roller_Head_Temp, 0);
    lv_obj_set_y(ui_Roller_Head_Temp, 20);

    lv_obj_clear_flag(ui_Roller_Head_Temp, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_Roller_Head_Temp, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Roller_Head_Temp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Roller_Head_Temp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Roller_Head_Temp, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Roller_Head_Temp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Roller_Head_Temp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Roller_Head_Temp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Roller_Head_Temp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Roller_Head_Temp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Image1

    ui_Image1 = lv_img_create(ui_Roller_Head_Temp);
    lv_img_set_src(ui_Image1, &ui_img_roller_bg_png);

    lv_obj_set_width(ui_Image1, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Image1, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Image1, 0);
    lv_obj_set_y(ui_Image1, 0);

    lv_obj_set_align(ui_Image1, LV_ALIGN_CENTER);

    lv_obj_add_flag(ui_Image1, LV_OBJ_FLAG_ADV_HITTEST);

    // ui_Roller2

    ui_Roller2 = lv_roller_create(ui_Roller_Head_Temp);
    lv_roller_set_options(ui_Roller2, "160°\n170°\n180°\n190°\n200°\n210°\n220°\n230°", LV_ROLLER_MODE_INFINITE);

    lv_obj_set_width(ui_Roller2, 142);
    lv_obj_set_height(ui_Roller2, 131);

    lv_obj_set_x(ui_Roller2, 0);
    lv_obj_set_y(ui_Roller2, 0);

    lv_obj_set_align(ui_Roller2, LV_ALIGN_CENTER);

    lv_obj_set_style_text_color(ui_Roller2, lv_color_hex(0x3F475E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Roller2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Roller2, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Roller2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Roller2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Roller2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Roller2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Roller2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_text_color(ui_Roller2, lv_color_hex(0xDBE6FF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Roller2, 255, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Roller2, &lv_font_montserrat_26, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Roller2, lv_color_hex(0x3A445C), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Roller2, 255, LV_PART_SELECTED | LV_STATE_DEFAULT);

    // ui_Label_Head_Temp1

    ui_Label_Head_Temp1 = lv_label_create(ui_Roller_Head_Temp);

    lv_obj_set_width(ui_Label_Head_Temp1, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Head_Temp1, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Head_Temp1, 0);
    lv_obj_set_y(ui_Label_Head_Temp1, 0);

    lv_obj_set_align(ui_Label_Head_Temp1, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Label_Head_Temp1, "HEAD TEMP.");
    lv_label_set_recolor(ui_Label_Head_Temp1, "true");

    lv_obj_set_style_text_color(ui_Label_Head_Temp1, lv_color_hex(0xD2DCF2), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Head_Temp1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_Head_Temp1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Head_Temp1, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_Heat_Head

    ui_BTN_Heat_Head = lv_imgbtn_create(ui_S1_Content_Panel2);
    lv_imgbtn_set_src(ui_BTN_Heat_Head, LV_IMGBTN_STATE_RELEASED, NULL, &ui_img_btn_main_off_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Heat_Head, LV_IMGBTN_STATE_PRESSED, NULL, &ui_img_btn_main_on_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Heat_Head, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &ui_img_btn_main_on_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Heat_Head, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &ui_img_btn_main_on_png, NULL);

    lv_obj_set_height(ui_BTN_Heat_Head, 66);
    lv_obj_set_width(ui_BTN_Heat_Head, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_BTN_Heat_Head, 12);
    lv_obj_set_y(ui_BTN_Heat_Head, 40);

    lv_obj_set_align(ui_BTN_Heat_Head, LV_ALIGN_LEFT_MID);

    lv_obj_add_flag(ui_BTN_Heat_Head, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_set_style_text_color(ui_BTN_Heat_Head, lv_color_hex(0xABC1ED), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_BTN_Heat_Head, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_BTN_Heat_Head, lv_color_hex(0x00B9FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(ui_BTN_Heat_Head, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(ui_BTN_Heat_Head, lv_color_hex(0x06CEFB), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_opa(ui_BTN_Heat_Head, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    // ui_Label_Reset22

    ui_Label_Reset22 = lv_label_create(ui_BTN_Heat_Head);

    lv_obj_set_width(ui_Label_Reset22, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Reset22, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Reset22, 0);
    lv_obj_set_y(ui_Label_Reset22, 0);

    lv_obj_set_align(ui_Label_Reset22, LV_ALIGN_CENTER);

    lv_label_set_text(ui_Label_Reset22, "HEAT HEAD");
    lv_label_set_recolor(ui_Label_Reset22, "true");

    lv_obj_set_style_text_align(ui_Label_Reset22, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Reset22, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_Heat_Bed

    ui_BTN_Heat_Bed = lv_imgbtn_create(ui_S1_Content_Panel2);
    lv_imgbtn_set_src(ui_BTN_Heat_Bed, LV_IMGBTN_STATE_RELEASED, NULL, &ui_img_btn_main_off_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Heat_Bed, LV_IMGBTN_STATE_PRESSED, NULL, &ui_img_btn_main_on_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Heat_Bed, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &ui_img_btn_main_on_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Heat_Bed, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &ui_img_btn_main_on_png, NULL);

    lv_obj_set_height(ui_BTN_Heat_Bed, 66);
    lv_obj_set_width(ui_BTN_Heat_Bed, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_BTN_Heat_Bed, 185);
    lv_obj_set_y(ui_BTN_Heat_Bed, 40);

    lv_obj_set_align(ui_BTN_Heat_Bed, LV_ALIGN_LEFT_MID);

    lv_obj_add_flag(ui_BTN_Heat_Bed, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_set_style_text_color(ui_BTN_Heat_Bed, lv_color_hex(0xABC1ED), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_BTN_Heat_Bed, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_BTN_Heat_Bed, lv_color_hex(0x00B9FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(ui_BTN_Heat_Bed, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(ui_BTN_Heat_Bed, lv_color_hex(0x06CEFB), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_opa(ui_BTN_Heat_Bed, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    // ui_Label_Heat_Bed

    ui_Label_Heat_Bed = lv_label_create(ui_BTN_Heat_Bed);

    lv_obj_set_width(ui_Label_Heat_Bed, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Heat_Bed, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Heat_Bed, 0);
    lv_obj_set_y(ui_Label_Heat_Bed, 0);

    lv_obj_set_align(ui_Label_Heat_Bed, LV_ALIGN_CENTER);

    lv_label_set_text(ui_Label_Heat_Bed, "HEAT BED");
    lv_label_set_recolor(ui_Label_Heat_Bed, "true");

    lv_obj_set_style_text_align(ui_Label_Heat_Bed, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Heat_Bed, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_BTN_Remove_Filament

    ui_BTN_Remove_Filament = lv_imgbtn_create(ui_S1_Content_Panel2);
    lv_imgbtn_set_src(ui_BTN_Remove_Filament, LV_IMGBTN_STATE_RELEASED, NULL, &ui_img_btn_main_off_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Remove_Filament, LV_IMGBTN_STATE_PRESSED, NULL, &ui_img_btn_main_on_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Remove_Filament, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &ui_img_btn_main_on_png, NULL);
    lv_imgbtn_set_src(ui_BTN_Remove_Filament, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &ui_img_btn_main_on_png, NULL);

    lv_obj_set_height(ui_BTN_Remove_Filament, 66);
    lv_obj_set_width(ui_BTN_Remove_Filament, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_BTN_Remove_Filament, 360);
    lv_obj_set_y(ui_BTN_Remove_Filament, 40);

    lv_obj_set_align(ui_BTN_Remove_Filament, LV_ALIGN_LEFT_MID);

    lv_obj_add_flag(ui_BTN_Remove_Filament, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_set_style_text_color(ui_BTN_Remove_Filament, lv_color_hex(0xABC1ED), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_BTN_Remove_Filament, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_BTN_Remove_Filament, lv_color_hex(0x00B9FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(ui_BTN_Remove_Filament, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(ui_BTN_Remove_Filament, lv_color_hex(0x06CEFB), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_opa(ui_BTN_Remove_Filament, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    // ui_Label_Remove_Filament

    ui_Label_Remove_Filament = lv_label_create(ui_BTN_Remove_Filament);

    lv_obj_set_width(ui_Label_Remove_Filament, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Remove_Filament, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Remove_Filament, 0);
    lv_obj_set_y(ui_Label_Remove_Filament, 0);

    lv_obj_set_align(ui_Label_Remove_Filament, LV_ALIGN_CENTER);

    lv_label_set_text(ui_Label_Remove_Filament, "REMOVE\nFILAMENT");
    lv_label_set_recolor(ui_Label_Remove_Filament, "true");

    lv_obj_set_style_text_align(ui_Label_Remove_Filament, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Remove_Filament, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Panel_Slider

    ui_Panel_Slider = lv_obj_create(ui_S1_Content_Panel2);

    lv_obj_set_width(ui_Panel_Slider, 550);
    lv_obj_set_height(ui_Panel_Slider, 121);

    lv_obj_set_x(ui_Panel_Slider, 0);
    lv_obj_set_y(ui_Panel_Slider, 0);

    lv_obj_set_align(ui_Panel_Slider, LV_ALIGN_BOTTOM_LEFT);

    lv_obj_set_style_bg_color(ui_Panel_Slider, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Panel_Slider, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Panel_Slider, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Panel_Slider, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Panel_Slider, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Slider_Print_Speed

    ui_Slider_Print_Speed = lv_slider_create(ui_Panel_Slider);
    lv_slider_set_range(ui_Slider_Print_Speed, 0, 100);
    lv_slider_set_value(ui_Slider_Print_Speed, 50, LV_ANIM_OFF);
    if(lv_slider_get_mode(ui_Slider_Print_Speed) == LV_SLIDER_MODE_RANGE) lv_slider_set_left_value(ui_Slider_Print_Speed, 0,
                                                                                                       LV_ANIM_OFF);

    lv_obj_set_width(ui_Slider_Print_Speed, 475);
    lv_obj_set_height(ui_Slider_Print_Speed, 35);

    lv_obj_set_x(ui_Slider_Print_Speed, 0);
    lv_obj_set_y(ui_Slider_Print_Speed, 0);

    lv_obj_set_align(ui_Slider_Print_Speed, LV_ALIGN_CENTER);

    lv_obj_add_event_cb(ui_Slider_Print_Speed, ui_event_Slider_Print_Speed, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_color(ui_Slider_Print_Speed, lv_color_hex(0x222733), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider_Print_Speed, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_Slider_Print_Speed, lv_color_hex(0x191D26), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(ui_Slider_Print_Speed, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_Slider_Print_Speed, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_pad(ui_Slider_Print_Speed, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Slider_Print_Speed, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Slider_Print_Speed, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Slider_Print_Speed, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Slider_Print_Speed, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_Slider_Print_Speed, lv_color_hex(0x1DE8FF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider_Print_Speed, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui_Slider_Print_Speed, lv_color_hex(0x0962B7), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_Slider_Print_Speed, LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_Slider_Print_Speed, lv_color_hex(0x7689AC), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Slider_Print_Speed, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui_Slider_Print_Speed, lv_color_hex(0x556483), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_Slider_Print_Speed, LV_GRAD_DIR_VER, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Slider_Print_Speed, lv_color_hex(0x28DCFD), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_Slider_Print_Speed, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Slider_Print_Speed, 1, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_Slider_Print_Speed, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui_Slider_Print_Speed, -2, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui_Slider_Print_Speed, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Slider_Print_Speed, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Slider_Print_Speed, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Slider_Print_Speed, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Slider_Print_Speed, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    // ui_Label_Print_Speed

    ui_Label_Print_Speed = lv_label_create(ui_Panel_Slider);

    lv_obj_set_width(ui_Label_Print_Speed, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Print_Speed, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Print_Speed, 25);
    lv_obj_set_y(ui_Label_Print_Speed, -10);

    lv_label_set_text(ui_Label_Print_Speed, "PRIT SPEED");
    lv_label_set_recolor(ui_Label_Print_Speed, "true");

    lv_obj_set_style_text_color(ui_Label_Print_Speed, lv_color_hex(0xD2DCF2), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Print_Speed, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_Print_Speed, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Print_Speed, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_Print_Speed_Number

    ui_Label_Print_Speed_Number = lv_label_create(ui_Panel_Slider);

    lv_obj_set_width(ui_Label_Print_Speed_Number, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Print_Speed_Number, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Print_Speed_Number, -30);
    lv_obj_set_y(ui_Label_Print_Speed_Number, -10);

    lv_obj_set_align(ui_Label_Print_Speed_Number, LV_ALIGN_TOP_RIGHT);

    lv_label_set_text(ui_Label_Print_Speed_Number, "50%");
    lv_label_set_recolor(ui_Label_Print_Speed_Number, "true");

    lv_obj_set_style_text_color(ui_Label_Print_Speed_Number, lv_color_hex(0xD2DCF2), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Print_Speed_Number, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_Print_Speed_Number, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Print_Speed_Number, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Swich_Group

    ui_Swich_Group = lv_obj_create(ui_Screen_3_Setting);

    lv_obj_set_width(ui_Swich_Group, 160);
    lv_obj_set_height(ui_Swich_Group, 422);

    lv_obj_set_x(ui_Swich_Group, -20);
    lv_obj_set_y(ui_Swich_Group, 50);

    lv_obj_set_align(ui_Swich_Group, LV_ALIGN_TOP_RIGHT);

    lv_obj_set_style_bg_color(ui_Swich_Group, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Swich_Group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Swich_Group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Swich_Group, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Swich_Group, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Label_Light

    ui_Label_Light = lv_label_create(ui_Swich_Group);

    lv_obj_set_width(ui_Label_Light, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Light, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Light, 0);
    lv_obj_set_y(ui_Label_Light, 0);

    lv_obj_set_align(ui_Label_Light, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Label_Light, "MATERIAL");
    lv_label_set_recolor(ui_Label_Light, "true");

    lv_obj_set_style_text_color(ui_Label_Light, lv_color_hex(0xD2DCF2), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Light, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_Light, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Light, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Switch_Light

    ui_Switch_Light = lv_switch_create(ui_Swich_Group);

    lv_obj_set_width(ui_Switch_Light, 90);
    lv_obj_set_height(ui_Switch_Light, 40);

    lv_obj_set_x(ui_Switch_Light, 0);
    lv_obj_set_y(ui_Switch_Light, 30);

    lv_obj_set_align(ui_Switch_Light, LV_ALIGN_TOP_MID);

    lv_obj_set_style_bg_color(ui_Switch_Light, lv_color_hex(0x191D26), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Switch_Light, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_border_color(ui_Switch_Light, lv_color_hex(0x191D26), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_Switch_Light, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Switch_Light, 5, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_Switch_Light, LV_BORDER_SIDE_FULL, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Switch_Light, lv_color_hex(0x00D4FF), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_Switch_Light, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_color(ui_Switch_Light, lv_color_hex(0x0179FF), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui_Switch_Light, LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_color(ui_Switch_Light, lv_color_hex(0x191D26), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_opa(ui_Switch_Light, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui_Switch_Light, 5, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_side(ui_Switch_Light, LV_BORDER_SIDE_FULL, LV_PART_INDICATOR | LV_STATE_CHECKED);

    lv_obj_set_style_bg_color(ui_Switch_Light, lv_color_hex(0x7C92BA), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Switch_Light, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui_Switch_Light, lv_color_hex(0x536483), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Switch_Light, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Switch_Light, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_Switch_Light, LV_GRAD_DIR_VER, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Switch_Light, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Switch_Light, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Switch_Light, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Switch_Light, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Switch_Light, lv_color_hex(0x00FEFF), LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_opa(ui_Switch_Light, 255, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(ui_Switch_Light, 5, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_spread(ui_Switch_Light, 0, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_ofs_x(ui_Switch_Light, -3, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_ofs_y(ui_Switch_Light, 0, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_pad_left(ui_Switch_Light, -5, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_pad_right(ui_Switch_Light, -5, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_pad_top(ui_Switch_Light, -5, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_pad_bottom(ui_Switch_Light, -5, LV_PART_KNOB | LV_STATE_CHECKED);

    // ui_Label_Fan_1

    ui_Label_Fan_1 = lv_label_create(ui_Swich_Group);

    lv_obj_set_width(ui_Label_Fan_1, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Fan_1, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Fan_1, 0);
    lv_obj_set_y(ui_Label_Fan_1, 100);

    lv_obj_set_align(ui_Label_Fan_1, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Label_Fan_1, "FAN 1");
    lv_label_set_recolor(ui_Label_Fan_1, "true");

    lv_obj_set_style_text_color(ui_Label_Fan_1, lv_color_hex(0xD2DCF2), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Fan_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_Fan_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Fan_1, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Switch_Fan_1

    ui_Switch_Fan_1 = lv_switch_create(ui_Swich_Group);

    lv_obj_set_width(ui_Switch_Fan_1, 90);
    lv_obj_set_height(ui_Switch_Fan_1, 40);

    lv_obj_set_x(ui_Switch_Fan_1, 0);
    lv_obj_set_y(ui_Switch_Fan_1, 130);

    lv_obj_set_align(ui_Switch_Fan_1, LV_ALIGN_TOP_MID);

    lv_obj_set_style_bg_color(ui_Switch_Fan_1, lv_color_hex(0x191D26), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Switch_Fan_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_border_color(ui_Switch_Fan_1, lv_color_hex(0x191D26), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_Switch_Fan_1, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Switch_Fan_1, 5, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_Switch_Fan_1, LV_BORDER_SIDE_FULL, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Switch_Fan_1, lv_color_hex(0x00D4FF), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_Switch_Fan_1, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_color(ui_Switch_Fan_1, lv_color_hex(0x0179FF), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui_Switch_Fan_1, LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_color(ui_Switch_Fan_1, lv_color_hex(0x191D26), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_opa(ui_Switch_Fan_1, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui_Switch_Fan_1, 5, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_side(ui_Switch_Fan_1, LV_BORDER_SIDE_FULL, LV_PART_INDICATOR | LV_STATE_CHECKED);

    lv_obj_set_style_bg_color(ui_Switch_Fan_1, lv_color_hex(0x7C92BA), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Switch_Fan_1, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui_Switch_Fan_1, lv_color_hex(0x536483), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Switch_Fan_1, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Switch_Fan_1, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_Switch_Fan_1, LV_GRAD_DIR_VER, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Switch_Fan_1, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Switch_Fan_1, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Switch_Fan_1, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Switch_Fan_1, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Switch_Fan_1, lv_color_hex(0x00FEFF), LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_opa(ui_Switch_Fan_1, 255, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(ui_Switch_Fan_1, 5, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_spread(ui_Switch_Fan_1, 0, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_ofs_x(ui_Switch_Fan_1, -3, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_ofs_y(ui_Switch_Fan_1, 0, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_pad_left(ui_Switch_Fan_1, -5, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_pad_right(ui_Switch_Fan_1, -5, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_pad_top(ui_Switch_Fan_1, -5, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_pad_bottom(ui_Switch_Fan_1, -5, LV_PART_KNOB | LV_STATE_CHECKED);

    // ui_Label_Fan_2

    ui_Label_Fan_2 = lv_label_create(ui_Swich_Group);

    lv_obj_set_width(ui_Label_Fan_2, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Fan_2, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Fan_2, 0);
    lv_obj_set_y(ui_Label_Fan_2, 200);

    lv_obj_set_align(ui_Label_Fan_2, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Label_Fan_2, "FAN 2\n");
    lv_label_set_recolor(ui_Label_Fan_2, "true");

    lv_obj_set_style_text_color(ui_Label_Fan_2, lv_color_hex(0xD2DCF2), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Fan_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_Fan_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Fan_2, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Switch_Fan_2

    ui_Switch_Fan_2 = lv_switch_create(ui_Swich_Group);

    lv_obj_set_width(ui_Switch_Fan_2, 90);
    lv_obj_set_height(ui_Switch_Fan_2, 40);

    lv_obj_set_x(ui_Switch_Fan_2, 0);
    lv_obj_set_y(ui_Switch_Fan_2, 230);

    lv_obj_set_align(ui_Switch_Fan_2, LV_ALIGN_TOP_MID);

    lv_obj_set_style_bg_color(ui_Switch_Fan_2, lv_color_hex(0x191D26), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Switch_Fan_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_border_color(ui_Switch_Fan_2, lv_color_hex(0x191D26), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_Switch_Fan_2, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Switch_Fan_2, 5, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_Switch_Fan_2, LV_BORDER_SIDE_FULL, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Switch_Fan_2, lv_color_hex(0x00D4FF), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_Switch_Fan_2, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_color(ui_Switch_Fan_2, lv_color_hex(0x0179FF), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui_Switch_Fan_2, LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_color(ui_Switch_Fan_2, lv_color_hex(0x191D26), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_opa(ui_Switch_Fan_2, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui_Switch_Fan_2, 5, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_side(ui_Switch_Fan_2, LV_BORDER_SIDE_FULL, LV_PART_INDICATOR | LV_STATE_CHECKED);

    lv_obj_set_style_bg_color(ui_Switch_Fan_2, lv_color_hex(0x7C92BA), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Switch_Fan_2, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui_Switch_Fan_2, lv_color_hex(0x536483), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Switch_Fan_2, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Switch_Fan_2, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_Switch_Fan_2, LV_GRAD_DIR_VER, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Switch_Fan_2, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Switch_Fan_2, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Switch_Fan_2, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Switch_Fan_2, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Switch_Fan_2, lv_color_hex(0x00FEFF), LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_opa(ui_Switch_Fan_2, 255, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(ui_Switch_Fan_2, 5, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_spread(ui_Switch_Fan_2, 0, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_ofs_x(ui_Switch_Fan_2, -3, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_ofs_y(ui_Switch_Fan_2, 0, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_pad_left(ui_Switch_Fan_2, -5, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_pad_right(ui_Switch_Fan_2, -5, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_pad_top(ui_Switch_Fan_2, -5, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_pad_bottom(ui_Switch_Fan_2, -5, LV_PART_KNOB | LV_STATE_CHECKED);

    // ui_Label_Fan_3

    ui_Label_Fan_3 = lv_label_create(ui_Swich_Group);

    lv_obj_set_width(ui_Label_Fan_3, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Fan_3, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Fan_3, 0);
    lv_obj_set_y(ui_Label_Fan_3, 300);

    lv_obj_set_align(ui_Label_Fan_3, LV_ALIGN_TOP_MID);

    lv_label_set_text(ui_Label_Fan_3, "FAN 3");
    lv_label_set_recolor(ui_Label_Fan_3, "true");

    lv_obj_set_style_text_color(ui_Label_Fan_3, lv_color_hex(0xD2DCF2), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_Fan_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label_Fan_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label_Fan_3, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Switch_Fan_3

    ui_Switch_Fan_3 = lv_switch_create(ui_Swich_Group);

    lv_obj_set_width(ui_Switch_Fan_3, 90);
    lv_obj_set_height(ui_Switch_Fan_3, 40);

    lv_obj_set_x(ui_Switch_Fan_3, 0);
    lv_obj_set_y(ui_Switch_Fan_3, 330);

    lv_obj_set_align(ui_Switch_Fan_3, LV_ALIGN_TOP_MID);

    lv_obj_set_style_bg_color(ui_Switch_Fan_3, lv_color_hex(0x191D26), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Switch_Fan_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_border_color(ui_Switch_Fan_3, lv_color_hex(0x191D26), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_Switch_Fan_3, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Switch_Fan_3, 5, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_Switch_Fan_3, LV_BORDER_SIDE_FULL, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Switch_Fan_3, lv_color_hex(0x00D4FF), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_Switch_Fan_3, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_color(ui_Switch_Fan_3, lv_color_hex(0x0179FF), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui_Switch_Fan_3, LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_color(ui_Switch_Fan_3, lv_color_hex(0x191D26), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_opa(ui_Switch_Fan_3, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui_Switch_Fan_3, 5, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_side(ui_Switch_Fan_3, LV_BORDER_SIDE_FULL, LV_PART_INDICATOR | LV_STATE_CHECKED);

    lv_obj_set_style_bg_color(ui_Switch_Fan_3, lv_color_hex(0x7C92BA), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Switch_Fan_3, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui_Switch_Fan_3, lv_color_hex(0x536483), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Switch_Fan_3, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Switch_Fan_3, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_Switch_Fan_3, LV_GRAD_DIR_VER, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Switch_Fan_3, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Switch_Fan_3, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Switch_Fan_3, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Switch_Fan_3, -5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Switch_Fan_3, lv_color_hex(0x00FEFF), LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_opa(ui_Switch_Fan_3, 255, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(ui_Switch_Fan_3, 5, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_spread(ui_Switch_Fan_3, 0, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_ofs_x(ui_Switch_Fan_3, -3, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_ofs_y(ui_Switch_Fan_3, 0, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_pad_left(ui_Switch_Fan_3, -5, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_pad_right(ui_Switch_Fan_3, -5, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_pad_top(ui_Switch_Fan_3, -5, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_pad_bottom(ui_Switch_Fan_3, -5, LV_PART_KNOB | LV_STATE_CHECKED);

}

void ui_printer_init(void)
{
    lv_disp_t * dispp = lv_disp_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    ui_Screen_1_Print_screen_init();
    ui_Screen_2_Move_screen_init();
    ui_Screen_3_Setting_screen_init();
    lv_disp_load_scr(ui_Screen_1_Print);
}

