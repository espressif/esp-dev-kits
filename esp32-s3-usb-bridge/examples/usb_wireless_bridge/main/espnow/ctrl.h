/* SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "espnow_ctrl.h"

/**
 * @brief The responder gets bound list
 *
 * @param[out] list the buffer that stores the bound list
 * @param[inout] size input maximum bound list size, output the real bound list size
 *
 * @return
 *    - ESP_OK: succeed
 *    - others: fail
 */
esp_err_t ctrl_responder_get_bindlist(espnow_ctrl_bind_info_t *list, size_t *size);

/**
 * @brief The responder sets bound list
 *
 * @attention The bound information will be stored to flash
 *
 * @param[in] info the bound information to be set
 *
 * @return
 *    - ESP_OK: succeed
 *    - others: fail
 */
esp_err_t ctrl_responder_set_bindlist(const espnow_ctrl_bind_info_t *info);

/**
 * @brief The responder removes bound list
 *
 * @attention The bound information will be removed from flash
 *
 * @param[in] info the bound information to be removed
 *
 * @return
 *    - ESP_OK: succeed
 *    - others: fail
 */
esp_err_t ctrl_responder_remove_bindlist(const espnow_ctrl_bind_info_t *info);

/**
 * @brief set bind window
 * 
 * @param wait_ms wait time for bind window
 * @param rssi the minimum bind frame RSSI
 * @return
 *    - ESP_OK: succeed
 *    - others: fail
 */
esp_err_t ctrl_responder_bind_window_set(uint32_t wait_ms, int8_t rssi);

/**
 * @brief get bind info from flash, and begin to receive bind packet
 * 
 * @return
 *    - ESP_OK: succeed
 *    - others: fail
 */
esp_err_t ctrl_responder_init(void);

#ifdef __cplusplus
}
#endif
