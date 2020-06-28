// Copyright 2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the GPIO port of the SCCB 
 */
int SCCB_Init(int pin_sda, int pin_scl);

/**
 * @brief Gets the slave address
 * @return sensor address (slv_addr) ov2640: 0x30
 *                                   ov3660: 0x3c
 */
uint8_t SCCB_Probe();

/**
 * @brief Write the register
 * 
 * @param slv_addr slave address
 * @param dat register
 * @param data write data
 * 
 * @return - 0 success
 *         - 1 fail
 */
uint8_t SCCB_Write(uint8_t slv_addr, uint8_t reg, uint8_t data);
/**
 * @brief read the register
 *
 * @param slv_addr slave address
 * @param reg register
 * 
 * @return - Read the data
 */
uint8_t SCCB_Read(uint8_t slv_addr, uint8_t reg);

/**
 * @brief read the 16-bit register
 *
 * @param slv_addr slave address
 * @param reg register
 * 
 * @return - Read the data
 */
uint8_t SCCB_Read16(uint8_t slv_addr, uint16_t reg);

/**
 * @brief write the 16-bit register
 *
 * @param slv_addr slave address
 * @param reg register
 * @param data write data
 * 
 * @return - 0 success
 *          -1 fail
 */
uint8_t SCCB_Write16(uint8_t slv_addr, uint16_t reg, uint8_t data);

#ifdef __cplusplus
}
#endif







