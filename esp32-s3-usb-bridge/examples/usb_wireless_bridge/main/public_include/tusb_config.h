// Copyright 2023 Espressif Systems (Shanghai) CO LTD
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

# pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_TUSB_RHPORT0_MODE       (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)
#define CFG_TUSB_OS                 OPT_OS_FREERTOS

#define CFG_TUD_ENDPOINT0_SIZE      64

#define CFG_TUD_CDC                 1
#define CFG_TUD_CDC_RX_BUFSIZE      64
#define CFG_TUD_CDC_TX_BUFSIZE      64

#define CFG_TUD_MSC                 1
#define CFG_TUD_MSC_BUFSIZE         512

#define CFG_TUD_VENDOR              1
#define CFG_TUD_VENDOR_RX_BUFSIZE   64
#define CFG_TUD_VENDOR_TX_BUFSIZE   64

#define CFG_TUD_HID                 0

#define CFG_TUD_MIDI                0

#ifdef __cplusplus
}
#endif
