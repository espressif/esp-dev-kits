/**
 * @file qcloud.h
 * @brief 
 * @version 0.1
 * @date 2021-04-06
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include "esp_wifi.h"

#include "esp_qcloud_iothub.h"
#include "esp_qcloud_prov.h"
#include "esp_qcloud_storage.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 
 * 
 */
void qcloud_init(void);

#ifdef __cplusplus
}
#endif
