// Copyright 2022 Espressif Systems (Shanghai) CO., LTD
// All rights reserved.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define ESP_JPEG_VERION "1.2.1"

/**
 * @version 1.2.1:
 *     - use heap_caps_calloc_prefer to instead of heap_caps_calloc
 *
 * @version 1.2.0:
 *     - Add CbYCrY output type for decoder.
 *     - Support 0, 90 180 270 degree clockwise rotation for CbYCrY output type in decoder.
 *
 * @version 1.1.0:
 *     - Add gray decoder.
 *     - Decouple between ADF and encoder/decoder
 *
 * @version 1.0.0:
 *    Features:
 *      Encoder:
 *          - Support variety of width and height to encoder
 *          - Support RGB888 RGBA YCbYCr YCbY2YCrY2 GRAY raw data to encode
 *          - Support sub-sample(YUV444 YUV422 Yuv420)
 *          - Support quality(1-100)
 *          - Support 0, 90 180 270 degree clockwise rotation, under src_type = JPEG_RAW_TYPE_YCbYCr,
 *            subsampling = JPEG_SUB_SAMPLE_YUV420, width and height are multiply of 16  and
 *            src_type = JPEG_RAW_TYPE_YCbYCr, subsampling = JPEG_SUB_SAMPLE_Y, width and height are multiply of 8.
 *          - Support dual-task
 *          - Support two mode encoder, respectively block encoder and one image encoder
 *      Decoder:
 *          - Support variety of width and height to decoder
 *          - Support three channels decoder
 *          - Support RGB888 RGB565(big end) RGB565(little end) raw data to output
 *          - Support 0, 90 180 270 degree clockwise rotation, under width and height are multiply of 8.
 *
 * @note :
 *     - The encoder/decoder do ASM optimization in ESP32S3. Frame rate performs better than the others chips.
 *     - In encoder, under src_type = JPEG_RAW_TYPE_YCbYCr, subsampling = JPEG_SUB_SAMPLE_YUV420, width % 16 == 0. height % 16 = 0 conditions,
 *        memory consumption is about 9K.Others, with wider the image and larger `subsampling`, the greater memory consumption will be.
 *     - `esp_jpeg_dec.h` have one decoder example
 *     - `esp_jpeg_enc.h` have some encoder examples
 *
 */

/**
 * @brief         Get JPEG version string
 */
const char *esp_jpeg_get_version();

#ifdef __cplusplus
}
#endif
