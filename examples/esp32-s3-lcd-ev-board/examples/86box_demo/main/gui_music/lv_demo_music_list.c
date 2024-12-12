/**
 * @file lv_demo_music_list.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_example_pub.h"
#include "lv_demo_music_list.h"
#if LV_USE_DEMO_MUSIC

#include "lv_demo_music_main.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_obj_t *add_list_btn(lv_obj_t *parent, uint32_t track_id);
static void btn_click_event_cb(lv_event_t *e);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t *list;
LV_IMG_DECLARE(img_lv_demo_music_btn_list_play);
LV_IMG_DECLARE(img_lv_demo_music_btn_list_pause);

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t *_lv_demo_music_list_create(lv_obj_t *parent)
{
#if LV_DEMO_MUSIC_LARGE
    lv_style_set_pad_right(&style_btn, 30);
#else
    lv_style_set_pad_right(&style_btn, 20);
#endif
    /*Create an empty transparent container*/
    list = lv_obj_create(parent);
    lv_obj_remove_style_all(list);
    lv_obj_set_size(list, LV_HOR_RES, LV_VER_RES - LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_set_y(list, LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_add_style(list, &style_scrollbar, LV_PART_SCROLLBAR);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);

    uint32_t track_id;
    for (track_id = 0; _lv_demo_music_get_title(track_id); track_id++) {
        add_list_btn(list,  track_id);
    }

#if LV_DEMO_MUSIC_SQUARE || LV_DEMO_MUSIC_ROUND
    lv_obj_set_scroll_snap_y(list, LV_SCROLL_SNAP_CENTER);
#endif

    _lv_demo_music_list_btn_check(0, true);

    return list;
}

void _lv_demo_music_list_btn_check(uint32_t track_id, bool state)
{
    lv_obj_t *btn = lv_obj_get_child(list, track_id);
    lv_obj_t *icon = lv_obj_get_child(btn, 0);

    if (state) {
        lv_obj_add_state(btn, LV_STATE_CHECKED);
        lv_img_set_src(icon, &img_lv_demo_music_btn_list_pause);
        lv_obj_scroll_to_view(btn, LV_ANIM_ON);
    } else {
        lv_obj_clear_state(btn, LV_STATE_CHECKED);
        lv_img_set_src(icon, &img_lv_demo_music_btn_list_play);
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_obj_t *add_list_btn(lv_obj_t *parent, uint32_t track_id)
{
    uint32_t t = _lv_demo_music_get_track_length(track_id);
    char time[32];
    lv_snprintf(time, sizeof(time), "%"LV_PRIu32":%02"LV_PRIu32, t / 60, t % 60);
    const char *title = _lv_demo_music_get_title(track_id);
    const char *artist = _lv_demo_music_get_artist(track_id);

    lv_obj_t *btn = lv_obj_create(parent);
    lv_obj_remove_style_all(btn);
#if LV_DEMO_MUSIC_LARGE
    lv_obj_set_size(btn, lv_pct(100), 110);
#else
    lv_obj_set_size(btn, lv_pct(100), 60);
#endif

    lv_obj_add_style(btn, &style_btn, 0);
    lv_obj_add_style(btn, &style_btn_pr, LV_STATE_PRESSED);
    lv_obj_add_style(btn, &style_btn_chk, LV_STATE_CHECKED);
    lv_obj_add_style(btn, &style_btn_dis, LV_STATE_DISABLED);
    lv_obj_add_event_cb(btn, btn_click_event_cb, LV_EVENT_CLICKED, NULL);

    if (track_id >= ACTIVE_TRACK_CNT) {
        lv_obj_add_state(btn, LV_STATE_DISABLED);
    }

    lv_obj_t *icon = lv_img_create(btn);
    lv_img_set_src(icon, &img_lv_demo_music_btn_list_play);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0, 2);

    lv_obj_t *title_label = lv_label_create(btn);
    lv_label_set_text(title_label, title);
    lv_obj_set_grid_cell(title_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_style(title_label, &style_title, 0);

    lv_obj_t *artist_label = lv_label_create(btn);
    lv_label_set_text(artist_label, artist);
    lv_obj_add_style(artist_label, &style_artist, 0);
    lv_obj_set_grid_cell(artist_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_t *time_label = lv_label_create(btn);
    lv_label_set_text(time_label, time);
    lv_obj_add_style(time_label, &style_time, 0);
    lv_obj_set_grid_cell(time_label, LV_GRID_ALIGN_END, 2, 1, LV_GRID_ALIGN_CENTER, 0, 2);

    LV_IMG_DECLARE(img_lv_demo_music_list_border);
    lv_obj_t *border = lv_img_create(btn);
    lv_img_set_src(border, &img_lv_demo_music_list_border);
    lv_obj_set_width(border, lv_pct(120));
    lv_obj_align(border, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(border, LV_OBJ_FLAG_IGNORE_LAYOUT);

    return btn;
}


static void btn_click_event_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);

    uint32_t idx = lv_obj_get_child_id(btn);

    _lv_demo_music_play(idx);
}
#endif /*LV_USE_DEMO_MUSIC*/
