/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "esp_log.h"
#include "esp_check.h"

#include "app_sr_handler.h"
#include "ui_main.h"
#include "settings.h"

/* UI function declaration */
ui_func_desc_t ui_setting_func = {
    .name = "ui_setting",
    .init = ui_setting_init,
    .show = ui_setting_show,
    .hide = ui_setting_hide,
};

/* LVGL objects defination */
static lv_obj_t *obj_page_app = NULL;
static lv_obj_t *obj_page_bluetooth = NULL;
static lv_obj_t *obj_page_about = NULL;
static lv_obj_t *obj_page_factory = NULL;

/* Extern image resources data. Loaded in `ui_main.c` */
extern void *data_icon_app;
extern void *data_icon_about;
extern void *other_device;

/* Static function forward declaration */
static void btn_app_cb(lv_obj_t *obj, lv_event_t event);
static void btn_bluetooth_cb(lv_obj_t *obj, lv_event_t event);
static void btn_about_cb(lv_obj_t *obj, lv_event_t event);
static void btn_factory_cb(lv_obj_t *obj, lv_event_t event);

void ui_setting_init(void *data)
{
    (void)data;

    obj_page_app = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(obj_page_app, 200, 200);
    lv_obj_set_style_local_bg_color(obj_page_app, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
    lv_obj_set_style_local_bg_color(obj_page_app, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_radius(obj_page_app, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 20);
    lv_obj_set_style_local_border_width(obj_page_app, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(obj_page_app, NULL, LV_ALIGN_CENTER, -250, 25);

    obj_page_bluetooth = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(obj_page_bluetooth, 200, 200);
    lv_obj_set_style_local_bg_color(obj_page_bluetooth, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
    lv_obj_set_style_local_bg_color(obj_page_bluetooth, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_radius(obj_page_bluetooth, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 20);
    lv_obj_set_style_local_border_width(obj_page_bluetooth, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(obj_page_bluetooth, NULL, LV_ALIGN_CENTER, -90, 25);

    obj_page_about = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(obj_page_about, 200, 200);
    lv_obj_set_style_local_bg_color(obj_page_about, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
    lv_obj_set_style_local_bg_color(obj_page_about, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_radius(obj_page_about, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 20);
    lv_obj_set_style_local_border_width(obj_page_about, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(obj_page_about, NULL, LV_ALIGN_CENTER, 90, 25);

    obj_page_factory = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(obj_page_factory, 200, 200);
    lv_obj_set_style_local_bg_color(obj_page_factory, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
    lv_obj_set_style_local_bg_color(obj_page_factory, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_radius(obj_page_factory, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 20);
    lv_obj_set_style_local_border_width(obj_page_factory, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(obj_page_factory, NULL, LV_ALIGN_CENTER, 270, 25);

    lv_obj_t *img_app = lv_img_create(obj_page_app, NULL);
    lv_img_set_src(img_app, data_icon_app);
    lv_obj_align(img_app, obj_page_app, LV_ALIGN_CENTER, 0, -30);

    lv_obj_t *img_bluetooth = lv_img_create(obj_page_bluetooth, NULL);
    lv_img_set_src(img_bluetooth, data_icon_app);
    lv_obj_align(img_bluetooth, obj_page_bluetooth, LV_ALIGN_CENTER, 0, -30);

    lv_obj_t *img_about = lv_img_create(obj_page_about, NULL);
    lv_img_set_src(img_about, data_icon_about);
    lv_obj_align(img_about, obj_page_about, LV_ALIGN_CENTER, 0, -30);

    lv_obj_t *img_factory = lv_img_create(obj_page_factory, NULL);
    lv_img_set_src(img_factory, other_device);
    lv_obj_align(img_factory, obj_page_factory, LV_ALIGN_CENTER, 0, -30);

    lv_obj_set_style_local_value_str(obj_page_about,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "About");
    lv_obj_set_style_local_value_font(obj_page_about, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_style_local_value_align(obj_page_about,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_ofs_y(obj_page_about,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 50);
    lv_obj_set_style_local_value_color(obj_page_about,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_value_color(obj_page_about,  LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);

    lv_obj_set_style_local_value_str(obj_page_app,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "Wi-Fi Connect");
    lv_obj_set_style_local_value_font(obj_page_app, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_style_local_value_align(obj_page_app,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_ofs_y(obj_page_app,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 50);
    lv_obj_set_style_local_value_color(obj_page_app,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_value_color(obj_page_app,  LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);

    lv_obj_set_style_local_value_str(obj_page_bluetooth,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "Bluetooth");
    lv_obj_set_style_local_value_font(obj_page_bluetooth, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_style_local_value_align(obj_page_bluetooth,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_ofs_y(obj_page_bluetooth,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 50);
    lv_obj_set_style_local_value_color(obj_page_bluetooth,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_value_color(obj_page_bluetooth,  LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);

    lv_obj_set_style_local_value_str(obj_page_factory,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "Factory Reset");
    lv_obj_set_style_local_value_font(obj_page_factory, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_style_local_value_align(obj_page_factory,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_ofs_y(obj_page_factory,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 50);
    lv_obj_set_style_local_value_color(obj_page_factory,  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_value_color(obj_page_factory,  LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);

    lv_obj_set_event_cb(obj_page_app, btn_app_cb);
    lv_obj_set_event_cb(obj_page_bluetooth, btn_bluetooth_cb);
    lv_obj_set_event_cb(obj_page_about, btn_about_cb);
    lv_obj_set_event_cb(obj_page_factory, btn_factory_cb);
}

void ui_setting_show(void *data)
{
    if (NULL == obj_page_about) {
        ui_setting_init(data);
        ui_status_bar_time_show(true);
    } else {
        lv_obj_set_hidden(obj_page_about, false);
        lv_obj_set_hidden(obj_page_app, false);
        lv_obj_set_hidden(obj_page_bluetooth, false);
        lv_obj_set_hidden(obj_page_factory, false);
        ui_status_bar_time_show(true);
    }
}

void ui_setting_hide(void *data)
{
    (void)data;

    if (NULL != obj_page_about) {
        lv_obj_set_hidden(obj_page_app, true);
        lv_obj_set_hidden(obj_page_bluetooth, true);
        lv_obj_set_hidden(obj_page_about, true);
        lv_obj_set_hidden(obj_page_factory, true);
    }

}

/* ******************************** Event Handler(s) ******************************** */
static void btn_app_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        ui_show(&ui_app_func, UI_SHOW_PEDDING);
    }
}

static void btn_bluetooth_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        ui_show(&ui_bluetooth_func, UI_SHOW_PEDDING);
    }
}

static void btn_about_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        ui_show(&ui_about_func, UI_SHOW_PEDDING);
    }
}

static lv_obj_t *bg_factory_reset;
static uint8_t reboot_wait_time;

static void ui_factory_reset_init()
{
    bg_factory_reset = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(bg_factory_reset, LV_HOR_RES, LV_VER_RES);
    lv_obj_align(bg_factory_reset, NULL, LV_ALIGN_CENTER, 0, 0);

    reboot_wait_time = 50;
    lv_obj_t *label_reboot = lv_label_create(bg_factory_reset, NULL);
    lv_obj_set_style_local_text_font(label_reboot, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_28);
    lv_label_set_text(label_reboot, "Rebooting  6 sec");
    lv_obj_align(label_reboot, bg_factory_reset, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_hidden(bg_factory_reset, false);

    return;
}

static void update_reboot_step(lv_task_t *task)
{
    if (false == bg_factory_reset->hidden) {
        if (reboot_wait_time / 10) {

            char reboot_info[30] = {0};
            lv_obj_t *label_reboot = lv_obj_get_child(bg_factory_reset, 0);

            sprintf(reboot_info, "Rebooting  %d sec", reboot_wait_time / 10);
            reboot_wait_time--;
            lv_label_set_text(label_reboot, reboot_info);
        } else {
            esp_restart();
        }
    }
    return;
}

static void btn_factory_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        settings_factory_reset();
        ui_factory_reset_init();
        lv_task_create(update_reboot_step, 100, 1, NULL);
    }
}
