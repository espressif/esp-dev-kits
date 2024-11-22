/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "lvgl.h"
#include "esp_brookesia.hpp"
#include "app_video.h"
#include "esp_video_init.h"

class Camera: public ESP_Brookesia_PhoneApp {
public:
    Camera(uint16_t hor_res, uint16_t ver_res);
    ~Camera();

    bool run(void);
    bool pause(void);
    bool resume(void);
    bool back(void);
    bool close(void);

    bool init(void) override;

private:
    static void taskCameraInit(Camera *app);
    static void onScreenCameraShotBtnClick(lv_event_t *e);
    static void onScreenCameraShotAlbumClick(lv_event_t *e);
    static void camera_dectect_task(Camera *app);

    enum {
        SCREEN_CAMERA_SHOT,
        SCREEN_CAMERA_PHOTO,
        SCREEN_CAMERA_AI,
    } _screen_index;
    uint16_t _hor_res;
    uint16_t _ver_res;
    uint16_t _img_album_dsc_size;
    uint32_t _img_album_buf_bytes;
    uint8_t *_img_album_buffer;
    SemaphoreHandle_t _camera_init_sem;
    int _camera_ctlr_handle;
    lv_img_dsc_t _img_refresh_dsc;
    lv_img_dsc_t _img_album_dsc;
    lv_img_dsc_t _img_photo_dsc;
    lv_obj_t *_img_album;
    TaskHandle_t _detect_task_handle;
    uint8_t *_cam_buffer[EXAMPLE_CAM_BUF_NUM];
    size_t _cam_buffer_size[EXAMPLE_CAM_BUF_NUM];
};

