// Copyright (c) 2021 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
// All rights reserved.

#ifndef _ADPCM_ENCODER_H_
#define _ADPCM_ENCODER_H_

#include "esp_err.h"
#include "audio_element.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ADPCM_ENCODER_SAMPLE_RATE         (44100)
#define ADPCM_ENCODER_CHANNELS            (2)

/**
 * @brief      ADPCM Encoder configurations
 */
typedef struct {
    int                     sample_rate;                /*!< the sample rate of OPUS audio*/
    int                     channel;                    /*!< the numble of channels of OPUS audio*/
    int                     out_rb_size;                /*!< Size of output ringbuffer */
    int                     task_stack;                 /*!< Task stack size */
    int                     task_core;                  /*!< Task running in core (0 or 1) */
    int                     task_prio;                  /*!< Task priority (based on freeRTOS priority) */
    bool                    stack_in_ext;               /*!< Try to allocate stack in external memory */
    bool                    contain_wav_adpcm_header;   /*!< Choose to contain wav adpcm header in adpcm encoder whether or not (true or false, true means choose to contain adpcm header) */
} adpcm_encoder_cfg_t;

#define ADPCM_ENCODER_TASK_STACK          (10 * 1024)
#define ADPCM_ENCODER_TASK_CORE           (0)
#define ADPCM_ENCODER_TASK_PRIO           (5)
#define ADPCM_ENCODER_RINGBUFFER_SIZE     (2 * 1024)

#define DEFAULT_ADPCM_ENCODER_CONFIG() {\
    .sample_rate              = ADPCM_ENCODER_SAMPLE_RATE,\
    .channel                  = ADPCM_ENCODER_CHANNELS,\
    .out_rb_size              = ADPCM_ENCODER_RINGBUFFER_SIZE,\
    .task_stack               = ADPCM_ENCODER_TASK_STACK,\
    .task_core                = ADPCM_ENCODER_TASK_CORE,\
    .task_prio                = ADPCM_ENCODER_TASK_PRIO,\
    .stack_in_ext             = true,\
    .contain_wav_adpcm_header = true,\
}

/**
 * @brief      Create a handle to an Audio Element to encode incoming data using ADPCM format
 *
 * @param      config  The configuration
 *
 * @return     The audio element handle
 */
audio_element_handle_t adpcm_encoder_init(adpcm_encoder_cfg_t *config);

#ifdef __cplusplus
}
#endif

#endif
