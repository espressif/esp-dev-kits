
#include "ui_main.h"
#include "lvgl.h"
#include "ui_pomodoro_analytics.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



/* UI function declaration */
ui_func_desc_t ui_pomodoro_analytics_func = {
    .name = "Pomodoro Analytics",
    .init = ui_pomodoro_analytics_init,
    .show = ui_pomodoro_analytics_show,
    .hide = ui_pomodoro_analytics_hide,
};

static ui_state_t ui_pomodoro_analytics_state = ui_state_dis;

static lv_obj_t *obj_page_pomodoro_analytics = NULL;
static lv_obj_t *_btn_back = NULL;

// left panel - stats
static lv_obj_t *_stats_cont = NULL;
static lv_obj_t *_label_total_sessions = NULL;
static lv_obj_t *_label_total_time = NULL;
static lv_obj_t *title = NULL;

// right panel - chart
static lv_obj_t *_chart_focus_time = NULL;
static lv_obj_t *_chart_title = NULL;
static lv_chart_series_t *_focus_time_series = NULL;

// sample data
static uint16_t focus_time_data[] = { 25, 50, 75, 100, 50, 75, 180 }; // min/day

/* Forward declarations */
static void _btn_back_cb(lv_obj_t *obj, lv_event_t event);
static void _anim_chart_task(void *data);


void ui_pomodoro_analytics_init(void *data)
{
    ui_page_show("Pomodoro Analytics");
    obj_page_pomodoro_analytics = ui_page_get_obj();

    // back button (top left)
    _btn_back = lv_btn_create(obj_page_pomodoro_analytics, NULL);
    lv_obj_set_size(_btn_back, 50, 50);
    lv_obj_align(_btn_back, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);
    lv_obj_set_style_local_radius(_btn_back, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_obj_set_style_local_value_str(_btn_back, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_LEFT);
    lv_obj_set_style_local_bg_color(_btn_back, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(220, 220, 220));
    lv_obj_set_event_cb(_btn_back, _btn_back_cb);

    // left (stats)
    _stats_cont = lv_cont_create(obj_page_pomodoro_analytics, NULL);
    lv_obj_set_size(_stats_cont, 250, 280);
    lv_obj_align(_stats_cont, NULL, LV_ALIGN_IN_LEFT_MID, 10, 30);
    lv_cont_set_layout(_stats_cont, LV_LAYOUT_COLUMN_LEFT);
    lv_obj_set_style_local_bg_color(_stats_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(245, 245, 245));
    lv_obj_set_style_local_pad_all(_stats_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 20);
    lv_obj_set_style_local_pad_inner(_stats_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 20);
    lv_obj_set_style_local_radius(_stats_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);

    // title
    title = lv_label_create(_stats_cont, NULL);
    lv_label_set_text(title, "Weekly Insights");
    lv_obj_set_style_local_text_font(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_24);

    // total sess
    _label_total_sessions = lv_label_create(_stats_cont, NULL);
    lv_label_set_text(_label_total_sessions, "Total Sessions:\n24");
    lv_obj_set_style_local_text_font(_label_total_sessions, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_label_set_long_mode(_label_total_sessions, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(_label_total_sessions, 240);

    // total Time
    _label_total_time = lv_label_create(_stats_cont, NULL);
    lv_label_set_text(_label_total_time, "Total Time in Focus:\n600 minutes");
    lv_obj_set_style_local_text_font(_label_total_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_label_set_long_mode(_label_total_time, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(_label_total_time, 240);

    // chart title 
    _chart_title = lv_label_create(obj_page_pomodoro_analytics, NULL);
    lv_label_set_text(_chart_title, "Focus Time This Week");
    lv_obj_set_style_local_text_font(_chart_title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    lv_obj_align(_chart_title, _stats_cont, LV_ALIGN_OUT_RIGHT_TOP, 20, 5);

    // right (graph)
    _chart_focus_time = lv_chart_create(obj_page_pomodoro_analytics, NULL);
    lv_obj_set_size(_chart_focus_time, 440, 240);

    // chart
    lv_chart_set_range(_chart_focus_time, 0, 200); // 0 to 200 minutes to accommodate 180
    lv_chart_set_point_count(_chart_focus_time, 7); // 7 days
    lv_obj_set_style_local_border_width(_chart_focus_time, LV_CHART_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_left(_chart_focus_time, LV_CHART_PART_BG, LV_STATE_DEFAULT, 60);
    lv_obj_set_style_local_pad_bottom(_chart_focus_time, LV_CHART_PART_BG, LV_STATE_DEFAULT, 40);
    lv_obj_set_style_local_pad_right(_chart_focus_time, LV_CHART_PART_BG, LV_STATE_DEFAULT, 20);
    lv_obj_set_style_local_pad_top(_chart_focus_time, LV_CHART_PART_BG, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_text_font(_chart_focus_time, LV_CHART_PART_BG, LV_STATE_DEFAULT, &font_en_20);
    lv_obj_set_style_local_text_color(_chart_focus_time, LV_CHART_PART_BG, LV_STATE_DEFAULT, COLOR_BAR);
    lv_chart_set_div_line_count(_chart_focus_time, 0, 0);

    lv_chart_set_x_tick_texts(_chart_focus_time, "Mon\nTue\nWed\nThu\nFri\nSat\nSun", 2, LV_CHART_AXIS_DRAW_LAST_TICK | LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_y_tick_texts(_chart_focus_time, "0\n50\n100\n150\n200", 2, LV_CHART_AXIS_PRIMARY_Y | LV_CHART_AXIS_INVERSE_LABELS_ORDER);
    lv_chart_set_x_tick_length(_chart_focus_time, 5, 5);
    lv_chart_set_y_tick_length(_chart_focus_time, 5, 5);

    lv_obj_align(_chart_focus_time, _chart_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    ui_pomodoro_analytics_state = ui_state_show;
}

void ui_pomodoro_analytics_show(void *data)
{
    if (ui_state_dis == ui_pomodoro_analytics_state)
    {
        ui_pomodoro_analytics_init(data);
        
        // start animation task
        xTaskCreate(
            (TaskFunction_t)        _anim_chart_task,
            (const char *const)     "Analytics Chart Task",
            (const uint32_t)        3 * 1024,
            (void *const)           NULL,
            (UBaseType_t)           1,
            (TaskHandle_t *const)   NULL);
    }
    else if (ui_state_hide == ui_pomodoro_analytics_state)
    {
        lv_obj_set_hidden(_btn_back, false);
        lv_obj_set_hidden(_stats_cont, false);
        lv_obj_set_hidden(_chart_focus_time, false);
        lv_obj_set_hidden(_chart_title, false);
        lv_obj_set_hidden(_label_total_sessions, false);
        lv_obj_set_hidden(_label_total_time, false);
        lv_obj_set_hidden(title, false);
        ui_page_show("Pomodoro Analytics");
        ui_pomodoro_analytics_state = ui_state_show;
        
        // restart animation task
        xTaskCreate(
            (TaskFunction_t)        _anim_chart_task,
            (const char *const)     "Analytics Chart Task",
            (const uint32_t)        3 * 1024,
            (void *const)           NULL,
            (UBaseType_t)           1,
            (TaskHandle_t *const)   NULL);
    }
}

void ui_pomodoro_analytics_hide(void *data)
{
    if (ui_state_show == ui_pomodoro_analytics_state)
    {
        lv_obj_set_hidden(_btn_back, true);
        lv_obj_set_hidden(_stats_cont, true);
        lv_obj_set_hidden(_chart_focus_time, true);
        lv_obj_set_hidden(_chart_title, true);
        lv_obj_set_hidden(_label_total_sessions, true);
        lv_obj_set_hidden(_label_total_time, true);
        lv_obj_set_hidden(title, true);

        
        // clear chart series
        if (_focus_time_series) {
            lv_chart_clear_series(_chart_focus_time, _focus_time_series);
            _focus_time_series = NULL;
        }
        
        ui_page_hide(NULL);
        ui_pomodoro_analytics_state = ui_state_hide;
    }
}

// back button callback
static void _btn_back_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        ui_pomodoro_analytics_hide(NULL);
        ui_pomodoro_timer_show(NULL);
    }
}

// animation for task
static void _anim_chart_task(void *data)
{
    TickType_t tick = 0;
    int i = 0;
    
    // reset chart
    bsp_display_lock(0);
    _focus_time_series = lv_chart_add_series(_chart_focus_time, LV_COLOR_MAKE(100, 150, 200));
    lv_chart_set_type(_chart_focus_time, LV_CHART_TYPE_LINE);
    
    lv_obj_set_style_local_line_width(_chart_focus_time, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_size(_chart_focus_time, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, 6);
    
    for (int j = 0; j < 7; j++) {
        lv_chart_set_next(_chart_focus_time, _focus_time_series, 0);
    }
    
    // update stats with predefined data
    lv_label_set_text(_label_total_sessions, "Total Sessions:\n24");
    lv_label_set_text(_label_total_time, "Total Time:\n555 minutes");
    bsp_display_unlock();
    
    tick = xTaskGetTickCount();
    
    while (1) {
        // delete task if go to other UI
        if (ui_state_hide == ui_pomodoro_analytics_state) {
            vTaskDelete(NULL);
        }
        
        if (i < 7) {
            bsp_display_lock(0);
            lv_chart_set_point_id(_chart_focus_time, _focus_time_series, focus_time_data[i], i);
            lv_chart_refresh(_chart_focus_time);
            bsp_display_unlock();
            
            i++;
            vTaskDelayUntil(&tick, pdMS_TO_TICKS(200));
        } else {
            vTaskDelete(NULL);
        }
    }
}