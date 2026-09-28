/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "compass_mag.hpp"

#include <cmath>
#include <cstddef>
#include <cstring>
#include <new>
#include <strings.h>

#include "bmm350.h"
#include "bmm350_common.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "CompassMag";

struct compass_mag {
    i2c_master_bus_handle_t i2c_bus;
    struct bmm350_dev bmm350;
};

static bool is_bmm350_chip(const char *chip)
{
    return chip == nullptr || chip[0] == '\0' || strcasecmp(chip, "bmm350") == 0;
}

static bool configure_bmm350(struct bmm350_dev *dev)
{
    struct bmm350_pmu_cmd_status_0 pmu0 = {};
    for (int t = 0; t < 10; ++t) {
        (void)bmm350_get_pmu_cmd_status_0(&pmu0, dev);
        if (pmu0.pmu_cmd_busy == 0) {
            break;
        }
        bmm350_delay(5000, dev);
    }

    if (bmm350_set_odr_performance(BMM350_DATA_RATE_100HZ, BMM350_AVERAGING_4, dev) != BMM350_OK ||
            bmm350_enable_axes(BMM350_X_EN, BMM350_Y_EN, BMM350_Z_EN, dev) != BMM350_OK ||
            bmm350_set_powermode(BMM350_NORMAL_MODE, dev) != BMM350_OK) {
        return false;
    }
    return true;
}

static bool probe_bmm350(compass_mag_t *mag, uint8_t addr)
{
    memset(&mag->bmm350, 0, sizeof(mag->bmm350));
    bmm350_set_i2c_address(addr);
    if (bmm350_interface_init(&mag->bmm350) != BMM350_OK) {
        ESP_LOGE(TAG, "BMM350 interface init failed at 0x%02X", addr);
        return false;
    }

    int8_t rslt = bmm350_init(&mag->bmm350);
    ESP_LOGI(TAG, "Init at 0x%02X -> rslt=%d, chip_id=0x%02X (expect 0x33)",
             addr, rslt, mag->bmm350.chip_id);

    if (mag->bmm350.chip_id != BMM350_CHIP_ID) {
        return false;
    }

    if (rslt != BMM350_OK) {
        (void)bmm350_soft_reset(&mag->bmm350);
        bmm350_delay(BMM350_SOFT_RESET_DELAY + 10000, &mag->bmm350);
    }

    if (!configure_bmm350(&mag->bmm350)) {
        ESP_LOGE(TAG, "BMM350 configuration failed at 0x%02X", addr);
        return false;
    }
    return true;
}

static esp_err_t bmm350_backend_init(compass_mag_t *mag, uint8_t addr)
{
    uint8_t candidates[2] = { BMM350_I2C_ADSEL_SET_LOW, BMM350_I2C_ADSEL_SET_HIGH };
    size_t count = 2;
    if (addr != 0) {
        candidates[0] = addr;
        count = 1;
    }

    for (size_t i = 0; i < count; ++i) {
        esp_err_t ret = bmm350_set_i2c_master_bus_handle(mag->i2c_bus);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "BMM350 device cleanup failed: %s", esp_err_to_name(ret));
            return ret;
        }
        if (probe_bmm350(mag, candidates[i])) {
            float probe[3] = {};
            bool got_sample = false;
            for (int n = 0; n < 20; ++n) {
                if (compass_mag_read(mag, probe) == ESP_OK) {
                    got_sample = true;
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            if (!got_sample) {
                ESP_LOGE(TAG, "BMM350 did not produce a valid compensated sample");
                ret = bmm350_set_i2c_master_bus_handle(NULL);
                if (ret != ESP_OK) {
                    ESP_LOGE(TAG, "BMM350 cleanup failed: %s", esp_err_to_name(ret));
                }
                return ESP_ERR_TIMEOUT;
            }
            ESP_LOGI(TAG, "BMM350 ready at 0x%02X, first sample=[%.1f, %.1f, %.1f] uT",
                     candidates[i], probe[0], probe[1], probe[2]);
            return ESP_OK;
        }
    }

    ESP_LOGE(TAG, "BMM350 probe failed");
    return ESP_ERR_NOT_FOUND;
}

esp_err_t compass_mag_create(const compass_mag_config_t *config, compass_mag_t **out_mag)
{
    if (config == nullptr || out_mag == nullptr || config->i2c_bus == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!is_bmm350_chip(config->chip)) {
        ESP_LOGE(TAG, "Unsupported magnetometer chip '%s' (ESP-SensairShuttle uses bmm350)",
                 config->chip != nullptr ? config->chip : "");
        return ESP_ERR_NOT_SUPPORTED;
    }

    *out_mag = nullptr;
    auto *mag = new (std::nothrow) compass_mag_t();
    if (mag == nullptr) {
        return ESP_ERR_NO_MEM;
    }
    memset(mag, 0, sizeof(*mag));
    mag->i2c_bus = config->i2c_bus;

    ESP_LOGI(TAG, "Creating BMM350 magnetometer backend");
    esp_err_t ret = bmm350_backend_init(mag, config->i2c_addr);
    if (ret != ESP_OK) {
        esp_err_t cleanup_ret = compass_mag_delete(&mag);
        if (cleanup_ret != ESP_OK) {
            *out_mag = mag;
            ESP_LOGE(TAG, "Keep failed BMM350 instance for cleanup retry: %s", esp_err_to_name(cleanup_ret));
        }
        return ret;
    }

    *out_mag = mag;
    return ESP_OK;
}

esp_err_t compass_mag_read(compass_mag_t *mag, float xyz[3])
{
    if (mag == nullptr || xyz == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }

    struct bmm350_mag_temp_data data = {};
    if (bmm350_get_compensated_mag_xyz_temp_data(&data, &mag->bmm350) != BMM350_OK) {
        return ESP_FAIL;
    }
    if (!std::isfinite(data.x) || !std::isfinite(data.y) || !std::isfinite(data.z) ||
            (data.x == 0.0f && data.y == 0.0f && data.z == 0.0f)) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    xyz[0] = data.x;
    xyz[1] = data.y;
    xyz[2] = data.z;
    return ESP_OK;
}

const char *compass_mag_chip_name(const compass_mag_t *mag)
{
    return (mag == nullptr) ? "none" : "bmm350";
}

esp_err_t compass_mag_delete(compass_mag_t **mag)
{
    if (mag == nullptr || *mag == nullptr) {
        return ESP_OK;
    }
    esp_err_t ret = bmm350_set_i2c_master_bus_handle(NULL);
    if (ret != ESP_OK) {
        return ret;
    }
    delete *mag;
    *mag = nullptr;
    return ESP_OK;
}
