/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ui_main.h"
#include "sys_check.h"
#include "settings.h"

static const char *TAG = "ui_main";

#define LOG_TRACE(...)  ESP_LOGD(TAG, ##__VA_ARGS__)

void *data_icon_weather = NULL;
void *data_icon_humidity = NULL;
void *data_icon_uv = NULL;
void *data_icon_pm2p5 = NULL;
void *data_icon_pm2p5_press = NULL;

void *data_icon_app = NULL;
void *data_icon_about = NULL;

void *living_room = NULL;
void *kitchen = NULL;
void *dining_room = NULL;
void *bathroom = NULL;

void *data_ico_auto = NULL;
void *data_ico_cool = NULL;
void *data_ico_heat = NULL;
void *data_ico_dry = NULL;
void *data_ico_fan = NULL;
void *data_ico_auto_down = NULL;
void *data_ico_cool_down = NULL;
void *data_ico_heat_down = NULL;
void *data_ico_dry_down = NULL;
void *data_ico_fan_down = NULL;

void *light_control = NULL;
void *air_conditioner = NULL;
void *pomodoro_timer = NULL;
void *habit_tracker = NULL;
void *fan = NULL;
void *video_entertainment = NULL;
void *smart_security = NULL;
void *other_device = NULL;

void *data_music_album[1];

void *weather_icon_data[5] = { [0 ... 4] = NULL };

/* Resources loading list */
static ui_data_fetch_t img_fetch_list[] = {
    /* Main Clock Page */
    { "S:/Icon/101.bin", &data_icon_weather },
    { "S:/UI/humid.bin", &data_icon_humidity },
    { "S:/UI/uv.bin", &data_icon_uv },
    { "S:/UI/pm2p5.bin", &data_icon_pm2p5 },
    { "S:/UI/pm2p5_press.bin", &data_icon_pm2p5_press },

    /* Setting Page */
    { "S:/UI/app.bin", &data_icon_app },
    { "S:/UI/about.bin", &data_icon_about },

    /* Air Conditioner Control Page */
    { "S:/UI/icon_auto.bin", &data_ico_auto },
    { "S:/UI/icon_cool.bin", &data_ico_cool },
    { "S:/UI/icon_heat.bin", &data_ico_heat },
    { "S:/UI/icon_dry.bin", &data_ico_dry },
    { "S:/UI/icon_fan.bin", &data_ico_fan },
    { "S:/UI/icon_auto_down.bin", &data_ico_auto_down },
    { "S:/UI/icon_cool_down.bin", &data_ico_cool_down },
    { "S:/UI/icon_heat_down.bin", &data_ico_heat_down },
    { "S:/UI/icon_dry_down.bin", &data_ico_dry_down },
    { "S:/UI/icon_fan_down.bin", &data_ico_fan_down },

    /* Smart Switch Page */
    { "S:/UI/light_control.bin", &light_control },
    { "S:/UI/air_conditioner.bin", &air_conditioner },
    { "S:/UI/pomodoro_timer.bin", &pomodoro_timer },
    { "S:/UI/habit_tracker.bin", &habit_tracker },
    { "S:/UI/fan.bin", &fan },
    { "S:/UI/video_entertainment.bin", &video_entertainment },
    { "S:/UI/smart_security.bin", &smart_security },
    { "S:/UI/other_device.bin", &other_device },

    /* Music Player Album */
    { "S:/UI/Album_0.bin", &(data_music_album[0]) },

    /* Weather Pager */
    { "S:/Icon/999.bin", &weather_icon_data[0] },
    { "S:/Icon/999.bin", &weather_icon_data[1] },
    { "S:/Icon/999.bin", &weather_icon_data[2] },
    { "S:/Icon/999.bin", &weather_icon_data[3] },
    { "S:/Icon/999.bin", &weather_icon_data[4] },
};

static void ui_init_internal(void);

esp_err_t ui_call_stack_push(ui_func_desc_t *func);
static esp_err_t ui_call_stack_pop(ui_func_desc_t *func);
static esp_err_t ui_call_stack_peek(ui_func_desc_t *func);
static esp_err_t ui_call_stack_clear(void);

void ui_main(void)
{
    /* Init main screen */
    bsp_display_lock(0);
    lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);

    /* Show logo */
    LV_IMG_DECLARE(esp_logo)
    lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_bg_color(img, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_img_set_src(img, &esp_logo);
    lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, -50);
    bsp_display_unlock();

    if (ESP_OK != sys_check()) {
        while (1) {
            /* Stop at here if system check not pass */
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    bsp_display_lock(0);
    /* Create a bar to update loading progress */
    lv_obj_t *bar = lv_bar_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_bg_color(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, COLOR_BG);
    lv_obj_set_style_local_border_color(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_border_color(bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, COLOR_DEEP);
    lv_obj_set_style_local_border_width(bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, 2);
    lv_obj_set_size(bar, 400, 12);
    lv_obj_align(bar, img, LV_ALIGN_OUT_BOTTOM_MID, 0, 50);
    lv_bar_set_range(bar, 0, sizeof(img_fetch_list) / sizeof(ui_data_fetch_t));
    lv_bar_set_value(bar, 1, LV_ANIM_ON);

    lv_obj_t *label_loading_hint = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text_static(label_loading_hint, " ");
    lv_obj_set_style_local_text_font(label_loading_hint, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_style_local_text_color(label_loading_hint, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_align(label_loading_hint, bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    bsp_display_unlock();

    /* Load resource from SD crad to PSARM */
    TickType_t tick = xTaskGetTickCount();
    for (size_t i = 0; i < sizeof(img_fetch_list) / sizeof(ui_data_fetch_t); i++) {
        bsp_display_lock(0);
        lv_bar_set_value(bar, i + 1, LV_ANIM_ON);
        lv_label_set_text_fmt(label_loading_hint, "Loading \"%s\"", img_fetch_list[i].path);
        lv_obj_align(label_loading_hint, bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
        bsp_display_unlock();
        if (ESP_OK != img_file_check(img_fetch_list[i].path)) {
            /* If any of file was missing, block the task */
            while (1) {
                vTaskDelay(1000);
            }
        }

        ui_laod_resource(img_fetch_list[i].path, img_fetch_list[i].data);
        /* Yeah, it's only because that makes ui more flexible */
        vTaskDelayUntil(&tick, pdMS_TO_TICKS(50));
    }

    vTaskDelay(pdMS_TO_TICKS(500));

    /* Remove image and bar, reset background color */
    bsp_display_lock(0);
    lv_obj_del(img);
    lv_obj_del(bar);
    lv_obj_del(label_loading_hint);
    lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);
    bsp_display_unlock();

    /* Init call queue and pre-init widgets */
    ui_init_internal();

    /* Entering main UI */
    bsp_display_lock(0);
    ui_status_bar_init();
    sys_param_t *sys_set = settings_get_parameter();

    if (sys_set->need_hint) {
        ESP_LOGI(TAG, "@@@Enter user_guide_layer");
        ui_guide_show(NULL);
    } else {
        if (sys_set->demo_gui) {
            ESP_LOGI(TAG, "@@@Enter main_Layer");
            ui_clock_show(NULL);
            ui_call_stack_push(&ui_clock_func);
        } else {
            ESP_LOGI(TAG, "@@@Enter sr_layer");
            ui_sr_show(NULL);
        }
    }
    bsp_display_unlock();
}


void ui_laod_resource(const char *path, void **dst)
{
    void *_data = NULL;
    lv_fs_file_t file_img;
    uint32_t file_size = 0;
    uint32_t real_size = 0;

    /* Try to open file */
    if (LV_FS_RES_OK != lv_fs_open(&file_img, path, LV_FS_MODE_RD)) {
        ESP_LOGE(TAG, "Can't open file : %s", path);
        return;
    }

    /* Get size of file */
    lv_fs_size(&file_img, &file_size);
    file_size -= 1;
    _data = heap_caps_malloc(file_size + 8, MALLOC_CAP_SPIRAM);
    lv_fs_read(&file_img, _data + 8, file_size, &real_size);
    lv_fs_close(&file_img);

    /* Change lv_img_dsc_t's data pointer */
    memmove(_data, _data + 8, 4);

    /* Set image information */
    *((uint32_t *)(_data + 4)) = file_size - 4;
    *((uint32_t *)(_data + 8)) = (uint32_t)(_data + 12);
    *dst = _data;
}

/* ******************************** CALL STACK ******************************** */
/**
 * @brief About call stack:
 *   - As for pages without back:
 *      ui_show(&ui_func_desc, UI_SHOW_OVERRIDE);
 *   - As for page with call queue:
 *      ui_show(&ui_func_desc, UI_SHOW_PEDDING);
 *   - As for back button:
 *      ui_show(&ui_func_desc, UI_SHOW_BACKPORT);
 */

static const int call_stack_size = 8;
typedef ui_func_desc_t call_stack_type_t;
static call_stack_type_t *call_stack = NULL;
static volatile size_t call_stack_index = 0;

/**
 * @brief An initialize task before UI start. Basicly create a call stack.
 *
 */
static void ui_init_internal(void)
{
    assert(call_stack_size);
    call_stack = heap_caps_malloc(call_stack_size * sizeof(ui_func_desc_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
}

/**
 * @brief Push operator for call stack.
 *
 * @param func pointer to `ui_func_desc_t`.
 * @return esp_err_t Push result.
 */
esp_err_t ui_call_stack_push(ui_func_desc_t *func)
{
    if (call_stack_index <= call_stack_size - 1) {
        memcpy(&call_stack[call_stack_index], func, sizeof(call_stack_type_t));
        call_stack_index++;
    } else {
        ESP_LOGE(TAG, "Call stack full");
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

/**
 * @brief Pop operator for call stack.
 *
 * @param func pointer to `ui_func_desc_t`.
 * @return esp_err_t Pop result.
 */
static esp_err_t ui_call_stack_pop(ui_func_desc_t *func)
{
    if (0 != call_stack_index) {
        call_stack_index--;
        memcpy(func, &call_stack[call_stack_index], sizeof(call_stack_type_t));
    } else {
        ESP_LOGE(TAG, "Call queue empty");
        return ESP_FAIL;
    }

    return ESP_OK;
}

/**
 * @brief Peek top element from call stack. Keep data in call stack.
 *
 * @param func pointer to `ui_func_desc_t`.
 * @return esp_err_t Peek result.
 */
static esp_err_t ui_call_stack_peek(ui_func_desc_t *func)
{
    if (0 != call_stack_index) {
        memcpy(func, &call_stack[call_stack_index - 1], sizeof(call_stack_type_t));
        ESP_LOGI(TAG, "Peek : %s", func->name);
    } else {
        ESP_LOGE(TAG, "Call queue empty");
        return ESP_FAIL;
    }

    return ESP_OK;
}

/**
 * @brief Clear all data in call stack.
 *
 * @return esp_err_t Always return `ESP_OK`.
 */
static esp_err_t ui_call_stack_clear(void)
{
    ui_func_desc_t func;

    while (call_stack_index) {
        call_stack_index--;
        memcpy(&func, &call_stack[call_stack_index], sizeof(call_stack_type_t));
        ESP_LOGI(TAG, "Clear : %s", func.name);
    }

    call_stack_index = 0;

    return ESP_OK;
}

void ui_show(ui_func_desc_t *ui, ui_show_mode_t mode)
{
    static ui_func_desc_t ui_now;

    switch (mode) {
    case UI_SHOW_OVERRIDE:
        ui_call_stack_pop(&ui_now);
        ui_now.hide(NULL);
        ui->show(NULL);
        ui_call_stack_clear();
        ui_call_stack_push(ui);
        break;
    case UI_SHOW_PEDDING:
        ui_call_stack_peek(&ui_now);
        ui_now.hide(NULL);
        ui->show(NULL);
        ui_call_stack_push(ui);
        break;
    case UI_SHOW_BACKPORT:
        ui_call_stack_pop(&ui_now);
        ui_now.hide(NULL);
        ui_call_stack_peek(&ui_now);
        ui_now.show(NULL);
        break;
    }
}
