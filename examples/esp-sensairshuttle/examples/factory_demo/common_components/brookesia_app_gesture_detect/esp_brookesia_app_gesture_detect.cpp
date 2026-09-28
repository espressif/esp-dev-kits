/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "esp_brookesia_app_gesture_detect.hpp"
#include "esp_log.h"
#include "ui/ui.h"
#include "ui/ui_app.h"
#include "bmi270_api.h"
#include "driver/gpio.h"
#include "esp_lib_utils.h"
#include "esp_board_manager.h"
#include <cstddef>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "GestureDetectApp";

using namespace esp_brookesia::systems::phone;

#define APP_NAME "Gesture Detect"

#define I2C_MASTER_SDO_IO GPIO_NUM_9
#define I2C_INT_IO GPIO_NUM_28

LV_IMG_DECLARE(img_app_gesture_detect);

namespace esp_brookesia::apps {

constexpr systems::base::App::Config CORE_DATA = {
    .name = APP_NAME,
    .launcher_icon = gui::StyleImage::IMAGE(&img_app_gesture_detect),
    .screen_size = gui::StyleSize::RECT_PERCENT(100, 100),
    .flags = {
        .enable_default_screen = 1,
        .enable_recycle_resource = 0,
        .enable_resize_visual_area = 1,
    },
};
constexpr App::Config APP_DATA = {
    .app_launcher_page_index = 0,
    .flags = {
        .enable_navigation_gesture = 1,
    },
};

GestureDetect::GestureDetect()
    : App(CORE_DATA, APP_DATA)
{
}

GestureDetect::~GestureDetect()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();
    deinit();
}

GestureDetect *GestureDetect::requestInstance()
{
    if (_instance == nullptr) {
        _instance = new GestureDetect();
    }
    return _instance;
}

bool GestureDetect::init()
{
    return true;
}

bool GestureDetect::deinit()
{
    return true;
}

bool GestureDetect::run()
{

    if (!initSensors()) {
        ESP_LOGE(TAG, "Failed to initialize sensors");
        is_initialized_ = false;
        is_app_running_ = false;
    } else {
        is_initialized_ = true;
        is_app_running_ = true;
    }

    gesture_detect_queue_handle_ = xQueueCreate(5, sizeof(int));
    if (gesture_detect_queue_handle_ == nullptr) {
        ESP_LOGE(TAG, "Failed to create gesture detect queue");
        return false;
    }

    createGestureDetectUI();

    gesture_detect_thread_ = boost::thread(&GestureDetect::gestureDetectThread, this);
    gesture_detect_event_thread_ = boost::thread(&GestureDetect::gestureDetectEventThread, this);

    return true;
}

bool GestureDetect::back()
{
    return close();
}

bool GestureDetect::close()
{
    return true;
}

bool GestureDetect::pause()
{

    is_app_running_ = false;
    if (!deinitSensors()) {
        ESP_LOGE(TAG, "Failed to deinitialize sensors");
    }
    is_initialized_ = false;

    return true;
}
bool GestureDetect::resume()
{

    if (!initSensors()) {
        ESP_LOGE(TAG, "Failed to initialize sensors");
        is_initialized_ = false;
    } else {
        is_initialized_ = true;
    }
    is_app_running_ = true;

    return true;
}

void GestureDetect::createGestureDetectUI()
{
    gesture_detect_ui_init(is_initialized_);
}

void GestureDetect::destroyGestureDetectUI()
{
    gesture_detect_ui_destroy();
}

int8_t GestureDetect::set_accel_gyro_config(bmi270_handle_t bmi2_dev)
{
    int8_t rslt;
    struct bmi2_sens_config config[2];
    struct bmi2_int_pin_config pin_config = { 0 };

    config[BMI2_ACCEL].type = BMI2_ACCEL;
    config[BMI2_GYRO].type = BMI2_GYRO;

    rslt = bmi2_get_int_pin_config(&pin_config, bmi2_dev);
    bmi2_error_codes_print_result(rslt);

    rslt = bmi2_get_sensor_config(config, 2, bmi2_dev);
    bmi2_error_codes_print_result(rslt);

    if (rslt == BMI2_OK) {
        config[BMI2_ACCEL].cfg.acc.odr = BMI2_ACC_ODR_200HZ;
        config[BMI2_ACCEL].cfg.acc.range = BMI2_ACC_RANGE_16G;
        config[BMI2_ACCEL].cfg.acc.bwp = BMI2_ACC_NORMAL_AVG4;
        config[BMI2_ACCEL].cfg.acc.filter_perf = BMI2_PERF_OPT_MODE;

        config[BMI2_GYRO].cfg.gyr.odr = BMI2_GYR_ODR_200HZ;
        config[BMI2_GYRO].cfg.gyr.range = BMI2_GYR_RANGE_2000;
        config[BMI2_GYRO].cfg.gyr.bwp = BMI2_GYR_NORMAL_MODE;
        config[BMI2_GYRO].cfg.gyr.noise_perf = BMI2_POWER_OPT_MODE;
        config[BMI2_GYRO].cfg.gyr.filter_perf = BMI2_PERF_OPT_MODE;
        pin_config.pin_type = BMI2_INT1;
        pin_config.pin_cfg[0].input_en = BMI2_INT_INPUT_DISABLE;
        pin_config.pin_cfg[0].lvl = BMI2_INT_ACTIVE_LOW;
        pin_config.pin_cfg[0].od = BMI2_INT_PUSH_PULL;
        pin_config.pin_cfg[0].output_en = BMI2_INT_OUTPUT_ENABLE;
        pin_config.int_latch = BMI2_INT_LATCH;

        rslt = bmi2_set_int_pin_config(&pin_config, bmi2_dev);
        bmi2_error_codes_print_result(rslt);

        rslt = bmi2_set_sensor_config(config, 2, bmi2_dev);
        bmi2_error_codes_print_result(rslt);
    }

    return rslt;
}

int8_t GestureDetect::set_feature_interrupt(bmi270_handle_t bmi2_dev)
{
    int8_t rslt;
    uint8_t data = BMI270_TOY_INT_TOY_MOTION_MASK | BMI270_TOY_INT_SHAKE_MASK | BMI270_TOY_INT_GI_INS1_ROLLING_MASK;
    struct bmi2_int_pin_config pin_config = { 0 };

    rslt = bmi2_set_regs(BMI2_INT1_MAP_FEAT_ADDR, &data, 1, bmi2_dev);
    bmi2_error_codes_print_result(rslt);

    if (rslt == BMI2_OK) {
        pin_config.pin_type = BMI2_INT1;
        pin_config.pin_cfg[0].input_en = BMI2_INT_INPUT_DISABLE;
        pin_config.pin_cfg[0].lvl = BMI2_INT_ACTIVE_HIGH;
        pin_config.pin_cfg[0].od = BMI2_INT_PUSH_PULL;
        pin_config.pin_cfg[0].output_en = BMI2_INT_OUTPUT_ENABLE;
        pin_config.int_latch = BMI2_INT_LATCH;

        rslt = bmi2_set_int_pin_config(&pin_config, bmi2_dev);
        bmi2_error_codes_print_result(rslt);
    }

    return rslt;
}

bool GestureDetect::initSensors()
{
    std::lock_guard<std::mutex> guard(sensor_mutex_);
    if (!deinitSensorsLocked()) {
        return false;
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << I2C_MASTER_SDO_IO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SDO pin configuration failed: %s", esp_err_to_name(ret));
    }
    gpio_set_level(I2C_MASTER_SDO_IO, 0);

    ret = esp_board_manager_get_periph_handle("i2c_master", (void **)&i2c_bus_);
    if (ret != ESP_OK || i2c_bus_ == nullptr) {
        ESP_LOGE(TAG, "Failed to get I2C master bus from Board Manager: %s", esp_err_to_name(ret));
        return false;
    }

    ret = bmi270_sensor_create_from_master_bus(i2c_bus_, &bmi_handle_, bmi270_toy_config_file, 0);
    if (ret != ESP_OK || bmi_handle_ == NULL) {
        ESP_LOGE(TAG, "BMI270 TOY create failed: %s", esp_err_to_name(ret));
        return false;
    }

    int8_t rslt;
    uint8_t sens_list[2] = { BMI2_ACCEL, BMI2_GYRO };

    rslt = bmi2_set_adv_power_save(BMI2_DISABLE, bmi_handle_);
    if (rslt != BMI2_OK) {
        ESP_LOGE(TAG, "BMI270 set adv power save failed: %d", rslt);
        deinitSensorsLocked();
        return false;
    }

    rslt = set_accel_gyro_config(bmi_handle_);
    if (rslt != BMI2_OK) {
        ESP_LOGE(TAG, "BMI270 set accel gyro config failed: %d", rslt);
        deinitSensorsLocked();
        return false;
    }

    rslt = set_feature_interrupt(bmi_handle_);
    if (rslt != BMI2_OK) {
        ESP_LOGE(TAG, "BMI270 set feature interrupt failed: %d", rslt);
        deinitSensorsLocked();
        return false;
    }

    rslt = bmi2_sensor_enable(sens_list, 2, bmi_handle_);
    if (rslt != BMI2_OK) {
        ESP_LOGE(TAG, "BMI270 enable sensors failed: %d", rslt);
        deinitSensorsLocked();
        return false;
    }

    rslt = bmi270_enable_toy_motion(bmi_handle_, BMI2_ENABLE);
    if (rslt != BMI2_OK) {
        ESP_LOGE(TAG, "BMI270 enable toy motion failed: %d", rslt);
        deinitSensorsLocked();
        return false;
    }

    rslt = bmi270_enable_toy_shake(bmi_handle_, BMI2_ENABLE);
    if (rslt != BMI2_OK) {
        ESP_LOGE(TAG, "BMI270 enable toy shake failed: %d", rslt);
        deinitSensorsLocked();
        return false;
    }

    rslt = bmi270_enable_toy_rolling(bmi_handle_, BMI2_ENABLE);
    if (rslt != BMI2_OK) {
        ESP_LOGE(TAG, "BMI270 enable toy rolling failed: %d", rslt);
        deinitSensorsLocked();
        return false;
    }
    ESP_LOGI(TAG, "BMI270 roll feature enabled");
    return true;
}

bool GestureDetect::deinitSensors()
{
    std::lock_guard<std::mutex> guard(sensor_mutex_);
    return deinitSensorsLocked();
}

bool GestureDetect::deinitSensorsLocked()
{
    if (bmi_handle_) {
        esp_err_t ret = bmi270_sensor_del(&bmi_handle_);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "BMI270 deletion failed: %s", esp_err_to_name(ret));
            return false;
        }
    }
    return true;
}

int8_t GestureDetect::readSensorRegs(uint8_t reg_addr, uint8_t *data, uint16_t length)
{
    std::lock_guard<std::mutex> guard(sensor_mutex_);
    if (!is_app_running_ || !is_initialized_ || bmi_handle_ == nullptr) {
        return BMI2_E_NULL_PTR;
    }
    return bmi2_get_regs(reg_addr, data, length, bmi_handle_);
}

void GestureDetect::gestureDetectThread()
{
    while (1) {
        if (!is_app_running_) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        int gesture_type;
        if (gesture_detect_queue_handle_ != nullptr) {
            xQueueReceive(gesture_detect_queue_handle_, &gesture_type, portMAX_DELAY);

            if (gesture_type == GESTURE_DETECT_EVENT_SHAKE) {
                pending_gesture_ = GESTURE_SHAKE;
            } else if (gesture_type == (0x10 + 0x01)) {
                pending_gesture_ = GESTURE_PICKUP;
            } else if (gesture_type == (0x10 + 0x02)) {
                pending_gesture_ = GESTURE_PUTDOWN;
            } else if (gesture_type == GESTURE_DETECT_EVENT_ROLL) {
                pending_gesture_ = GESTURE_ROLL;
            } else {
                continue;
            }

            lv_async_call([](void *user_data) {
                GestureDetect *self = static_cast<GestureDetect *>(user_data);
                if (self->pending_gesture_ != GESTURE_NONE) {
                    update_gesture_ui(self->pending_gesture_);
                    if (self->pending_gesture_ == GESTURE_ROLL ||
                            self->pending_gesture_ == GESTURE_SHAKE) {
                        update_gesture_detail(self->gesture_detail_text_);
                    } else {
                        update_gesture_detail("");
                    }
                }
            }, this);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void GestureDetect::gestureDetectEventThread()
{
    while (1) {
        if (!is_app_running_ || !is_initialized_) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        uint8_t int_status = 0;
        int8_t rslt = BMI2_E_NULL_PTR;
        const int max_retries = 3;
        for (int retry = 0; retry < max_retries; retry++) {
            rslt = readSensorRegs(BMI2_INT_STATUS_0_ADDR, &int_status, 1);
            if (rslt == BMI2_OK) {
                consecutive_i2c_failures_ = 0;
                break;
            }
            if (retry < max_retries - 1) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
        if (rslt != BMI2_OK) {
            if (!is_app_running_) {
                continue;
            }
            consecutive_i2c_failures_++;
            const uint32_t MAX_CONSECUTIVE_FAILURES = 20;
            if (consecutive_i2c_failures_ >= MAX_CONSECUTIVE_FAILURES) {
                ESP_LOGW(TAG, "Too many I2C failures (%u), attempting sensor reinitialization...", consecutive_i2c_failures_);
                deinitSensors();
                vTaskDelay(pdMS_TO_TICKS(100));
                if (initSensors()) {
                    ESP_LOGI(TAG, "Sensor reinitialization successful");
                    consecutive_i2c_failures_ = 0;
                } else {
                    ESP_LOGE(TAG, "Sensor reinitialization failed");
                    vTaskDelay(pdMS_TO_TICKS(500));
                    continue;
                }
            } else {
                TickType_t delay_ms = 20;
                if (consecutive_i2c_failures_ > 10) {
                    delay_ms = 100;
                } else if (consecutive_i2c_failures_ > 5) {
                    delay_ms = 50;
                }
                vTaskDelay(pdMS_TO_TICKS(delay_ms));
            }
            continue;
        }

        int gesture_type = 0;
        TickType_t current_time = xTaskGetTickCount();
        const TickType_t SHAKE_DEBOUNCE_MS = pdMS_TO_TICKS(500);
        const TickType_t LEFT_GESTURE_DEBOUNCE_MS = pdMS_TO_TICKS(1000);

        if (int_status & BMI270_TOY_INT_SHAKE_MASK) {
            uint8_t data;
            rslt = readSensorRegs(0x1f, &data, 1);

            if (rslt == BMI2_OK) {
                if (data & 0x80) {
                    if (last_left_gesture_time_ > 0 &&
                            (current_time - last_left_gesture_time_) < LEFT_GESTURE_DEBOUNCE_MS) {
                        vTaskDelay(pdMS_TO_TICKS(10));
                        continue;
                    }
                    gesture_type = GESTURE_DETECT_EVENT_SHAKE;
                    last_shake_time_ = current_time;
                    last_left_gesture_time_ = current_time;
                    char axis_str[16] = "";
                    if (data & 0x10) {
                        strcat(axis_str, "X");
                    }
                    if (data & 0x20) {
                        if (strlen(axis_str) > 0) {
                            strcat(axis_str, "/");
                        }
                        strcat(axis_str, "Y");
                    }
                    if (data & 0x40) {
                        if (strlen(axis_str) > 0) {
                            strcat(axis_str, "/");
                        }
                        strcat(axis_str, "Z");
                    }
                    if (strlen(axis_str) == 0) {
                        strcpy(axis_str, "?");
                    }
                    snprintf(gesture_detail_text_, sizeof(gesture_detail_text_), "Shake heavy on %s", axis_str);
                } else if (data & 0x08) {
                    if (last_left_gesture_time_ > 0 &&
                            (current_time - last_left_gesture_time_) < LEFT_GESTURE_DEBOUNCE_MS) {
                        vTaskDelay(pdMS_TO_TICKS(10));
                        continue;
                    }
                    gesture_type = GESTURE_DETECT_EVENT_SHAKE;
                    last_shake_time_ = current_time;
                    last_left_gesture_time_ = current_time;
                    char axis_str[16] = "";
                    if (data & 0x01) {
                        strcat(axis_str, "X");
                    }
                    if (data & 0x02) {
                        if (strlen(axis_str) > 0) {
                            strcat(axis_str, "/");
                        }
                        strcat(axis_str, "Y");
                    }
                    if (data & 0x04) {
                        if (strlen(axis_str) > 0) {
                            strcat(axis_str, "/");
                        }
                        strcat(axis_str, "Z");
                    }
                    if (strlen(axis_str) == 0) {
                        strcpy(axis_str, "?");
                    }
                    snprintf(gesture_detail_text_, sizeof(gesture_detail_text_), "Shake slight on %s", axis_str);
                }
            }
        } else if (int_status & BMI270_TOY_INT_TOY_MOTION_MASK) {
            if (last_shake_time_ > 0 &&
                    (current_time - last_shake_time_) < SHAKE_DEBOUNCE_MS) {
                vTaskDelay(pdMS_TO_TICKS(10));
                continue;
            }

            uint8_t data;
            rslt = readSensorRegs(0x1e, &data, 1);
            if (rslt == BMI2_OK) {
                uint8_t raw_gesture_type = (data & 0x1c) >> 2;
                if (raw_gesture_type == 0x01 || raw_gesture_type == 0x02) {
                    if (last_left_gesture_time_ > 0 &&
                            (current_time - last_left_gesture_time_) < LEFT_GESTURE_DEBOUNCE_MS) {
                        vTaskDelay(pdMS_TO_TICKS(10));
                        continue;
                    }
                    gesture_type = 0x10 + raw_gesture_type;
                    last_left_gesture_time_ = current_time;
                }
            }
        } else if (int_status & BMI270_TOY_INT_GI_INS1_ROLLING_MASK) {
            uint8_t data;
            rslt = readSensorRegs(0x1e, &data, 1);
            if (rslt == BMI2_OK) {
                data = data >> 5;
                switch (data) {
                case 1:
                    snprintf(gesture_detail_text_, sizeof(gesture_detail_text_), "X Up [^^^]");
                    break;
                case 2:
                    snprintf(gesture_detail_text_, sizeof(gesture_detail_text_), "X Down [vvv]");
                    break;
                case 3:
                    snprintf(gesture_detail_text_, sizeof(gesture_detail_text_), "Y Down [>>>]");
                    break;
                case 4:
                    snprintf(gesture_detail_text_, sizeof(gesture_detail_text_), "Y Up [<<<]");
                    break;
                case 5:
                    snprintf(gesture_detail_text_, sizeof(gesture_detail_text_), "Z Down [X]");
                    break;
                case 6:
                    snprintf(gesture_detail_text_, sizeof(gesture_detail_text_), "Z Up [O]");
                    break;
                default:
                    snprintf(gesture_detail_text_, sizeof(gesture_detail_text_), "Unknown");
                    break;
                }
                gesture_type = GESTURE_DETECT_EVENT_ROLL;
            }
        }

        if (gesture_type > 0 && gesture_detect_queue_handle_ != nullptr) {
            xQueueSend(gesture_detect_queue_handle_, &gesture_type, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

ESP_UTILS_REGISTER_PLUGIN_WITH_CONSTRUCTOR(systems::base::App, GestureDetect, "Gesture Detect", []()
{
    return std::shared_ptr<GestureDetect>(GestureDetect::requestInstance(), [](GestureDetect * p) {});
})

} // namespace esp_brookesia::apps
