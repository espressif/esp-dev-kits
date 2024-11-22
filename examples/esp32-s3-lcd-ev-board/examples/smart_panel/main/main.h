/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#pragma once

/** Major version number (X.x.x) */
#define BOX_DEMO_VERSION_MAJOR 0
/** Minor version number (x.X.x) */
#define BOX_DEMO_VERSION_MINOR 5
/** Patch version number (x.x.X) */
#define BOX_DEMO_VERSION_PATCH 2

/**
 * Macro to convert version number into an integer
 *
 * To be used in comparisons, such as BOX_DEMO_VERSION >= BOX_DEMO_VERSION_VAL(4, 0, 0)
 */
#define BOX_DEMO_VERSION_VAL(major, minor, patch) ((major << 16) | (minor << 8) | (patch))

/**
 * Current version, as an integer
 *
 * To be used in comparisons, such as BOX_DEMO_VERSION >= BOX_DEMO_VERSION_VAL(4, 0, 0)
 */
#define BOX_DEMO_VERSION BOX_DEMO_VERSION_VAL(BOX_DEMO_VERSION_MAJOR, \
                                              BOX_DEMO_VERSION_MINOR, \
                                              BOX_DEMO_VERSION_PATCH)
