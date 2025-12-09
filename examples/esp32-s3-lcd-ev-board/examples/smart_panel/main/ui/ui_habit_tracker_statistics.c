#include "ui_habit_tracker_statistics.h"
#include "ui_main.h"
#include "ui_habit_tracker_manage_habits.h"
#include <string.h>
#include <time.h>
#include <math.h>

/* LVGL objects definition */
static lv_obj_t *stats_container = NULL;
static lv_obj_t *view_tabs = NULL;
static lv_obj_t *content_area = NULL;
// container objects
static lv_obj_t *overview_view = NULL;
static lv_obj_t *trends_view = NULL;
static lv_obj_t *habits_view = NULL;
// currently displayed view
static stats_view_type_t current_view = STATS_VIEW_OVERVIEW;
// month/year navigation
static int displayed_month = -1;  // -1 = current month
static int displayed_year = -1;   // -1 = current year

/* Static function forward declaration */
static void create_overview_view(void);
static void create_trends_view(void);
static void create_habits_view(void);
static void tab_event_cb(lv_obj_t *obj, lv_event_t event);
static void calculate_statistics(uint16_t *completed, uint16_t *started, uint16_t *not_done, uint16_t *completion_rate);
static void draw_circular_progress(lv_obj_t *parent, int16_t x, int16_t y, uint16_t percentage);

void ui_habit_tracker_statistics_init(void)
{
    lv_obj_t *obj_page = ui_page_get_obj();
    
    stats_container = lv_obj_create(obj_page, NULL);
    lv_obj_set_size(stats_container, 520, 300);
    lv_obj_align(stats_container, NULL, LV_ALIGN_IN_RIGHT_MID, -40, 30);
    lv_obj_set_hidden(stats_container, true);
    
    // statistics title label
    lv_obj_t *main_title = lv_label_create(stats_container, NULL);
    lv_label_set_text(main_title, "Statistics");
    lv_obj_align(main_title, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
    lv_obj_set_style_local_text_font(main_title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_20);
    
    // tab button matrix for view selection
    static const char *tab_map[] = {"Overview", "Trends", "Habits", ""};
    view_tabs = lv_btnmatrix_create(stats_container, NULL);
    lv_btnmatrix_set_map(view_tabs, tab_map);
    lv_obj_set_size(view_tabs, 480, 35);
    lv_obj_align(view_tabs, main_title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_event_cb(view_tabs, tab_event_cb);
    lv_btnmatrix_set_btn_ctrl(view_tabs, 0, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_btnmatrix_set_one_check(view_tabs, true);
    
    // content area for different views
    content_area = lv_obj_create(stats_container, NULL);
    lv_obj_set_size(content_area, 480, 230);
    lv_obj_align(content_area, view_tabs, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_obj_set_style_local_bg_opa(content_area, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_border_width(content_area, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    
    create_overview_view();
    create_trends_view();
    create_habits_view();
}

void ui_habit_tracker_statistics_show(void)
{
    if (stats_container) {
        lv_obj_set_hidden(stats_container, false);
        ui_habit_tracker_statistics_refresh();
    }
}

void ui_habit_tracker_statistics_hide(void)
{
    if (stats_container) {
        lv_obj_set_hidden(stats_container, true);
    }
}

void ui_habit_tracker_statistics_refresh(void)
{   
    // delete existing views
    if (overview_view) {
        lv_obj_del(overview_view);
        overview_view = NULL;
    }
    if (trends_view) {
        lv_obj_del(trends_view);
        trends_view = NULL;
    }
    if (habits_view) {
        lv_obj_del(habits_view);
        habits_view = NULL;
    }
    
    // recreate all views
    create_overview_view();
    create_trends_view();
    create_habits_view();
    
    // show current view based on selection
    switch (current_view) {
        case STATS_VIEW_OVERVIEW:
            if (overview_view) lv_obj_set_hidden(overview_view, false);
            break;
        case STATS_VIEW_TRENDS:
            if (trends_view) lv_obj_set_hidden(trends_view, false);
            break;
        case STATS_VIEW_HABITS:
            if (habits_view) lv_obj_set_hidden(habits_view, false);
            break;
    }
}

void ui_habit_tracker_statistics_set_view(stats_view_type_t view)
{
    current_view = view;
    
    // hide all views
    if (overview_view) lv_obj_set_hidden(overview_view, true);
    if (trends_view) lv_obj_set_hidden(trends_view, true);
    if (habits_view) lv_obj_set_hidden(habits_view, true);
    
    // show selected view
    switch (view) {
        case STATS_VIEW_OVERVIEW:
            if (overview_view) lv_obj_set_hidden(overview_view, false);
            if (view_tabs) {
                lv_btnmatrix_clear_btn_ctrl_all(view_tabs, LV_BTNMATRIX_CTRL_CHECK_STATE);
                lv_btnmatrix_set_btn_ctrl(view_tabs, 0, LV_BTNMATRIX_CTRL_CHECK_STATE);
            }
            break;
        case STATS_VIEW_TRENDS:
            if (trends_view) lv_obj_set_hidden(trends_view, false);
            if (view_tabs) {
                lv_btnmatrix_clear_btn_ctrl_all(view_tabs, LV_BTNMATRIX_CTRL_CHECK_STATE);
                lv_btnmatrix_set_btn_ctrl(view_tabs, 1, LV_BTNMATRIX_CTRL_CHECK_STATE);
            }
            break;
        case STATS_VIEW_HABITS:
            if (habits_view) lv_obj_set_hidden(habits_view, false);
            if (view_tabs) {
                lv_btnmatrix_clear_btn_ctrl_all(view_tabs, LV_BTNMATRIX_CTRL_CHECK_STATE);
                lv_btnmatrix_set_btn_ctrl(view_tabs, 2, LV_BTNMATRIX_CTRL_CHECK_STATE);
            }
            break;
    }
}

// month/year navigation button callbacks
static void prev_month_btn_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        if (displayed_month == -1) {
            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            displayed_month = tm_info->tm_mon;
            displayed_year = tm_info->tm_year + 1900;
        }
        
        displayed_month--;
        if (displayed_month < 0) {
            displayed_month = 11;
            displayed_year--;
        }
        
        // refresh trends view with new month data
        ui_habit_tracker_statistics_refresh();
    }
}

static void next_month_btn_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        if (displayed_month == -1) {
            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            displayed_month = tm_info->tm_mon;
            displayed_year = tm_info->tm_year + 1900;
        }
        
        displayed_month++;
        if (displayed_month > 11) {
            displayed_month = 0;
            displayed_year++;
        }
        
        // refresh trends view with new month data
        ui_habit_tracker_statistics_refresh();
    }
}

// tab button event callback handler
static void tab_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        uint16_t tab_id = lv_btnmatrix_get_active_btn(obj);
        
        // clear all button states
        lv_btnmatrix_clear_btn_ctrl_all(obj, LV_BTNMATRIX_CTRL_CHECK_STATE);
        
        // hide all views
        if (overview_view) lv_obj_set_hidden(overview_view, true);
        if (trends_view) lv_obj_set_hidden(trends_view, true);
        if (habits_view) lv_obj_set_hidden(habits_view, true);
        
        // show selected view and update button state
        switch (tab_id) {
            case 0: // overview
                current_view = STATS_VIEW_OVERVIEW;
                if (overview_view) lv_obj_set_hidden(overview_view, false);
                lv_btnmatrix_set_btn_ctrl(obj, 0, LV_BTNMATRIX_CTRL_CHECK_STATE);
                break;
            case 1: // trends
                current_view = STATS_VIEW_TRENDS;
                if (trends_view) lv_obj_set_hidden(trends_view, false);
                lv_btnmatrix_set_btn_ctrl(obj, 1, LV_BTNMATRIX_CTRL_CHECK_STATE);
                break;
            case 2: // habits
                current_view = STATS_VIEW_HABITS;
                if (habits_view) lv_obj_set_hidden(habits_view, false);
                lv_btnmatrix_set_btn_ctrl(obj, 2, LV_BTNMATRIX_CTRL_CHECK_STATE);
                break;
        }
    }
}

// helper: get current day of week (0=mon, 6=sun)
static int get_current_day_of_week(void)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    int day = tm_info->tm_wday;
    return (day == 0) ? 6 : day - 1;
}

// helper: check if habit is scheduled for today
static bool is_habit_scheduled_today(const habit_t *habit, int day_of_week)
{
    if (!habit->is_active) return false;
    
    if (habit->frequency_type == 0) {
        // weekly: check if today's day is selected
        return habit->days_selected[day_of_week];
    } else if (habit->frequency_type == 1) {
        // custom: check if today's date matches any custom date
        return false;
    }
    
    return false;
}

//overall stats - today's completion rate
static void calculate_statistics(uint16_t *completed, uint16_t *started, uint16_t *not_done, uint16_t *completion_rate)
{
    *completed = 0;
    *started = 0;
    *not_done = 0;
    
    // get current time
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    int current_hour = current_time->tm_hour;
    int current_minute = current_time->tm_min;
    
    // get today's date (start of day at 00:00:00)
    struct tm *timeinfo = localtime(&now);
    timeinfo->tm_hour = 0;
    timeinfo->tm_min = 0;
    timeinfo->tm_sec = 0;
    time_t today_start = mktime(timeinfo);
    
    // get current day of week for filtering
    int current_day = get_current_day_of_week();
    
    uint16_t total_habits = ui_manage_habits_get_count();
    uint16_t total_today_habits = 0; // habits that should be done today
    
    for (uint16_t i = 0; i < total_habits; i++) {
        habit_t *habit = ui_manage_habits_get_habit(i);
        
        // Only count habits scheduled for today
        if (habit && is_habit_scheduled_today(habit, current_day)) {
            total_today_habits++;
            
            // check if completed today (last_completed_date is on or after today 00:00:00)
            if (habit->last_completed_date > 0 && habit->last_completed_date >= today_start) {
                (*completed)++;
            } 
            // not completed today - check if scheduled time has passed
            else {
                bool time_has_passed = false;
                
                // Check if scheduled time has passed based on time_option
                switch (habit->time_option) {
                    case 0: // Anytime - never count as missed until end of day
                        time_has_passed = false;
                        break;
                    case 1: // Specific time
                        if (current_hour > habit->hour || 
                            (current_hour == habit->hour && current_minute > habit->minute)) {
                            time_has_passed = true;
                        }
                        break;
                    case 2: // Morning (8:00 AM)
                        if (current_hour > 8 || (current_hour == 8 && current_minute > 0)) {
                            time_has_passed = true;
                        }
                        break;
                    case 3: // Afternoon (2:00 PM = 14:00)
                        if (current_hour > 14 || (current_hour == 14 && current_minute > 0)) {
                            time_has_passed = true;
                        }
                        break;
                    case 4: // Night (8:00 PM = 20:00)
                        if (current_hour > 20 || (current_hour == 20 && current_minute > 0)) {
                            time_has_passed = true;
                        }
                        break;
                    default:
                        time_has_passed = false;
                        break;
                }
                
                if (time_has_passed) {
                    // Scheduled time passed but not completed
                    (*started)++;
                } else {
                    // Still have time to complete it today
                    (*not_done)++;
                }
            }
        }
    }
    
    // completion rate = habits completed today / habits scheduled for today
    if (total_today_habits > 0) {
        *completion_rate = (*completed * 100) / total_today_habits;
    } else {
        *completion_rate = 0;
    }
}

// circular progress indicator
static void draw_circular_progress(lv_obj_t *parent, int16_t x, int16_t y, uint16_t percentage)
{
    // cont for circle
    lv_obj_t *circle_cont = lv_obj_create(parent, NULL);
    lv_obj_set_size(circle_cont, 180, 180);
    lv_obj_align(circle_cont, NULL, LV_ALIGN_IN_TOP_MID, x, y);
    lv_obj_set_style_local_bg_opa(circle_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_border_width(circle_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    
    // background arc (gray)
    lv_obj_t *arc_bg = lv_arc_create(circle_cont, NULL);
    lv_obj_set_size(arc_bg, 160, 160);
    lv_obj_align(arc_bg, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_arc_set_bg_angles(arc_bg, 0, 360);
    lv_arc_set_angles(arc_bg, 0, 360);
    lv_obj_set_style_local_line_width(arc_bg, LV_ARC_PART_BG, LV_STATE_DEFAULT, 15);
    lv_obj_set_style_local_line_color(arc_bg, LV_ARC_PART_BG, LV_STATE_DEFAULT, LV_COLOR_MAKE(230, 230, 230));
    lv_obj_set_style_local_line_width(arc_bg, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 0);
    
    // progress arc (colored)
    lv_obj_t *arc_progress = lv_arc_create(circle_cont, NULL);
    lv_obj_set_size(arc_progress, 160, 160);
    lv_obj_align(arc_progress, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_arc_set_bg_angles(arc_progress, 0, 360);
    uint16_t end_angle = (percentage * 360) / 100;
    lv_arc_set_angles(arc_progress, 270, 270 + end_angle);
    lv_obj_set_style_local_line_width(arc_progress, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 15);
    lv_obj_set_style_local_line_color(arc_progress, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_MAKE(0, 150, 255));
    lv_obj_set_style_local_line_width(arc_progress, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);
    
    // percetnage txt centre
    lv_obj_t *label_percent = lv_label_create(circle_cont, NULL);
    char buf[10];
    snprintf(buf, sizeof(buf), "%d%%", percentage);
    lv_label_set_text(label_percent, buf);
    lv_obj_align(label_percent, NULL, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_style_local_text_font(label_percent, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    
    // "Completion Rate" text
    lv_obj_t *label_text = lv_label_create(circle_cont, NULL);
    lv_label_set_text(label_text, "COMPLETION\nRATE");
    lv_obj_align(label_text, label_percent, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_obj_set_style_local_text_color(label_text, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_label_set_align(label_text, LV_LABEL_ALIGN_CENTER);
}

// helper: get monthly completion data
static void get_monthly_completion_data(int *completion_rates, int *days_in_month)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    // get current month and year
    int month = tm_info->tm_mon;
    int year = tm_info->tm_year + 1900;
    
    // determine days in current month
    int month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 1 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
        month_days[1] = 29; // leap year
    }
    *days_in_month = month_days[month];
    
    // ** get daily completion rates from cloud database for the selected month/year
    
    // predefined sample data - replace with actual completion rates from database
    int sample_rates[] = {0, 60, 85, 70, 100, 50, 75, 90, 80, 65, 100, 75, 85, 95, 70, 
                          80, 100, 65, 75, 90, 85, 70, 100, 80, 75, 65, 100, 90, 85, 70, 0};
    
    for (int i = 0; i < *days_in_month; i++) {
        completion_rates[i] = sample_rates[i];
    }
}

// Helper function: Draw pie chart style indicator for completion rate
static void draw_completion_indicator(lv_obj_t *parent, int16_t x, int16_t y, int completion_rate)
{
    // Background circle (light gray, unfilled portion)
    lv_obj_t *circle_bg = lv_arc_create(parent, NULL);
    lv_obj_set_size(circle_bg, 22, 22);
    lv_obj_align(circle_bg, NULL, LV_ALIGN_IN_TOP_LEFT, x, y);
    lv_arc_set_bg_angles(circle_bg, 0, 360);
    lv_arc_set_angles(circle_bg, 0, 360);
    lv_obj_set_style_local_line_width(circle_bg, LV_ARC_PART_BG, LV_STATE_DEFAULT, 22);
    lv_obj_set_style_local_line_color(circle_bg, LV_ARC_PART_BG, LV_STATE_DEFAULT, LV_COLOR_MAKE(220, 220, 220));
    lv_obj_set_style_local_line_width(circle_bg, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 0);
    
    // Completion rate arc (filled portion - blue)
    lv_obj_t *circle_fill = lv_arc_create(parent, NULL);
    lv_obj_set_size(circle_fill, 22, 22);
    lv_obj_align(circle_fill, NULL, LV_ALIGN_IN_TOP_LEFT, x, y);
    lv_arc_set_bg_angles(circle_fill, 0, 360);
    
    // Calculate angle based on completion percentage (0-360 degrees)
    // Start from top (270 degrees) and go clockwise
    uint16_t end_angle = (completion_rate * 360) / 100;
    lv_arc_set_angles(circle_fill, 270, 270 + end_angle);
    
    lv_obj_set_style_local_line_width(circle_fill, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 22);
    
    // Color based on completion rate
    lv_color_t fill_color;
    if (completion_rate == 0) {
        fill_color = LV_COLOR_MAKE(200, 200, 200);
    } else if (completion_rate <= 25) {
        fill_color = LV_COLOR_MAKE(255, 150, 150);  // Light red
    } else if (completion_rate <= 50) {
        fill_color = LV_COLOR_MAKE(255, 200, 100);  // Yellow
    } else if (completion_rate <= 75) {
        fill_color = LV_COLOR_MAKE(150, 200, 255);  // Light blue
    } else {
        fill_color = LV_COLOR_MAKE(0, 120, 215);    // Dark blue
    }
    
    lv_obj_set_style_local_line_color(circle_fill, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, fill_color);
    lv_obj_set_style_local_line_width(circle_fill, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);
}

// trends view - Monthly completion calendar
static void create_trends_view(void)
{
    // scrollable container
    trends_view = lv_page_create(content_area, NULL);
    lv_obj_set_size(trends_view, 480, 230);
    lv_obj_align(trends_view, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_hidden(trends_view, true);
    lv_page_set_scrl_layout(trends_view, LV_LAYOUT_COLUMN_LEFT);
    
    // Get current or displayed month/year
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    int current_month = tm_info->tm_mon;
    int current_year = tm_info->tm_year + 1900;
    
    // Use displayed month/year if set, otherwise use current
    int display_month = (displayed_month == -1) ? current_month : displayed_month;
    int display_year = (displayed_year == -1) ? current_year : displayed_year;
    
    // Create a tm struct for the displayed month
    struct tm display_tm = *tm_info;
    display_tm.tm_mon = display_month;
    display_tm.tm_year = display_year - 1900;
    display_tm.tm_mday = 1;
    mktime(&display_tm);
    
    // Title bar with navigation
    lv_obj_t *title_cont = lv_obj_create(trends_view, NULL);
    lv_obj_set_size(title_cont, 450, 35);
    lv_obj_set_style_local_bg_opa(title_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_border_width(title_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    
    // Previous month button
    lv_obj_t *prev_btn = lv_btn_create(title_cont, NULL);
    lv_obj_set_size(prev_btn, 35, 30);
    lv_obj_align(prev_btn, NULL, LV_ALIGN_IN_LEFT_MID, 5, 0);
    lv_obj_set_event_cb(prev_btn, prev_month_btn_cb);
    
    lv_obj_t *prev_label = lv_label_create(prev_btn, NULL);
    lv_label_set_text(prev_label, "<");
    lv_obj_set_style_local_text_font(prev_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_20);
    
    // Month/Year title
    lv_obj_t *title = lv_label_create(title_cont, NULL);
    char title_buf[30];
    const char *months[] = {"January", "February", "March", "April", "May", "June",
                            "July", "August", "September", "October", "November", "December"};
    snprintf(title_buf, sizeof(title_buf), "%s %d", months[display_month], display_year);
    lv_label_set_text(title, title_buf);
    lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_MID, 0, 5);
    lv_obj_set_style_local_text_font(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_20);
    
    // Next month button
    lv_obj_t *next_btn = lv_btn_create(title_cont, NULL);
    lv_obj_set_size(next_btn, 35, 30);
    lv_obj_align(next_btn, NULL, LV_ALIGN_IN_RIGHT_MID, -5, 0);
    lv_obj_set_event_cb(next_btn, next_month_btn_cb);
    
    lv_obj_t *next_label = lv_label_create(next_btn, NULL);
    lv_label_set_text(next_label, ">");
    lv_obj_set_style_local_text_font(next_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_20);
    
    // calendar container
    lv_obj_t *calendar_cont = lv_obj_create(trends_view, NULL);
    lv_obj_set_size(calendar_cont, 450, 145);
    lv_obj_set_style_local_bg_opa(calendar_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_border_width(calendar_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    
    // Day labels (Su, Mo, Tu, We, Th, Fr, Sa)
    const char *day_names[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
    int col_spacing = 62;
    int start_x = 2;
    
    for (int i = 0; i < 7; i++) {
        lv_obj_t *day_label = lv_label_create(calendar_cont, NULL);
        lv_label_set_text(day_label, day_names[i]);
        lv_obj_align(day_label, NULL, LV_ALIGN_IN_TOP_LEFT, start_x + i * col_spacing, 3);
        lv_obj_set_style_local_text_font(day_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
        lv_obj_set_style_local_text_color(day_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    }
    
    // Get monthly completion data
    int completion_rates[31];
    int days_in_month;
    get_monthly_completion_data(completion_rates, &days_in_month);
    
    // Get first day of month
    int first_day_of_week = display_tm.tm_wday;
    
    // Draw calendar days
    int row = 0;
    int col = first_day_of_week;
    
    for (int day = 1; day <= days_in_month; day++) {
        int x = start_x + col * col_spacing;
        int y = 25 + row * 25;
        
        draw_completion_indicator(calendar_cont, x, y, completion_rates[day - 1]);
        
        // Add day number as small text
        lv_obj_t *day_num = lv_label_create(calendar_cont, NULL);
        char day_buf[3];
        snprintf(day_buf, sizeof(day_buf), "%d", day);
        lv_label_set_text(day_num, day_buf);
        lv_obj_align(day_num, NULL, LV_ALIGN_IN_TOP_LEFT, x + 24, y + 3);
        lv_obj_set_style_local_text_font(day_num, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
        lv_obj_set_style_local_text_color(day_num, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
        
        // Move to next position
        col++;
        if (col >= 7) {
            col = 0;
            row++;
        }
    }
}

//indv habits view
static void create_habits_view(void)
{
    habits_view = lv_page_create(content_area, NULL);
    lv_obj_set_size(habits_view, 480, 230);
    lv_obj_align(habits_view, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_hidden(habits_view, true);
    
    lv_obj_t *title = lv_label_create(habits_view, NULL);
    lv_label_set_text(title, "Habit Details");
    lv_obj_set_style_local_text_font(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_20);
    
    lv_obj_t *habit_list = lv_list_create(habits_view, NULL);
    lv_obj_set_size(habit_list, 460, 190);
    
    uint16_t total_habits = ui_manage_habits_get_count();
    for (uint16_t i = 0; i < total_habits; i++) {
        habit_t *habit = ui_manage_habits_get_habit(i);
        if (habit && habit->is_active) {
            lv_obj_t *btn = lv_list_add_btn(habit_list, LV_SYMBOL_CHARGE, "");
            lv_obj_set_height(btn, 55);
            lv_btn_set_layout(btn, LV_LAYOUT_OFF);
            
            lv_obj_t *name = lv_label_create(btn, NULL);
            lv_label_set_text(name, habit->name);
            lv_obj_align(name, NULL, LV_ALIGN_IN_LEFT_MID, 40, -8);
            lv_obj_set_style_local_text_font(name, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_18);
            
            char stats_buf[50];
            snprintf(stats_buf, sizeof(stats_buf), "Streak: %d days", habit->streak_count);
            lv_obj_t *stats = lv_label_create(btn, NULL);
            lv_label_set_text(stats, stats_buf);
            lv_obj_align(stats, name, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);
            lv_obj_set_style_local_text_color(stats, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
            lv_obj_set_style_local_text_font(stats, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_18);
            
            lv_obj_t *bar = lv_bar_create(btn, NULL);
            lv_obj_set_size(bar, 90, 8);
            lv_obj_align(bar, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);
            uint16_t completion = (habit->streak_count > 0) ? (habit->streak_count * 10 > 100 ? 100 : habit->streak_count * 10) : 0;
            lv_bar_set_value(bar, completion, LV_ANIM_OFF);
        }
    }
}

// overview view 
static void create_overview_view(void)
{
    overview_view = lv_list_create(content_area, NULL);
    lv_obj_set_size(overview_view, 480, 230);
    lv_obj_align(overview_view, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_opa(overview_view, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_border_width(overview_view, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_list_set_scrollbar_mode(overview_view, LV_SCROLLBAR_MODE_AUTO);

    // calc stats
    uint16_t completed, started, not_done, completion_rate;
    calculate_statistics(&completed, &started, &not_done, &completion_rate);

    // completion rate 
    lv_obj_t *rate_btn = lv_list_add_btn(overview_view, NULL, "");
    lv_obj_set_height(rate_btn, 200);
    lv_btn_set_layout(rate_btn, LV_LAYOUT_OFF);
    
    // circular progress
    lv_obj_t *circle_cont = lv_obj_create(rate_btn, NULL);
    lv_obj_set_size(circle_cont, 180, 180);
    lv_obj_set_style_local_bg_opa(circle_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_border_width(circle_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(circle_cont, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *arc_bg = lv_arc_create(circle_cont, NULL);
    lv_obj_set_size(arc_bg, 160, 160);
    lv_obj_align(arc_bg, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_arc_set_bg_angles(arc_bg, 0, 360);
    lv_arc_set_angles(arc_bg, 0, 360);
    lv_obj_set_style_local_line_width(arc_bg, LV_ARC_PART_BG, LV_STATE_DEFAULT, 15);
    lv_obj_set_style_local_line_color(arc_bg, LV_ARC_PART_BG, LV_STATE_DEFAULT, LV_COLOR_MAKE(230, 230, 230));
    lv_obj_set_style_local_line_width(arc_bg, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 0);

    lv_obj_t *arc_progress = lv_arc_create(circle_cont, NULL);
    lv_obj_set_size(arc_progress, 160, 160);
    lv_obj_align(arc_progress, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_arc_set_bg_angles(arc_progress, 0, 360);
    uint16_t end_angle = (completion_rate * 360) / 100;
    lv_arc_set_angles(arc_progress, 270, 270 + end_angle);
    lv_obj_set_style_local_line_width(arc_progress, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 15);
    lv_obj_set_style_local_line_color(arc_progress, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_MAKE(0, 150, 255));
    lv_obj_set_style_local_line_width(arc_progress, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);

    lv_obj_t *label_percent = lv_label_create(circle_cont, NULL);
    char buf[20];
    snprintf(buf, sizeof(buf), "%d%%", completion_rate);
    lv_label_set_text(label_percent, buf);
    lv_obj_align(label_percent, NULL, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_style_local_text_font(label_percent, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_28);

    lv_obj_t *label_text = lv_label_create(circle_cont, NULL);
    lv_label_set_text(label_text, "TODAY'S\nCOMPLETION");
    lv_obj_align(label_text, label_percent, LV_ALIGN_OUT_BOTTOM_MID, 0, 3);
    lv_obj_set_style_local_text_color(label_text, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_label_set_align(label_text, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_style_local_text_font(label_text, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_18);

    lv_obj_t *completed_btn = lv_list_add_btn(overview_view, NULL, "");
    lv_obj_set_height(completed_btn, 60);
    lv_btn_set_layout(completed_btn, LV_LAYOUT_OFF);
    lv_obj_set_style_local_bg_color(completed_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(240, 255, 240));

    snprintf(buf, sizeof(buf), "%d", completed);
    lv_obj_t *completed_num = lv_label_create(completed_btn, NULL);
    lv_label_set_text(completed_num, buf);
    lv_obj_align(completed_num, NULL, LV_ALIGN_IN_LEFT_MID, 20, -6);
    lv_obj_set_style_local_text_font(completed_num, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_style_local_text_color(completed_num, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0, 150, 0));

    lv_obj_t *completed_label = lv_label_create(completed_btn, NULL);
    lv_label_set_text(completed_label, "DONE TODAY");
    lv_obj_align(completed_label, completed_num, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
    lv_obj_set_style_local_text_color(completed_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_style_local_text_font(completed_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_18);

    // started stats cont
    lv_obj_t *started_btn = lv_list_add_btn(overview_view, NULL, "");
    lv_obj_set_height(started_btn, 60);
    lv_btn_set_layout(started_btn, LV_LAYOUT_OFF);
    lv_obj_set_style_local_bg_color(started_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(255, 245, 230));

    snprintf(buf, sizeof(buf), "%d", started);
    lv_obj_t *started_num = lv_label_create(started_btn, NULL);
    lv_label_set_text(started_num, buf);
    lv_obj_align(started_num, NULL, LV_ALIGN_IN_LEFT_MID, 20, -6);
    lv_obj_set_style_local_text_font(started_num, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_style_local_text_color(started_num, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);

    lv_obj_t *started_label = lv_label_create(started_btn, NULL);
    lv_label_set_text(started_label, "MISSED TODAY");
    lv_obj_align(started_label, started_num, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
    lv_obj_set_style_local_text_color(started_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_style_local_text_font(started_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_18);

    // not done stats cont
    lv_obj_t *notdone_btn = lv_list_add_btn(overview_view, NULL, "");
    lv_obj_set_height(notdone_btn, 60);
    lv_btn_set_layout(notdone_btn, LV_LAYOUT_OFF);
    lv_obj_set_style_local_bg_color(notdone_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(255, 240, 240));

    snprintf(buf, sizeof(buf), "%d", not_done);
    lv_obj_t *notdone_num = lv_label_create(notdone_btn, NULL);
    lv_label_set_text(notdone_num, buf);
    lv_obj_align(notdone_num, NULL, LV_ALIGN_IN_LEFT_MID, 20, -6);
    lv_obj_set_style_local_text_font(notdone_num, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_style_local_text_color(notdone_num, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(200, 0, 0));

    lv_obj_t *notdone_label = lv_label_create(notdone_btn, NULL);
    lv_label_set_text(notdone_label, "NOT STARTED");
    lv_obj_align(notdone_label, notdone_num, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
    lv_obj_set_style_local_text_color(notdone_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_style_local_text_font(notdone_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_18);
}

