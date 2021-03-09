/* HTTP File Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <sys/param.h>

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "tinyusb.h"
#include "tusb.h"

#include "bsp_ext_io.h"
#include "bsp_i2c.h"
#include "bsp_sdcard.h"
#include "tca9554.h"

static const char *TAG = "main";

/**
 * @brief USB Device Driver task
 * 
 * @param param Not used
 */
static void usb_device_task(void *param)
{
    (void) param;

    while (1) {
        if(tusb_inited()) {
            tud_task();
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

//--------------------------------------------------------------------+
// tinyusb callbacks
//--------------------------------------------------------------------+

extern void usb_msc_mount();
extern void usb_msc_umount();

// Invoked when device is mounted
void tud_mount_cb(void)
{
    usb_msc_mount();
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    usb_msc_umount();
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allows us to perform remote wakeup
// USB Specs: Within 7ms, device must draw an average current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
}

static tinyusb_config_t tusb_cfg = {
    .descriptor = NULL,
    .string_descriptor = NULL,
    .external_phy = false
};

void app_main(void)
{
    ext_io_t io_config = BSP_EXT_IO_DEFAULT_CONFIG();
    ext_io_t io_level = BSP_EXT_IO_DEFAULT_LEVEL();

    ESP_ERROR_CHECK(bsp_i2c_init(I2C_NUM_0, 400000));

    ESP_ERROR_CHECK(tca9554_init());
    ESP_ERROR_CHECK(tca9554_write_output_pins(io_config.val));
    ESP_ERROR_CHECK(tca9554_write_output_pins(io_level.val));

    bsp_sdcard_init();

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    
    xTaskCreate(
        (TaskFunction_t)        usb_device_task,
        (const char * const)    "USB Device Task",
        (const uint32_t)        4 * 1024,
        (void * const)          NULL,
        (UBaseType_t)           configMAX_PRIORITIES,
        (TaskHandle_t * const)  NULL);
}
