
#include "ui_habit_tracker_today_view.h"
#include "ui_main.h"
#include "ui_habit_tracker_manage_habits.h"
#include "ui_habit_tracker_statistics.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>

/* LVGL objects */
static lv_obj_t *today_container = NULL;
static lv_obj_t *today_list = NULL;

// completion tracking
static bool *habit_completed = NULL;
static uint16_t max_habits = 0;

/* Forward declarations */
static void refresh_today_list(void);
static void checkbox_event_cb(lv_obj_t *obj, lv_event_t event);
static int get_current_day_of_week(void);
static bool is_habit_scheduled_today(const habit_t *habit, int day_of_week);
static int get_habit_time_value(const habit_t *habit);
static int compare_habits_by_time(const void *a, const void *b);

typedef struct {
    habit_t *habit;
    uint16_t index;
} habit_item_t;


void ui_today_view_init(void)
{
    lv_obj_t *obj_page = ui_page_get_obj();
    
    // initialize completion tracking
    max_habits = 10;
    habit_completed = (bool *)malloc(max_habits * sizeof(bool));
    if (habit_completed) {
        for (uint16_t i = 0; i < max_habits; i++) {
            habit_completed[i] = false;
        }
    }
    
    // cont
    today_container = lv_obj_create(obj_page, NULL);
    lv_obj_set_size(today_container, 520, 300);
    lv_obj_align(today_container, NULL, LV_ALIGN_IN_RIGHT_MID, -40, 30);
    lv_obj_set_hidden(today_container, true);
    
    lv_obj_t *title = lv_label_create(today_container, NULL);
    lv_label_set_text(title, "Today's Habits");
    lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
    lv_obj_set_style_local_text_font(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_20);
    
    // scrollable list -> vertical stacking
    today_list = lv_list_create(today_container, NULL);
    lv_obj_set_size(today_list, 480, 250);
    lv_obj_align(today_list, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_list_set_scrollbar_mode(today_list, LV_SCROLLBAR_MODE_AUTO);
}

void ui_today_view_show(void)
{
    if (today_container) {
        lv_obj_set_hidden(today_container, false);
        refresh_today_list();
    }
}

void ui_today_view_hide(void)
{
    if (today_container) {
        lv_obj_set_hidden(today_container, true);
    }
}

void ui_today_view_refresh(void)
{
    refresh_today_list();
}



// get current day of week (0=mon, 6=sun)
static int get_current_day_of_week(void)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    int day = tm_info->tm_wday;
    return (day == 0) ? 6 : day - 1;
}

// check if habit is scheduled for tdy
static bool is_habit_scheduled_today(const habit_t *habit, int day_of_week)
{
    if (!habit->is_active) return false;
    
    if (habit->frequency_type == 0) {
        // Weekly: check if today's day is selected
        return habit->days_selected[day_of_week];
    } else if (habit->frequency_type == 1) {
        // Custom: check if today's date matches any custom date
        return false;
    }
    
    return false;
}

// get time value for sorting (in min since midnight, -1 for "anytime")
static int get_habit_time_value(const habit_t *habit)
{
    switch (habit->time_option) {
        case 0: return -1;  // Anytime - will be sorted first
        case 1: return habit->hour * 60 + habit->minute;  // Specific time
        case 2: return 8 * 60;   // Morning (8:00 AM)
        case 3: return 14 * 60;  // Afternoon (2:00 PM)
        case 4: return 20 * 60;  // Night (8:00 PM)
        default: return -1;
    }
}

//qsort (completed goes to the bottom)
static int compare_habits_by_time(const void *a, const void *b)
{
    const habit_item_t *habit_a = (const habit_item_t *)a;
    const habit_item_t *habit_b = (const habit_item_t *)b;
    
    // Check completion status
    bool a_completed = (habit_a->index < max_habits) ? habit_completed[habit_a->index] : false;
    bool b_completed = (habit_b->index < max_habits) ? habit_completed[habit_b->index] : false;
    
    // Completed habits always go to bottom
    if (a_completed && !b_completed) return 1;
    if (!a_completed && b_completed) return -1;
    
    // If both have same completion status, sort by time
    int time_a = get_habit_time_value(habit_a->habit);
    int time_b = get_habit_time_value(habit_b->habit);
    
    if (time_a == -1 && time_b == -1) return 0;
    if (time_a == -1) return -1;
    if (time_b == -1) return 1;
    
    return time_a - time_b;
}

// time string for display
static void get_time_string(const habit_t *habit, char *buffer, size_t size)
{
    switch (habit->time_option) {
        case 0: snprintf(buffer, size, "Anytime"); break;
        case 1: snprintf(buffer, size, "%02d:%02d", habit->hour, habit->minute); break;
        case 2: snprintf(buffer, size, "Morning"); break;
        case 3: snprintf(buffer, size, "Afternoon"); break;
        case 4: snprintf(buffer, size, "Night"); break;
        default: snprintf(buffer, size, ""); break;
    }
}

// checkbox callback
static void checkbox_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED) {
        uint16_t habit_idx = (uint16_t)(uintptr_t)lv_obj_get_user_data(obj);
        bool checked = lv_checkbox_is_checked(obj);
        
        // Ensure we have space to track this habit
        if (habit_idx >= max_habits) {
            uint16_t new_max = habit_idx + 10;
            bool *new_completed = (bool *)realloc(habit_completed, new_max * sizeof(bool));
            if (new_completed) {
                for (uint16_t i = max_habits; i < new_max; i++) {
                    new_completed[i] = false;
                }
                habit_completed = new_completed;
                max_habits = new_max;
            }
        }
        
        // Track completion state
        if (habit_idx < max_habits) {
            habit_completed[habit_idx] = checked;
        }
        
        // Mark habit as completed/uncompleted (updates last_completed_date and streak)
        ui_manage_habits_mark_completed(habit_idx, checked);
        
        // Refresh the entire list to move completed habits to bottom
        refresh_today_list();
        
        // Refresh statistics view if it's visible
        ui_habit_tracker_statistics_refresh();
    }
}

// refresh tdy's hbait list
static void refresh_today_list(void)
{
    if (!today_list) return;
    
    lv_list_clean(today_list);
    
    int current_day = get_current_day_of_week();
    uint16_t total_habits = ui_manage_habits_get_count();
    
    if (total_habits == 0) {
        lv_obj_t *btn = lv_list_add_btn(today_list, NULL, "No habits scheduled for today!");
        lv_obj_set_style_local_text_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
        return;
    }
    
    habit_item_t *today_habits = malloc(sizeof(habit_item_t) * total_habits);
    uint16_t today_count = 0;
    
    if (!today_habits) return;
    
    // all habits scheduled for today
    for (uint16_t i = 0; i < total_habits; i++) {
        habit_t *habit = ui_manage_habits_get_habit(i);
        if (habit && is_habit_scheduled_today(habit, current_day)) {
            today_habits[today_count].habit = habit;
            today_habits[today_count].index = i;
            today_count++;
        }
    }
    
    // sort by time
    qsort(today_habits, today_count, sizeof(habit_item_t), compare_habits_by_time);
    
    // create list items for each habit
    for (uint16_t i = 0; i < today_count; i++) {
        habit_t *habit = today_habits[i].habit;
        uint16_t habit_idx = today_habits[i].index;
        
        // create list button as cont
        lv_obj_t *btn = lv_list_add_btn(today_list, NULL, "");
        lv_obj_set_height(btn, 70);
        lv_btn_set_layout(btn, LV_LAYOUT_OFF);
        
        // restore completion state if previously checked
        bool is_completed = (habit_idx < max_habits) ? habit_completed[habit_idx] : false;
        
        // checkbox
        lv_obj_t *cb = lv_checkbox_create(btn, NULL);
        lv_checkbox_set_text(cb, "");
        lv_obj_align(cb, NULL, LV_ALIGN_IN_LEFT_MID, 5, 0);
        lv_obj_set_user_data(cb, (void *)(uintptr_t)habit_idx);
        lv_obj_set_event_cb(cb, checkbox_event_cb);
        
        // set checkbox state
        if (is_completed) {
            lv_checkbox_set_checked(cb, true);
        }
        
        // time label
        char time_str[20];
        get_time_string(habit, time_str, sizeof(time_str));
        lv_obj_t *time_label = lv_label_create(btn, NULL);
        lv_label_set_text(time_label, time_str);
        lv_obj_align(time_label, cb, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
        lv_obj_set_style_local_text_color(time_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
        
        // habit name
        lv_obj_t *name_label = lv_label_create(btn, NULL);
        lv_label_set_text(name_label, habit->name);
        lv_obj_align(name_label, time_label, LV_ALIGN_OUT_RIGHT_MID, 15, -10);
        lv_obj_set_style_local_text_font(name_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_18);
        
        // description (if exists)
        if (strlen(habit->description) > 0) {
            lv_obj_t *desc_label = lv_label_create(btn, NULL);
            lv_label_set_text(desc_label, habit->description);
            lv_obj_align(desc_label, name_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 3);
            lv_obj_set_style_local_text_color(desc_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
            lv_obj_set_style_local_text_font(desc_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_18);
        }
        
        // streak counter
        char streak_str[30];
        snprintf(streak_str, sizeof(streak_str), LV_SYMBOL_CHARGE " %d", habit->streak_count);
        lv_obj_t *streak_label = lv_label_create(btn, NULL);
        lv_label_set_text(streak_label, streak_str);
        lv_obj_align(streak_label, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);
        lv_obj_set_user_data(streak_label, (void *)0xFFFFFFFF); // Mark as streak label
        
        if (habit->streak_count > 0) {
            lv_obj_set_style_local_text_color(streak_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
        } else {
            lv_obj_set_style_local_text_color(streak_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
        }
        
        // fade if completed
        if (is_completed) {
            lv_obj_set_style_local_text_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
        }
    }
    
    free(today_habits);
    
    // show message if no habits tdy
    if (today_count == 0) {
        lv_obj_t *btn = lv_list_add_btn(today_list, NULL, "No habits scheduled for today!");
        lv_obj_set_style_local_text_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    }
}

