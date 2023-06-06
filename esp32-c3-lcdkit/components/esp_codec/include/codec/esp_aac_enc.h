// Copyright (c) 2022 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
// All rights reserved.

#ifndef ESP_AAC_ENC_H
#define ESP_AAC_ENC_H

#include <stdio.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief      AAC Encoder information
 */
typedef struct {
    int inbuf_size;     /*!< Input data buffer size */
    int outbuf_size;    /*!< Output data buffer size */
} esp_aac_enc_info_t;

/**
 * @brief      AAC Encoder configurations
 */
typedef struct {
    int sample_rate;     /*!< Support sample rate(Hz) : 96000, 88200, 64000, 48000,
                              44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000 */
    int channel;         /*!< Support channel : mono, dual */
    int bit;             /*!< Support bit per sample : 16 bit */
    int bit_rate;        /*!< Support bitrate(bps) range on mono stream :
                              |samplerate(Hz)|bitrate range(bps)|
                              |    8000      |    12k - 48k     |
                              |    11025     |    18k - 66k     |
                              |    12000     |    20k - 72k     |
                              |    16000     |    22k - 96k     |       
                              |    22050     |    25k - 132k    |
                              |    24000     |    31k - 144k    |
                              |    32000     |    33k - 160k    |
                              |    44100     |    57k - 160k    |
                              |    48000     |    59k - 160k    |
                              |    64000     |    65k - 160k    |
                              |    88200     |    67k - 160k    |
                              |    96000     |    70k - 160k    | 
                              Note : 1) This table shows the bitrate range corresponding to each samplerate.
                                     2) The bitrate range of dual stream is twice that of mono. */
    int adts_used;       /*!< Whether write ADTS header, 1 means add ADTS header */
} esp_aac_enc_config_t;

#define DEFAULT_ESP_AAC_ENC_CONFIG() {              \
    .sample_rate = 44100,                           \
    .channel = 2,                                   \
    .bit = 16,                                      \
    .bit_rate = 80000,                              \
    .adts_used = 1,                                 \
}

/**
 * @brief       Create an AAC encoder handle, set user config info to encoder handle
 * 
 * @param[in]   cfg          The configuration information
 * @param[out]  out_handle   The AAC encoder handle
 * 
 * @return      esp_err_t
 *              - ESP_OK: on success
 *              - ESP_FAIL: error occurs
 */
esp_err_t esp_aac_enc_open(esp_aac_enc_config_t *cfg, void **out_handle);

/**
 * @brief      Get AAC encoder stream information from encoder handle. User need use this function to get inbuf size and outbuf size to malloc inbuf and outbuf.
 * 
 * @param[in]  in_handle      The AAC encoder handle     
 * @param[in]  stream_info    The AAC encoder stream info     
 * 
 * @return     esp_err_t
 *             - ESP_OK: on success
 *             - ESP_FAIL: error occurs 
 */
esp_err_t esp_aac_enc_get_info(void *in_handle, esp_aac_enc_info_t *stream_info);

/**
 * @brief       Encode pcm to AAC
 * 
 * @param[in]   in_handle      The AAC encoder handle
 * @param[in]   in_buf         The input pcm data buffer, which the in_buf size is from "esp_aac_enc_get_info" and must be aligned 16 byte.
 * @param[in]   in_byte_len    The input pcm data length
 * @param[out]  out_buf        The output AAC data buffer, which the out_buf size is from "esp_aac_enc_get_info" and must be aligned 16 byte.
 * @param[out]  out_byte_len   The output AAC data length
 * 
 * @return      esp_err_t
 *              - ESP_OK: on success
 *              - ESP_FAIL: error occurs  
 */
esp_err_t esp_aac_enc_process(void *in_handle, uint8_t *in_buf, int in_byte_len, uint8_t *out_buf, int *out_byte_len);

/**
 * @brief      Deinitialize AAC encoder handle 
 * 
 * @param[in]  in_handle    The AAC encoder handle     
 * 
 * @return     esp_err_t
 *             - ESP_OK: on success
 *             - ESP_FAIL: error occurs 
 */
esp_err_t esp_aac_enc_close(void *in_handle);

#ifdef __cplusplus
}
#endif

#endif
