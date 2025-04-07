#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"
#include "bsp/esp-bsp.h"

static const char *TAG = "bsp_p4_eye";

static void __attribute__((constructor)) bsp_p4_eye_auto_init(void)
{
    ESP_LOGW(TAG, "Auto-initializing ESP32-P4-EYE board using constructor attribute");
    esp_err_t ret = bsp_p4_eye_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize ESP32-P4-EYE board: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "ESP32-P4-EYE board initialized successfully");
    }
}