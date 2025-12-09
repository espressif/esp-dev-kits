#include "ui_keyboard.h"
#include "lvgl.h"

lv_obj_t *ui_keyboard_create(lv_obj_t *textarea)
{   
    //create keyboard
    lv_obj_t *kb = lv_keyboard_create(lv_scr_act(), NULL);
    
    //position keyboard at the bottom of screen
    lv_obj_set_size(kb, LV_HOR_RES, LV_VER_RES / 2);
    lv_obj_align(kb, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    
    //text input
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
    
    //attach to text area
    if (textarea) {
        lv_keyboard_set_textarea(kb, textarea);
    }
    
    return kb;
}

void ui_keyboard_delete(lv_obj_t *keyboard)
{
    if (keyboard) {
        lv_obj_del(keyboard);
    }
}

void ui_keyboard_set_textarea(lv_obj_t *keyboard, lv_obj_t *textarea)
{
    if (keyboard && textarea) {
        lv_keyboard_set_textarea(keyboard, textarea);
    }
}