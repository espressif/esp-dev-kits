#pragma once

#include "lvgl.h"
#include "esp_brookesia.hpp"

class Calculator: public ESP_Brookesia_PhoneApp
{
public:
	Calculator();
	~Calculator();

    bool run(void);
    bool back(void);
    bool close(void);

    bool init(void) override;

    bool isStartZero(void);
    bool isStartNum(void);
    bool isStartPercent(void);
    bool isLegalDot(void);
    double calculate(const char *input);

    int formula_len;
    lv_obj_t *keyboard;
    lv_obj_t *history_label;
    lv_obj_t *formula_label;
    lv_obj_t *result_label;
    uint16_t _height;
    uint16_t _width;

private:
    static void keyboard_event_cb(lv_event_t *e);
};
