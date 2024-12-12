/* SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ui.h"
#include "app_ui_control.h"
#include "audio_player.h"
#include "lv_file_explorer.h"

typedef struct {
    ui_file_type_t type;
    uint8_t currentIndex;
    uint8_t totalNum;
    uint8_t *fileIndices;
} file_list_t;

static const char *TAG = "app_ui_control";
static file_list_t *g_file_list = NULL;
static lv_obj_t *g_tb = NULL;
static lv_obj_t *g_file_explorer = NULL;
static bool if_audio_playing = false;

static const char* getFileExtension(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (dot && dot != filename && dot[1] != '\0') {
        return dot;
    }
    return "";
}

static const char *get_real_path(const char * path)
{
    path++; /*Ignore the driver letter*/
    if(*path == ':') path++;

    return path;
}

static bool if_file_in_suffix(const char *fn, const char *suffix)
{

    size_t filename_len = strlen(fn);
    size_t suffix_len = strlen(suffix);
    if (filename_len >= suffix_len && strcmp(fn + filename_len - suffix_len, suffix) == 0) {
        return true;
    }
    return false;
}

static file_list_t *ui_control_get_file_list(lv_obj_t *obj)
{
    lv_obj_t * tb = lv_file_explorer_get_file_table(obj);
    const char *sel_fn = lv_file_explorer_get_selected_file_name(obj);
    const char *suffix = getFileExtension(sel_fn);

    uint16_t sum = lv_table_get_row_cnt(tb);
    file_list_t *file_list = (file_list_t *)calloc(1, sizeof(file_list_t));
    file_list->fileIndices = (uint8_t *)calloc(sum, sizeof(uint8_t));

    if (strcmp(suffix, ".JPG") == 0 || strcmp(suffix, ".jpg") == 0 ) {
        file_list->type = UI_FILE_TYPE_JPG;
    } else if (strcmp(suffix, ".PNG") == 0 || strcmp(suffix, ".png") == 0) {
        file_list->type = UI_FILE_TYPE_PNG;
    } else if (strcmp(suffix, ".BMP") == 0 || strcmp(suffix, ".bmp") == 0) {
        file_list->type = UI_FILE_TYPE_BMP;
    } else if (strcmp(suffix, ".GIF") == 0 || strcmp(suffix, ".gif") == 0) {
        file_list->type = UI_FILE_TYPE_GIF;
    } else if (strcmp(suffix, ".MP3") == 0 || strcmp(suffix, ".mp3") == 0) {
        file_list->type = UI_FILE_TYPE_MP3;
    } else {
        file_list->type = UI_FILE_TYPE_UNKNOWN;
    }

    uint16_t row, col = 0;
    lv_table_get_selected_cell(tb, &row, &col);
    for (uint16_t i = 0; i < sum; i++) {
        const char *fn = lv_table_get_cell_value(tb, i, 0);
        if (if_file_in_suffix(fn, suffix)) {
            file_list->fileIndices[file_list->totalNum++] = i;
            if (i == row) {
                file_list->currentIndex = file_list->totalNum - 1;
            }
        }
    }
    return file_list;
}

void ui_control_audio_control_stop(void)
{
    if (if_audio_playing) {
        audio_player_stop();
        if_audio_playing = false;
    }
}

void ui_control_display(void)
{
    const char *sel_fn = lv_table_get_cell_value(g_tb, g_file_list->fileIndices[g_file_list->currentIndex], 0);
    lv_obj_t *path_label = lv_file_explorer_get_path_label(g_file_explorer);
    const char *path = lv_label_get_text(path_label);
    char final_sel_fn[strlen(sel_fn) + strlen(path) + 1 + 1];
    memset(final_sel_fn, 0, sizeof(final_sel_fn));
    strncpy(final_sel_fn, path + 4, strlen(path)-4);
    if (path[strlen(path) - 1] != '/') {
        strcat(final_sel_fn, "/");
    }
    strcat(final_sel_fn, sel_fn + 5);
    ESP_LOGI(TAG, "final_sel_fn: %s", final_sel_fn);
    switch (g_file_list->type) {
        case UI_FILE_TYPE_JPG:
        case UI_FILE_TYPE_PNG:
        case UI_FILE_TYPE_BMP:
            lv_label_set_text(ui_Filename, sel_fn);
            lv_img_set_src(ui_displayObj, final_sel_fn);
            break;
        case UI_FILE_TYPE_GIF:
            lv_label_set_text(ui_Filename, sel_fn);
            lv_gif_set_src(ui_displayObj, final_sel_fn);
            break;
        case UI_FILE_TYPE_MP3:{
            lv_label_set_text(ui_Filename, sel_fn);
            lv_gif_set_src(ui_displayObj, &ui_gif_music_gif);
            FILE *fp = fopen(get_real_path(final_sel_fn), "rb");
            if (fp != NULL) {
                ESP_LOGD(TAG, "fp = %p\n", fp);
                audio_player_play(fp);
                if_audio_playing = true;
            }
            break;
        }
        default:
            break;
    }

    lv_label_set_text_fmt(ui_Label2, "%d", g_file_list->currentIndex + 1);
    lv_label_set_text_fmt(ui_Label1, "%d", g_file_list->totalNum);
}

void ui_control_up_display(void)
{
    g_file_list->currentIndex = (g_file_list->currentIndex + g_file_list->totalNum - 1) % g_file_list->totalNum;
    ui_control_display();
}

void ui_control_down_display(void)
{
    g_file_list->currentIndex = (g_file_list->currentIndex + 1) % g_file_list->totalNum;
    ui_control_display();
}

static void ui_control_process(lv_obj_t *obj)
{
    g_file_list = ui_control_get_file_list(obj);
    g_tb = lv_file_explorer_get_file_table(obj);
    g_file_explorer = obj;
    if (ui_displayPage_create_dispalyObj(g_file_list->type)) {
        ui_control_display();
        _ui_screen_change(ui_displayPage, LV_SCR_LOAD_ANIM_NONE, 0, 1000);
    }
}

static void file_explorer_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        const char * cur_path =  lv_file_explorer_get_current_path(obj);
        const char * sel_fn = lv_file_explorer_get_selected_file_name(obj);
        uint16_t path_len = strlen(cur_path);
        uint16_t fn_len = strlen(sel_fn);

        if((path_len + fn_len) <= LV_FILE_EXPLORER_PATH_MAX_LEN) {
            ui_control_process(obj);
        } else {
            LV_LOG_USER("%s%s", cur_path, sel_fn);
            printf("file_info: %s%s\n", cur_path, sel_fn);
        } 
    }
}

void lv_example_file_explorer_1(void)
{
    lv_obj_t * file_explorer = lv_file_explorer_create(ui_HomePage);
    lv_file_explorer_set_sort(file_explorer, LV_EXPLORER_SORT_KIND);

#if LV_USE_FS_WIN32
    lv_file_explorer_open_dir(file_explorer, "S:");
#if LV_FILE_EXPLORER_QUICK_ACCESS
    lv_file_explorer_set_quick_access_path(file_explorer, LV_EXPLORER_HOME_DIR, "S:/Users/Public/Desktop");
    lv_file_explorer_set_quick_access_path(file_explorer, LV_EXPLORER_VIDEO_DIR, "S:/Users/Public/Videos");
    lv_file_explorer_set_quick_access_path(file_explorer, LV_EXPLORER_PICTURES_DIR, "S:/Users/Public/Pictures");
    lv_file_explorer_set_quick_access_path(file_explorer, LV_EXPLORER_MUSIC_DIR, "S:/Users/Public/Music");
    lv_file_explorer_set_quick_access_path(file_explorer, LV_EXPLORER_DOCS_DIR, "S:/Users/Public/Documents");
    lv_file_explorer_set_quick_access_path(file_explorer, LV_EXPLORER_FS_DIR, "S:");
#endif

#else
    /* linux */
    lv_file_explorer_open_dir(file_explorer, "S:/");

#if LV_FILE_EXPLORER_QUICK_ACCESS
    char * envvar = "HOME";
    char home_dir[LV_FS_MAX_PATH_LENGTH];
    strcpy(home_dir, "S:");
    // get the user's home directory from the HOME enviroment variable
    strcat(home_dir, getenv(envvar));
    LV_LOG_USER("home_dir: %s\n", home_dir);
    lv_file_explorer_set_quick_access_path(file_explorer, LV_EXPLORER_HOME_DIR, home_dir);
    char video_dir[LV_FS_MAX_PATH_LENGTH];
    strcpy(video_dir, home_dir);
    strcat(video_dir, "/Videos");
    lv_file_explorer_set_quick_access_path(file_explorer, LV_EXPLORER_VIDEO_DIR, video_dir);
    char picture_dir[LV_FS_MAX_PATH_LENGTH];
    strcpy(picture_dir, home_dir);
    strcat(picture_dir, "/Pictures");
    lv_file_explorer_set_quick_access_path(file_explorer, LV_EXPLORER_PICTURES_DIR, picture_dir);
    char music_dir[LV_FS_MAX_PATH_LENGTH];
    strcpy(music_dir, home_dir);
    strcat(music_dir, "/Music");
    lv_file_explorer_set_quick_access_path(file_explorer, LV_EXPLORER_MUSIC_DIR, music_dir);
    char document_dir[LV_FS_MAX_PATH_LENGTH];
    strcpy(document_dir, home_dir);
    strcat(document_dir, "/Documents");
    lv_file_explorer_set_quick_access_path(file_explorer, LV_EXPLORER_DOCS_DIR, document_dir);

    lv_file_explorer_set_quick_access_path(file_explorer, LV_EXPLORER_FS_DIR, "S:/");
#endif
#endif

    lv_obj_add_event_cb(file_explorer, file_explorer_event_handler, LV_EVENT_ALL, NULL);
}

void ui_control_msc_push(void)
{
    _ui_image_set_property(ui_imageMSC, _UI_IMAGE_PROPERTY_IMAGE, (uint8_t *)& ui_img_pushmscbig_png);
    _ui_label_set_property(ui_startLable, _UI_LABEL_PROPERTY_TEXT, "The USB stick is ready");
}

void ui_control_msc_ready(void)
{
    _ui_screen_change(ui_HomePage, LV_SCR_LOAD_ANIM_OVER_LEFT, 500, 0);
    lv_example_file_explorer_1();
}

void ui_control_msc_pull(void)
{
    _ui_image_set_property(ui_imageMSC, _UI_IMAGE_PROPERTY_IMAGE, (uint8_t *)& ui_img_pullmscbig_png);
    _ui_label_set_property(ui_startLable, _UI_LABEL_PROPERTY_TEXT, "Please insert a USB stick");
    _ui_screen_change(ui_StartPage, LV_SCR_LOAD_ANIM_OVER_RIGHT, 500, 0);
    ui_displayObj_clear();
    ui_control_audio_control_stop();
}