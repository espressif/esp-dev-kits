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

#define SCCB_ID   			0X60  			/*!< OV2640 ID */

/**
 * @brief Initialize SCCB
 */
void SCCB_Init(void);

/**
 * @brief delete SCCB
 */
void SCCB_Deinit(void);
/**
 * @brief Write the register
 *
 * @param dat register
 * @param dat write data
 * 
 * @return - 0 success
 *         - 1 fail
 */
uint8_t SCCB_WR_Reg(uint8_t reg, uint8_t data);

/**
 * @brief read the register
 *
 * @param reg register
 * 
 * @return - data
 */
uint8_t SCCB_RD_Reg(uint8_t reg);

#ifdef __cplusplus
}
#endif













