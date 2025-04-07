#ifndef __APP_ISP_H__
#define __APP_ISP_H__

esp_err_t app_isp_init(int cam_fd);

esp_err_t app_isp_set_contrast(uint32_t percent);
esp_err_t app_isp_set_saturation(uint32_t percent);
esp_err_t app_isp_set_brightness(uint32_t percent);
esp_err_t app_isp_set_hue(uint32_t percent);

#endif
