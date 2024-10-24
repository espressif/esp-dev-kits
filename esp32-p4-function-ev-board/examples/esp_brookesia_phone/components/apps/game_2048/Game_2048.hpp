#pragma once

#include "nvs.h"
#include "lvgl.h"
#include "bsp_board_extra.h"
#include "esp_brookesia.hpp"

typedef struct {
	// Index of row
	int x;
	// Index of column
	int y;
	int weight;
} cell_weight_t;

class Game2048: public ESP_Brookesia_PhoneApp
{
public:
	Game2048();
	~Game2048();

    bool run(void);
    bool back(void);
    bool close(void);

    bool init(void) override;
    bool pause(void) override;
    bool resume(void) override;

    void showEmojiHello(void);
    void showEmojiScore(int score);
    void showEmojiGameOver(void);
    void debugCells(void);
    void debugCells(int cell[4][4]);
    void debugCells(lv_obj_t *cell[4][4]);
    void debugCells(cell_weight_t cell[4][4]);
    void debugCells(lv_obj_t *cell[4]);
    void cleanForegroundCells(void);
    void generateForegroundCell(void);
    void addRemoveReadyCell(lv_obj_t *cell);
    void cleanRemoveReadyCell(void);
    void newGame(void);
    void updateCellValue(void);
    void updateCurrentScore(int score);
    void updateBestScore(int score);
    void updateCellsStyle(void);
    int maxWeight(void);
    int moveLeft(void);
    int moveRight(void);
    int moveUp(void);
    int moveDown(void);
    bool isGameOver(void);

private:
    lv_obj_t *addBackgroundCell(lv_obj_t *parent);
    void startAnimationX(lv_obj_t *target, int x, int time);
    void startAnimationY(lv_obj_t *target, int y, int time);

    static void new_game_event_cb(lv_event_t *e);
    static void motion_event_cb(lv_event_t *e);
    static void anim_finish_cb(struct _lv_anim_t *a);

    bool _is_paused;
    uint16_t _height;
    uint16_t _width;
    uint16_t current_score;
    uint16_t best_score;
    uint16_t _weight_max;
    nvs_handle_t nvs_handle;
    file_iterator_instance_t *_file_iterator;
    cell_weight_t _cells_weight[4][4];
    lv_obj_t *_cur_score_label, *_best_score_label;
    lv_obj_t *_background_cells[4][4];
    lv_obj_t *_foreground_cells[4][4];
    lv_obj_t *_emoji_imgs[6];
    lv_obj_t *_emoji_label;
    lv_obj_t *_remove_ready_cells[4][4];
    lv_obj_t *_foreground_grid;
    lv_color_t  _cell_colors[11];
    const ESP_Brookesia_Gesture *_gesture;
};
