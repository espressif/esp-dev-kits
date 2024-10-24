# ESP_Brookesia Phone

[中文版本](./README_CN.md)

This example, based on [ESP_Brookesia](https://github.com/espressif/esp-brookesia), demonstrates an Android-like interface that includes many different applications. The example utilizes the development board's MIPI-DSI, MIPI-CSI, ESP32-C6, SD card, and audio interfaces. Based on this example, a use case can be created using ESP_Brookesia, enabling efficient development of multimedia applications.

## Getting Started


### Prerequisites

* An ESP32-P4-Function-EV-Board.
* A 7-inch 1024 x 600 LCD screen powered by the [EK79007](https://docs.espressif.com/projects/esp-dev-kits/en/latest/_static/esp32-p4-function-ev-board/camera_display_datasheet/display_driver_chip_EK79007AD_datasheet.pdf) IC, accompanied by a 32-pin FPC connection [adapter board](https://docs.espressif.com/projects/esp-dev-kits/en/latest/_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-lcd-subboard-schematics.pdf) ([LCD Specifications](https://docs.espressif.com/projects/esp-dev-kits/en/latest/_static/esp32-p4-function-ev-board/camera_display_datasheet/display_datasheet.pdf)).
* A MIPI-CSI camera powered by the SC2336 IC, accompanied by a 32-pin FPC connection [adapter board](https://docs.espressif.com/projects/esp-dev-kits/en/latest/_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-camera-subboard-schematics.pdf) ([Camera Specifications](https://docs.espressif.com/projects/esp-dev-kits/en/latest/_static/esp32-p4-function-ev-board/camera_display_datasheet/camera_datasheet.pdf)).
* A USB-C cable for power supply and programming.
* Please refer to the following steps for the connection:
    * **Step 1**. According to the table below, connect the pins on the back of the screen adapter board to the corresponding pins on the development board.

        | Screen Adapter Board | ESP32-P4-Function-EV-Board |
        | -------------------- | -------------------------- |
        | 5V (any one)         | 5V (any one)               |
        | GND (any one)        | GND (any one)              |
        | PWM                  | GPIO26                     |
        | LCD_RST              | GPIO27                     |

    * **Step 2**. Connect the FPC of LCD through the `MIPI_DSI` interface.
    * **Step 3**. Connect the FPC of Camera through the `MIPI_CSI` interface.
    * **Step 4**. Use a USB-C cable to connect the `USB-UART` port to a PC (Used for power supply and viewing serial output).
    * **Step 5**. Turn on the power switch of the board.


### ESP-IDF Required

- This example supports ESP-IDF release/v5.4 and later branches. By default, it runs on ESP-IDF release/v5.4.
- Please follow the [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) to set up the development environment. **We highly recommend** you [Build Your First Project](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#build-your-first-project) to get familiar with ESP-IDF and make sure the environment is set up correctly.

### Get the esp-dev-kits Repository

To start from the examples in esp-dev-kits, clone the repository to the local PC by running the following commands in the terminal:

```
git clone --recursive https://github.com/espressif/esp-dev-kits.git
```

### Configuration

Run ``idf.py menuconfig`` and go to ``Board Support Package(ESP32-P4)``:

```
menuconfig > Component config > Board Support Package
```

To use the SD card and enable the "Video Player" APP, run ``idf.py menuconfig`` and then select ``Example Configurations`` > ``Enable SD Card``

**Note:**
To experience video playback, save MJPEG format videos on an SD card and insert the SD card into the SD card slot. **Currently, only MJPEG format videos are supported**. After inserting the SD card, the video playback app will automatically appear on the interface. The method for video format conversion is as follows:

* Install ffmpeg.
```
    sudo apt update
    sudo apt install ffmpeg
```
* Use ffmpeg to convert video.
```
   ffmpeg -i YOUR_INPUT_FILE_NAME.mp4 -vcodec mjpeg -q:v 2 -vf "scale=1024:600" -acodec copy YOUR_OUTPUT_FILE_NAME.mjpeg
```

## How to Use the Example


### Build and Flash the Example

Build the project and flash it to the board, then run monitor tool to view serial output (replace `PORT` with your board's serial port name):

```c
idf.py -p PORT flash monitor
```

To exit the serial monitor, type ``Ctrl-]``.

See the [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.


### Example Output

- The complete log is as follows:

    ```c
    I (672) esp_image: segment 1: paddr=00458028 vaddr=301000ESP-ROM:esp32p4-eco1-20240205
    Build:Feb  5 2024
    rst:0x17 (CHIP_USB_UART_RESET),boot:0xc (SPI_FAST_FLASH_BOOT)
    Core0 Saved PC:0x4ff2ebfa
    Core1 Saved PC:0x4fc05716
    SPI mode:DIO, clock div:1
    load:0x4ff33ce0,len:0x1a54
    load:0x4ff2abd0,len:0xf24
    load:0x4ff2cbd0,len:0x33ac
    entry 0x4ff2abda
    I (27) boot: ESP-IDF v5.4-dev-3432-g906f36bdb9 2nd stage bootloader
    I (28) boot: compile time Oct 17 2024 18:21:49
    I (29) boot: Multicore bootloader
    I (33) boot: chip revision: v0.1
    I (36) boot: efuse block revision: v0.0
    I (41) qio_mode: Enabling default flash chip QIO
    I (46) boot.esp32p4: SPI Speed      : 80MHz
    I (51) boot.esp32p4: SPI Mode       : QIO
    I (55) boot.esp32p4: SPI Flash Size : 16MB
    I (60) boot: Enabling RNG early entropy source...
    I (66) boot: Partition Table:
    I (69) boot: ## Label            Usage          Type ST Offset   Length
    I (77) boot:  0 nvs              WiFi data        01 02 00009000 00006000
    I (84) boot:  1 phy_init         RF data          01 01 0000f000 00001000
    I (91) boot:  2 factory          factory app      00 00 00010000 00900000
    I (99) boot:  3 storage          Unknown data     01 82 00910000 00400000
    I (107) boot: End of partition table
    I (111) esp_image: segment 0: paddr=00010020 vaddr=481a0020 size=448000h (4489216) map
    I (672) esp_image: segment 1: paddr=00458028 vaddr=30100000 size=0002ch (    44) load
    I (674) esp_image: segment 2: paddr=0045805c vaddr=3010002c size=0003ch (    60) load
    I (679) esp_image: segment 3: paddr=004580a0 vaddr=4ff00000 size=07f78h ( 32632) load
    I (692) esp_image: segment 4: paddr=00460020 vaddr=48000020 size=192130h (1646896) map
    I (899) esp_image: segment 5: paddr=005f2158 vaddr=4ff07f78 size=12740h ( 75584) load
    I (912) esp_image: segment 6: paddr=006048a0 vaddr=4ff1a700 size=038e0h ( 14560) load
    I (916) esp_image: segment 7: paddr=00608188 vaddr=50108080 size=00018h (    24) load
    I (923) boot: Loaded app from partition at offset 0x10000
    I (924) boot: Disabling RNG early entropy source...
    I (941) hex_psram: vendor id    : 0x0d (AP)
    I (942) hex_psram: Latency      : 0x01 (Fixed)
    I (942) hex_psram: DriveStr.    : 0x00 (25 Ohm)
    I (945) hex_psram: dev id       : 0x03 (generation 4)
    I (951) hex_psram: density      : 0x07 (256 Mbit)
    I (956) hex_psram: good-die     : 0x06 (Pass)
    I (961) hex_psram: SRF          : 0x02 (Slow Refresh)
    I (967) hex_psram: BurstType    : 0x00 ( Wrap)
    I (972) hex_psram: BurstLen     : 0x03 (2048 Byte)
    I (978) hex_psram: BitMode      : 0x01 (X16 Mode)
    I (983) hex_psram: Readlatency  : 0x04 (14 cycles@Fixed)
    I (989) hex_psram: DriveStrength: 0x00 (1/1)
    I (994) MSPI DQS: tuning success, best phase id is 2
    I (1177) MSPI DQS: tuning success, best delayline id is 11
    I esp_psram: Found 32MB PSRAM device
    I esp_psram: Speed: 200MHz
    I (1391) mmu_psram: .rodata xip on psram
    I (1471) mmu_psram: .text xip on psram
    I (1472) hex_psram: psram CS IO is dedicated
    I (1473) cpu_start: Multicore app
    I (1797) esp_psram: SPI SRAM memory test OK
    I (1807) cpu_start: Pro cpu start user code
    I (1807) cpu_start: cpu freq: 360000000 Hz
    I (1807) app_init: Application information:
    I (1810) app_init: Project name:     esp_brookesia_demo
    I (1816) app_init: App version:      da1c00bd-dirty
    I (1822) app_init: Compile time:     Oct 17 2024 18:21:44
    I (1828) app_init: ELF file SHA256:  25e462383...
    I (1833) app_init: ESP-IDF:          v5.4-dev-3432-g906f36bdb9
    I (1840) efuse_init: Min chip rev:     v0.1
    I (1844) efuse_init: Max chip rev:     v0.99 
    I (1850) efuse_init: Chip rev:         v0.1
    I (1854) heap_init: Initializing. RAM available for dynamic allocation:
    I (1862) heap_init: At 4FF23590 len 00017A30 (94 KiB): RAM
    I (1868) heap_init: At 4FF3AFC0 len 00004BF0 (18 KiB): RAM
    I (1874) heap_init: At 4FF40000 len 00040000 (256 KiB): RAM
    I (1880) heap_init: At 50108098 len 00007F68 (31 KiB): RTCRAM
    I (1887) heap_init: At 30100068 len 00001F98 (7 KiB): TCM
    I (1893) esp_psram: Adding pool of 22272K of PSRAM memory to heap allocator
    I (1901) spi_flash: detected chip: generic
    W (1905) spi_flash: Detected flash size > 16 MB, but access beyond 16 MB is not supported for this flash model yet.
    I (1916) spi_flash: flash io: qio
    W (1920) spi_flash: Detected size(32768k) larger than the size in the binary image header(16384k). Using the size in the binary image header.
    I (1934) host_init: ESP Hosted : Host chip_ip[18]
    I (1939) H_API: ESP-Hosted starting. Hosted_Tasks: prio:23, stack: 5120 RPC_task_stack: 5120
    sdio_mempool_create free:22930068 min-free:22930068 lfb-def:22544384 lfb-8bit:22544384

    I (1956) gpio: GPIO[18]| InputEn: 0| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (1966) gpio: GPIO[19]| InputEn: 0| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (1975) gpio: GPIO[14]| InputEn: 0| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (1984) gpio: GPIO[15]| InputEn: 0| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (1994) gpio: GPIO[16]| InputEn: 0| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (2003) gpio: GPIO[17]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    ...
    ```

## Technical Support and Feedback

Please use the following feedback channels:

- For technical queries, go to the [esp32.com](https://esp32.com/viewforum.php?f=22) forum.
- For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-dev-kits/issues).

We will get back to you as soon as possible.
