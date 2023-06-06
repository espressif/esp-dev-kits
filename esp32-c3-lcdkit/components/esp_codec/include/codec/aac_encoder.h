// Copyright (c) 2022 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
// All rights reserved.

#ifndef _AAC_ENCODER_H_
#define _AAC_ENCODER_H_

#include "audio_element.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AAC_ENCODER_SAMPLE_RATE (44100)
#define AAC_ENCODER_CHANNELS    (2)
#define AAC_ENCODER_BITRATE     (80000)
#define AAC_ENCODER_BIT         (16)

/**
 * @brief      AAC Encoder configurations
 */
typedef struct {
    int sample_rate;     /*!< Support sample rate(Hz) : 96000, 88200, 64000, 48000,
                              44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000 */
    int channel;         /*!< Support channel : mono, dual */
    int bit;             /*!< Support bit per sample : 16 bit */
    int bitrate;         /*!< Support bitrate(bps) range on mono stream :
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
    int out_rb_size;     /*!< Size of output ringbuffer */
    int task_stack;      /*!< Task stack size */
    int task_core;       /*!< Task running in core (0 or 1) */
    int task_prio;       /*!< Task priority (based on freeRTOS priority) */
    bool stack_in_ext;   /*!< Try to allocate stack in external memory */
} aac_encoder_cfg_t;

#define AAC_ENCODER_TASK_STACK      (10 * 1024)
#define AAC_ENCODER_TASK_CORE       (0)
#define AAC_ENCODER_TASK_PRIO       (5)
#define AAC_ENCODER_RINGBUFFER_SIZE (2 * 1024)

#define DEFAULT_AAC_ENCODER_CONFIG() {                         \
    .sample_rate              = AAC_ENCODER_SAMPLE_RATE,       \
    .channel                  = AAC_ENCODER_CHANNELS,          \
    .bit                      = AAC_ENCODER_BIT,               \
    .bitrate                  = AAC_ENCODER_BITRATE,           \
    .out_rb_size              = AAC_ENCODER_RINGBUFFER_SIZE,   \
    .task_stack               = AAC_ENCODER_TASK_STACK,        \
    .task_core                = AAC_ENCODER_TASK_CORE,         \
    .task_prio                = AAC_ENCODER_TASK_PRIO,         \
    .stack_in_ext             = true,                          \
}

/**
 * @brief      Create a handle to an Audio Element to encode incoming data using AAC format
 *
 * @param      config  The configuration
 *
 * @return     The audio element handle
 */
audio_element_handle_t aac_encoder_init(aac_encoder_cfg_t *config);

#ifdef __cplusplus
}
#endif

#endif
