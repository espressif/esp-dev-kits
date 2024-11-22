// Copyright 2020-2021 Espressif Systems (Shanghai) CO LTD
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

#include <stdint.h>
#include "esp_chip_info.h"

/* Be carefull if you want to change the index. It must be bigger then the size of string_desc_arr
   For now 7 would be also ok. But lets reserve some fields fot the future additions
   Also it must be match with the value in the openocd/esp_usb_bridge.cfg file
   Currently it is defined as < esp_usb_jtag_caps_descriptor 0x030A >
*/
#define JTAG_STR_DESC_INX   0x0A

/**
 * @brief Get the protocol capabilities of the JTAG interface
 * 
 * @param dest Pointer to a 16-bit integer where the protocol capabilities will be stored
 * @return size of jtag_proto_caps_t
 */
int jtag_get_proto_caps(uint16_t *dest);

/**
 * @brief Get the target model of the JTAG interface
 * 
 * @return Chip models
 */
int jtag_get_target_model(void);

/**
 * @brief Initialize the JTAG interface
 * 
 */
void jtag_init(void);

/**
 * @brief Suspend the JTAG task
 * 
 */
void jtag_task_suspend(void);

/**
 * @brief Resume the JTAG task
 * 
 */
void jtag_task_resume(void);