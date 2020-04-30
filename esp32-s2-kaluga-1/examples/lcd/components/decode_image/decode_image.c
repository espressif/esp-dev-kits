// Copyright 2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string.h>
#include "esp_log.h"
#include "decode_image.h"
#include "tjpgd.h"

/*!< Size of the work space for the jpeg decoder */
#define WORKSZ 3100

/*!< Data that is passed from the decoder function to the infunc/outfunc functions */
typedef struct {
    const unsigned char *inData; /*!< Pointer to jpeg data */
    uint16_t inPos;              /*!< Current position in jpeg data */
    uint16_t **outData;          /*!< Array of IMAGE_H pointers to arrays of width 16-bit pixel values */
    int outW;                    /*!< Width of the resulting file */
    int outH;                    /*!< Height of the resulting file */
} JpegDev;

const char *TAG = "ImageDec";

/*!< Input function for jpeg decoder. Just returns bytes from the inData field of the JpegDev structure */
static uint16_t infunc(JDEC *decoder, uint8_t *buf, uint16_t len)
{
    /*!< Read bytes from input file */
    JpegDev *jd = (JpegDev *)decoder->device;

    if (buf != NULL) {
        memcpy(buf, jd->inData + jd->inPos, len);
    }

    jd->inPos += len;
    return len;
}

/*!< Output function. Re-encodes the RGB888 data from the decoder as big-endian RGB565 an */
/*!< stores it in the outData array of the JpegDev structure */
static uint16_t outfunc(JDEC *decoder, void *bitmap, JRECT *rect)
{
    JpegDev *jd = (JpegDev *)decoder->device;
    uint8_t *in = (uint8_t *)bitmap;

    for (int y = rect->top; y <= rect->bottom; y++) {
        for (int x = rect->left; x <= rect->right; x++) {
            /*!< We need to convert the 3 bytes in `in` to a rgb565 value. */
            uint16_t v = 0;
            v |= ((in[0] >> 3) << 11);
            v |= ((in[1] >> 2) << 5);
            v |= ((in[2] >> 3) << 0);
            /*!< The LCD wants the 16-bit value in big-endian, so swap bytes */
            v = (v >> 8) | (v << 8);
            jd->outData[y][x] = v;
            in += 3;
        }
    }

    return 1;
}

/*!< Decode the embedded image into pixel lines that can be used with the rest of the logic */
esp_err_t decode_image(uint16_t ***pixels, const uint8_t *image_jpg_start, int width, int height)
{
    char *work = NULL;
    int r;
    JDEC decoder;
    JpegDev jd;
    *pixels = NULL;
    esp_err_t ret = ESP_OK;

    /*!< Alocate pixel memory. Each line is an array of width 16-bit pixels; the `*pixels` array itself contains pointers to these lines. */
    *pixels = calloc(height, sizeof(uint16_t *));

    if (*pixels == NULL) {
        ESP_LOGE(TAG, "Error allocating memory for lines");
        ret = ESP_ERR_NO_MEM;
        goto err;
    }

    for (int i = 0; i < height; i++) {
        (*pixels)[i] = malloc(width * sizeof(uint16_t));

        if ((*pixels)[i] == NULL) {
            ESP_LOGE(TAG, "Error allocating memory for line %d", i);
            ret = ESP_ERR_NO_MEM;
            goto err;
        }
    }

    /*!< Allocate the work space for the jpeg decoder. */
    work = calloc(WORKSZ, 1);

    if (work == NULL) {
        ESP_LOGE(TAG, "Cannot allocate workspace");
        ret = ESP_ERR_NO_MEM;
        goto err;
    }

    /*!< Populate fields of the JpegDev struct. */
    jd.inData = image_jpg_start;
    jd.inPos = 0;
    jd.outData = *pixels;
    jd.outW = width;
    jd.outH = height;

    /*!< Prepare and decode the jpeg. */
    r = jd_prepare(&decoder, infunc, work, WORKSZ, (void *)&jd);

    if (r != JDR_OK) {
        ESP_LOGE(TAG, "Image decoder: jd_prepare failed (%d)", r);
        ret = ESP_ERR_NOT_SUPPORTED;
        goto err;
    }

    r = jd_decomp(&decoder, outfunc, 0);

    if (r != JDR_OK && r != JDR_FMT1) {
        ESP_LOGE(TAG, "Image decoder: jd_decode failed (%d)", r);
        ret = ESP_ERR_NOT_SUPPORTED;
        goto err;
    }

    /*!< All done! Free the work area (as we don't need it anymore) and return victoriously. */
    free(work);
    return ret;
err:

    /*!< Something went wrong! Exit cleanly, de-allocating everything we allocated. */
    if (*pixels != NULL) {
        for (int i = 0; i < height; i++) {
            free((*pixels)[i]);
        }

        free(*pixels);
    }

    free(work);
    return ret;
}
