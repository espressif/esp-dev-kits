

#include "ui_main.h"
#include "bsp_board_extra.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "lvgl.h"
#include "ui_keyboard.h"
#include "ui_pomodoro_analytics.h"
#include "audio_player.h"
#include <math.h>
#include <stdlib.h>

/* UI function declaration */
ui_func_desc_t ui_pomodoro_timer_func = {
    .name = "Pomodoro Timer",
    .init = ui_pomodoro_timer_init,
    .show = ui_pomodoro_timer_show,
    .hide = ui_pomodoro_timer_hide,
};

//internal state
static ui_state_t ui_pomodoro_state = ui_state_dis;
static lv_obj_t *obj_page_pomodoro = NULL;
static lv_obj_t *_label_timer = NULL;
static lv_obj_t *_label_state = NULL;
static lv_obj_t *_label_session_tracker = NULL; //track sessions
static lv_obj_t *_btn_start = NULL;
static lv_obj_t *_btn_reset = NULL;
static lv_obj_t *_btn_skip = NULL;
static lv_obj_t *main_cont = NULL;
static lv_obj_t *left_panel = NULL;
static lv_obj_t *right_panel = NULL;
static lv_obj_t *mode_cont = NULL;
static lv_obj_t *btn_cont = NULL;
static lv_obj_t *input_cont = NULL;
// custom time
static lv_obj_t *_time_picker_popup = NULL;
static lv_obj_t *_roller_minutes = NULL;
static lv_obj_t *_roller_seconds = NULL;
//keyboard
static lv_obj_t *_keyboard = NULL;
// analytics 
static lv_obj_t *_btn_analytics = NULL;

static bool timer_running = false;
typedef enum {
    MODE_FOCUS,
    MODE_SHORT_BREAK,
    MODE_LONG_BREAK
} timer_mode_t;
static timer_mode_t current_mode = MODE_FOCUS; //current timer mode
static int remaining_time = 0; // 1 min default
static int completed_pomodoros = 0; //Track completed focus sessions
static int current_session = 0; //Track current session number (0 = not started)
static esp_timer_handle_t pomodoro_timer = NULL;
static lv_task_t *ui_update_task = NULL; // LVGL task for UI updates
//custom time
static int custom_focus_time = 15;      //predefined, 30s for testing
static int custom_short_break_time = 2; // 10s for testing
static int custom_long_break_time = 3; // 20s for testing
static bool auto_start = false;
static volatile bool timer_update_needed = false; // Flag for UI updates from ISR
static volatile bool timer_completed = false; // Flag for mode transition

// mode selection buttons
static lv_obj_t *_btn_mode_focus = NULL;
static lv_obj_t *_btn_mode_short = NULL;
static lv_obj_t *_btn_mode_long = NULL;

// to-do list/tasks
static lv_obj_t *_textarea_task = NULL;
static lv_obj_t *_btn_add_task = NULL;
static lv_obj_t *_task_list = NULL;

// volume control
static lv_obj_t *_slider_volume = NULL;
static int current_volume = 70;

// music playback tracking
static FILE *current_music_fp = NULL;
static bool music_is_playing = false;  // actively playing
static bool music_is_paused = false;   // paused (can resume)
static char currently_playing_path[128] = "";  // track which file is currently playing
static char focus_music_file_path[128] = "";       // store full path
static char short_break_music_file_path[128] = "";
static char long_break_music_file_path[128] = "";
static int music_file_count = 0;    // number of actual music files
static char music_file_paths[32][128];  // store full paths for each file
static int music_file_indices[32];  // map dropdown index -> file index

// music state for each mode (to resume where left off)
typedef struct {
    int file_index;         // which file is playing for this mode
    int elapsed_ms;         // how many ms have elapsed
} music_state_t;

static music_state_t focus_music_state = {-1, 0};
static music_state_t short_break_music_state = {-1, 0};
static music_state_t long_break_music_state = {-1, 0};

/* Forward declarations */
static void _btn_start_cb(lv_obj_t *obj, lv_event_t event);
static void _btn_reset_cb(lv_obj_t *obj, lv_event_t event);
static void _btn_skip_cb(lv_obj_t *obj, lv_event_t event);
static void _update_timer_display(void);
static void _pomodoro_tick(void *arg);
static void _ui_update_task_cb(lv_task_t *task); // LVGL task callback for UI updates
static void _btn_add_task_cb(lv_obj_t *obj, lv_event_t event); //tasks
static void _checkbox_event_cb(lv_obj_t *obj, lv_event_t event); //checkboxes
static void _btn_mode_cb(lv_obj_t *obj, lv_event_t event);
static void _update_mode_buttons(void);
static void _set_timer_mode(timer_mode_t mode);
static void _update_session_tracker(void);
static void _slider_volume_cb(lv_obj_t *obj, lv_event_t event);
static void _play_music_for_mode(timer_mode_t mode);
static void _stop_music(void);
// custom time
static void _show_time_picker(void);
static void _hide_time_picker(void);
static void _time_picker_confirm_cb(lv_obj_t *obj, lv_event_t event);
static void _time_picker_cancel_cb(lv_obj_t *obj, lv_event_t event);
static void _timer_label_click_cb(lv_obj_t *obj, lv_event_t event);
//keyboard
static void _textarea_focus_cb(lv_obj_t *obj, lv_event_t event);
//settings 
static int focus_sessions_before_long_break = 2;
static int temp_sessions_count = 4;
static lv_obj_t *_btn_settings = NULL;
static lv_obj_t *_settings_popup = NULL;
static lv_obj_t *_label_sessions_count = NULL;
static lv_obj_t *_settings_scroll = NULL;
static void _btn_settings_cb(lv_obj_t *obj, lv_event_t event);
static void _show_settings_popup(void);
static void _hide_settings_popup(void);

static void _settings_confirm_cb(lv_obj_t *obj, lv_event_t event);
static void _settings_cancel_cb(lv_obj_t *obj, lv_event_t event);
static void _btn_decrease_sessions_cb(lv_obj_t *obj, lv_event_t event);
static void _btn_increase_sessions_cb(lv_obj_t *obj, lv_event_t event);
static void _update_sessions_display(void);
//text input
static lv_obj_t *_textarea_sessions = NULL;
static void _textarea_sessions_focus_cb(lv_obj_t *obj, lv_event_t event);
// settings: configuring time
static int temp_focus_time = 30;
static int temp_short_break_time = 10;
static int temp_long_break_time = 20;
static lv_obj_t *_roller_focus_min = NULL, *_roller_focus_sec = NULL;
static lv_obj_t *_roller_short_min = NULL, *_roller_short_sec = NULL;
static lv_obj_t *_roller_long_min = NULL, *_roller_long_sec = NULL;

static lv_obj_t *_dropdown_focus_music = NULL;
static lv_obj_t *_dropdown_short_music = NULL;
static lv_obj_t *_dropdown_long_music = NULL;

// helper function to build music dropdown options from available files
static char* _build_music_options(void)
{
    static char options[512] = "";
    options[0] = '\0';
    music_file_count = 0;
    
    file_iterator_instance_t *file_iterator = bsp_extra_get_file_instance();
    size_t list_len = file_iterator_get_count(file_iterator);
    ESP_LOGI("POMODORO", "building music options: total files=%zu", list_len);
    
    char filename[128];
    char clean_name[64];
    bool first = true;
    
    for (size_t i = 0; i < list_len; i++) {
        file_iterator_get_full_path_from_index(file_iterator, i, filename, sizeof(filename));
        
        // store mapping: dropdown index -> file iterator index and full path
        music_file_indices[music_file_count] = i;
        strncpy(music_file_paths[music_file_count], filename, sizeof(music_file_paths[0]) - 1);
        music_file_count++;
        ESP_LOGI("POMODORO", "found music: %s (idx=%zu)", filename, i);
        
        // Extract just the filename without path
        char *name = strrchr(filename, '/');
        if (name) {
            name++;  // Skip the '/'
        } else {
            name = filename;
        }
        
        // Remove file extension (.mp3, .wav, etc.)
        strncpy(clean_name, name, sizeof(clean_name) - 1);
        clean_name[sizeof(clean_name) - 1] = '\0';
        
        char *ext = strrchr(clean_name, '.');
        if (ext) {
            *ext = '\0';  // Remove extension
        }
        
        // Add to options string with newline separator
        if (!first) {
            strcat(options, "\n");
        }
        strcat(options, clean_name);
        first = false;
    }
    
    // Add "None" option at the end
    if (!first) {
        strcat(options, "\n");
    }
    strcat(options, "None");
    
    return options;
}

//analytics
static void _btn_analytics_cb(lv_obj_t *obj, lv_event_t event);

void ui_pomodoro_timer_init(void *data)
{
    ui_page_show("Pomodoro Timer");
    obj_page_pomodoro = ui_page_get_obj();

    // main horizontal cont
    main_cont = lv_cont_create(obj_page_pomodoro, NULL);
    lv_obj_set_size(main_cont, lv_obj_get_width(obj_page_pomodoro), lv_obj_get_height(obj_page_pomodoro)-70);
    lv_obj_align(main_cont, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_cont_set_layout(main_cont, LV_LAYOUT_ROW_MID);
    lv_obj_set_style_local_pad_all(main_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_align(main_cont, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    // left panel for Pomodoro timer
    left_panel = lv_cont_create(main_cont, NULL);
    lv_obj_set_size(left_panel, lv_obj_get_width(obj_page_pomodoro) / 2 - 15, lv_obj_get_height(obj_page_pomodoro) - 90);
    lv_cont_set_layout(left_panel, LV_LAYOUT_COLUMN_MID);
    lv_obj_set_style_local_bg_color(left_panel, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(245, 245, 245));
    lv_obj_set_style_local_pad_top(left_panel, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
    lv_obj_set_style_local_pad_bottom(left_panel, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
    lv_obj_set_style_local_pad_inner(left_panel, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);

    // mode selection container
    mode_cont = lv_cont_create(left_panel, NULL);
    lv_obj_set_size(mode_cont, lv_obj_get_width(left_panel) - 20, 50);
    lv_cont_set_layout(mode_cont, LV_LAYOUT_ROW_MID);
    lv_obj_set_style_local_pad_inner(mode_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);

    // Focus mode button
    _btn_mode_focus = lv_btn_create(mode_cont, NULL);
    lv_obj_set_size(_btn_mode_focus, 85, 40);
    lv_obj_set_event_cb(_btn_mode_focus, _btn_mode_cb);
    lv_obj_set_style_local_value_str(_btn_mode_focus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Focus");
    lv_obj_set_user_data(_btn_mode_focus, (void*)MODE_FOCUS);

    //Short break button
    _btn_mode_short = lv_btn_create(mode_cont, NULL);
    lv_obj_set_size(_btn_mode_short, 85, 40);
    lv_obj_set_event_cb(_btn_mode_short, _btn_mode_cb);
    lv_obj_set_style_local_value_str(_btn_mode_short, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Short");
    lv_obj_set_user_data(_btn_mode_short, (void*)MODE_SHORT_BREAK);

    //Long break buton
    _btn_mode_long = lv_btn_create(mode_cont, NULL);
    lv_obj_set_size(_btn_mode_long, 85, 40);
    lv_obj_set_event_cb(_btn_mode_long, _btn_mode_cb);
    lv_obj_set_style_local_value_str(_btn_mode_long, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Long");
    lv_obj_set_user_data(_btn_mode_long, (void*)MODE_LONG_BREAK);

    _btn_settings = lv_btn_create(mode_cont, NULL);
    lv_obj_set_size(_btn_settings, 40, 40);
    lv_obj_set_event_cb(_btn_settings, _btn_settings_cb);
    lv_obj_set_style_local_radius(_btn_settings, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_obj_set_style_local_value_str(_btn_settings, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_SETTINGS);

    _update_mode_buttons();

    // timer label
    _label_timer = lv_label_create(left_panel, NULL);
    lv_obj_set_style_local_text_font(_label_timer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_48);
    lv_obj_set_click(_label_timer, true); // make it clickable
    lv_obj_set_event_cb(_label_timer, _timer_label_click_cb);
    lv_obj_align(_label_timer, NULL, LV_ALIGN_CENTER, 0, -20);

    // sess label
    _label_state = lv_label_create(left_panel, NULL);
    lv_obj_set_style_local_text_font(_label_state, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_align(_label_state, _label_timer, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // button cont for circular buttons
    btn_cont = lv_cont_create(left_panel, NULL);
    lv_obj_set_size(btn_cont, lv_obj_get_width(left_panel) - 40, 80);
    lv_cont_set_layout(btn_cont, LV_LAYOUT_ROW_MID);
    lv_obj_set_style_local_pad_inner(btn_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 15);
    lv_obj_set_style_local_bg_opa(btn_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_border_width(btn_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(btn_cont, NULL, LV_ALIGN_CENTER, 0, 100);

    // reset button
    _btn_reset = lv_btn_create(btn_cont, NULL);
    lv_obj_set_size(_btn_reset, 70, 70);
    lv_obj_set_event_cb(_btn_reset, _btn_reset_cb);
    lv_obj_set_style_local_radius(_btn_reset, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_obj_set_style_local_value_str(_btn_reset, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Reset");

    // start/pause button
    _btn_start = lv_btn_create(btn_cont, NULL);
    lv_obj_set_size(_btn_start, 70, 70);
    lv_obj_set_event_cb(_btn_start, _btn_start_cb);
    lv_obj_set_style_local_radius(_btn_start, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_obj_set_style_local_value_str(_btn_start, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Start");

    // skip button
    _btn_skip = lv_btn_create(btn_cont, NULL);
    lv_obj_set_size(_btn_skip, 70, 70);
    lv_obj_set_event_cb(_btn_skip, _btn_skip_cb);
    lv_obj_set_style_local_radius(_btn_skip, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_obj_set_style_local_value_str(_btn_skip, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Skip");

    // create timer
    const esp_timer_create_args_t timer_args = {
        .callback = &_pomodoro_tick,
        .name = "pomodoro_tick"
    };
    esp_timer_create(&timer_args, &pomodoro_timer);
    
    // Create LVGL task for UI updates (runs every 100ms in LVGL context)
    ui_update_task = lv_task_create(_ui_update_task_cb, 100, LV_TASK_PRIO_MID, NULL);

    ui_pomodoro_state = ui_state_show;
    remaining_time = custom_focus_time;
    _update_timer_display();  // Update display to show correct time
    _update_session_tracker();

    // right panel for tasks
    right_panel = lv_cont_create(main_cont, NULL);
    lv_obj_set_size(right_panel, lv_obj_get_width(obj_page_pomodoro) / 2 - 15, lv_obj_get_height(obj_page_pomodoro) - 90);
    lv_cont_set_layout(right_panel, LV_LAYOUT_COLUMN_MID);
    lv_obj_set_style_local_bg_color(right_panel, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(250, 250, 250));

    // task input cont
    input_cont = lv_cont_create(right_panel, NULL);
    lv_obj_set_size(input_cont, lv_obj_get_width(right_panel) - 20, 50);
    lv_cont_set_layout(input_cont, LV_LAYOUT_ROW_MID);
    lv_obj_set_style_local_pad_inner(input_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
    lv_obj_set_style_local_border_width(input_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_bg_opa(input_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    // task input field
    _textarea_task = lv_textarea_create(input_cont, NULL);
    lv_obj_set_size(_textarea_task, lv_obj_get_width(right_panel) - 130, 40);
    lv_textarea_set_placeholder_text(_textarea_task, "Enter task...");
    lv_textarea_set_text(_textarea_task, "");
    lv_obj_set_event_cb(_textarea_task, _textarea_focus_cb);

    // add task button
    _btn_add_task = lv_btn_create(input_cont, NULL);
    lv_obj_set_size(_btn_add_task, 80, 40);
    lv_obj_set_event_cb(_btn_add_task, _btn_add_task_cb);
    lv_obj_set_style_local_value_str(_btn_add_task, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Add");

    // task list cont
    _task_list = lv_list_create(right_panel, NULL);
    lv_obj_set_size(_task_list, lv_obj_get_width(right_panel)-20, lv_obj_get_height(right_panel)-120); // fits input field
    lv_obj_set_drag_dir(_task_list, LV_DRAG_DIR_VER);
    lv_list_set_scrollbar_mode(_task_list, LV_SCROLLBAR_MODE_AUTO); 
    lv_obj_set_style_local_pad_all(_task_list, LV_LIST_PART_SCROLLABLE, LV_STATE_DEFAULT, 5);  

    // analytics button on right_panel
    _btn_analytics = lv_btn_create(right_panel, NULL);
    lv_obj_set_size(_btn_analytics, 70, 20);
    lv_obj_set_style_local_radius(_btn_analytics, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 8);
    lv_obj_set_style_local_bg_color(_btn_analytics, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(180, 167, 214));
    lv_obj_t *label = lv_label_create(_btn_analytics, NULL);
    lv_label_set_text(label, "Stats");
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);

    // event callback
    lv_obj_set_event_cb(_btn_analytics, _btn_analytics_cb);

    // volume slider on left panel
    _slider_volume = lv_slider_create(left_panel, NULL);
    lv_obj_set_size(_slider_volume, 200, 10);
    lv_slider_set_range(_slider_volume, 0, 100);
    lv_slider_set_value(_slider_volume, current_volume, LV_ANIM_OFF);
    lv_obj_set_style_local_radius(_slider_volume, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, 5);
    lv_obj_set_style_local_radius(_slider_volume, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, 5);
    lv_obj_set_style_local_bg_color(_slider_volume, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_MAKE(100, 150, 255));
    lv_obj_align(_slider_volume, btn_cont, LV_ALIGN_OUT_BOTTOM_MID, 50, 8);
    lv_obj_set_event_cb(_slider_volume, _slider_volume_cb);
    lv_obj_set_style_local_value_str(_slider_volume, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_SYMBOL_VOLUME_MID);
    lv_obj_set_style_local_value_align(_slider_volume, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_LEFT_MID);
    lv_obj_set_style_local_value_ofs_x(_slider_volume, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, -20);

}


void ui_pomodoro_timer_show(void *data)
{
    if (ui_state_dis == ui_pomodoro_state)
        ui_pomodoro_timer_init(data);
    else if (ui_state_hide == ui_pomodoro_state)
    {
        lv_obj_set_hidden(_btn_mode_focus, false);
        lv_obj_set_hidden(_btn_mode_short, false);
        lv_obj_set_hidden(_btn_mode_long, false);
        lv_obj_set_hidden(_btn_settings, false);
        lv_obj_set_hidden(_label_timer, false);
        lv_obj_set_hidden(_label_state, false);
        lv_obj_set_hidden(_btn_reset, false);
        lv_obj_set_hidden(_btn_start, false);
        lv_obj_set_hidden(_btn_skip, false);
        lv_obj_set_hidden(_textarea_task, false);
        lv_obj_set_hidden(_btn_add_task, false);
        lv_obj_set_hidden(_task_list, false);
        lv_obj_set_hidden(main_cont, false);
        lv_obj_set_hidden(left_panel, false);
        lv_obj_set_hidden(right_panel, false);
        lv_obj_set_hidden(mode_cont, false);
        lv_obj_set_hidden(btn_cont, false);
        lv_obj_set_hidden(input_cont, false);
        lv_obj_set_hidden(_btn_analytics, false); 
        ui_page_show("Pomodoro Timer");
        ui_pomodoro_state = ui_state_show;
    }
}

void ui_pomodoro_timer_hide(void *data)
{
    if (ui_state_show == ui_pomodoro_state)
    {
        // stop timer when hide
        if (timer_running) {
            esp_timer_stop(pomodoro_timer);
            timer_running = false;
        }
        
        // hide all cont
        lv_obj_set_hidden(main_cont, true);
        lv_obj_set_hidden(left_panel, true);
        lv_obj_set_hidden(right_panel, true);
        lv_obj_set_hidden(mode_cont, true);
        lv_obj_set_hidden(btn_cont, true);
        lv_obj_set_hidden(input_cont, true);
        
        // hide indv elements
        lv_obj_set_hidden(_btn_mode_focus, true);
        lv_obj_set_hidden(_btn_mode_short, true);
        lv_obj_set_hidden(_btn_mode_long, true);
        lv_obj_set_hidden(_btn_settings, true);
        lv_obj_set_hidden(_label_timer, true);
        lv_obj_set_hidden(_label_state, true);
        lv_obj_set_hidden(_btn_reset, true);
        lv_obj_set_hidden(_btn_start, true);
        lv_obj_set_hidden(_btn_skip, true);
        lv_obj_set_hidden(_textarea_task, true);
        lv_obj_set_hidden(_btn_add_task, true);
        lv_obj_set_hidden(_task_list, true);
        lv_obj_set_hidden(_btn_analytics, true); 
        
        // close keyboard if open
        if (_keyboard) {
            ui_keyboard_delete(_keyboard);
            _keyboard = NULL;
        }
        
        // close popups if open
        if (_time_picker_popup) {
            _hide_time_picker();
        }
        if (_settings_popup) {
            _hide_settings_popup();
        }

        ui_page_hide(NULL);
        ui_pomodoro_state = ui_state_hide;
    }
}

static void _btn_start_cb(lv_obj_t *obj, lv_event_t event){
    if (event == LV_EVENT_CLICKED){
        timer_running = !timer_running;

        if (timer_running){
            // increment sess counter when starting focus mode for the first time
            if (current_mode == MODE_FOCUS && current_session == 0) {
                current_session = 1;
                _update_session_tracker();
            }
            esp_timer_start_periodic(pomodoro_timer, 1000000); // 1s tick
            lv_obj_set_style_local_value_str(_btn_start, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Pause");
            
            // Play music when starting
            _play_music_for_mode(current_mode);
        }
        else{
            esp_timer_stop(pomodoro_timer);
            lv_obj_set_style_local_value_str(_btn_start, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Resume");
            
            // Pause music when pausing timer
            if (music_is_playing) {
                audio_player_pause();
                music_is_paused = true;
                music_is_playing = false;
            }
        }
    }
}

// reset button callback
static void _btn_reset_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        esp_timer_stop(pomodoro_timer);
        timer_running = false;
        current_mode = MODE_FOCUS;
        completed_pomodoros = 0;
        current_session = 0;
        
        // reset music states
        focus_music_state.elapsed_ms = 0;
        short_break_music_state.elapsed_ms = 0;
        long_break_music_state.elapsed_ms = 0;
        
        _update_session_tracker();
        _set_timer_mode(current_mode);
        lv_obj_set_style_local_value_str(_btn_start, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Start");
        _update_mode_buttons();
        _update_timer_display();
    }
}

//skip button callback
static void _btn_skip_cb(lv_obj_t *obj, lv_event_t event)
{   
    if (event == LV_EVENT_CLICKED)
    {
        // skip to next mode
        if (current_mode == MODE_FOCUS)
        {
            completed_pomodoros++;
            current_session++; // +1 session on skip
            _update_session_tracker();

            // mode now = short/long break
            if (completed_pomodoros >= focus_sessions_before_long_break)
            {
                current_mode = MODE_LONG_BREAK;
                completed_pomodoros = 0;
            }
            else
            {
                current_mode = MODE_SHORT_BREAK;
            }
        }
        else if (current_mode == MODE_SHORT_BREAK || current_mode == MODE_LONG_BREAK)
        {
            // skip break, go back to focus
            current_mode = MODE_FOCUS;
        }

        if (timer_running == false){
            lv_obj_set_style_local_value_str(_btn_start, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Start");
        }
        auto_start = true;
        _set_timer_mode(current_mode);
        _update_mode_buttons();
        _update_timer_display();
    }
}

// mode button callback
static void _btn_mode_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        timer_mode_t new_mode = (timer_mode_t)lv_obj_get_user_data(obj);
        
        // stop timer if running
        if (timer_running)
        {
            esp_timer_stop(pomodoro_timer);
            timer_running = false;
            lv_obj_set_style_local_value_str(_btn_start, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Start");
        }
        current_mode = new_mode;
        completed_pomodoros = 0; // reset counter when manually switch
        _set_timer_mode(current_mode);
        _update_mode_buttons();
        _update_timer_display();
    }
}

// update mode button appearance
static void _update_mode_buttons(void)
{
    // reset all buttons to default style
    lv_obj_set_style_local_bg_color(_btn_mode_focus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(_btn_mode_short, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(_btn_mode_long, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    
    // highlight current mode
    lv_obj_t *active_btn = NULL;
    lv_color_t active_color = LV_COLOR_MAKE(173, 216, 230);
    
    switch (current_mode)
    {
        case MODE_FOCUS:
            active_btn = _btn_mode_focus;
            break;
        case MODE_SHORT_BREAK:
            active_btn = _btn_mode_short;
            break;
        case MODE_LONG_BREAK:
            active_btn = _btn_mode_long;
            break;
    }
    
    if (active_btn)
    {
        lv_obj_set_style_local_bg_color(active_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, active_color);
    }
}

// set timer to mode with correct duration
static void _set_timer_mode(timer_mode_t mode){
    current_mode = mode;
    esp_timer_stop(pomodoro_timer);

    switch (mode)
    {
        case MODE_FOCUS:
            remaining_time = custom_focus_time;
            break;
        case MODE_SHORT_BREAK:
            remaining_time = custom_short_break_time;
            break;
        case MODE_LONG_BREAK:
            remaining_time = custom_long_break_time;
            break;
    }
    if (auto_start)
    {
        // auto start next timer
        auto_start = false;
        timer_running = true;
        esp_timer_start_periodic(pomodoro_timer, 1000000);
        lv_obj_set_style_local_value_str(_btn_start, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Pause");
        
        // play music for this mode when auto-starting (handles pause/stop internally)
        _play_music_for_mode(mode);
    }
    else
    {
        // manual mode change -> pause music and reset button
        if (music_is_playing) {
            audio_player_pause();
            music_is_paused = true;
            music_is_playing = false;
        }
        timer_running = false;
        lv_obj_set_style_local_value_str(_btn_start, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Start");
    }
    // update session label
    _update_session_tracker();
}

static void _pomodoro_tick(void *arg)
{
    if (remaining_time > 0) 
    {
        remaining_time--;
        
        // track music elapsed time for current mode
        if (music_is_playing) {
            switch (current_mode) {
                case MODE_FOCUS:
                    focus_music_state.elapsed_ms += 1000;
                    break;
                case MODE_SHORT_BREAK:
                    short_break_music_state.elapsed_ms += 1000;
                    break;
                case MODE_LONG_BREAK:
                    long_break_music_state.elapsed_ms += 1000;
                    break;
            }
        }
        
        timer_update_needed = true;
    }
    else 
    { // timer complete, auto switch to next mode
        esp_timer_stop(pomodoro_timer);
        timer_running = false;
        
        if (current_mode == MODE_FOCUS)
        {
            completed_pomodoros++;
            current_session++;
            
            // after # focus sessions, take long break
            if (completed_pomodoros >= focus_sessions_before_long_break)
            {
                current_mode = MODE_LONG_BREAK;
                completed_pomodoros = 0;
            }
            else
            {
                current_mode = MODE_SHORT_BREAK;
            }
            auto_start = true;
        }
        else if (current_mode == MODE_SHORT_BREAK || current_mode == MODE_LONG_BREAK)
        {
            // after any break, go back to focus
            current_mode = MODE_FOCUS;
            auto_start = true;
        }
        
        // music already faded out when remaining_time == 5, so don't fade again
        timer_completed = true;
        timer_update_needed = true;
    }
}

static void _update_timer_display(void)
{
    static char buf[8];
    int minutes = remaining_time / 60;
    int seconds = remaining_time % 60;
    sprintf(buf, "%02d:%02d", minutes, seconds);
    lv_label_set_text(_label_timer, buf);
}

// LVGL task callback - handles UI updates from timer ISR safely
static void _ui_update_task_cb(lv_task_t *task)
{
    if (timer_update_needed) {
        timer_update_needed = false;
        
        // Check if timer completed and mode transition is needed
        if (timer_completed) {
            timer_completed = false;
            //  set the new timer mode/duration bef updating display
            _set_timer_mode(current_mode);
            _update_mode_buttons();
            _update_session_tracker();
            _update_timer_display();  // update display aft mode is set
        } else {
            // Just update the display
            _update_timer_display();
        }
    }
}

// update session number
static void _update_session_tracker(void){
    static char session_buf[32];
    
    if (current_session == 0){
        switch (current_mode)
        {
            case MODE_FOCUS:
                lv_label_set_text(_label_state, "Time to focus!");
                return;
            case MODE_SHORT_BREAK:
                lv_label_set_text(_label_state, "Time for short break!");
                return;
            case MODE_LONG_BREAK:
                lv_label_set_text(_label_state, "Time for long break!");
                return;
        }
    }
    else{
        // current session number
        if (current_mode == MODE_FOCUS){
            sprintf(session_buf, "Focus Session %d", current_session);
        }
        else if (current_mode == MODE_SHORT_BREAK){
            sprintf(session_buf, "Short Break");
        }
        else if (current_mode == MODE_LONG_BREAK){
            sprintf(session_buf, "Long Break");
        }
        lv_label_set_text(_label_state, session_buf);
    }
}

// custom time
static void _show_time_picker(void)
{
    // modal background
    _time_picker_popup = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(_time_picker_popup, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_local_bg_opa(_time_picker_popup, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
    lv_obj_set_style_local_bg_color(_time_picker_popup, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    
    // picker container
    lv_obj_t *picker_cont = lv_obj_create(_time_picker_popup, NULL);
    lv_obj_set_size(picker_cont, 320, 280);
    lv_obj_align(picker_cont, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_color(picker_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_radius(picker_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
    
    // title
    lv_obj_t *title = lv_label_create(picker_cont, NULL);
    lv_label_set_text(title, "Set Timer Duration");
    lv_obj_set_style_local_text_font(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_MID, 0, 15);
    
    // get current mode's time
    int current_time;
    switch (current_mode)
    {
        case MODE_FOCUS:
            current_time = custom_focus_time;
            break;
        case MODE_SHORT_BREAK:
            current_time = custom_short_break_time;
            break;
        case MODE_LONG_BREAK:
            current_time = custom_long_break_time;
            break;
        default:
            current_time = 60; // fallback
            break;
    }
    
    int current_minutes = current_time / 60;
    int current_seconds = current_time % 60;
    
    // roller container
    lv_obj_t *roller_cont = lv_obj_create(picker_cont, NULL);
    lv_obj_set_size(roller_cont, 280, 120);
    lv_obj_align(roller_cont, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    lv_obj_set_style_local_border_width(roller_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_bg_opa(roller_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    
    // min roller
    _roller_minutes = lv_roller_create(roller_cont, NULL);
    lv_obj_set_size(_roller_minutes, 80, 100);
    lv_roller_set_options(_roller_minutes, 
        "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n"
        "11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n"
        "21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n"
        "31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n"
        "41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n"
        "51\n52\n53\n54\n55\n56\n57\n58\n59\n60",
        LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(_roller_minutes, 3);
    lv_roller_set_selected(_roller_minutes, current_minutes, LV_ANIM_OFF);
    lv_obj_align(_roller_minutes, NULL, LV_ALIGN_IN_LEFT_MID, 30, 0);
    
    // min label
    lv_obj_t *min_label = lv_label_create(roller_cont, NULL);
    lv_label_set_text(min_label, "min");
    lv_obj_align(min_label, _roller_minutes, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    
    // sec roller
    _roller_seconds = lv_roller_create(roller_cont, NULL);
    lv_obj_set_size(_roller_seconds, 80, 100);
    lv_roller_set_options(_roller_seconds,
        "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n"
        "11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n"
        "21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n"
        "31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n"
        "41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n"
        "51\n52\n53\n54\n55\n56\n57\n58\n59",
        LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(_roller_seconds, 3);
    lv_roller_set_selected(_roller_seconds, current_seconds, LV_ANIM_OFF);
    lv_obj_align(_roller_seconds, min_label, LV_ALIGN_OUT_RIGHT_MID, 25, 0);
    
    // sec label
    lv_obj_t *sec_label = lv_label_create(roller_cont, NULL);
    lv_label_set_text(sec_label, "sec");
    lv_obj_align(sec_label, _roller_seconds, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    
    // button container
    lv_obj_t *btn_cont = lv_obj_create(picker_cont, NULL);
    lv_obj_set_size(btn_cont, 280, 50);
    lv_obj_align(btn_cont, roller_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
    lv_obj_set_style_local_border_width(btn_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_bg_opa(btn_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    
    // cancel button
    lv_obj_t *btn_cancel = lv_btn_create(btn_cont, NULL);
    lv_obj_set_size(btn_cancel, 120, 45);
    lv_obj_align(btn_cancel, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);
    lv_obj_set_event_cb(btn_cancel, _time_picker_cancel_cb);
    lv_obj_set_style_local_value_str(btn_cancel, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Cancel");
    lv_obj_set_style_local_bg_color(btn_cancel, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(220, 220, 220));
    
    // confirm button
    lv_obj_t *btn_confirm = lv_btn_create(btn_cont, NULL);
    lv_obj_set_size(btn_confirm, 120, 45);
    lv_obj_align(btn_confirm, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);
    lv_obj_set_event_cb(btn_confirm, _time_picker_confirm_cb);
    lv_obj_set_style_local_value_str(btn_confirm, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Confirm");
    lv_obj_set_style_local_bg_color(btn_confirm, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(100, 180, 100));
}

static void _hide_time_picker(void)
{
    if (_time_picker_popup)
    {
        lv_obj_del(_time_picker_popup);
        _time_picker_popup = NULL;
        _roller_minutes = NULL;
        _roller_seconds = NULL;
    }
}

// timer label click -> adjust time
static void _timer_label_click_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        // Only allow time editing when timer is not running
        if (!timer_running)
        {
            _show_time_picker();
        }
    }
}

// time picker (confirm)
static void _time_picker_confirm_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        // get selected values
        uint16_t minutes = lv_roller_get_selected(_roller_minutes);
        uint16_t seconds = lv_roller_get_selected(_roller_seconds);
        
        int new_time = minutes * 60 + seconds;
        
        // at least 1 second
        if (new_time < 1)
        {
            new_time = 1;
        }
        
        // save to appropriate mode
        switch (current_mode)
        {
            case MODE_FOCUS:
                custom_focus_time = new_time;
                break;
            case MODE_SHORT_BREAK:
                custom_short_break_time = new_time;
                break;
            case MODE_LONG_BREAK:
                custom_long_break_time = new_time;
                break;
        }
        
        // update current timer
        remaining_time = new_time;
        _update_timer_display();
        
        _hide_time_picker();
    }
}

// time picker (cancel)
static void _time_picker_cancel_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        _hide_time_picker();
    }
}

// settings button
static void _btn_settings_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {   
        // initialize temp values with current settings
        temp_sessions_count = focus_sessions_before_long_break;
        temp_focus_time = custom_focus_time;
        temp_short_break_time = custom_short_break_time;
        temp_long_break_time = custom_long_break_time;
        _show_settings_popup();
    }
}

// settings UI
static void _show_settings_popup(void)
{
    // modal background
    _settings_popup = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(_settings_popup, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_local_bg_opa(_settings_popup, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
    lv_obj_set_style_local_bg_color(_settings_popup, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    
    //  settings container
    lv_obj_t *settings_cont = lv_obj_create(_settings_popup, NULL);
    lv_obj_set_size(settings_cont, 450, 400); 
    lv_obj_align(settings_cont, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_color(settings_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_radius(settings_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);

    // close button
    lv_obj_t *btn_close = lv_btn_create(settings_cont, NULL);
    lv_obj_set_size(btn_close, 40, 40);
    lv_obj_align(btn_close, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);
    lv_obj_set_event_cb(btn_close, _settings_cancel_cb);
    lv_obj_set_style_local_value_str(btn_close, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_CLOSE);
    lv_obj_set_style_local_radius(btn_close, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_obj_set_style_local_bg_color(btn_close, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(220, 220, 220));
    
    // title
    lv_obj_t *title = lv_label_create(settings_cont, NULL);
    lv_label_set_text(title, "Settings");
    lv_obj_set_style_local_text_font(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_MID, 0, 15);

    // scrollable page
    lv_obj_t *page = lv_page_create(settings_cont, NULL);
    lv_obj_set_size(page, 420, 260);  
    lv_obj_align(page, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_style_local_pad_all(page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 10);
    lv_page_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_AUTO);

    lv_page_set_scrl_layout(page, LV_LAYOUT_COLUMN_MID);
    lv_obj_set_style_local_pad_inner(page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 10);  
    
    // Build music options once for all dropdowns (to maintain same indices mapping)
    static char music_options_focus[512];
    static char music_options_short[512];
    static char music_options_long[512];

    // Build once and copy to all three buffers
    char *base_options = _build_music_options();
    strncpy(music_options_focus, base_options, sizeof(music_options_focus) - 1);
    strncpy(music_options_short, base_options, sizeof(music_options_short) - 1);
    strncpy(music_options_long, base_options, sizeof(music_options_long) - 1);
    
    // sess bef long break
    lv_obj_t *desc_sessions = lv_label_create(page, NULL);
    lv_label_set_text(desc_sessions, "Sessions before long break:");
    lv_obj_set_width(desc_sessions, 380); 
    lv_obj_set_style_local_text_font(desc_sessions, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24); 
    
    // Counter container for +/- buttons and number
    lv_obj_t *counter_cont = lv_cont_create(page, NULL);
    lv_obj_set_size(counter_cont, 200, 50);
    lv_cont_set_layout(counter_cont, LV_LAYOUT_OFF);
    lv_obj_set_style_local_border_width(counter_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_bg_opa(counter_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    
    // Decrease button (-)
    lv_obj_t *btn_decrease = lv_btn_create(counter_cont, NULL);
    lv_obj_set_size(btn_decrease, 45, 45);  
    lv_obj_align(btn_decrease, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);
    lv_obj_set_event_cb(btn_decrease, _btn_decrease_sessions_cb);
    lv_obj_set_style_local_value_str(btn_decrease, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "-");
    lv_obj_set_style_local_value_font(btn_decrease, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_style_local_radius(btn_decrease, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    
    // Number input
    _textarea_sessions = lv_textarea_create(counter_cont, NULL);
    lv_obj_set_size(_textarea_sessions, 70, 45);  
    lv_textarea_set_one_line(_textarea_sessions, true);
    lv_textarea_set_text_align(_textarea_sessions, LV_LABEL_ALIGN_CENTER);
    lv_textarea_set_accepted_chars(_textarea_sessions, "0123456789");
    lv_obj_set_style_local_text_font(_textarea_sessions, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_align(_textarea_sessions, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(_textarea_sessions, _textarea_sessions_focus_cb);
    
    static char buf[8];
    sprintf(buf, "%d", temp_sessions_count);
    lv_textarea_set_text(_textarea_sessions, buf);
    
    // Increase button (+)
    lv_obj_t *btn_increase = lv_btn_create(counter_cont, NULL);
    lv_obj_set_size(btn_increase, 45, 45); 
    lv_obj_align(btn_increase, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);
    lv_obj_set_event_cb(btn_increase, _btn_increase_sessions_cb);
    lv_obj_set_style_local_value_str(btn_increase, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "+");
    lv_obj_set_style_local_value_font(btn_increase, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_style_local_radius(btn_increase, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    
    // focus settings
    lv_obj_t *label_focus_header = lv_label_create(page, NULL);
    lv_label_set_text(label_focus_header, "FOCUS");
    lv_obj_set_style_local_text_font(label_focus_header, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_28);
    lv_obj_set_style_local_text_color(label_focus_header, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    
    // duration label
    lv_obj_t *label_focus_duration = lv_label_create(page, NULL);
    lv_label_set_text(label_focus_duration, "Duration");
    lv_obj_set_style_local_text_font(label_focus_duration, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    
    // Roller container for focus
    lv_obj_t *focus_roller_cont = lv_cont_create(page, NULL);
    lv_obj_set_size(focus_roller_cont, 250, 100);
    lv_cont_set_layout(focus_roller_cont, LV_LAYOUT_OFF);
    lv_obj_set_style_local_border_width(focus_roller_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_bg_opa(focus_roller_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    
    int focus_min = temp_focus_time / 60;
    int focus_sec = temp_focus_time % 60;
    
    _roller_focus_min = lv_roller_create(focus_roller_cont, NULL);
    lv_obj_set_size(_roller_focus_min, 70, 80);
    lv_roller_set_options(_roller_focus_min, 
        "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n"
        "21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n"
        "41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59\n60",
        LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(_roller_focus_min, 3);
    lv_roller_set_selected(_roller_focus_min, focus_min, LV_ANIM_OFF);
    lv_obj_align(_roller_focus_min, NULL, LV_ALIGN_CENTER, -65, 0);
    
    lv_obj_t *label_focus_min = lv_label_create(focus_roller_cont, NULL);
    lv_label_set_text(label_focus_min, "min");
    lv_obj_align(label_focus_min, _roller_focus_min, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    
    _roller_focus_sec = lv_roller_create(focus_roller_cont, NULL);
    lv_obj_set_size(_roller_focus_sec, 70, 80);
    lv_roller_set_options(_roller_focus_sec,
        "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n"
        "21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n"
        "41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59",
        LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(_roller_focus_sec, 3);
    lv_roller_set_selected(_roller_focus_sec, focus_sec, LV_ANIM_OFF);
    lv_obj_align(_roller_focus_sec, label_focus_min, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    
    lv_obj_t *label_focus_sec = lv_label_create(focus_roller_cont, NULL);
    lv_label_set_text(label_focus_sec, "sec");
    lv_obj_align(label_focus_sec, _roller_focus_sec, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    
    // Music label
    lv_obj_t *label_focus_music = lv_label_create(page, NULL);
    lv_label_set_text(label_focus_music, "Music");
    lv_obj_set_style_local_text_font(label_focus_music, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    
    // Music dropdown (use pre-built options)
    _dropdown_focus_music = lv_dropdown_create(page, NULL);
    lv_obj_set_width(_dropdown_focus_music, 250);
    lv_obj_set_style_local_text_font(_dropdown_focus_music, LV_DROPDOWN_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    lv_obj_set_style_local_text_font(_dropdown_focus_music, LV_DROPDOWN_PART_LIST, LV_STATE_DEFAULT, &font_en_20);
    lv_dropdown_set_options(_dropdown_focus_music, music_options_focus);
    lv_dropdown_set_symbol(_dropdown_focus_music, LV_SYMBOL_DOWN);
    lv_dropdown_set_dir(_dropdown_focus_music, LV_DROPDOWN_DIR_DOWN);
    
    // short break settings
    lv_obj_t *label_short_header = lv_label_create(page, NULL);
    lv_label_set_text(label_short_header, "SHORT BREAK");
    lv_obj_set_style_local_text_font(label_short_header, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_28);
    lv_obj_set_style_local_text_color(label_short_header, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    
    // Duration label
    lv_obj_t *label_short_duration = lv_label_create(page, NULL);
    lv_label_set_text(label_short_duration, "Duration");
    lv_obj_set_style_local_text_font(label_short_duration, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    
    // Roller container for short break
    lv_obj_t *short_roller_cont = lv_cont_create(page, NULL);
    lv_obj_set_size(short_roller_cont, 250, 100);
    lv_cont_set_layout(short_roller_cont, LV_LAYOUT_OFF);
    lv_obj_set_style_local_border_width(short_roller_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_bg_opa(short_roller_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    
    int short_min = temp_short_break_time / 60;
    int short_sec = temp_short_break_time % 60;
    
    _roller_short_min = lv_roller_create(short_roller_cont, NULL);
    lv_obj_set_size(_roller_short_min, 70, 80);
    lv_roller_set_options(_roller_short_min, 
        "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n"
        "21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n"
        "41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59\n60",
        LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(_roller_short_min, 3);
    lv_roller_set_selected(_roller_short_min, short_min, LV_ANIM_OFF);
    lv_obj_align(_roller_short_min, NULL, LV_ALIGN_CENTER, -65, 0);
    
    lv_obj_t *label_short_min = lv_label_create(short_roller_cont, NULL);
    lv_label_set_text(label_short_min, "min");
    lv_obj_align(label_short_min, _roller_short_min, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    
    _roller_short_sec = lv_roller_create(short_roller_cont, NULL);
    lv_obj_set_size(_roller_short_sec, 70, 80);
    lv_roller_set_options(_roller_short_sec,
        "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n"
        "21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n"
        "41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59",
        LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(_roller_short_sec, 3);
    lv_roller_set_selected(_roller_short_sec, short_sec, LV_ANIM_OFF);
    lv_obj_align(_roller_short_sec, label_short_min, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    
    lv_obj_t *label_short_sec = lv_label_create(short_roller_cont, NULL);
    lv_label_set_text(label_short_sec, "sec");
    lv_obj_align(label_short_sec, _roller_short_sec, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    
    // Music label
    lv_obj_t *label_short_music = lv_label_create(page, NULL);
    lv_label_set_text(label_short_music, "Music");
    lv_obj_set_style_local_text_font(label_short_music, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    
    // Music dropdown
    _dropdown_short_music = lv_dropdown_create(page, NULL);
    lv_obj_set_width(_dropdown_short_music, 250);
    lv_obj_set_style_local_text_font(_dropdown_short_music, LV_DROPDOWN_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    lv_obj_set_style_local_text_font(_dropdown_short_music, LV_DROPDOWN_PART_LIST, LV_STATE_DEFAULT, &font_en_20);
    lv_dropdown_set_options(_dropdown_short_music, music_options_short);
    lv_dropdown_set_symbol(_dropdown_short_music, LV_SYMBOL_DOWN);
    lv_dropdown_set_dir(_dropdown_short_music, LV_DROPDOWN_DIR_DOWN);
    
    // long break settings
    lv_obj_t *label_long_header = lv_label_create(page, NULL);
    lv_label_set_text(label_long_header, "LONG BREAK");
    lv_obj_set_style_local_text_font(label_long_header, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_28);
    lv_obj_set_style_local_text_color(label_long_header, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    
    // Duration label
    lv_obj_t *label_long_duration = lv_label_create(page, NULL);
    lv_label_set_text(label_long_duration, "Duration");
    lv_obj_set_style_local_text_font(label_long_duration, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    
    // Roller container for long break
    lv_obj_t *long_roller_cont = lv_cont_create(page, NULL);
    lv_obj_set_size(long_roller_cont, 250, 100);
    lv_cont_set_layout(long_roller_cont, LV_LAYOUT_OFF);
    lv_obj_set_style_local_border_width(long_roller_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_bg_opa(long_roller_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    
    int long_min = temp_long_break_time / 60;
    int long_sec = temp_long_break_time % 60;
    
    _roller_long_min = lv_roller_create(long_roller_cont, NULL);
    lv_obj_set_size(_roller_long_min, 70, 80);
    lv_roller_set_options(_roller_long_min, 
        "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n"
        "21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n"
        "41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59\n60",
        LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(_roller_long_min, 3);
    lv_roller_set_selected(_roller_long_min, long_min, LV_ANIM_OFF);
    lv_obj_align(_roller_long_min, NULL, LV_ALIGN_CENTER, -65, 0);
    
    lv_obj_t *label_long_min = lv_label_create(long_roller_cont, NULL);
    lv_label_set_text(label_long_min, "min");
    lv_obj_align(label_long_min, _roller_long_min, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    
    _roller_long_sec = lv_roller_create(long_roller_cont, NULL);
    lv_obj_set_size(_roller_long_sec, 70, 80);
    lv_roller_set_options(_roller_long_sec,
        "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n"
        "21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n"
        "41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59",
        LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(_roller_long_sec, 3);
    lv_roller_set_selected(_roller_long_sec, long_sec, LV_ANIM_OFF);
    lv_obj_align(_roller_long_sec, label_long_min, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    
    lv_obj_t *label_long_sec = lv_label_create(long_roller_cont, NULL);
    lv_label_set_text(label_long_sec, "sec");
    lv_obj_align(label_long_sec, _roller_long_sec, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    
    //music label
    lv_obj_t *label_long_music = lv_label_create(page, NULL);
    lv_label_set_text(label_long_music, "Music");
    lv_obj_set_style_local_text_font(label_long_music, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    
    // Music dropdown
    _dropdown_long_music = lv_dropdown_create(page, NULL);
    lv_obj_set_width(_dropdown_long_music, 250);
    lv_obj_set_style_local_text_font(_dropdown_long_music, LV_DROPDOWN_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    lv_obj_set_style_local_text_font(_dropdown_long_music, LV_DROPDOWN_PART_LIST, LV_STATE_DEFAULT, &font_en_20);
    lv_dropdown_set_options(_dropdown_long_music, music_options_long);
    lv_dropdown_set_symbol(_dropdown_long_music, LV_SYMBOL_DOWN);
    lv_dropdown_set_dir(_dropdown_long_music, LV_DROPDOWN_DIR_DOWN);
    
    // Save button (at bottom of settings_cont, not in scrollable page)
    lv_obj_t *btn_confirm = lv_btn_create(settings_cont, NULL);
    lv_obj_set_size(btn_confirm, 120, 45);
    lv_obj_align(btn_confirm, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    lv_obj_set_event_cb(btn_confirm, _settings_confirm_cb);
    lv_obj_set_style_local_value_str(btn_confirm, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Save");
    lv_obj_set_style_local_bg_color(btn_confirm, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(100, 180, 100));
    
    // set dropdown selections AFTER all UI elements are created
    // focus music selection
    if (focus_music_file_path[0] != '\0') {
        int dropdown_idx = 0;
        for (int i = 0; i < music_file_count; i++) {
            if (strcmp(music_file_paths[i], focus_music_file_path) == 0) {
                dropdown_idx = i;
                break;
            }
        }
        lv_dropdown_set_selected(_dropdown_focus_music, dropdown_idx);
    } else {
        lv_dropdown_set_selected(_dropdown_focus_music, music_file_count);
    }
    
    // short break music selection
    if (short_break_music_file_path[0] != '\0') {
        int dropdown_idx = 0;
        for (int i = 0; i < music_file_count; i++) {
            if (strcmp(music_file_paths[i], short_break_music_file_path) == 0) {
                dropdown_idx = i;
                break;
            }
        }
        lv_dropdown_set_selected(_dropdown_short_music, dropdown_idx);
    } else {
        lv_dropdown_set_selected(_dropdown_short_music, music_file_count);
    }
    
    // long break music selection
    if (long_break_music_file_path[0] != '\0') {
        int dropdown_idx = 0;
        for (int i = 0; i < music_file_count; i++) {
            if (strcmp(music_file_paths[i], long_break_music_file_path) == 0) {
                dropdown_idx = i;
                break;
            }
        }
        lv_dropdown_set_selected(_dropdown_long_music, dropdown_idx);
    } else {
        lv_dropdown_set_selected(_dropdown_long_music, music_file_count);
    }
}

// hide settings popup
static void _hide_settings_popup(void)
{
    if (_settings_popup)
    {
        lv_obj_del(_settings_popup);
        _settings_popup = NULL;
        _label_sessions_count = NULL;
        _textarea_sessions = NULL;
        _roller_focus_min = NULL;
        _roller_focus_sec = NULL;
        _roller_short_min = NULL;
        _roller_short_sec = NULL;
        _roller_long_min = NULL;
        _roller_long_sec = NULL;
        
        
        // close keyboard if open
        if (_keyboard) {
            ui_keyboard_delete(_keyboard);
            _keyboard = NULL;
        }
    }
}

// update sess count display
static void _update_sessions_display(void)
{
    if (_textarea_sessions)
    {
        static char buf[8];
        sprintf(buf, "%d", temp_sessions_count);
        lv_textarea_set_text(_textarea_sessions, buf);
    }
}

// decrease session # button callback
static void _btn_decrease_sessions_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        if (temp_sessions_count > 1)  // Changed variable name
        {
            temp_sessions_count--;
            _update_sessions_display();
        }
    }
}

// increase session # button callback
static void _btn_increase_sessions_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        temp_sessions_count++; 
        _update_sessions_display();
    }
}

//edit session counter with keyboard
static void _textarea_sessions_focus_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_FOCUSED || event == LV_EVENT_CLICKED) {
        if (!_keyboard) {
            // move popup up before showing keyboard
            if (_settings_popup) {
                lv_obj_t *settings_cont = lv_obj_get_child(_settings_popup, NULL);
                if (settings_cont) {
                    lv_obj_align(settings_cont, NULL, LV_ALIGN_CENTER, 0, -80); // Move up by 80px
                }
            }
            _keyboard = ui_keyboard_create(obj);
        }
    }
    else if (event == LV_EVENT_DEFOCUSED || event == LV_EVENT_APPLY) {
        // update temp_sessions_count from textarea
        const char *text = lv_textarea_get_text(_textarea_sessions);
        int new_value = atoi(text);
        
        // min 1
        if (new_value < 1) {
            new_value = 1;
        }
        
        temp_sessions_count = new_value;
        _update_sessions_display();
        
        if (_keyboard) {
            ui_keyboard_delete(_keyboard);
            _keyboard = NULL;
            
            // move popup back to center after hiding keyboard
            if (_settings_popup) {
                lv_obj_t *settings_cont = lv_obj_get_child(_settings_popup, NULL);
                if (settings_cont) {
                    lv_obj_align(settings_cont, NULL, LV_ALIGN_CENTER, 0, 0);
                }
            }
        }
    }
}

// settings: confirm callback 
static void _settings_confirm_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        // Save session count
        focus_sessions_before_long_break = temp_sessions_count;
        
        // Get roller values and save
        uint16_t focus_min = lv_roller_get_selected(_roller_focus_min);
        uint16_t focus_sec = lv_roller_get_selected(_roller_focus_sec);
        custom_focus_time = focus_min * 60 + focus_sec;
        if (custom_focus_time < 1) custom_focus_time = 1;
        
        uint16_t short_min = lv_roller_get_selected(_roller_short_min);
        uint16_t short_sec = lv_roller_get_selected(_roller_short_sec);
        custom_short_break_time = short_min * 60 + short_sec;
        if (custom_short_break_time < 1) custom_short_break_time = 1;
        
        uint16_t long_min = lv_roller_get_selected(_roller_long_min);
        uint16_t long_sec = lv_roller_get_selected(_roller_long_sec);
        custom_long_break_time = long_min * 60 + long_sec;
        if (custom_long_break_time < 1) custom_long_break_time = 1;
        
        // save music selections - store full file paths
        uint16_t focus_dropdown_sel = lv_dropdown_get_selected(_dropdown_focus_music);
        if (focus_dropdown_sel >= music_file_count) {
            focus_music_file_path[0] = '\0';  // "None" selected
        } else {
            strncpy(focus_music_file_path, music_file_paths[focus_dropdown_sel], sizeof(focus_music_file_path) - 1);
        }
        ESP_LOGI("POMODORO", "focus music: dropdown=%d, path=%s", focus_dropdown_sel, focus_music_file_path);
        
        uint16_t short_dropdown_sel = lv_dropdown_get_selected(_dropdown_short_music);
        if (short_dropdown_sel >= music_file_count) {
            short_break_music_file_path[0] = '\0';  // "None" selected
        } else {
            strncpy(short_break_music_file_path, music_file_paths[short_dropdown_sel], sizeof(short_break_music_file_path) - 1);
        }
        ESP_LOGI("POMODORO", "short music: dropdown=%d, path=%s", short_dropdown_sel, short_break_music_file_path);
        
        uint16_t long_dropdown_sel = lv_dropdown_get_selected(_dropdown_long_music);
        if (long_dropdown_sel >= music_file_count) {
            long_break_music_file_path[0] = '\0';  // "None" selected
        } else {
            strncpy(long_break_music_file_path, music_file_paths[long_dropdown_sel], sizeof(long_break_music_file_path) - 1);
        }
        ESP_LOGI("POMODORO", "long music: dropdown=%d, path=%s", long_dropdown_sel, long_break_music_file_path);
        
        // Update current timer if in matching mode and not running
        if (!timer_running) {
            _set_timer_mode(current_mode);
            _update_timer_display();
        }
        
        _hide_settings_popup();
    }
}

// settings: cancel callback
static void _settings_cancel_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        // discard changes
        _hide_settings_popup();
    }
}

//add task button 
static void _btn_add_task_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        const char *task_text = lv_textarea_get_text(_textarea_task);
        if (strlen(task_text) == 0) return;

        // get scrollable part of the list
        lv_obj_t *list_page = lv_page_get_scrollable(_task_list);

        // checkbox + task text cont
        lv_obj_t *task_cont = lv_cont_create(_task_list, NULL);
        lv_obj_set_size(task_cont, lv_obj_get_width(_task_list) - 40, 40);
        lv_cont_set_layout(task_cont, LV_LAYOUT_ROW_MID);
        lv_obj_set_style_local_pad_inner(task_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
        lv_obj_set_style_local_border_width(task_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 1);
        lv_obj_set_style_local_border_color(task_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(200, 200, 200));
        lv_obj_set_style_local_radius(task_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);

        // add checkbox
        lv_obj_t *checkbox = lv_checkbox_create(task_cont, NULL);
        lv_checkbox_set_text(checkbox, task_text);
        lv_obj_set_event_cb(checkbox, _checkbox_event_cb);

        // clear input field
        lv_textarea_set_text(_textarea_task, "");
    }
}

//checkbox 
static void _checkbox_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        if (lv_checkbox_is_checked(obj))
        {
            // get parent cont, delete it (removes entire task)
            lv_obj_t *task_cont = lv_obj_get_parent(obj);
            lv_obj_del(task_cont);
        }
    }
}

//keyboard for task input
static void _textarea_focus_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_FOCUSED || event == LV_EVENT_CLICKED) {
        if (!_keyboard) {
            _keyboard = ui_keyboard_create(obj);
        }
    }
    else if (event == LV_EVENT_DEFOCUSED || event == LV_EVENT_APPLY) {
        if (_keyboard) {
            ui_keyboard_delete(_keyboard);
            _keyboard = NULL;
        }
    }
}

//analytics button
static void _btn_analytics_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        ui_pomodoro_timer_hide(NULL);
        ui_pomodoro_analytics_show(NULL);
    }
}

// Volume slider callback
static void _slider_volume_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED) {
        current_volume = lv_slider_get_value(obj);
        ESP_LOGI("POMODORO", "Volume set to: %d", current_volume);
        
        // always set codec volume immediately so it takes effect when music starts
        bsp_extra_codec_volume_set(current_volume, NULL);
    }
}

// Stop music playback
static void _stop_music(void)
{
    if (music_is_playing) {
        audio_player_stop();
        music_is_playing = false;
    }
    
    if (current_music_fp) {
        fclose(current_music_fp);
        current_music_fp = NULL;
    }
}

// play or resume music for current mode
static void _play_music_for_mode(timer_mode_t mode)
{
    ESP_LOGI("POMODORO", ">>> _play_music_for_mode START: mode=%d", mode);
    
    // get music file path for this mode
    char *music_file_path = NULL;
    
    switch (mode) {
        case MODE_FOCUS:
            music_file_path = focus_music_file_path;
            ESP_LOGI("POMODORO", "   mode=FOCUS");
            break;
        case MODE_SHORT_BREAK:
            music_file_path = short_break_music_file_path;
            ESP_LOGI("POMODORO", "   mode=SHORT_BREAK");
            break;
        case MODE_LONG_BREAK:
            music_file_path = long_break_music_file_path;
            ESP_LOGI("POMODORO", "   mode=LONG_BREAK");
            break;
    }
    
    ESP_LOGI("POMODORO", "   path=%s, playing=%d, paused=%d, fp=%p", 
             music_file_path ? music_file_path : "NULL", music_is_playing, music_is_paused, current_music_fp);
    
    // empty string means "None" selected - don't play anything
    if (!music_file_path || music_file_path[0] == '\0') {
        ESP_LOGI("POMODORO", "   no music selected");
        if (music_is_playing) {
            ESP_LOGI("POMODORO", "   stopping player");
            audio_player_stop();
            ESP_LOGI("POMODORO", "   stopped");
            music_is_playing = false;
            music_is_paused = false;
            // DON'T close file here - let audio_player manage it
            // fclose can cause mutex issues in LVGL context
        }
        currently_playing_path[0] = '\0';
        ESP_LOGI("POMODORO", "<<< _play_music_for_mode END (no music)");
        return;
    }
    
    // if same music is paused or playing, just resume it
    if ((music_is_paused || music_is_playing) && strcmp(currently_playing_path, music_file_path) == 0) {
        ESP_LOGI("POMODORO", "   resuming same music");
        audio_player_resume();
        ESP_LOGI("POMODORO", "   resumed");
        music_is_playing = true;
        music_is_paused = false;
        ESP_LOGI("POMODORO", "<<< _play_music_for_mode END (resumed)");
        return;
    }
    
    // different music or first time - stop old and start new
    ESP_LOGI("POMODORO", "   switching to new music");
    if (music_is_playing || music_is_paused) {
        ESP_LOGI("POMODORO", "   stopping old");
        audio_player_stop();
        ESP_LOGI("POMODORO", "   stopped");
        music_is_playing = false;
        music_is_paused = false;
        // DON'T close file - let audio_player manage it
    }
    // NOTE: Don't close old file pointers in LVGL context - causes mutex deadlock
    // The file pointer will be overwritten by the next fopen() call
    
    ESP_LOGI("POMODORO", "   opening: %s", music_file_path);
    current_music_fp = fopen(music_file_path, "rb");
    if (!current_music_fp) {
        ESP_LOGE("POMODORO", "   ERROR: failed to open");
        ESP_LOGI("POMODORO", "<<< _play_music_for_mode END (open failed)");
        return;
    }
    ESP_LOGI("POMODORO", "   opened fp=%p", current_music_fp);
    
    strncpy(currently_playing_path, music_file_path, sizeof(currently_playing_path) - 1);
    
    ESP_LOGI("POMODORO", "   set volume to %d", current_volume);
    bsp_extra_codec_volume_set(current_volume, NULL);
    
    ESP_LOGI("POMODORO", "   calling play");
    audio_player_play(current_music_fp);
    ESP_LOGI("POMODORO", "   play returned");
    music_is_playing = true;
    music_is_paused = false;
    
    ESP_LOGI("POMODORO", "<<< _play_music_for_mode END (new music playing)");
}

