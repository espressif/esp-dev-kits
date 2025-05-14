#ifndef UI_EXTRA_H
#define UI_EXTRA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ui.h"
#include <stdbool.h>

#define DEFAULT_CONTRAST_PERCENT 53
#define DEFAULT_SATURATION_PERCENT 63
#define DEFAULT_BRIGHTNESS_PERCENT 54
#define DEFAULT_HUE_PERCENT 2

/**
 * @brief AI detection mode enumeration
 */
typedef enum {
    AI_DETECT_PEDESTRIAN = 0,  // Pedestrian detection
    AI_DETECT_FACE,            // Face detection
    AI_DETECT_MODE_MAX         // Maximum number of modes
} ai_detect_mode_t;

/**
 * @brief UI page enumeration
 */
typedef enum {
    UI_PAGE_MAIN,           // Main page
    UI_PAGE_CAMERA,         // Camera page
    UI_PAGE_INTERVAL_CAM,   // Interval camera page
    UI_PAGE_VIDEO_MODE,     // Video mode page
    UI_PAGE_ALBUM,          // Album page
    UI_PAGE_USB_DISK,       // USB disk page
    UI_PAGE_SETTINGS,       // Settings page
    UI_PAGE_AI_DETECT,      // AI detect page
    UI_PAGE_MAX             // Page count
} ui_page_t;

/**
 * @brief Settings information structure
 */
typedef struct {
    const char* gyroscope;  // Gyroscope setting
    const char* od;         // Object Detection setting
    const char* resolution; // Resolution setting
    const char* flash;      // Flash setting
} settings_info_t;

/* Initialization */
/**
 * @brief Initialize UI extra functionality
 */
void ui_extra_init(void);

/* Button handlers */
/**
 * @brief Menu button handler
 */
void ui_extra_btn_menu(void);

/**
 * @brief Up button handler
 */
void ui_extra_btn_up(void);

/**
 * @brief Down button handler
 */
void ui_extra_btn_down(void);

/**
 * @brief Right button handler
 */
void ui_extra_btn_right(void);

/**
 * @brief Left button handler
 */
void ui_extra_btn_left(void);

/**
 * @brief Encoder button handler
 */
void ui_extra_btn_encoder(void);

/* Page management */
/**
 * @brief Get current page
 * @return Current page enum value
 */
ui_page_t ui_extra_get_current_page(void);

/**
 * @brief Get chosen page
 * @return Chosen page enum value
 */
ui_page_t ui_extra_get_choosed_page(void);

/**
 * @brief Check if UI is initialized
 * @return True if UI is initialized, false otherwise
 */
bool ui_extra_is_ui_init(void);

/**
 * @brief Navigate to specified page
 * @param page Target page enum value
 */
void ui_extra_goto_page(ui_page_t page);

/**
 * @brief Clear page
 */
void ui_extra_clear_page(void);

/* Settings management */
/**
 * @brief Get current settings information
 * @return Pointer to settings information structure
 */
settings_info_t* ui_extra_get_settings(void);

/**
 * @brief Set magnification factor
 * @param factor Magnification factor value
 */
void app_extra_set_magnification_factor(uint16_t factor);

/**
 * @brief Get magnification factor
 * @return Current magnification factor value
 */
uint16_t app_extra_get_magnification_factor(void);

/**
 * @brief Set interval time
 * @param time Interval time in minutes
 */
void app_extra_set_interval_time(uint16_t time);

/**
 * @brief Get interval time
 * @return Current interval time in minutes
 */
uint16_t app_extra_get_interval_time(void);

/* Storage management */
/**
 * @brief Set SD card mount status
 * @param mounted Whether SD card is mounted
 */
void ui_extra_set_sd_card_mounted(bool mounted);

/**
 * @brief Get SD card mount status
 * @return Whether SD card is mounted
 */
bool ui_extra_get_sd_card_mounted(void);

/**
 * @brief Set saved photo count
 * @param count Number of saved photos
 */
void app_extra_set_saved_photo_count(uint16_t count);

/**
 * @brief Get saved photo count
 * @return Current saved photo count
 */
uint16_t app_extra_get_saved_photo_count(void);

/**
 * @brief Set USB disk mount status
 * @param mounted Whether USB disk is mounted
 */
void ui_extra_set_usb_disk_mounted(bool mounted);

/**
 * @brief Get USB disk mount status
 * @return Whether USB disk is mounted
 */
bool ui_extra_get_usb_disk_mounted(void);

/* Popup management */
/**
 * @brief Get popup window visible status
 * @return Whether popup window is visible
 */
bool ui_extra_get_popup_window_visible(void);

/**
 * @brief Start interval timer
 */
void ui_extra_start_interval_timer(void);

/**
 * @brief Show interval timer warning popup
 */
void ui_extra_popup_interval_timer_warning(void);

/**
 * @brief Handle USB disk page
 * @return Whether USB disk page is handled
 */
bool ui_extra_handle_usb_disk_page(void);

/**
 * @brief Show picture delete warning popup
 */
void ui_extra_popup_picture_delete_warning(void);

/**
 * @brief Show picture delete success popup
 */
void ui_extra_popup_picture_delete_success(void);

/**
 * @brief Hide camera SD space warning popup
 */
void ui_extra_popup_camera_sd_space_warning_end(void);

/**
 * @brief Show camera SD space warning popup
 */
void ui_extra_popup_camera_sd_space_warning(void);

/**
 * @brief Get current AI detection mode (for UI display only)
 * @return Current AI detection mode
 */
ai_detect_mode_t ui_extra_get_ai_detect_mode(void);

/**
 * @brief Set AI detection mode (for UI display only)
 * @param mode New AI detection mode
 */
void ui_extra_set_ai_detect_mode(ai_detect_mode_t mode);

/**
 * @brief Get gyroscope enabled status
 * @return True if gyroscope is enabled, false otherwise
 */
bool ui_extra_get_gyroscope_enabled(void);

#ifdef __cplusplus
}
#endif
#endif /* UI_EXTRA_H */