// Copyright 2021 Espressif Systems (Shanghai) CO., LTD
// All rights reserved.

#ifndef ESP_JPEG_DEC_H
#define ESP_JPEG_DEC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_jpeg_common.h"

#define DEFAULT_JPEG_DEC_CONFIG() {                           \
    .output_type               = JPEG_RAW_TYPE_RGB565_LE,     \
    .rotate                    = JPEG_ROTATE_0D,              \
}

#define JPEG_DEC_MAX_MARKER_CHECK_LEN (1024)

typedef void* jpeg_dec_handle_t;

/* Jpeg dec user need to config */
typedef struct {
    jpeg_raw_type_t output_type; /*!< jpeg_dec_out_type 1:rgb888 0:rgb565 */
    jpeg_rotate_t rotate;        /*!< Supports 0, 90 180 270 degree clockwise rotation.
                                      Under width % 8 == 0. height % 8 = 0 conditions, rotation enabled. Otherwise unsupported */
} jpeg_dec_config_t;

/* Jpeg dec out info */
typedef struct {
    int width;                      /* Number of pixels in the horizontal direction */
    int height;                     /* Number of pixels in the vertical direction */
    int component_num;              /* Number of color component*/
    uint8_t component_id[3];        /* ID of color component*/
    uint8_t x_factory[3];           /* Size factory in the x direction*/
    uint8_t y_factory[3];           /* Size factory in the y direction*/
    uint8_t huffbits[2][2][16];     /* Huffman bit distribution tables [id][dcac] */
    uint16_t huffdata[2][2][256];   /* Huffman decoded data tables [id][dcac] */
    uint8_t qtid[3];                /* Quantization table ID of each component */
    int16_t qt_tbl[4][64];          /* De-quantizer tables [id] */
} jpeg_dec_header_info_t;

/* Jpeg dec io control */
typedef struct {
    unsigned char *inbuf;           /* The input buffer pointer */
    int inbuf_len;                  /* The number of the input buffer */
    int inbuf_remain;               /* Not used number of the in buffer */
    unsigned char *outbuf;          /* The decoded data is placed.The buffer must be aligned 16 byte. */
} jpeg_dec_io_t;

/**
 * @brief      Create a Jpeg decode handle, set user config info to decode handle
 * 
 * @param[in]      config        The configuration information
 * 
 * @return     other values: The JPEG decoder handle
 *             NULL: failed  
 */
jpeg_dec_handle_t *jpeg_dec_open(jpeg_dec_config_t *config);

/**
 * @brief      Parse picture data header, and out put info to user
 *
 * @param[in]      jpeg_dec        jpeg decoder handle
 * 
 * @param[in]      io              struct of jpeg_dec_io_t
 * 
 * @param[out]     out_info        output info struct to user 
 * 
 * @return     jpeg_error_t
 *             - JPEG_ERR_OK: on success
 *             - Others: error occurs    
 */
jpeg_error_t jpeg_dec_parse_header(jpeg_dec_handle_t *jpeg_dec, jpeg_dec_io_t *io, jpeg_dec_header_info_t *out_info);

/**
 * @brief      Decode one Jpeg picture 
 *
 * @param[in]      jpeg_dec    jpeg decoder handle
 * 
 * @param[in]      io          struct of jpeg_dec_io_t 
 * 
 * @return     jpeg_error_t
 *             - JPEG_ERR_OK: on success
 *             - Others: error occurs    
 */
jpeg_error_t jpeg_dec_process(jpeg_dec_handle_t *jpeg_dec, jpeg_dec_io_t *io);

/**
 * @brief      Deinitialize Jpeg decode handle 
 * 
 * @param[in]      jpeg_dec    jpeg decoder handle     
 * 
 * @return     jpeg_error_t
 *             - JPEG_ERR_OK: on success
 *             - Others: error occurs    
 */
jpeg_error_t jpeg_dec_close(jpeg_dec_handle_t *jpeg_dec);

/**
 * Example usage:
 * @code{c}
 *
 * // Function for decode one jpeg picture
 * // input_buf   input picture data
 * // len         input picture data length
 * // output_buf  allocated in `esp_jpeg_decoder_one_picture` but it isn't to free. Please free this buffer.
 * jpeg_error_t esp_jpeg_decoder_one_picture(unsigned char *input_buf, int len, unsigned char **output_buf)
 * {
 *      int outbuf_len = 0;
 *      unsigned char *out_buf = NULL;
 *      jpeg_error_t ret = JPEG_ERR_OK;
 *      jpeg_dec_io_t *jpeg_io = NULL;
 *      jpeg_dec_header_info_t *out_info = NULL;
 *      // Generate default configuration
 *      jpeg_dec_config_t config = DEFAULT_JPEG_DEC_CONFIG();
 *
 *      // Empty handle to jpeg_decoder
 *      jpeg_dec_handle_t jpeg_dec = NULL;
 *
 *      // Create jpeg_dec
 *      jpeg_dec = jpeg_dec_open(&config);
 *      if(jpeg_dec == NULL) {
 *          ret = JPEG_ERR_PAR;
 *          goto jpeg_dec_failed;
 *      }
 *
 *      // Create io_callback handle
 *      jpeg_io = calloc(1, sizeof(jpeg_dec_io_t));
 *      if (jpeg_io == NULL) {
 *          ret = JPEG_ERR_MEM;
 *          goto jpeg_dec_failed;
 *      }
 *
 *      // Create out_info handle
 *      out_info = calloc(1, sizeof(jpeg_dec_header_info_t));
 *      if (out_info == NULL) {
 *          ret = JPEG_ERR_MEM;
 *          goto jpeg_dec_failed;
 *      }
 *
 *      // Set input buffer and buffer len to io_callback
 *      jpeg_io->inbuf = input_buf;
 *      jpeg_io->inbuf_len = len;
 *
 *      // Parse jpeg picture header and get picture for user and decoder
 *      ret = jpeg_dec_parse_header(jpeg_dec, jpeg_io, out_info);
 *      if (ret != JPEG_ERR_OK) {
 *           goto jpeg_dec_failed;
 *      }
 *
 *      // Calloc out_put data buffer and update inbuf ptr and inbuf_len
 *      if (config.output_type == JPEG_RAW_TYPE_RGB565_LE
 *          || config.output_type == JPEG_RAW_TYPE_RGB565_BE
 *          || config.output_type == JPEG_RAW_TYPE_CbYCrY) {
 *          outbuf_len = out_info->width * out_info->height * 2;
 *      } else if (config.output_type == JPEG_RAW_TYPE_RGB888) {
 *          outbuf_len = out_info->width * out_info->height * 3;
 *      } else {
 *          ret = JPEG_ERR_PAR;
 *          goto jpeg_dec_failed;
 *      }
 *      out_buf = jpeg_malloc_align(outbuf_len, 16);
 *      if (out_buf == NULL) {
 *          ret = JPEG_ERR_MEM;
 *          goto jpeg_dec_failed;
 *      }
 *      jpeg_io->outbuf = out_buf;
 *      *output_buf = out_buf;
 *      int inbuf_consumed = jpeg_io->inbuf_len - jpeg_io->inbuf_remain;
 *      jpeg_io->inbuf = input_buf + inbuf_consumed;
 *      jpeg_io->inbuf_len = jpeg_io->inbuf_remain;
 *
 *      // Start decode jpeg raw data
 *      ret = jpeg_dec_process(jpeg_dec, jpeg_io);
 *      if (ret != JPEG_ERR_OK) {
 *          goto jpeg_dec_failed;
 *      }
 *
 *      // Decoder deinitialize
 * jpeg_dec_failed:
 *      jpeg_free_align(out_buf);
 *      jpeg_dec_close(jpeg_dec);
 *      if(out_info) {
 *          free(out_info);
 *      }
 *      if(jpeg_io) {
 *         free(jpeg_io);
 *      }
 *      return ret;
 * }
 * 
 * @endcode
 *
 */

#ifdef __cplusplus
}
#endif

#endif
