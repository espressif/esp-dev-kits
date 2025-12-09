
#include "ui_main.h"
#include "ui_habit_tracker_manage_habits.h"
#include "bsp_board_extra.h"
#include "ui_habit_tracker_today_view.h"
#include "ui_habit_tracker_statistics.h"

/* UI function declaration */
ui_func_desc_t ui_habit_tracker_func = {
    .name = "Habit Tracker",
    .init = ui_habit_tracker_init,
    .show = ui_habit_tracker_show,
    .hide = ui_habit_tracker_hide,
};

static ui_state_t ui_habit_tracker_state = ui_state_dis;

/* LVGL objects definition */
static lv_obj_t *obj_page_habit = NULL;
static lv_obj_t *_btn_today = NULL;
static lv_obj_t *_btn_statistics = NULL;
static lv_obj_t *_btn_manage_habits = NULL;

/* Static function forward declaration */
static void _btn_cb(lv_obj_t *obj, lv_event_t event);

void ui_habit_tracker_init(void *data)
{
    ui_page_show("Habit Tracker");
    obj_page_habit = ui_page_get_obj();

    // sidebar button
    _btn_today = lv_btn_create(obj_page_habit, NULL);
    lv_obj_set_size(_btn_today, 150, 50);
    lv_obj_align(_btn_today, NULL, LV_ALIGN_IN_LEFT_MID, 0, -90);
    lv_obj_set_event_cb(_btn_today, _btn_cb);
    lv_obj_t *label_today = lv_label_create(_btn_today, NULL);
    lv_label_set_text(label_today, "Today");

    _btn_statistics = lv_btn_create(obj_page_habit, NULL);
    lv_obj_set_size(_btn_statistics, 150, 50);
    lv_obj_align(_btn_statistics, _btn_today, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_event_cb(_btn_statistics, _btn_cb);
    lv_obj_t *label_statistics = lv_label_create(_btn_statistics, NULL);
    lv_label_set_text(label_statistics, "Statistics");

    _btn_manage_habits = lv_btn_create(obj_page_habit, NULL);
    lv_obj_set_size(_btn_manage_habits, 150, 50);
    lv_obj_align(_btn_manage_habits, _btn_statistics, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_event_cb(_btn_manage_habits, _btn_cb);
    lv_obj_t *label_manage = lv_label_create(_btn_manage_habits, NULL);
    lv_label_set_text(label_manage, "Manage Habits");

    ui_manage_habits_init();
    ui_today_view_init();
    ui_habit_tracker_statistics_init();

    ui_habit_tracker_state = ui_state_show;
}

void ui_habit_tracker_show(void *data)
{
    if (ui_state_dis == ui_habit_tracker_state) {
        ui_habit_tracker_init(data);
        ui_today_view_show();
    } else if (ui_state_hide == ui_habit_tracker_state) {
        lv_obj_set_hidden(_btn_today, false);
        lv_obj_set_hidden(_btn_statistics, false);
        lv_obj_set_hidden(_btn_manage_habits, false);

        ui_page_show("Habit Tracker");
        ui_today_view_show();
        ui_habit_tracker_state = ui_state_show;
    }
}

void ui_habit_tracker_hide(void *data)
{
    if (ui_state_show == ui_habit_tracker_state) {
        lv_obj_set_hidden(_btn_today, true);
        lv_obj_set_hidden(_btn_statistics, true);
        lv_obj_set_hidden(_btn_manage_habits, true);
        
        ui_manage_habits_hide();
        ui_today_view_hide();
        ui_habit_tracker_statistics_hide();
        
        ui_page_hide(NULL);
        ui_habit_tracker_state = ui_state_hide;
    }
}

static void _btn_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        if (obj == _btn_today) {
            // hide other views
            ui_manage_habits_hide();
            ui_habit_tracker_statistics_hide();
            
            // show todays view
            ui_today_view_show();
            
        } else if (obj == _btn_statistics) {
            // hide other views
            ui_manage_habits_hide();
            ui_today_view_hide();
            
            // show stats view
            ui_habit_tracker_statistics_show();
            
        } else if (obj == _btn_manage_habits) {
            // hide other views
            ui_today_view_hide();
            ui_habit_tracker_statistics_hide();
            
            // show manage habits view
            ui_manage_habits_show();
        }
    }
}