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
#include "sensor.h"

const resolution_info_t resolution[FRAMESIZE_INVALID] = {
    {   96,   96, ASPECT_RATIO_1X1   }, /* 96x96 */
    {  160,  120, ASPECT_RATIO_4X3   }, /* QQVGA */
    {  176,  144, ASPECT_RATIO_5X4   }, /* QCIF  */
    {  240,  176, ASPECT_RATIO_4X3   }, /* HQVGA */
    {  240,  240, ASPECT_RATIO_1X1   }, /* 240x240 */
    {  320,  240, ASPECT_RATIO_4X3   }, /* QVGA  */
    {  400,  296, ASPECT_RATIO_4X3   }, /* CIF   */
    {  480,  320, ASPECT_RATIO_3X2   }, /* HVGA  */
    {  640,  480, ASPECT_RATIO_4X3   }, /* VGA   */
    {  800,  600, ASPECT_RATIO_4X3   }, /* SVGA  */
    { 1024,  768, ASPECT_RATIO_4X3   }, /* XGA   */
    { 1280,  720, ASPECT_RATIO_16X9  }, /* HD    */
    { 1280, 1024, ASPECT_RATIO_5X4   }, /* SXGA  */
    { 1600, 1200, ASPECT_RATIO_4X3   }, /* UXGA  */
    /*!< 3MP Sensors */
    { 1920, 1080, ASPECT_RATIO_16X9  }, /* FHD   */
    {  720, 1280, ASPECT_RATIO_9X16  }, /* Portrait HD   */
    {  864, 1536, ASPECT_RATIO_9X16  }, /* Portrait 3MP   */
    { 2048, 1536, ASPECT_RATIO_4X3   }, /* QXGA  */
    /*!< 5MP Sensors */
    { 2560, 1440, ASPECT_RATIO_16X9  }, /* QHD    */
    { 2560, 1600, ASPECT_RATIO_16X10 }, /* WQXGA  */
    { 1088, 1920, ASPECT_RATIO_9X16  }, /* Portrait FHD   */
    { 2560, 1920, ASPECT_RATIO_4X3   }, /* QSXGA  */
};