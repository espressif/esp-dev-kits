/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdint.h>
#include "driver/i2c_master.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct compass_mag compass_mag_t;

typedef struct {
    i2c_master_bus_handle_t i2c_bus;
    /** Board magnetometer chip string; ESP-SensairShuttle uses "bmm350". */
    const char *chip;
    /** Preferred I2C address; 0 probes 0x14 then 0x15. */
    uint8_t i2c_addr;
    uint32_t frequency_hz;
} compass_mag_config_t;

/**
 * Create the ShuttleBoard BMM350 magnetometer backend (direct I2C).
 */
esp_err_t compass_mag_create(const compass_mag_config_t *config, compass_mag_t **out_mag);

/**
 * Read one compensated XYZ sample in microtesla.
 * xyz is unchanged on every error.
 */
esp_err_t compass_mag_read(compass_mag_t *mag, float xyz[3]);

const char *compass_mag_chip_name(const compass_mag_t *mag);

esp_err_t compass_mag_delete(compass_mag_t **mag);

#ifdef __cplusplus
}
#endif
