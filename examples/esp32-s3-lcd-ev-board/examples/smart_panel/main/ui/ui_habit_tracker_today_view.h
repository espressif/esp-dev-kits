

#ifndef UI_HABIT_TRACKER_TODAY_VIEW_H
#define UI_HABIT_TRACKER_TODAY_VIEW_H

#ifdef __cplusplus
extern "C" {
#endif


void ui_today_view_init(void);
void ui_today_view_show(void);
void ui_today_view_hide(void);

//sync with updated habits
void ui_today_view_refresh(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_TODAY_VIEW_H */