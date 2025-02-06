/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void                *sub_src;   /*!< Sub source to keep media source extra data */
} media_src_t;

int media_src_storage_open(media_src_t *src);
int media_src_storage_connect(media_src_t *src, const char *uri);
int media_src_storage_disconnect(media_src_t *src);
int media_src_storage_read(media_src_t *src, void *data, size_t len);
int media_src_storage_seek(media_src_t *src, uint64_t position);
int media_src_storage_get_position(media_src_t *src, uint64_t *position);
int media_src_storage_get_size(media_src_t *src, uint64_t *size);
int media_src_storage_close(media_src_t *src);

#ifdef __cplusplus
}
#endif
