/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ui_main.h"

ui_func_desc_t ui_uv_func = {
    .name = "UV Index",
    .init = ui_uv_init,
    .show = ui_uv_show,
    .hide = ui_uv_hide,
};

static ui_state_t ui_uv_state = ui_state_dis;


/* LVGL objects defination */
static lv_obj_t *obj_page_uv = NULL;
static lv_obj_t *arc_uv_outline = NULL;
static lv_obj_t *arc_uv_val = NULL;
static lv_obj_t *label_uv_level = NULL;
static lv_obj_t *img_uv = NULL;
static lv_obj_t *label_uv_tips = NULL;
static lv_obj_t *label_tips = NULL;
static lv_obj_t *bar_top = NULL;
static lv_obj_t *bar_bot = NULL;

/* Default UV val, multiplied by 10 */
static int uv_val = 30;

/* Static function forward declaration */
static void anim_uv_task(void *data);
static void ui_uv_update_content(void);

/* Extern image resources data. Loaded in `ui_main.c` */
extern void *data_icon_uv;

void ui_uv_init(void *data)
{
    ui_page_show("UV Index");
    obj_page_uv = ui_page_get_obj();

    arc_uv_outline = lv_arc_create(obj_page_uv, NULL);
    lv_obj_set_size(arc_uv_outline, 350, 350);
    lv_arc_set_bg_angles(arc_uv_outline, 135, 45);
    lv_obj_set_style_local_bg_opa(arc_uv_outline, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_width(arc_uv_outline, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_line_width(arc_uv_outline, LV_ARC_PART_BG, LV_STATE_DEFAULT, 2);
    lv_obj_set_style_local_line_color(arc_uv_outline, LV_ARC_PART_BG, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_align(arc_uv_outline, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 35, 30);

    arc_uv_val = lv_arc_create(obj_page_uv, NULL);
    lv_obj_set_size(arc_uv_val, 330, 330);
    lv_arc_set_bg_angles(arc_uv_val, 135, 45);
    lv_obj_set_style_local_border_width(arc_uv_val, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_line_width(arc_uv_val, LV_ARC_PART_BG, LV_STATE_DEFAULT, 12);
    lv_obj_set_style_local_line_width(arc_uv_val, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 12);
    lv_obj_set_style_local_line_color(arc_uv_val, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_bg_opa(arc_uv_val, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_value_str(arc_uv_val, LV_ARC_PART_BG, LV_STATE_DEFAULT, "0.0");
    lv_obj_set_style_local_value_ofs_y(arc_uv_val, LV_ARC_PART_BG, LV_STATE_DEFAULT, -20);
    lv_obj_set_style_local_value_font(arc_uv_val, LV_ARC_PART_BG, LV_STATE_DEFAULT, &font_en_bold_72);
    lv_arc_set_range(arc_uv_val, 0, 115);
    lv_arc_set_value(arc_uv_val, 0);
    lv_obj_align(arc_uv_val, arc_uv_outline, LV_ALIGN_CENTER, 0, 0);

    label_uv_level = lv_label_create(obj_page_uv, NULL);
    lv_label_set_text(label_uv_level, "Moderate");
    lv_obj_set_style_local_text_font(label_uv_level, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_32);
    lv_obj_align(label_uv_level, arc_uv_val, LV_ALIGN_CENTER, 0, 45);

    label_uv_tips = lv_label_create(obj_page_uv, NULL);
    lv_obj_set_style_local_text_font(label_uv_tips, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_thin_20);
    lv_label_set_text(label_uv_tips,
                      "Wear sunglasses on bright days.\n"
                      "If you burn easily, cover up and\n"
                      "use broad spectrum SPF 30+\n"
                      "sunscreen.\n");

    lv_obj_set_style_local_text_line_space(label_uv_tips, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 5);
    lv_obj_set_style_local_text_letter_space(label_uv_tips, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 1);
    lv_obj_align(label_uv_tips, NULL, LV_ALIGN_IN_RIGHT_MID, -40, 70);

    img_uv = lv_img_create(obj_page_uv, NULL);
    lv_img_set_src(img_uv, data_icon_uv);
    lv_obj_align(img_uv, label_uv_tips, LV_ALIGN_OUT_TOP_LEFT, 0, -15);

    label_tips = lv_label_create(obj_page_uv, NULL);
    lv_label_set_text(label_tips, "Tips");
    lv_obj_set_style_local_text_font(label_tips, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_28);
    lv_obj_align(label_tips, img_uv, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    bar_top = lv_bar_create(obj_page_uv, NULL);
    lv_obj_set_style_local_bg_color(bar_top, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_make(198, 198, 198));
    lv_obj_set_size(bar_top, 300, 2);

    bar_bot = lv_bar_create(obj_page_uv, bar_top);

    lv_obj_align(bar_top, img_uv, LV_ALIGN_OUT_TOP_LEFT, 0, -20);
    lv_obj_align(bar_bot, label_uv_tips, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

    ui_uv_state = ui_state_show;
}

void ui_uv_show(void *data)
{
    if (ui_state_dis == ui_uv_state) {
        ui_uv_init(data);

        ui_uv_update_content();

        ui_status_bar_time_show(true);

        xTaskCreate(
            (TaskFunction_t)        anim_uv_task,
            (const char *const)    "UV Animate Task",
            (const uint32_t)        3 * 1024,
            (void *const)          NULL,
            (UBaseType_t)           1,
            (TaskHandle_t *const)  NULL);
    } else if (ui_state_hide == ui_uv_state) {
        lv_obj_set_hidden(arc_uv_outline, false);
        lv_obj_set_hidden(arc_uv_val, false);
        lv_obj_set_hidden(label_uv_tips, false);
        lv_obj_set_hidden(label_uv_level, false);
        lv_obj_set_hidden(label_tips, false);
        lv_obj_set_hidden(img_uv, false);
        lv_obj_set_hidden(bar_top, false);
        lv_obj_set_hidden(bar_bot, false);

        ui_uv_update_content();

        ui_page_show("UV Index");

        ui_status_bar_time_show(true);

        ui_uv_state = ui_state_show;

        xTaskCreate(
            (TaskFunction_t)        anim_uv_task,
            (const char *const)    "UV Animate Task",
            (const uint32_t)        3 * 1024,
            (void *const)          NULL,
            (UBaseType_t)           1,
            (TaskHandle_t *const)  NULL);

    }
}

void ui_uv_hide(void *data)
{
    if (ui_state_show == ui_uv_state) {
        lv_obj_set_hidden(arc_uv_outline, true);
        lv_obj_set_hidden(arc_uv_val, true);
        lv_obj_set_hidden(label_uv_tips, true);
        lv_obj_set_hidden(label_tips, true);
        lv_obj_set_hidden(label_uv_level, true);
        lv_obj_set_hidden(img_uv, true);
        lv_obj_set_hidden(bar_top, true);
        lv_obj_set_hidden(bar_bot, true);

        lv_obj_set_hidden(obj_page_uv, true);

        ui_uv_state = ui_state_hide;
    }
}

static void anim_uv_task(void *data)
{
    static TickType_t tick = 0;
    static char fmt_text[16];
    static int i = 0;

    tick = xTaskGetTickCount();
    bsp_display_lock(0);
    lv_arc_set_value(arc_uv_val, 0);
    bsp_display_unlock();

    /* Reset value */
    i = 0;

    while (1) {
        if (ui_state_hide == ui_uv_state) {
            vTaskDelete(NULL);
        }
        if (i <= uv_val) {
            sprintf(fmt_text, "%1.1f", i / 10.0f);
            bsp_display_lock(0);
            lv_obj_set_style_local_value_str(arc_uv_val, LV_ARC_PART_BG, LV_STATE_DEFAULT, fmt_text);
            lv_arc_set_value(arc_uv_val, i);
            bsp_display_unlock();
            i += 1;
            vTaskDelayUntil(&tick, pdMS_TO_TICKS(25));
        } else {
            vTaskDelete(NULL);
        }
    }
}

static const char *uv_index_get_level_str(int uv_level)
{
    static const char *uv_level_str[] = {
        "Low",
        "Moderate",
        "High",
        "Very High",
        "Extreme",
    };

    return (uv_level_str[uv_level]);
}

static int uv_index_get_level(float uv_index)
{
    if (uv_index < 2.5f) {
        return 0;
    } else if (uv_index < 5.5f) {
        return 1;
    } else if (uv_index < 7.5f) {
        return 2;
    } else if (uv_index < 10.5f) {
        return 3;
    }
    return 4;
}

static void ui_uv_update_content(void)
{
    lv_label_set_text_static(label_uv_level, uv_index_get_level_str(uv_index_get_level(uv_val / 10.0f)));

    lv_obj_align(label_uv_level, arc_uv_val, LV_ALIGN_CENTER, 0, 40);
}
