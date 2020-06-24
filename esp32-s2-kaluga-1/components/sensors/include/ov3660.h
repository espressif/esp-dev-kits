/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OV3660 driver.
 *
 */
#ifndef __OV3660_H__
#define __OV3660_H__

#include "sensor.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Initialize OV3660
 *
 * @param sensor   pointers of sensor 
 * 
 * @return - ESP_FAIL Initialization failure
 *         - ESP_OK on Successful initialization
 */
esp_err_t ov3660_init(sensor_t *sensor);

#ifdef __cplusplus
}
#endif


#endif // __OV3660_H__