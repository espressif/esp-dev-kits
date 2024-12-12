/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "lvgl.h"
#include "file_iterator.h"
#include "esp_brookesia.hpp"

class MusicPlayer: public ESP_Brookesia_PhoneApp {
public:
    MusicPlayer();
    ~MusicPlayer();

    bool run(void);
    bool back(void);
    bool close(void);

    bool init(void) override;
    bool pause(void) override;

private:
    file_iterator_instance_t *_file_iterator;
};
