/* SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _IO_H_
#define _IO_H_

/* jtag */
#define GPIO_TDI                  CONFIG_BRIDGE_GPIO_TDI
#define GPIO_TDO                  CONFIG_BRIDGE_GPIO_TDO
#define GPIO_TCK                  CONFIG_BRIDGE_GPIO_TCK
#define GPIO_TMS                  CONFIG_BRIDGE_GPIO_TMS

/* serial */
#define GPIO_BOOT                 CONFIG_BRIDGE_GPIO_BOOT
#define GPIO_RST                  CONFIG_BRIDGE_GPIO_RST
#define GPIO_RXD                  CONFIG_BRIDGE_GPIO_RXD
#define GPIO_TXD                  CONFIG_BRIDGE_GPIO_TXD

/* mode hue */
#define MODE_ERROR                CONFIG_BRIDGE_MODE_ERROR_HUE
#define MODE_WIRED_HUE            CONFIG_BRIDGE_MODE_WIRED_HUE
#define MODE_WIRELESS_HOST_HUE    CONFIG_BRIDGE_MODE_WIRELESS_HOST_HUE
#define MODE_WIRELESS_SLAVE_HUE   CONFIG_BRIDGE_MODE_WIRELESS_SLAVE_HUE

/* button */
#define SWITCH_BUTTON             CONFIG_BRIDGE_GPIO_SWITCH_BUTTON

#endif
