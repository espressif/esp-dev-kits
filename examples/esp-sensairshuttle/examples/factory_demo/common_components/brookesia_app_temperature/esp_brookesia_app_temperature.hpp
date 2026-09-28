/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESP_BROOKESIA_APP_Temperature_HPP
#define ESP_BROOKESIA_APP_Temperature_HPP

#include "bme69x_defs.h"
#include "bme69x.h"
#include "boost/signals2/signal.hpp"
#include "boost/thread.hpp"
#include "brookesia/system_phone/app.hpp"
#include "bsec_datatypes.h"
#include "bsec_interface.h"
#include "bme690_common.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "i2c_bus.h"
#include "lvgl.h"
namespace esp_brookesia::apps {

class Temperature: public systems::phone::App {

public:
    Temperature(const Temperature &) = delete;
    Temperature(Temperature &&) = delete;
    Temperature &operator=(const Temperature &) = delete;
    Temperature &operator=(Temperature &&) = delete;

    ~Temperature();

    static Temperature *requestInstance();

protected:
    bool run() override;
    bool back() override;
    bool close() override;
    bool init() override;
    bool deinit() override;
    void destroyTemperatureUI();
    void createTemperatureUI();

private:
    inline static Temperature *_instance = nullptr;
    Temperature();

    bool is_initialized_ = false;
    i2c_bus_handle_t i2c_bus_ = nullptr;
    struct bme69x_dev bme_;
    struct bme69x_data data;
    boost::thread _bme_thread;
    std::atomic<bool> _bme_running{false};

    // BSEC related
    uint8_t bsec_state[BSEC_MAX_STATE_BLOB_SIZE];
    int64_t time_stamp_last_state_save = 0;
    float iaq = 0.0f;
    uint8_t iaq_accuracy = 0;
    float co2_equivalent = 0.0f;
    float breath_voc_equivalent = 0.0f;
    float bsec_temperature = 0.0f;
    float bsec_humidity = 0.0f;
    float bsec_pressure = 0.0f;
    float previous_pressure = 0.0f;  // For pressure trend detection

    // Humidity sliding average (last 5 values)
    static constexpr int HUMIDITY_AVG_COUNT = 5;
    float humidity_history[HUMIDITY_AVG_COUNT] = {0.0f};
    int humidity_history_index = 0;
    int humidity_history_count = 0;  // Track how many values we have so far

    esp_err_t init_hardware();
    esp_err_t init_bme69x_sensor();
    bsec_library_return_t init_bsec();
    bool initSensors();
    bool deinitSensors();
    int64_t get_timestamp_us();
    void output_ready(int64_t timestamp, float iaq, uint8_t iaq_accuracy,
                      float temperature, float humidity, float pressure,
                      float raw_temperature, float raw_humidity,
                      float gas, bsec_library_return_t bsec_status,
                      float static_iaq, float co2_equivalent,
                      float breath_voc_equivalent);
    void updateTemperatureDisplay();
    void bmeDataThread();
    const char *getIAQLevel(float iaq_value);
    lv_color_t getIAQColor(float iaq_value);
};

} // namespace esp_brookesia::apps

#endif
