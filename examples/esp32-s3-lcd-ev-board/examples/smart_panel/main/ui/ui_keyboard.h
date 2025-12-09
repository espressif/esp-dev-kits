#ifndef UI_KEYBOARD_H
#define UI_KEYBOARD_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif


lv_obj_t *ui_keyboard_create(lv_obj_t *textarea);
void ui_keyboard_delete(lv_obj_t *keyboard);
/**
 * @brief keyboard control
 * @param keyboard pointer to keyboard obj
 * @param textarea control textarea
 */
void ui_keyboard_set_textarea(lv_obj_t *keyboard, lv_obj_t *textarea);

#ifdef __cplusplus
}
#endif

#endif // UI_KEYBOARD_H