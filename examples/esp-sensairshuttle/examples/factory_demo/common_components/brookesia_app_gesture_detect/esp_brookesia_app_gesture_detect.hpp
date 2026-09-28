/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESP_BROOKESIA_APP_GESTURE_HPP
#define ESP_BROOKESIA_APP_GESTURE_HPP

#include "bmi270_api.h"
#include "boost/thread.hpp"
#include "brookesia/system_phone/app.hpp"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "ui/ui_app.h"
#include <atomic>
#include <mutex>
namespace esp_brookesia::apps {

typedef enum {
    GESTURE_DETECT_EVENT_PICK_UP,
    GESTURE_DETECT_EVENT_PUT_DOWN,
    GESTURE_DETECT_EVENT_SHAKE,
    GESTURE_DETECT_EVENT_ROLL,
    GESTURE_DETECT_EVENT_COUNT,
} gesture_detect_event_id_t;

typedef struct {
    gesture_detect_event_id_t event_id;
    void *event_data;
} gesture_detect_event_t;

class GestureDetect: public systems::phone::App {

public:
    GestureDetect(const GestureDetect &) = delete;
    GestureDetect(GestureDetect &&) = delete;
    GestureDetect &operator=(const GestureDetect &) = delete;
    GestureDetect &operator=(GestureDetect &&) = delete;

    ~GestureDetect();

    static GestureDetect *requestInstance();

protected:
    bool run() override;
    bool back() override;
    bool close() override;
    bool init() override;
    bool deinit() override;
    bool pause() override;
    bool resume() override;
    void destroyGestureDetectUI();
    void createGestureDetectUI();

private:
    inline static GestureDetect *_instance = nullptr;
    GestureDetect();

    std::atomic<bool> is_app_running_{false};
    std::atomic<bool> is_initialized_{false};
    gesture_t pending_gesture_ = GESTURE_NONE;
    TickType_t last_shake_time_ = 0;
    TickType_t last_left_gesture_time_ = 0;
    uint32_t consecutive_i2c_failures_ = 0;
    char gesture_detail_text_[64] = {0};

    bmi270_handle_t bmi_handle_ = nullptr;
    i2c_master_bus_handle_t i2c_bus_ = nullptr;
    std::mutex sensor_mutex_;

    boost::thread gesture_detect_thread_;
    boost::thread gesture_detect_event_thread_;
    QueueHandle_t gesture_detect_queue_handle_ = nullptr;

    int8_t set_accel_gyro_config(bmi270_handle_t bmi_handle);
    int8_t set_feature_interrupt(bmi270_handle_t bmi_handle);
    bool initSensors();
    bool deinitSensors();
    bool deinitSensorsLocked();
    int8_t readSensorRegs(uint8_t reg_addr, uint8_t *data, uint16_t length);
    void gestureDetectThread();
    void gestureDetectEventThread();
};

}// namespace esp_brookesia::apps

#endif
