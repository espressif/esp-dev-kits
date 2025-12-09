
#include "ui_habit_tracker_manage_habits.h"
#include "ui_main.h"
#include "ui_keyboard.h"
#include "ui_habit_tracker_today_view.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

// habit storage (dynamic)
#define INITIAL_HABITS_CAPACITY 10
static habit_t *habits = NULL;
static uint16_t habit_count = 0;
static uint16_t habits_capacity = 0;
static int16_t selected_habit_idx = -1; // -1: new habit

/* LVGL objects */
static lv_obj_t *list_container = NULL;
static lv_obj_t *habit_list = NULL;
static lv_obj_t *edit_panel = NULL;
static lv_obj_t *ta_habit_name = NULL;
static lv_obj_t *ta_habit_description = NULL;
static lv_obj_t *dd_frequency = NULL;
static lv_obj_t *day_buttons[7] = {NULL};
static lv_obj_t *label_days = NULL;
static lv_obj_t *dd_time_option = NULL;
static lv_obj_t *roller_hour = NULL;
static lv_obj_t *roller_minute = NULL;
static lv_obj_t *btn_save = NULL;
static lv_obj_t *btn_give_up = NULL;
static lv_obj_t *btn_add_new = NULL;
static lv_obj_t *_keyboard = NULL;
static lv_obj_t *calendar = NULL;
static lv_obj_t *calendar_popup = NULL;

// day selection state 
static bool day_selected[7] = {false};

// day labels 
static const char *day_labels[] = {"M", "T", "W", "T", "F", "S", "S"};

/* Static function forward declaration */
static void refresh_habit_list(void);
static void show_edit_panel(int16_t habit_idx);
static void hide_edit_panel(void);
static void save_current_habit(void);
static void delete_current_habit(void);
static void give_up_current_habit(void);
static void frequency_option_event_cb(lv_obj_t *obj, lv_event_t event);
static void calendar_close_event_cb(lv_obj_t *obj, lv_event_t event);
static void calendar_event_cb(lv_obj_t *obj, lv_event_t event);
static bool ensure_habits_capacity(void);

// calendar
#define MAX_CUSTOM_DATES 31
static lv_calendar_date_t selected_custom_dates[MAX_CUSTOM_DATES];
static uint16_t selected_custom_dates_count = 0;

static void habit_list_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        uint16_t idx = lv_list_get_btn_index(habit_list, obj);
        if (idx < habit_count) {
            show_edit_panel(idx);
        }
    }
}

static void add_new_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        show_edit_panel(-1);
    }
}

static void day_btn_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        for (int i = 0; i < 7; i++) {
            if (obj == day_buttons[i]) {
                day_selected[i] = !day_selected[i];
                if (day_selected[i]) {
                    lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
                } else {
                    lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
                }
                break;
            }
        }
    }
}

static void save_btn_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        save_current_habit();
        hide_edit_panel();
        refresh_habit_list();
        // update today's view when habits are modified
        ui_today_view_refresh();
    }
}

static void give_up_btn_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        if (selected_habit_idx >= 0 && selected_habit_idx < habit_count) {
            habits[selected_habit_idx].is_active = 0;
        }
        hide_edit_panel();
        refresh_habit_list();
        ui_today_view_refresh();
    }
}

static void cancel_edit_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        hide_edit_panel();
    }
}

static void ta_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_FOCUSED || event == LV_EVENT_CLICKED) {
        if (!_keyboard) {
            if (edit_panel) {
                lv_obj_align(edit_panel, NULL, LV_ALIGN_CENTER, 0, -80);
            }
            _keyboard = ui_keyboard_create(obj);
        }
    }
    else if (event == LV_EVENT_DEFOCUSED || event == LV_EVENT_APPLY) {
        if (_keyboard) {
            ui_keyboard_delete(_keyboard);
            _keyboard = NULL;
            
            if (edit_panel) {
                lv_obj_align(edit_panel, NULL, LV_ALIGN_CENTER, 0, 0);
            }
        }
    }
}

static void time_option_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED) {
        uint16_t option = lv_dropdown_get_selected(obj);
        if (option == 1) {
            lv_obj_set_hidden(roller_hour, false);
            lv_obj_set_hidden(roller_minute, false);
        } else {
            lv_obj_set_hidden(roller_hour, true);
            lv_obj_set_hidden(roller_minute, true);
        }
    }
}

static void frequency_option_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED) {
        uint16_t option = lv_dropdown_get_selected(obj);
        
        if (option == 1) {
            for (int i = 0; i < 7; i++) {
                lv_obj_set_hidden(day_buttons[i], true);
            }
            lv_label_set_text(label_days, "Select dates:");
            
            if (!calendar_popup) {
                calendar_popup = lv_obj_create(lv_scr_act(), NULL);
                lv_obj_set_size(calendar_popup, 380, 320);
                lv_obj_align(calendar_popup, NULL, LV_ALIGN_CENTER, 0, 0);
                
                lv_obj_t *cal_title = lv_label_create(calendar_popup, NULL);
                lv_label_set_text(cal_title, "Select Custom Dates");
                lv_obj_align(cal_title, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
                
                calendar = lv_calendar_create(calendar_popup, NULL);
                lv_obj_set_size(calendar, 320, 220);
                lv_obj_align(calendar, cal_title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
                
                // ignore "today" highlight (deafult) -> set date as invalid
                lv_calendar_date_t no_today = {.year = 1970, .month = 1, .day = 1};
                lv_calendar_set_today_date(calendar, &no_today);
                
                // highlilght selected dates
                lv_obj_set_style_local_bg_color(calendar, LV_CALENDAR_PART_DATE, LV_STATE_PRESSED, LV_COLOR_BLACK);
                lv_obj_set_style_local_bg_color(calendar, LV_CALENDAR_PART_DATE, LV_STATE_FOCUSED, LV_COLOR_BLACK);
                lv_obj_set_style_local_bg_color(calendar, LV_CALENDAR_PART_DATE, LV_STATE_CHECKED, LV_COLOR_BLACK);
                lv_obj_set_style_local_text_color(calendar, LV_CALENDAR_PART_DATE, LV_STATE_CHECKED, LV_COLOR_WHITE);
                
                lv_obj_set_event_cb(calendar, calendar_event_cb);
                
                lv_obj_t *btn_cal_close = lv_btn_create(calendar_popup, NULL);
                lv_obj_set_size(btn_cal_close, 100, 35);
                lv_obj_align(btn_cal_close, calendar, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
                lv_obj_set_event_cb(btn_cal_close, calendar_close_event_cb);
                lv_obj_t *label_close = lv_label_create(btn_cal_close, NULL);
                lv_label_set_text(label_close, "Done");
            } else {
                lv_obj_set_hidden(calendar_popup, false);
                lv_obj_move_foreground(calendar_popup);
            }
        } else {
            for (int i = 0; i < 7; i++) {
                lv_obj_set_hidden(day_buttons[i], false);
            }
            lv_label_set_text(label_days, "On days:");
        }
    }
}

static void calendar_close_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        if (calendar_popup) {
            lv_obj_set_hidden(calendar_popup, true);
            
            // update label to show how many dates were selected
            char buf[32];
            snprintf(buf, sizeof(buf), "Selected: %d dates", selected_custom_dates_count);
            lv_label_set_text(label_days, buf);
        }
    }
}

static void calendar_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_calendar_date_t *date = lv_calendar_get_pressed_date(obj);
        if (date) {
            // check if date alr selected
            bool found = false;
            int found_idx = -1;
            
            for (int i = 0; i < selected_custom_dates_count; i++) {
                if (selected_custom_dates[i].year == date->year &&
                    selected_custom_dates[i].month == date->month &&
                    selected_custom_dates[i].day == date->day) {
                    found = true;
                    found_idx = i;
                    break;
                }
            }
            
            if (found) {
                // remove date (deselect)
                for (int i = found_idx; i < selected_custom_dates_count - 1; i++) {
                    selected_custom_dates[i] = selected_custom_dates[i + 1];
                }
                selected_custom_dates_count--;
            } else {
                // add date (select)
                if (selected_custom_dates_count < MAX_CUSTOM_DATES) {
                    selected_custom_dates[selected_custom_dates_count] = *date;
                    selected_custom_dates_count++;
                }
            }
            
            // update calendar highlights
            if (selected_custom_dates_count > 0) {
                lv_calendar_set_highlighted_dates(calendar, selected_custom_dates, selected_custom_dates_count);
            } else {
                lv_calendar_set_highlighted_dates(calendar, NULL, 0);
            }
        }
    }
}

void ui_manage_habits_init(void)
{
    lv_obj_t *obj_page = ui_page_get_obj();
    
    // habit list cont
    list_container = lv_obj_create(obj_page, NULL);
    lv_obj_set_size(list_container, 520, 300);
    lv_obj_align(list_container, NULL, LV_ALIGN_IN_RIGHT_MID, -40, 30);
    lv_obj_set_hidden(list_container, true);
    
    // title
    lv_obj_t *list_title = lv_label_create(list_container, NULL);
    lv_label_set_text(list_title, "Your Habits");
    lv_obj_align(list_title, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
    
    // scrollable
    habit_list = lv_list_create(list_container, NULL);
    lv_obj_set_size(habit_list, 480, 200);
    lv_obj_align(habit_list, list_title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    
    // "Add New Habit" button 
    btn_add_new = lv_btn_create(list_container, NULL);
    lv_obj_set_size(btn_add_new, 460, 45);
    lv_obj_align(btn_add_new, habit_list, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_event_cb(btn_add_new, add_new_event_cb);
    lv_obj_t *label_add = lv_label_create(btn_add_new, NULL);
    lv_label_set_text(label_add, LV_SYMBOL_PLUS " Add New Habit");
    lv_obj_set_style_local_bg_color(btn_add_new, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(100, 200, 255));
    
    // edit panel 
    edit_panel = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(edit_panel, 500, 480);
    lv_obj_align(edit_panel, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_hidden(edit_panel, true);
    
    lv_obj_t *edit_title = lv_label_create(edit_panel, NULL);
    lv_label_set_text(edit_title, "Edit Habit");
    lv_obj_align(edit_title, NULL, LV_ALIGN_IN_TOP_MID, 0, 15);
    
    lv_obj_t *label_name = lv_label_create(edit_panel, NULL);
    lv_label_set_text(label_name, "Habit name:");
    lv_obj_align(label_name, edit_title, LV_ALIGN_OUT_BOTTOM_MID, -100, 20);
    
    ta_habit_name = lv_textarea_create(edit_panel, NULL);
    lv_obj_set_size(ta_habit_name, 280, 40);
    lv_obj_align(ta_habit_name, label_name, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_textarea_set_placeholder_text(ta_habit_name, "Enter habit name");
    lv_textarea_set_one_line(ta_habit_name, true);
    lv_obj_set_event_cb(ta_habit_name, ta_event_cb);
    
    lv_obj_t *label_desc = lv_label_create(edit_panel, NULL);
    lv_label_set_text(label_desc, "Description (optional):");
    lv_obj_align(label_desc, ta_habit_name, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    
    ta_habit_description = lv_textarea_create(edit_panel, NULL);
    lv_obj_set_size(ta_habit_description, 280, 40);
    lv_obj_align(ta_habit_description, label_desc, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_textarea_set_placeholder_text(ta_habit_description, "Add a description");
    lv_textarea_set_one_line(ta_habit_description, true);
    lv_obj_set_event_cb(ta_habit_description, ta_event_cb);
    
    lv_obj_t *label_freq = lv_label_create(edit_panel, NULL);
    lv_label_set_text(label_freq, "Frequency:");
    lv_obj_align(label_freq, ta_habit_description, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    
    dd_frequency = lv_dropdown_create(edit_panel, NULL);
    lv_dropdown_set_options(dd_frequency, "Weekly\nCustom");
    lv_obj_set_size(dd_frequency, 180, 40);
    lv_obj_align(dd_frequency, label_freq, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_set_event_cb(dd_frequency, frequency_option_event_cb);
    
    label_days = lv_label_create(edit_panel, NULL);
    lv_label_set_text(label_days, "On days:");
    lv_obj_align(label_days, label_freq, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    
    for (int i = 0; i < 7; i++) {
        day_buttons[i] = lv_btn_create(edit_panel, NULL);
        lv_obj_set_size(day_buttons[i], 45, 45);
        
        if (i == 0) {
            lv_obj_align(day_buttons[i], label_days, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
        } else {
            lv_obj_align(day_buttons[i], day_buttons[i-1], LV_ALIGN_OUT_RIGHT_MID, 8, 0);
        }
        
        lv_obj_set_event_cb(day_buttons[i], day_btn_event_cb);
        lv_obj_t *label_day = lv_label_create(day_buttons[i], NULL);
        lv_label_set_text(label_day, day_labels[i]);
        
        day_selected[i] = false;
        lv_obj_set_style_local_bg_color(day_buttons[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    }
    
    lv_obj_t *label_time = lv_label_create(edit_panel, NULL);
    lv_label_set_text(label_time, "Time:");
    lv_obj_align(label_time, day_buttons[0], LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    
    dd_time_option = lv_dropdown_create(edit_panel, NULL);
    lv_dropdown_set_options(dd_time_option, "No time\nSpecific time\nMorning\nAfternoon\nNight");
    lv_obj_set_size(dd_time_option, 180, 40);
    lv_obj_align(dd_time_option, label_time, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_set_event_cb(dd_time_option, time_option_event_cb);
    
    roller_hour = lv_roller_create(edit_panel, NULL);
    lv_roller_set_options(roller_hour, 
        "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n"
        "12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23", 
        LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(roller_hour, 3);
    lv_obj_set_size(roller_hour, 70, 90);
    lv_obj_align(roller_hour, dd_time_option, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_obj_set_hidden(roller_hour, true);
    
    roller_minute = lv_roller_create(edit_panel, NULL);
    lv_roller_set_options(roller_minute, "00\n15\n30\n45", LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(roller_minute, 3);
    lv_obj_set_size(roller_minute, 70, 90);
    lv_obj_align(roller_minute, roller_hour, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_set_hidden(roller_minute, true);

    lv_obj_t *btn_cancel = lv_btn_create(edit_panel, NULL);
    lv_obj_set_size(btn_cancel, 110, 45);
    lv_obj_align(btn_cancel, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -15);
    lv_obj_set_event_cb(btn_cancel, cancel_edit_event_cb);
    lv_obj_t *label_cancel = lv_label_create(btn_cancel, NULL);
    lv_label_set_text(label_cancel, "Cancel");

    btn_give_up = lv_btn_create(edit_panel, NULL);
    lv_obj_set_size(btn_give_up, 100, 40);
    lv_obj_align(btn_give_up, btn_cancel, LV_ALIGN_OUT_LEFT_MID, -20, 0);
    lv_obj_set_event_cb(btn_give_up, give_up_btn_event_cb);
    lv_obj_t *label_give_up = lv_label_create(btn_give_up, NULL);
    lv_label_set_text(label_give_up, "Give up");
    lv_obj_set_style_local_bg_color(btn_give_up, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(255, 100, 100));
    
    btn_save = lv_btn_create(edit_panel, NULL);
    lv_obj_set_size(btn_save, 110, 45);
    lv_obj_align(btn_save, btn_cancel, LV_ALIGN_OUT_RIGHT_MID, 20, 0); 
    lv_obj_set_event_cb(btn_save, save_btn_event_cb);
    lv_obj_t *label_save = lv_label_create(btn_save, NULL);
    lv_label_set_text(label_save, "Save");
    lv_obj_set_style_local_bg_color(btn_save, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(100, 255, 100));
    
    ui_manage_habits_add_predefined();
}

void ui_manage_habits_show(void)
{
    if (list_container) {
        lv_obj_set_hidden(list_container, false);
        refresh_habit_list();
    }
}

void ui_manage_habits_hide(void)
{
    if (_keyboard) {
        ui_keyboard_delete(_keyboard);
        _keyboard = NULL;
    }
    
    if (list_container) {
        lv_obj_set_hidden(list_container, true);
    }
    
    hide_edit_panel();
}

static bool ensure_habits_capacity(void)
{
    if (habits == NULL) {
        habits = (habit_t *)malloc(INITIAL_HABITS_CAPACITY * sizeof(habit_t));
        if (habits == NULL) return false;
        habits_capacity = INITIAL_HABITS_CAPACITY;
        return true;
    }
    
    if (habit_count >= habits_capacity) {
        uint16_t new_capacity = habits_capacity * 2;
        habit_t *new_habits = (habit_t *)realloc(habits, new_capacity * sizeof(habit_t));
        if (new_habits == NULL) return false;
        habits = new_habits;
        habits_capacity = new_capacity;
    }
    
    return true;
}

void ui_manage_habits_add_predefined(void)
{
    habit_t predefined[] = {
        {"Drink Water", "Stay hydrated throughout the day", 0, {1,1,1,1,1,1,1}, 7, 2, 9, 0, 1, 1, {{0}}, 0, 0, 0},
        {"Exercise", "Get moving and stay fit", 0, {1,0,1,0,1,0,0}, 3, 2, 7, 0, 1, 1, {{0}}, 0, 0, 0},
        {"Read Book", "Read for personal growth", 0, {1,1,1,1,1,0,0}, 5, 4, 20, 0, 1, 1, {{0}}, 0, 0, 0},
        {"Meditate", "Practice mindfulness daily", 0, {1,1,1,1,1,1,1}, 7, 2, 6, 0, 1, 1, {{0}}, 0, 0, 0},
    };
    
    int count = sizeof(predefined) / sizeof(predefined[0]);
    for (int i = 0; i < count; i++) {
        if (ensure_habits_capacity()) {
            memcpy(&habits[habit_count], &predefined[i], sizeof(habit_t));
            //  streak count = 0 for new habits
            habits[habit_count].streak_count = 0;
            habits[habit_count].last_completed_date = 0;
            habit_count++;
        }
    }
}

static void refresh_habit_list(void)
{
    lv_list_clean(habit_list);
    
    for (uint16_t i = 0; i < habit_count; i++) {
        if (habits[i].is_active) {
            lv_obj_t *btn = lv_list_add_btn(habit_list, LV_SYMBOL_EDIT, habits[i].name);
            lv_obj_set_event_cb(btn, habit_list_event_cb);
        }
    }
}

static void show_edit_panel(int16_t habit_idx)
{
    selected_habit_idx = habit_idx;
    
    if (habit_idx >= 0) {
        habit_t *h = &habits[habit_idx];
        lv_textarea_set_text(ta_habit_name, h->name);
        lv_textarea_set_text(ta_habit_description, h->description);
        lv_dropdown_set_selected(dd_frequency, h->frequency_type);
        
        // custom dates if freq selected is Custom
        if (h->frequency_type == 1) { // Custom
            selected_custom_dates_count = 0; // load from habit storage
        } else {
            selected_custom_dates_count = 0;
        }
        
        for (int i = 0; i < 7; i++) {
            day_selected[i] = h->days_selected[i];
            if (day_selected[i]) {
                lv_obj_set_style_local_bg_color(day_buttons[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
            } else {
                lv_obj_set_style_local_bg_color(day_buttons[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
            }
        }
        
        lv_dropdown_set_selected(dd_time_option, h->time_option);
        lv_roller_set_selected(roller_hour, h->hour, LV_ANIM_OFF);
        lv_roller_set_selected(roller_minute, h->minute / 15, LV_ANIM_OFF);
        
        // give up button only for existing habits
        lv_obj_set_hidden(btn_give_up, false);
    } else {
        // new habit
        lv_textarea_set_text(ta_habit_name, "");
        lv_textarea_set_text(ta_habit_description, "");
        lv_dropdown_set_selected(dd_frequency, 0);
        
        // reset custom dates for new habit
        selected_custom_dates_count = 0;
        
        for (int i = 0; i < 7; i++) {
            day_selected[i] = false;
            lv_obj_set_style_local_bg_color(day_buttons[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
        }
        
        lv_dropdown_set_selected(dd_time_option, 0);
        
        // hide give up button for new habits
        lv_obj_set_hidden(btn_give_up, true);
    }
    
    lv_obj_set_hidden(edit_panel, false);
    lv_obj_move_foreground(edit_panel);
}

static void hide_edit_panel(void)
{
    if (_keyboard) {
        ui_keyboard_delete(_keyboard);
        _keyboard = NULL;
    }
    
    lv_obj_set_hidden(edit_panel, true);
    selected_habit_idx = -1;
}

static void save_current_habit(void)
{
    habit_t *h;
    bool is_new_habit = false;
    
    if (selected_habit_idx >= 0) {
        h = &habits[selected_habit_idx];
    } else {
        if (!ensure_habits_capacity()) return;
        h = &habits[habit_count++];
        is_new_habit = true;
    }
    
    const char *text = lv_textarea_get_text(ta_habit_name);
    strncpy(h->name, text, sizeof(h->name) - 1);
    h->name[sizeof(h->name) - 1] = '\0';
    
    const char *desc_text = lv_textarea_get_text(ta_habit_description);
    strncpy(h->description, desc_text, sizeof(h->description) - 1);
    h->description[sizeof(h->description) - 1] = '\0';
    
    h->frequency_type = lv_dropdown_get_selected(dd_frequency);
    
    // save custom dates if frequency is Custom
    if (h->frequency_type == 1) { // Custom
        // store custom dates count
        h->num_days = selected_custom_dates_count;
        
        // copy selected custom dates to habit structure
        for (int i = 0; i < selected_custom_dates_count && i < 31; i++) {
            h->custom_dates[i].year = selected_custom_dates[i].year;
            h->custom_dates[i].month = selected_custom_dates[i].month;
            h->custom_dates[i].day = selected_custom_dates[i].day;
        }
    } else {
        // weekly frequency
        h->num_days = 0;
        for (int i = 0; i < 7; i++) {
            h->days_selected[i] = day_selected[i];
            if (day_selected[i]) {
                h->num_days++;
            }
        }
    }
    
    h->time_option = lv_dropdown_get_selected(dd_time_option);
    h->hour = lv_roller_get_selected(roller_hour);
    h->minute = lv_roller_get_selected(roller_minute) * 15;
    h->show_in_weekly_todo = 1;
    h->is_active = 1;
    
    if (is_new_habit) {
        h->streak_count = 0;
        h->last_completed_date = 0;
    }
}

uint16_t ui_manage_habits_get_count(void)
{
    return habit_count;
}

habit_t* ui_manage_habits_get_habit(uint16_t index)
{
    if (index < habit_count) {
        return &habits[index];
    }
    return NULL;
}

void ui_manage_habits_mark_completed(uint16_t index, bool completed)
{
    if (index < habit_count) {
        habit_t *h = &habits[index];
        time_t now = time(NULL);
        
        if (completed) {
            // check for consecutive completion
            time_t last_completion = (time_t)h->last_completed_date;
            
            // calc days difference
            int days_diff = (now - last_completion) / (24 * 60 * 60);
            
            // if completed within expected frequency, increment streak
            if (days_diff <= 7) { // weekly habits
                h->streak_count++;
            } else {
                // reset streak if too long since last completion
                h->streak_count = 1;
            }
            
            h->last_completed_date = (uint32_t)now;
        }
    }
}