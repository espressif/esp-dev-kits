/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include "soc/soc_caps.h"

#if SOC_MIPI_DSI_SUPPORTED
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_mipi_dsi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LCD panel initialization commands.
 *
 */
typedef struct {
    int cmd;                /*<! The specific LCD command */
    const void *data;       /*<! Buffer that holds the command specific data */
    size_t data_bytes;      /*<! Size of `data` in memory, in bytes */
    unsigned int delay_ms;  /*<! Delay in milliseconds after this command */
} ili9881c_lcd_init_cmd_t;

/**
 * @brief LCD panel vendor configuration.
 *
 * @note  This structure needs to be passed to the `vendor_config` field in `esp_lcd_panel_dev_config_t`.
 *
 */
typedef struct {
    const ili9881c_lcd_init_cmd_t *init_cmds;       /*!< Pointer to initialization commands array. Set to NULL if using default commands.
                                                     *   The array should be declared as `static const` and positioned outside the function.
                                                     *   Please refer to `vendor_specific_init_default` in source file.
                                                     */
    uint16_t init_cmds_size;                        /*<! Number of commands in above array */
    struct {
        esp_lcd_dsi_bus_handle_t dsi_bus;               /*!< MIPI-DSI bus configuration */
        const esp_lcd_dpi_panel_config_t *dpi_config;   /*!< MIPI-DPI panel configuration */
    } mipi_config;
} ili9881c_vendor_config_t;

/**
 * @brief Create LCD panel for model ILI9881C
 *
 * @note  Vendor specific initialization can be different between manufacturers, should consult the LCD supplier for initialization sequence code.
 *
 * @param[in]  io LCD panel IO handle
 * @param[in]  panel_dev_config General panel device configuration
 * @param[out] ret_panel Returned LCD panel handle
 * @return
 *      - ESP_ERR_INVALID_ARG   if parameter is invalid
 *      - ESP_OK                on success
 *      - Otherwise             on fail
 */
esp_err_t esp_lcd_new_panel_ili9881c(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config,
                                     esp_lcd_panel_handle_t *ret_panel);

/**
 * @brief MIPI-DSI bus configuration structure
 *
 * @note  The ESP32-P4 DSI host supports at most 2 data lanes
 *        (MIPI_DSI_LL_MAX_DATA_LANES), 80-1500 Mbps per lane. The panel FPC
 *        is drawn for 4 lanes, but the ILI9881C drives whichever lanes are
 *        clocked; 2 lanes @ 800 Mbps carry 800x1280 @ 60 fps in RGB565
 *        (~1.15 Gbps payload < 1.6 Gbps link rate). 800 Mbps chosen over
 *        1200 for HS eye margin through the 40-pin adapter board.
 *
 * @param[in] lane_num Number of data lanes
 * @param[in] lane_mbps Lane bit rate in Mbps
 *
 */
#define ILI9881C_PANEL_BUS_DSI_2CH_CONFIG()              \
    {                                                    \
        .bus_id = 0,                                     \
        .num_data_lanes = 2,                             \
        .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT,     \
        .lane_bit_rate_mbps = 800,                       \
    }

/**
 * @brief MIPI-DBI panel IO configuration structure
 *
 */
#define ILI9881C_PANEL_IO_DBI_CONFIG()  \
    {                                   \
        .virtual_channel = 0,           \
        .lcd_cmd_bits = 8,              \
        .lcd_param_bits = 8,            \
    }

/**
 * @brief MIPI DPI configuration structure
 *
 * @note  refresh_rate = (dpi_clock_freq_mhz * 1000000) / (h_res + hsync_pulse_width + hsync_back_porch + hsync_front_porch)
 *                                                      / (v_res + vsync_pulse_width + vsync_back_porch + vsync_front_porch)
 *
 * @note  Timing from the panel vendor reference code: HS=8 HBP=48 HFP=52,
 *        VS=6 VBP=15 VFP=16 -> 908x1317 total. 60 Hz needs ~71.75 MHz pixel
 *        clock, hence dpi_clock_freq_mhz = 72.
 *
 * @param[in] px_format Pixel format of the panel
 *
 */
#define ILI9881C_800_1280_PANEL_60HZ_DPI_CONFIG(px_format) \
    {                                                      \
        .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,       \
        .dpi_clock_freq_mhz = 72,                          \
        .virtual_channel = 0,                              \
        .pixel_format = px_format,                         \
        .num_fbs = 2,                                      \
        .video_timing = {                                  \
            .h_size = 800,                                 \
            .v_size = 1280,                                \
            .hsync_back_porch = 48,                        \
            .hsync_pulse_width = 8,                        \
            .hsync_front_porch = 52,                       \
            .vsync_back_porch = 15,                        \
            .vsync_pulse_width = 6,                        \
            .vsync_front_porch = 16,                       \
        },                                                 \
        .flags.use_dma2d = true,                           \
    }

#ifdef __cplusplus
}
#endif

#endif
