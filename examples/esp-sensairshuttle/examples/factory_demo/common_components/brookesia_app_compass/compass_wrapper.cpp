/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "compass_wrapper.hpp"
#include "esp_brookesia_app_compass.hpp"
#include "esp_log.h"

static const char *TAG = "CompassWrapper";

namespace esp_brookesia::apps {

// Safely get Compass instance
static Compass *get_compass_instance_safe(void)
{
    Compass *instance = Compass::requestInstance();
    if (!instance) {
        ESP_LOGE(TAG, "Failed to get Compass instance");
        return nullptr;
    }
    return instance;
}

extern "C" bool compass_back(void)
{
    Compass *compass = get_compass_instance_safe();
    if (!compass) {
        return false;
    }

    compass->externalBack();

    ESP_LOGI(TAG, "Compass back called");
    return true;
}

extern "C" bool compass_set_correct(bool correct)
{
    Compass *compass = get_compass_instance_safe();
    if (!compass) {
        return false;
    }

    compass->setCorrect(correct);
    ESP_LOGI(TAG, "Compass calibration set to: %s", correct ? "true" : "false");
    return true;
}

extern "C" void compass_save_calibration_to_nvs_async(void)
{
    Compass *compass = get_compass_instance_safe();
    if (!compass) {
        ESP_LOGE(TAG, "Compass instance not available for async save");
        return;
    }

    compass->saveCalibrationToNVSAsync();
}

}
