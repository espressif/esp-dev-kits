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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get touch status
 *
 * @param status get status
 *             
 */
void touch_get_status(uint32_t *status);

/**
 * @brief Get touch num
 *
 * @param status get num
 *             
 */
void touch_get_num(uint32_t *num);

/**
 * @brief Get touch mask (Each state of each pad corresponds to a mask) 
 *
 * @param status get mask          
 */
void touch_get_mask(uint32_t *mask);

/**
 * @brief Get touch status flag if it's triggered
 *
 * @param flag True : be actived    
 *             flase: be inactived   
 */
void touch_get_flag_status(bool *flag);

/**
 * @brief Initialize the touch        
 */
void touch_init();

#ifdef __cplusplus
}
#endif

