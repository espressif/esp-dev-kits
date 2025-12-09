
#ifndef UI_HABIT_TRACKER_STATISTICS_H
#define UI_HABIT_TRACKER_STATISTICS_H

#include "lvgl.h"
#include <stdint.h>
#include <stdbool.h>


void ui_habit_tracker_statistics_init(void);
void ui_habit_tracker_statistics_show(void);
void ui_habit_tracker_statistics_hide(void);
//refresh stats data
void ui_habit_tracker_statistics_refresh(void);

// switch btwn diff views
typedef enum {
    STATS_VIEW_OVERVIEW,
    STATS_VIEW_TRENDS,
    STATS_VIEW_HABITS
} stats_view_type_t;

void ui_habit_tracker_statistics_set_view(stats_view_type_t view);

#endif // UI_HABIT_TRACKER_STATISTICS_H