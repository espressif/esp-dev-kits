#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Init Wi-Fi STA
 * 
 * @return esp_err_t 
 */
esp_err_t wifi_sta_start(void);

/**
 * @brief Start to scan AP list
 * 
 * @return esp_err_t AP scan task create state
 */
esp_err_t wifi_scan_start(void);

/**
 * @brief Create a task to connect to AP
 * 
 * @return esp_err_t 
 */
esp_err_t wifi_sta_connect(void);

/**
 * @brief Wiat for STA connect to AP
 * 
 * @param ticks_to_wait Ticks to wait
 * @return esp_err_t 
 */
esp_err_t wifi_sta_wait_for_connect(TickType_t ticks_to_wait, bool clear_bits);

/**
 * @brief Set AP's SSID
 * 
 * @param ssid SSID string
 * @return esp_err_t 
 */
esp_err_t wifi_sta_set_ssid(const char *ssid);

/**
 * @brief Set AP's password
 * 
 * @param pswd Password string
 * @return esp_err_t 
 */
esp_err_t wifi_sta_set_pswd(const char *pswd);

/**
 * @brief Get total scaned AP count
 * 
 * @param p_ap_count pointer to ap_cont value
 */
void wifi_scan_get_count(uint16_t *p_ap_count);

void wifi_scan_get_ap_info(size_t index, wifi_ap_record_t **p_info);

/**
 * @brief Wait for scan done
 * 
 * @param ticks_to_wait Ticks to wait
 * @return esp_err_t 
 */
esp_err_t wifi_scan_wait_done(TickType_t ticks_to_wait);

/**
 * @brief Free storage of AP information
 * 
 */
void wifi_scan_free_memory(void);

/**
 * @brief Connect AP with given param
 * 
 * @return esp_err_t 
 */
esp_err_t wifi_sta_connect(void);

#ifdef __cplusplus
}
#endif
