/**
 * @file bsp_board.h
 * @brief 
 * @version 0.1
 * @date 2021-02-26
 * 
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */
#pragma once

#include "sdkconfig.h"

#if defined(CONFIG_ESP32_S2_HMI_DEVKIT_BOARD)
#include "bsp_hmi_dev_kit.h"
#elif defined(CONFIG_HMI_BOARD_CUSTOM)
#include "bsp_custom_board.h"
#elif defined (CONFIG_ESP32_S2_HMI_DEVKIT_V2_BOARD)
#include "bsp_hmi_dev_kit_v2.h"
#endif

#define BOARD_FUNC_IS_EN(func_name) ((int)func_name)
