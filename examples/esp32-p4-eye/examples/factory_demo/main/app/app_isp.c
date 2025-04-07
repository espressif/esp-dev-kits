/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/errno.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_check.h"

#include "linux/videodev2.h"
#include "esp_video_isp_ioctl.h"
#include "esp_video_device.h"

static const char *TAG = "app_isp";

static int video_fd = -1;

esp_err_t app_isp_init(int cam_fd)
{
    video_fd = open(ESP_VIDEO_ISP1_DEVICE_NAME, O_RDWR);

    return ESP_OK;
}

esp_err_t app_isp_set_contrast(uint32_t percent)
{
    uint32_t contrast_val = 0xff * percent / 100;

    esp_err_t ret;
    struct v4l2_ext_controls controls;
    struct v4l2_ext_control control[1];

    controls.ctrl_class = V4L2_CID_USER_CLASS;
    controls.count      = 1;
    controls.controls   = control;
    control[0].id       = V4L2_CID_CONTRAST;
    control[0].value     = contrast_val;

    ret = ioctl(video_fd, VIDIOC_S_EXT_CTRLS, &controls);
    if (ret != 0) {
        ESP_LOGE(TAG, "failed to set contrast");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t app_isp_set_saturation(uint32_t percent)
{
    uint32_t sat_val = 0xff * percent / 100;

    esp_err_t ret;
    struct v4l2_ext_controls controls;
    struct v4l2_ext_control control[1];

    controls.ctrl_class = V4L2_CID_USER_CLASS;
    controls.count      = 1;
    controls.controls   = control;
    control[0].id       = V4L2_CID_SATURATION;
    control[0].value     = sat_val;
    ret = ioctl(video_fd, VIDIOC_S_EXT_CTRLS, &controls);
    if (ret != 0) {
        ESP_LOGE(TAG, "failed to set saturation");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t app_isp_set_brightness(uint32_t percent)
{
    int32_t bright_val = 0xff * percent / 100 - 127;

    esp_err_t ret;
    struct v4l2_ext_controls controls;
    struct v4l2_ext_control control[1];

    controls.ctrl_class = V4L2_CID_USER_CLASS;
    controls.count      = 1;
    controls.controls   = control;
    control[0].id       = V4L2_CID_BRIGHTNESS;
    control[0].value     = bright_val;
    ret = ioctl(video_fd, VIDIOC_S_EXT_CTRLS, &controls);
    if (ret != 0) {
        ESP_LOGE(TAG, "failed to set brightness");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t app_isp_set_hue(uint32_t percent)
{
    uint32_t hue_val = 360 * percent / 100;

    esp_err_t ret;
    struct v4l2_ext_controls controls;
    struct v4l2_ext_control control[1];

    controls.ctrl_class = V4L2_CID_USER_CLASS;
    controls.count      = 1;
    controls.controls   = control;
    control[0].id       = V4L2_CID_HUE;
    control[0].value     = hue_val; 
    ret = ioctl(video_fd, VIDIOC_S_EXT_CTRLS, &controls);
    if (ret != 0) {
        ESP_LOGE(TAG, "failed to set hue");
        return ESP_FAIL;
    }

    return ESP_OK;
}