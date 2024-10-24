#include <cstdio>
#include <cstdlib>
#include <ctime>
#include "bsp/esp-bsp.h"
#include "Game_2048.hpp"
#include "esp_log.h"

#define ENABLE_CELL_DEBUG       (0)

#define BOARD_BG_COLOR			lv_color_white()
#define BOARD_TITLE_FONT		&lv_font_montserrat_28
#define BOARD_TITLE_COLOR		lv_palette_main(LV_PALETTE_BROWN)
#define SCORE_TITLE_FONT		&lv_font_montserrat_24
#define SCORE_TITLE_COLOR		lv_color_white()
#define SCORE_CONTEN_FONT		&lv_font_montserrat_28
#define SCORE_HEIGHT			100
#define SCORE_WIDTH				150

#define GRID_BG_COLOR			lv_palette_main(LV_PALETTE_BROWN)
#define GRID_FONT				&lv_font_montserrat_38
#define GRID_PAD				15

#define CELL_BG_COLOR			lv_color_make(255, 255, 255)
#define CELL_RADIUS				5
#define CELL_SIZE				((short int)((_width - 5 * GRID_PAD) / 4))
#define CELL_OPA_1				LV_OPA_10
#define CELL_OPA_2				LV_OPA_COVER

#define ANIM_PERIOD				200

#define EMOJI_SCORE_NORMAL		8
#define EMOJI_SCORE_GOOD		64

#define NVS_STORAGE_NAMESPACE   "storage"
#define NVS_BEST_SCORE          "score"
#define MUSIC_DIR               "/2048"
#define MUSIC_WEAK              BSP_SPIFFS_MOUNT_POINT MUSIC_DIR "/weak.mp3"
#define MUSIC_NORM              BSP_SPIFFS_MOUNT_POINT MUSIC_DIR "/normal.mp3"
#define MUSIC_GOOD              BSP_SPIFFS_MOUNT_POINT MUSIC_DIR "/good.mp3"
#define MUSIC_EXCL              BSP_SPIFFS_MOUNT_POINT MUSIC_DIR "/excellent.mp3"

#define randint_between(min, max)		(rand() % (max - min) + min)
#define rand_1_2()						(randint_between(1, 2))

using namespace std;

static const char *TAG = "Game2048";
static bool anim_running_flag = false;
static bool generate_cell_flag = false;

LV_IMG_DECLARE(img_app_2048);
LV_IMG_DECLARE(img_game2048_excellent);
LV_IMG_DECLARE(img_game2048_fail);
LV_IMG_DECLARE(img_game2048_good);
LV_IMG_DECLARE(img_game2048_normal);
LV_IMG_DECLARE(img_game2048_week);
LV_IMG_DECLARE(img_game2048_hello);

Game2048::Game2048():
    ESP_Brookesia_PhoneApp("2048 Game", &img_app_2048, true),
    _is_paused(false),
    _height(0),
    _width(0),
    current_score(0),
    best_score(0),
    _weight_max(0),
    nvs_handle(NULL),
    _file_iterator(NULL),
    _cur_score_label(NULL),
    _best_score_label(NULL),
    _background_cells({}),
    _emoji_imgs({}),
    _emoji_label(NULL),
    _foreground_grid(NULL)
{
    best_score = 0;
    for (int i = 0; i < 16; i++) {
        _cells_weight[i/4][i%4].weight = 0;
        _cells_weight[i/4][i%4].x = 1 << (i/4);
        _cells_weight[i/4][i%4].y = 1 << (i%4);
        _foreground_cells[i/4][i%4] = NULL;
        _remove_ready_cells[i/4][i%4] = NULL;
    }
    _cell_colors[0] = CELL_BG_COLOR;
    // Yellow
    _cell_colors[1] = lv_color_make(0xFF, 0xFF, 0x99);
    _cell_colors[2] = lv_color_make(0xFF, 0xFF, 0x33);
    // Orange
    _cell_colors[3] = lv_color_make(0xFF, 0xCC, 0x99);
    _cell_colors[4] = lv_color_make(0xFF, 0xCC, 0x33);
    // Green
    _cell_colors[5] = lv_color_make(0x00, 0xCC, 0x99);
    _cell_colors[6] = lv_color_make(0x00, 0xCC, 0x66);
    // Blue
    _cell_colors[7] = lv_color_make(0x00, 0x66, 0xFF);
    _cell_colors[8] = lv_color_make(0x00, 0x33, 0x99);
    // Red
    _cell_colors[9] = lv_color_make(0xFF, 0x33, 0x99);
    _cell_colors[10] = lv_color_make(0xFF, 0x33, 0x00);
}


Game2048::~Game2048()
{
}

bool Game2048::run(void)
{
    srand(time(NULL));

    // lv_area_t area = getVisualArea();
    // _width = area.x2 - area.x1;
    // _height = area.y2 - area.y1;
    _width = 400;
    _height = 600;

    /* Setup board */
    lv_obj_t *board = lv_obj_create(lv_scr_act());
    // Size
    lv_obj_set_size(board, _width, _height - _width);
    // Position
    lv_obj_align(board, LV_ALIGN_TOP_MID, 0, 0);
    // Shape
    lv_obj_set_style_radius(board, 0, 0);
    lv_obj_set_style_border_width(board, 0, 0);
    lv_obj_set_style_pad_all(board, 0, 0);
    // Background
    lv_obj_set_style_bg_color(board, BOARD_BG_COLOR, 0);

    lv_obj_t *title = lv_label_create(board);
    lv_obj_set_style_text_font(title, BOARD_TITLE_FONT, 0);
    lv_obj_set_style_text_color(title,BOARD_TITLE_COLOR, 0);
    lv_label_set_text(title, "2048");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 20, 20);

    const lv_img_dsc_t *imgs[] = {
        &img_game2048_hello,
        &img_game2048_week,
        &img_game2048_normal,
        &img_game2048_good,
        &img_game2048_excellent,
        &img_game2048_fail
    };
    for (int i = 0; i < 6; i++) {
        _emoji_imgs[i] = lv_img_create(board);
        lv_img_set_src(_emoji_imgs[i], imgs[i]);
        lv_obj_align_to(_emoji_imgs[i], title, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
        lv_obj_add_flag(_emoji_imgs[i], LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_t *cur = lv_obj_create(lv_scr_act());
    // Size
    lv_obj_set_size(cur, SCORE_WIDTH, SCORE_HEIGHT);
    // Position
    lv_obj_align(cur, LV_ALIGN_BOTTOM_LEFT, 60, -100);
    // Shape
    lv_obj_set_style_radius(cur, CELL_RADIUS, 0);
    lv_obj_set_style_border_width(cur, 0, 0);
    lv_obj_set_style_pad_all(cur, 0, 0);
    // Background
    lv_obj_set_style_bg_color(cur, GRID_BG_COLOR, 0);

    title = lv_label_create(cur);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(title,SCORE_TITLE_COLOR, 0);
    lv_label_set_text(title, "SCORE");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    _cur_score_label = lv_label_create(cur);
    lv_obj_set_style_text_font(_cur_score_label, SCORE_CONTEN_FONT, 0);
    lv_obj_set_style_text_color(_cur_score_label, SCORE_TITLE_COLOR, 0);
    lv_label_set_text(_cur_score_label, "0");
    lv_obj_align(_cur_score_label, LV_ALIGN_BOTTOM_MID, 0, -10);

    lv_obj_t *best = lv_obj_create(lv_scr_act());
    // Size
    lv_obj_set_size(best, SCORE_WIDTH, SCORE_HEIGHT);
    // Position
    // lv_obj_align_to(best, cur, LV_ALIGN_TOP_LEFT, 0, 50);
    lv_obj_align(best, LV_ALIGN_TOP_LEFT, 60, 130);
    // Shape
    lv_obj_set_style_radius(best, CELL_RADIUS, 0);
    lv_obj_set_style_border_width(best, 0, 0);
    lv_obj_set_style_pad_all(best, 0, 0);
    // Background
    lv_obj_set_style_bg_color(best, GRID_BG_COLOR, 0);

    title = lv_label_create(best);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(title,SCORE_TITLE_COLOR, 0);
    lv_label_set_text(title, "BEST");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    _best_score_label = lv_label_create(best);
    lv_obj_set_style_text_font(_best_score_label, SCORE_CONTEN_FONT, 0);
    lv_obj_set_style_text_color(_best_score_label,SCORE_TITLE_COLOR, 0);
    lv_label_set_text_fmt(_best_score_label, "%d", best_score);
    lv_obj_align(_best_score_label, LV_ALIGN_BOTTOM_MID, 0, -10);

    lv_obj_t *btn = lv_btn_create(board);
    // Size
    lv_obj_set_size(btn, 150, 70);
    // Position
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -10, -90);
    // Shape
    lv_obj_set_style_radius(btn, CELL_RADIUS, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_pad_all(btn, 10, 0);
    // Background
    lv_obj_set_style_bg_color(btn, GRID_BG_COLOR, 0);
    // Others
    lv_obj_add_event_cb(btn, new_game_event_cb, LV_EVENT_CLICKED, this);

    title = lv_label_create(btn);
    lv_obj_set_style_text_font(title, SCORE_TITLE_FONT, 0);
    lv_obj_set_style_text_color(title,SCORE_TITLE_COLOR, 0);
    lv_label_set_text(title, "New Game");
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    /* Setup grid */
    static lv_coord_t col_dsc[] = {CELL_SIZE, CELL_SIZE, CELL_SIZE, CELL_SIZE, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {CELL_SIZE, CELL_SIZE, CELL_SIZE, CELL_SIZE, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *background_grid = lv_obj_create(lv_scr_act());
    // Size
    lv_obj_set_size(background_grid, _width, _width);
    // Position
    lv_obj_align(background_grid, LV_ALIGN_BOTTOM_MID, 0, 0);
    // Shape
    lv_obj_set_style_radius(background_grid, 0, 0);
    lv_obj_set_style_border_width(background_grid, 0, 0);
    lv_obj_set_style_pad_all(background_grid, 0, 0);
    // Background
    lv_obj_set_style_bg_color(background_grid, GRID_BG_COLOR, 0);
    // Font
    lv_obj_set_style_text_font(background_grid, GRID_FONT, 0);
    // Layout
    lv_obj_set_style_grid_column_dsc_array(background_grid, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(background_grid, row_dsc, 0);
    lv_obj_set_layout(background_grid, LV_LAYOUT_GRID);
    lv_obj_set_grid_align(background_grid, LV_GRID_ALIGN_CENTER, LV_GRID_ALIGN_CENTER);

    lv_obj_t *cell;
    for (int i = 0; i < 16; i++) {
        cell = addBackgroundCell(background_grid);
        _background_cells[i/4][i%4] = cell;
        lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, i % 4, 1,
                                   LV_GRID_ALIGN_STRETCH, i / 4, 1);
        lv_obj_update_layout(cell);
    }

    _foreground_grid = lv_obj_create(lv_scr_act());
    // Size
    lv_obj_set_size(_foreground_grid, _width, _width);
    // Position
    lv_obj_align(_foreground_grid, LV_ALIGN_BOTTOM_MID, 0, 0);
    // Shape
    lv_obj_set_style_radius(_foreground_grid, 0, 0);
    lv_obj_set_style_border_width(_foreground_grid, 0, 0);
    // lv_obj_set_style_border_color(_foreground_grid, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_pad_all(_foreground_grid, 0, 0);
    // Background
    lv_obj_set_style_bg_opa(_foreground_grid, LV_OPA_TRANSP, 0);
    // Font
    lv_obj_set_style_text_font(_foreground_grid, GRID_FONT, 0);
    // Others
    lv_obj_add_flag(_foreground_grid, LV_OBJ_FLAG_CLICKABLE);

    _emoji_label = lv_label_create(board);
    lv_obj_set_style_text_font(_emoji_label, SCORE_TITLE_FONT, 0);
    lv_obj_align(_emoji_label, LV_ALIGN_BOTTOM_LEFT, 10, -40);
    lv_label_set_text(_emoji_label, "Welcome to paly!");
    lv_obj_add_flag(_emoji_label, LV_OBJ_FLAG_HIDDEN);

    /* Add motion detect module */
    lv_obj_add_event_cb(_gesture->getEventObj(), motion_event_cb, _gesture->getReleaseEventCode(), this);

    _is_paused = false;

    newGame();

    return true;
}

/**
 * @brief The function will be called when the left button of navigate bar is clicked.
 */
bool Game2048::back(void)
{
    notifyCoreClosed();

    return true;
}

/**
 * @brief The function will be called when app should be closed.
 */
bool Game2048::close(void)
{
    lv_obj_remove_event_cb(_gesture->getEventObj(), motion_event_cb);

    return true;
}

bool Game2048::init(void)
{
    esp_err_t err = ESP_OK;

    ESP_Brookesia_Phone *phone = getPhone();
    ESP_Brookesia_PhoneManager& manager = phone->getManager();
    _gesture = manager.getGesture();

    if (bsp_extra_player_init() != ESP_OK) {
        ESP_LOGE(TAG, "Play init with SPIFFS failed");
        return false;
    }

    if (bsp_extra_file_instance_init(BSP_SPIFFS_MOUNT_POINT MUSIC_DIR, &_file_iterator) != ESP_OK) {
        ESP_LOGE(TAG, "bsp_extra_file_instance_init failed");
        return false;
    }

    err = nvs_open(NVS_STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return false;
    }

    int32_t value = 0;
    err = nvs_get_i32(nvs_handle, NVS_BEST_SCORE, &value);
    switch (err) {
    case ESP_OK:
        ESP_LOGI(TAG, "Load %s: %d", NVS_BEST_SCORE, value);
        break;
    case ESP_ERR_NVS_NOT_FOUND:
        err = nvs_set_i32(nvs_handle, NVS_BEST_SCORE, value);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) setting %s", esp_err_to_name(err), NVS_BEST_SCORE);
        }
        ESP_LOGW(TAG, "The value of %s is not initialized yet, set it to default value: %d", NVS_BEST_SCORE, value);
        break;
    default:
        return false;
    }
    best_score = value;

    return true;
}

bool Game2048::pause(void)
{
    _is_paused = true;

    return true;
}

bool Game2048::resume(void)
{
    _is_paused = false;

    return true;
}

void Game2048::showEmojiHello(void)
{
    for (int i = 0; i < 6; i++) {
        if (i != 0) {
            lv_obj_add_flag(_emoji_imgs[i], LV_OBJ_FLAG_HIDDEN);
        }
        else {
            lv_obj_clear_flag(_emoji_imgs[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    lv_label_set_text(_emoji_label, "Welcome to play!");
}

void Game2048::showEmojiScore(int score)
{
    int index = 0;

    if (score < 0) {
        return;
    } else if (score == 0) {
        index = 1;
        lv_label_set_text_fmt(_emoji_label, "Score[%d]: Weak...", score);
        bsp_extra_player_play_file(MUSIC_WEAK);
    } else if (score < EMOJI_SCORE_NORMAL) {
        index = 2;
        lv_label_set_text_fmt(_emoji_label, "Score[%d]: Normal.", score);
        bsp_extra_player_play_file(MUSIC_NORM);
    }
    else if (score < EMOJI_SCORE_GOOD) {
        index = 3;
        lv_label_set_text_fmt(_emoji_label, "Score[%d]: Good!", score);
        bsp_extra_player_play_file(MUSIC_GOOD);
    }
    else {
        index = 4;
        lv_label_set_text_fmt(_emoji_label, "Score[%d]: Excellent!", score);
        bsp_extra_player_play_file(MUSIC_EXCL);
    }

    for (int i = 0; i < 6; i++) {
        if (i != index) {
            lv_obj_add_flag(_emoji_imgs[i], LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(_emoji_imgs[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void Game2048::showEmojiGameOver(void)
{
    for (int i = 0; i < 6; i++) {
        if (i != 5) {
            lv_obj_add_flag(_emoji_imgs[i], LV_OBJ_FLAG_HIDDEN);
        }
        else {
            lv_obj_clear_flag(_emoji_imgs[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    lv_label_set_text(_emoji_label, "Sorry, game over!");
}

void Game2048::debugCells(void)
{
#if ENABLE_CELL_DEBUG
    debugCells(_foreground_cells);
    debugCells(_background_cells);
    debugCells(_cells_weight);
#endif
}

void Game2048::debugCells(int cell[4][4])
{
#if ENABLE_CELL_DEBUG
    printf("cell \n");
    for (int i = 0; i < 4; i++) {
        printf("\t%d\t%d\t%d\t%d\n", cell[i][0], cell[i][1], cell[i][2], cell[i][3]);
    }
    printf("\n");
#endif
}

void Game2048::debugCells(lv_obj_t *cell[4][4])
{
#if ENABLE_CELL_DEBUG
    for (int i = 0; i < 4; i++) {
        printf("\t%p\t%p\t%p\t%p\n", cell[i][0], cell[i][1], cell[i][2], cell[i][3]);
    }
    printf("\n");
#endif
}

void Game2048::debugCells(cell_weight_t cell[4][4])
{
#if ENABLE_CELL_DEBUG
    for (int i = 0; i < 4; i++) {
        printf(
            "\t(%d,%d,%d)\t(%d,%d,%d)\t(%d,%d,%d)\t(%d,%d,%d)\n",
            cell[i][0].x, cell[i][0].y, cell[i][0].weight,
            cell[i][1].x, cell[i][1].y, cell[i][1].weight,
            cell[i][2].x, cell[i][2].y, cell[i][2].weight,
            cell[i][3].x, cell[i][3].y, cell[i][3].weight
        );
    }
    printf("\n");
#endif
}

void Game2048::debugCells(lv_obj_t *cell[4])
{
#if ENABLE_CELL_DEBUG
    for (int i = 0; i < 4; i++) {
        printf("\t%p\t%p\t%p\t%p\n", cell[i], cell[i], cell[i], cell[i]);
    }
    printf("\n");
#endif
}

void Game2048::cleanForegroundCells(void)
{
    lv_obj_t *child = lv_obj_get_child(_foreground_grid, 0);

    while (child) {
        lv_obj_del(child);
        child = lv_obj_get_child(_foreground_grid, 0);
    }
    for (int i = 0; i < 16; i++) {
        _cells_weight[i/4][i%4].weight = 0;
        _cells_weight[i/4][i%4].x = 1 << (i/4);
        _cells_weight[i/4][i%4].y = 1 << (i%4);
        _foreground_cells[i/4][i%4] = NULL;
    }
}

void Game2048::newGame(void)
{
    _weight_max = 0;
    current_score = 0;
    updateCurrentScore(current_score);
    cleanForegroundCells();
    generateForegroundCell();
    generateForegroundCell();
    updateCellsStyle();
    showEmojiHello();
}

lv_obj_t *Game2048::addBackgroundCell(lv_obj_t *parent)
{
    lv_obj_t *cell = lv_obj_create(parent);
    // Shape
    lv_obj_set_style_radius(cell, CELL_RADIUS, 0);
    lv_obj_set_style_border_width(cell, 0, 0);
    lv_obj_set_style_pad_all(cell, 0, 0);
    // Background
    lv_obj_set_style_bg_color(cell, CELL_BG_COLOR, 0);
    lv_obj_set_style_bg_opa(cell, CELL_OPA_1, 0);

    return cell;
}

void Game2048::generateForegroundCell(void)
{
    int zero_amount = 0, zero_index = 0;
    int target = 0;
    int target_i = 0;
    int target_j = 0;
    int target_weight = rand_1_2();

    _weight_max = (target_weight > _weight_max) ?
                   target_weight : _weight_max;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (_cells_weight[i][j].weight == 0) {
                zero_amount++;
            }
        }
    }
    if (zero_amount == 0) {
        return;
    }

    target = randint_between(0, zero_amount);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if ((_cells_weight[i][j].weight == 0) &&
                (zero_index++ == target)) {
                    target_i = i;
                    target_j = j;
                    _cells_weight[i][j].weight = target_weight;
            }
        }
    }

    /* Add a new object of cell */
    lv_obj_t *cell = lv_obj_create(_foreground_grid);
    _foreground_cells[target_i][target_j] = cell;
    // Size
    lv_obj_set_size(cell, CELL_SIZE, CELL_SIZE);
    // Position
    lv_obj_set_pos(
        cell,
        lv_obj_get_x(_background_cells[target_i][target_j]),
        lv_obj_get_y(_background_cells[target_i][target_j])
    );
    // Shape
    lv_obj_set_style_radius(cell, CELL_RADIUS, 0);
    lv_obj_set_style_border_width(cell, 0, 0);
    lv_obj_set_style_pad_all(cell, 0, 0);
    // Background
    lv_obj_set_style_bg_color(cell, CELL_BG_COLOR, 0);
    lv_obj_set_style_opa(cell, CELL_OPA_2, 0);
    // Others
    lv_obj_clear_flag(cell, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *label = lv_label_create(cell);
    lv_label_set_text_fmt(label, "%d", (int)lv_pow(2, target_weight));
    lv_obj_center(label);

    debugCells();
}

void Game2048::addRemoveReadyCell(lv_obj_t *cell)
{
    if (cell == NULL) {
        return;
    }
    for (int i = 0; i < 16; i++) {
        if (_remove_ready_cells[i/4][i%4] == cell) {
            return;
        }
    }
    for (int i = 0; i < 16; i++) {
        if (_remove_ready_cells[i/4][i%4] == NULL) {
            _remove_ready_cells[i/4][i%4] = cell;
            return;
        }
    }
}

void Game2048::cleanRemoveReadyCell(void)
{
    for (int i = 0; i < 16; i++) {
        if (_remove_ready_cells[i/4][i%4] != NULL) {
            lv_obj_del(_remove_ready_cells[i/4][i%4]);
            _remove_ready_cells[i/4][i%4] = NULL;
        }
    }
}

void Game2048::startAnimationX(lv_obj_t *target, int x, int time)
{
    lv_anim_t a;

    anim_running_flag = true;
    lv_anim_init(&a);
    lv_anim_set_var(&a, target);
    lv_anim_set_user_data(&a, this);
    lv_obj_update_layout(target);
    lv_anim_set_values(&a, lv_obj_get_x(target), x);
    lv_anim_set_time(&a, time);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_set_ready_cb(&a, anim_finish_cb);
    lv_anim_start(&a);
}

void Game2048::startAnimationY(lv_obj_t *target, int y, int time)
{
    lv_anim_t a;

    anim_running_flag = true;
    lv_anim_init(&a);
    lv_anim_set_var(&a, target);
    lv_anim_set_user_data(&a, this);
    lv_obj_update_layout(target);
    lv_anim_set_values(&a, lv_obj_get_y(target), y);
    lv_anim_set_time(&a, time);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_set_ready_cb(&a, anim_finish_cb);
    lv_anim_start(&a);
}

void Game2048::updateCellValue(void)
{
    for (int i = 0; i < 16; i++) {
        if (_foreground_cells[i/4][i%4] != NULL) {
            lv_obj_t *label = lv_obj_get_child(_foreground_cells[i/4][i%4], 0);
            lv_label_set_text_fmt(label, "%d", lv_pow(2, _cells_weight[i/4][i%4].weight));
        }
    }
}

void Game2048::updateCurrentScore(int score)
{
    lv_label_set_text_fmt(_cur_score_label, "%d", score);
}

void Game2048::updateBestScore(int score)
{
    lv_label_set_text_fmt(_best_score_label, "%d", score);

    esp_err_t err = nvs_set_i32(nvs_handle, NVS_BEST_SCORE, score);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) setting %s", esp_err_to_name(err), NVS_BEST_SCORE);
    }
}

void Game2048::updateCellsStyle(void)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (_foreground_cells[i][j] != NULL) {
                lv_obj_set_style_bg_color(
                    _foreground_cells[i][j],
                    _cell_colors[_cells_weight[i][j].weight - 1],
                    0
                );
            }
        }
    }
}

int Game2048::maxWeight(void)
{
    return _weight_max;
}

int Game2048::moveLeft(void)
{
    int target_cells[4][4];
    int score = -1;

    for (int i = 0; i < 16; i++) {
        target_cells[i/4][i%4] = i%4;
    }

    debugCells(_cells_weight);
    debugCells(target_cells);

    for (int i = 0; i < 4; i++) {
        for (int j = 1; j < 4; j++) {
            if (_cells_weight[i][j].weight != 0) {
                int merge_flag = false;
                int cur_j = j;
                while (cur_j != 0) {
                    if ((_cells_weight[i][cur_j-1].weight == _cells_weight[i][cur_j].weight) &&
                        (!merge_flag)) {
                        merge_flag = true;
                        for (int k = 0; k < 4; k++) {
                            int target_flag = _cells_weight[i][cur_j].y & (1 << k);
                            if (target_flag) {
                                target_cells[i][k] = cur_j - 1;
                            }
                        }

                        for (int k = 0; k < 4; k++) {
                            int target_flag = _cells_weight[i][cur_j-1].y & (1 << k);
                            if (target_flag) {
                                addRemoveReadyCell(_foreground_cells[i][k]);
                            }
                        }

                        _cells_weight[i][cur_j-1].y += _cells_weight[i][cur_j].y;
                        _cells_weight[i][cur_j-1].weight++;
                        _cells_weight[i][cur_j].weight = 0;
                        _weight_max = (_cells_weight[i][cur_j-1].weight > _weight_max) ?
                                       _cells_weight[i][cur_j-1].weight : _weight_max;

                        score = (score < 0) ? 0 : score;
                        score += lv_pow(2, _cells_weight[i][cur_j-1].weight);
                    }
                    else if (_cells_weight[i][cur_j-1].weight == 0) {

                        for (int k = 0; k < 4; k++) {
                            int target_flag = _cells_weight[i][cur_j].y & (1 << k);
                            if (target_flag) {
                                target_cells[i][k] = cur_j - 1;
                            }
                        }

                        score = (score < 0) ? 0 : score;
                        _cells_weight[i][cur_j-1] = _cells_weight[i][cur_j];
                        _cells_weight[i][cur_j].weight = 0;
                    }
                    else {
                        break;
                    }
                    cur_j--;
                }
            }
        }
    }

    debugCells(_cells_weight);
    debugCells(target_cells);
    debugCells(_remove_ready_cells);
    debugCells(_foreground_cells);

    for (int i = 0; i < 4; i++) {
        for (int j = 1; j < 4; j++) {
            int target_j = target_cells[i][j];
            if ((_foreground_cells[i][j] != NULL) && (j != target_j)) {
                // printf(NULL, "i,j: %d,%d", i, j);
                int target_x = lv_obj_get_x(_background_cells[i][target_j]);
                startAnimationX(_foreground_cells[i][j], target_x, ANIM_PERIOD);
                _foreground_cells[i][target_j] = _foreground_cells[i][j];
                _foreground_cells[i][j] = NULL;
            }
        }
    }

    debugCells(_foreground_cells);

    // Reset x,y of weight
    for (int i = 0; i < 16; i++) {
        _cells_weight[i/4][i%4].x = 1 << (i/4);
        _cells_weight[i/4][i%4].y = 1 << (i%4);
    }

    return score;
}

int Game2048::moveRight(void)
{
    int target_cells[4][4];
    int score = -1;

    for (int i = 0; i < 16; i++) {
        target_cells[i/4][i%4] = i%4;
    }

    debugCells(_cells_weight);
    debugCells(target_cells);

    for (int i = 0; i < 4; i++) {
        for (int j = 2; j >= 0; j--) {
            if (_cells_weight[i][j].weight != 0) {
                int merge_flag = false;
                int cur_j = j;
                while (cur_j != 3) {
                    if ((_cells_weight[i][cur_j+1].weight == _cells_weight[i][cur_j].weight) &&
                        (!merge_flag)) {
                        merge_flag = true;
                        for (int k = 0; k < 4; k++) {
                            int target_flag = _cells_weight[i][cur_j].y & (1 << k);
                            if (target_flag) {
                                target_cells[i][k] = cur_j + 1;
                            }
                        }

                        for (int k = 0; k < 4; k++) {
                            int target_flag = _cells_weight[i][cur_j+1].y & (1 << k);
                            if (target_flag) {
                                addRemoveReadyCell(_foreground_cells[i][k]);
                            }
                        }

                        _cells_weight[i][cur_j+1].y += _cells_weight[i][cur_j].y;
                        _cells_weight[i][cur_j+1].weight++;
                        _cells_weight[i][cur_j].weight = 0;
                        _weight_max = (_cells_weight[i][cur_j+1].weight > _weight_max) ?
                                       _cells_weight[i][cur_j+1].weight : _weight_max;

                        score = (score < 0) ? 0 : score;
                        score += lv_pow(2, _cells_weight[i][cur_j+1].weight);
                    }
                    else if (_cells_weight[i][cur_j+1].weight == 0) {

                        for (int k = 0; k < 4; k++) {
                            int target_flag = _cells_weight[i][cur_j].y & (1 << k);
                            if (target_flag) {
                                target_cells[i][k] = cur_j + 1;
                            }
                        }

                        score = (score < 0) ? 0 : score;
                        _cells_weight[i][cur_j+1] = _cells_weight[i][cur_j];
                        _cells_weight[i][cur_j].weight = 0;
                    }
                    else {
                        break;
                    }
                    cur_j++;
                }
            }
        }
    }

    debugCells(_cells_weight);
    debugCells(target_cells);
    debugCells(_remove_ready_cells);
    debugCells(_foreground_cells);

    for (int i = 0; i < 4; i++) {
        for (int j = 2; j >= 0; j--) {
            int target_j = target_cells[i][j];
            if ((_foreground_cells[i][j] != NULL) && (j != target_j)) {
                // printf(NULL, "i,j: %d,%d", i, j);
                int target_x = lv_obj_get_x(_background_cells[i][target_j]);
                startAnimationX(_foreground_cells[i][j], target_x, ANIM_PERIOD);
                _foreground_cells[i][target_j] = _foreground_cells[i][j];
                _foreground_cells[i][j] = NULL;
            }
        }
    }

    debugCells(_foreground_cells);

    // Reset x,y of weight
    for (int i = 0; i < 16; i++) {
        _cells_weight[i/4][i%4].x = 1 << (i/4);
        _cells_weight[i/4][i%4].y = 1 << (i%4);
    }

    return score;
}

int Game2048::moveUp(void)
{
    int target_cells[4][4];
    int score = -1;

    for (int i = 0; i < 16; i++) {
        target_cells[i/4][i%4] = i/4;
    }

    debugCells(_cells_weight);
    debugCells(target_cells);

    for (int j = 0; j < 4; j++) {
        for (int i = 1; i < 4; i++) {
            if (_cells_weight[i][j].weight != 0) {
                int merge_flag = false;
                int cur_i = i;
                while (cur_i != 0) {
                    if ((_cells_weight[cur_i-1][j].weight == _cells_weight[cur_i][j].weight) &&
                        (!merge_flag)) {
                        merge_flag = true;
                        for (int k = 0; k < 4; k++) {
                            int target_flag = _cells_weight[cur_i][j].x & (1 << k);
                            if (target_flag) {
                                target_cells[k][j] = cur_i - 1;
                            }
                        }

                        for (int k = 0; k < 4; k++) {
                            int target_flag = _cells_weight[cur_i-1][j].x & (1 << k);
                            if (target_flag) {
                                addRemoveReadyCell(_foreground_cells[k][j]);
                            }
                        }

                        _cells_weight[cur_i-1][j].x += _cells_weight[cur_i][j].x;
                        _cells_weight[cur_i-1][j].weight++;
                        _cells_weight[cur_i][j].weight = 0;
                        _weight_max = (_cells_weight[cur_i-1][j].weight > _weight_max) ?
                                       _cells_weight[cur_i-1][j].weight : _weight_max;

                        score = (score < 0) ? 0 : score;
                        score += lv_pow(2, _cells_weight[cur_i-1][j].weight);
                    }
                    else if (_cells_weight[cur_i-1][j].weight == 0) {

                        for (int k = 0; k < 4; k++) {
                            int target_flag = _cells_weight[cur_i][j].x & (1 << k);
                            if (target_flag) {
                                target_cells[k][j] = cur_i - 1;
                            }
                        }

                        score = (score < 0) ? 0 : score;
                        _cells_weight[cur_i-1][j] = _cells_weight[cur_i][j];
                        _cells_weight[cur_i][j].weight = 0;
                    }
                    else {
                        break;
                    }
                    cur_i--;
                }
            }
        }
    }

    debugCells(_cells_weight);
    debugCells(target_cells);
    debugCells(_remove_ready_cells);
    debugCells(_foreground_cells);

    for (int j = 0; j < 4; j++) {
        for (int i = 1; i < 4; i++) {
            int target_i = target_cells[i][j];
            if ((_foreground_cells[i][j] != NULL) && (i != target_i)) {
                // printf(NULL, "i,j: %d,%d", i, j);
                int target_y = lv_obj_get_y(_background_cells[target_i][j]);
                startAnimationY(_foreground_cells[i][j], target_y, ANIM_PERIOD);
                _foreground_cells[target_i][j] = _foreground_cells[i][j];
                _foreground_cells[i][j] = NULL;
            }
        }
    }

    debugCells(_foreground_cells);

    // Reset x,y of weight
    for (int i = 0; i < 16; i++) {
        _cells_weight[i/4][i%4].x = 1 << (i/4);
        _cells_weight[i/4][i%4].y = 1 << (i%4);
    }

    return score;
}

int Game2048::moveDown(void)
{
    int target_cells[4][4];
    int score = -1;

    for (int i = 0; i < 16; i++) {
        target_cells[i/4][i%4] = i/4;
    }

    debugCells(_cells_weight);
    debugCells(target_cells);

    for (int j = 0; j < 4; j++) {
        for (int i = 2; i >= 0; i--) {
            if (_cells_weight[i][j].weight != 0) {
                int merge_flag = false;
                int cur_i = i;
                while (cur_i != 3) {
                    if ((_cells_weight[cur_i+1][j].weight == _cells_weight[cur_i][j].weight) &&
                        (!merge_flag)) {
                        merge_flag = true;
                        for (int k = 0; k < 4; k++) {
                            int target_flag = _cells_weight[cur_i][j].x & (1 << k);
                            if (target_flag) {
                                target_cells[k][j] = cur_i + 1;
                            }
                        }

                        for (int k = 0; k < 4; k++) {
                            int target_flag = _cells_weight[cur_i+1][j].x & (1 << k);
                            if (target_flag) {
                                addRemoveReadyCell(_foreground_cells[k][j]);
                            }
                        }

                        _cells_weight[cur_i+1][j].x += _cells_weight[cur_i][j].x;
                        _cells_weight[cur_i+1][j].weight++;
                        _cells_weight[cur_i][j].weight = 0;
                        _weight_max = (_cells_weight[cur_i+1][j].weight > _weight_max) ?
                                       _cells_weight[cur_i+1][j].weight : _weight_max;

                        score = (score < 0) ? 0 : score;
                        score += lv_pow(2, _cells_weight[cur_i+1][j].weight);
                    }
                    else if (_cells_weight[cur_i+1][j].weight == 0) {

                        for (int k = 0; k < 4; k++) {
                            int target_flag = _cells_weight[cur_i][j].x & (1 << k);
                            if (target_flag) {
                                target_cells[k][j] = cur_i + 1;
                            }
                        }

                        score = (score < 0) ? 0 : score;
                        _cells_weight[cur_i+1][j] = _cells_weight[cur_i][j];
                        _cells_weight[cur_i][j].weight = 0;
                    }
                    else {
                        break;
                    }
                    cur_i++;
                }
            }
        }
    }

    debugCells(_cells_weight);
    debugCells(target_cells);
    debugCells(_remove_ready_cells);
    debugCells(_foreground_cells);

    for (int j = 0; j < 4; j++) {
        for (int i = 2; i >= 0; i--) {
            int target_i = target_cells[i][j];
            if ((_foreground_cells[i][j] != NULL) && (i != target_i)) {
                // printf(NULL, "i,j: %d,%d", i, j);
                int target_y = lv_obj_get_y(_background_cells[target_i][j]);
                startAnimationY(_foreground_cells[i][j], target_y, ANIM_PERIOD);
                _foreground_cells[target_i][j] = _foreground_cells[i][j];
                _foreground_cells[i][j] = NULL;
            }
        }
    }

    debugCells(_foreground_cells);

    // Reset x,y of weight
    for (int i = 0; i < 16; i++) {
        _cells_weight[i/4][i%4].x = 1 << (i/4);
        _cells_weight[i/4][i%4].y = 1 << (i%4);
    }

    return score;
}

bool Game2048::isGameOver(void)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 1; j < 4; j++) {
            if ((_cells_weight[i][j].weight == _cells_weight[i][j-1].weight) ||
                (_cells_weight[j][i].weight == _cells_weight[j-1][i].weight) ||
                (_cells_weight[i][j].weight == 0)) {
                    return false;
            }
        }
    }

    return true;
}

void Game2048::new_game_event_cb(lv_event_t *e)
{
    Game2048 *app= (Game2048 *)lv_event_get_user_data(e);

    app->newGame();
}

void Game2048::motion_event_cb(lv_event_t *e)
{
    int score;
    ESP_Brookesia_GestureInfo_t *type = (ESP_Brookesia_GestureInfo_t *)lv_event_get_param(e);
    Game2048 *app= (Game2048 *)lv_event_get_user_data(e);

    if (app->_is_paused) {
        return;
    }

    if (!anim_running_flag) {
        switch (type->direction) {
            case ESP_BROOKESIA_GESTURE_DIR_UP:
                // printf(NULL, "up");
                score = app->moveUp();
                break;
            case ESP_BROOKESIA_GESTURE_DIR_DOWN:
                // printf(NULL, "down");
                score = app->moveDown();
                break;
            case ESP_BROOKESIA_GESTURE_DIR_LEFT:
                // printf(NULL, "left");
                score = app->moveLeft();
                break;
            case ESP_BROOKESIA_GESTURE_DIR_RIGHT:
                // printf(NULL, "right");
                score = app->moveRight();
                break;
            default:
                return;
        }

        printf("score: %d\n", score);

        if (score >= 0) {
            generate_cell_flag = true;
            app->showEmojiScore(score);
            app->current_score += score;
            app->updateCurrentScore(app->current_score);
            if (app->current_score > app->best_score) {
                app->best_score = app->current_score;
                app->updateBestScore(app->best_score);
            }
        }
        if (app->maxWeight() == 2048) {
            printf("Congratualation! You win!\n");
            app->newGame();
        }
        if (app->isGameOver()) {
            printf("Game Over\n");
            app->showEmojiGameOver();
        }
    }
}

void Game2048::anim_finish_cb(struct _lv_anim_t *a)
{
    Game2048 *app= (Game2048 *)lv_anim_get_user_data(a);

    app->cleanRemoveReadyCell();
    app->updateCellValue();
    if (generate_cell_flag) {
        generate_cell_flag = false;
        app->generateForegroundCell();
        app->updateCellsStyle();
    }
    if (anim_running_flag) {
        anim_running_flag = false;
    }
}
