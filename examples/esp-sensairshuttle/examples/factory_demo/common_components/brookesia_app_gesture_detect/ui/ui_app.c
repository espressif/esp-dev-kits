/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "ui_app.h"
#include "screens/ui_GestureDetectScreen.h"
#include "ui.h"
#include <string.h>

static gesture_t current_gesture = GESTURE_PICKUP;

static void set_gesture_image(const lv_img_dsc_t *img_dsc)
{
    if (img_dsc == NULL) {
        lv_obj_add_flag(ui_GestureImage, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_image_set_src(ui_GestureImage, img_dsc);
    lv_obj_clear_flag(ui_GestureImage, LV_OBJ_FLAG_HIDDEN);
}

void set_current_gesture(gesture_t gesture)
{
    current_gesture = gesture;
}

gesture_t get_current_gesture(void)
{
    return current_gesture;
}

void update_gesture_detail(const char *detail_text)
{
    if (detail_text && strlen(detail_text) > 0) {
        lv_label_set_text(ui_GestureDetailLabel, detail_text);
        lv_obj_clear_flag(ui_GestureDetailLabel, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_label_set_text(ui_GestureDetailLabel, "");
        lv_obj_add_flag(ui_GestureDetailLabel, LV_OBJ_FLAG_HIDDEN);
    }
}

void update_gesture_ui(gesture_t gesture)
{
    lv_obj_clear_state(ui_ShakeSwitch, LV_STATE_CHECKED);
    lv_obj_clear_state(ui_RollSwitch, LV_STATE_CHECKED);
    switch (gesture) {
    case GESTURE_PICKUP:
        lv_obj_add_state(ui_PickupStatusLight, LV_STATE_CHECKED);
        lv_label_set_text(ui_PickupLabel, "moving");
        set_gesture_image(NULL);
        lv_obj_add_flag(ui_ReadyLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_MotionDetectedLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_GestureDetailLabel, LV_OBJ_FLAG_HIDDEN);
        set_current_gesture(GESTURE_PICKUP);
        break;
    case GESTURE_PUTDOWN:
        lv_obj_clear_state(ui_PickupStatusLight, LV_STATE_CHECKED);
        lv_label_set_text(ui_PickupLabel, "no motion");
        set_gesture_image(NULL);
        lv_obj_add_flag(ui_MotionDetectedLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_ReadyLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_GestureDetailLabel, LV_OBJ_FLAG_HIDDEN);
        set_current_gesture(GESTURE_PUTDOWN);
        break;
    case GESTURE_SHAKE:
        set_gesture_image(&ui_img_shake_png);
        lv_obj_add_flag(ui_ReadyLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_MotionDetectedLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_state(ui_ShakeSwitch, LV_STATE_CHECKED);
        set_current_gesture(GESTURE_SHAKE);
        break;
    case GESTURE_ROLL:
        lv_obj_add_flag(ui_ReadyLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_MotionDetectedLabel, LV_OBJ_FLAG_HIDDEN);
        set_gesture_image(&ui_img_pickup_png);
        lv_obj_add_state(ui_RollSwitch, LV_STATE_CHECKED);
        set_current_gesture(GESTURE_ROLL);
        break;
    default:
        break;
    }
}
