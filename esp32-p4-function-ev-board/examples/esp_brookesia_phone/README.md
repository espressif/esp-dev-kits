# ESP_Brookesia Phone

[中文版本](./README_CN.md)

This example, based on [ESP_Brookesia](https://github.com/espressif/esp-brookesia), demonstrates an Android-like interface that includes many different applications. The example utilizes the development board's MIPI-DSI, ESP32-C6, and audio interfaces. Based on this example, a use case can be created for ESP_Brookesia to efficiently develop HMI applications.


## Getting Started


### Prerequisites

* An ESP32-P4-Function-EV-Board.
* A 7-inch 1024 x 600 LCD screen powered by the [EK79007](../../docs/_static/esp32-p4-function-ev-board/camera_display_datasheet/display_driver_chip_EK79007AD_datasheet.pdf) IC, accompanied by a 32-pin FPC connection [adapter board](../../docs/_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-lcd-subboard-schematics.pdf) ([LCD Specifications](../../docs/_static/esp32-p4-function-ev-board/camera_display_datasheet/display_datasheet.pdf)).
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
    * **Step 3**. Use a USB-C cable to connect the `USB-UART` port to a PC (Used for power supply and viewing serial output).
    * **Step 4**. Turn on the power switch of the board.


### ESP-IDF Required

- This example supports ESP-IDF release/v5.3 and later branches. By default, it runs on ESP-IDF release/v5.3.
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
    I (25) boot: ESP-IDF v5.4-dev-2167-gdef35b1ca7-dirty 2nd stage bootloader
    I (26) boot: compile time Sep 27 2024 15:32:33
    I (27) boot: Multicore bootloader
    I (32) boot: chip revision: v0.1
    I (35) qio_mode: Enabling default flash chip QIO
    I (40) boot.esp32p4: SPI Speed      : 80MHz
    I (45) boot.esp32p4: SPI Mode       : QIO
    I (49) boot.esp32p4: SPI Flash Size : 16MB
    I (54) boot: Enabling RNG early entropy source...
    I (60) boot: Partition Table:
    I (63) boot: ## Label            Usage          Type ST Offset   Length
    I (70) boot:  0 nvs              WiFi data        01 02 00009000 00006000
    I (78) boot:  1 phy_init         RF data          01 01 0000f000 00001000
    I (85) boot:  2 factory          factory app      00 00 00010000 00900000
    I (93) boot:  3 storage          Unknown data     01 82 00910000 00400000
    I (101) boot: End of partition table
    I (105) esp_image: segment 0: paddr=00010020 vaddr=480f0020 size=3c1fb0h (3940272) map
    I (726) esp_image: segment 1: paddr=003d1fd8 vaddr=30100000 size=00020h (    32) load
    I (728) esp_image: segment 2: paddr=003d2000 vaddr=30100020 size=0003ch (    60) load
    I (733) esp_image: segment 3: paddr=003d2044 vaddr=4ff00000 size=0dfd4h ( 57300) load
    I (752) esp_image: segment 4: paddr=003e0020 vaddr=48000020 size=e1950h (923984) map
    I (897) esp_image: segment 5: paddr=004c1978 vaddr=4ff0dfd4 size=0c820h ( 51232) load
    I (908) esp_image: segment 6: paddr=004ce1a0 vaddr=4ff1a800 size=03774h ( 14196) load
    I (918) boot: Loaded app from partition at offset 0x10000
    I (918) boot: Disabling RNG early entropy source...
    I (930) hex_psram: vendor id    : 0x0d (AP)
    I (930) hex_psram: Latency      : 0x01 (Fixed)
    I (930) hex_psram: DriveStr.    : 0x00 (25 Ohm)
    I (934) hex_psram: dev id       : 0x03 (generation 4)
    I (939) hex_psram: density      : 0x07 (256 Mbit)
    I (945) hex_psram: good-die     : 0x06 (Pass)
    I (950) hex_psram: SRF          : 0x02 (Slow Refresh)
    I (955) hex_psram: BurstType    : 0x00 ( Wrap)
    I (961) hex_psram: BurstLen     : 0x03 (2048 Byte)
    I (966) hex_psram: BitMode      : 0x01 (X16 Mode)
    I (971) hex_psram: Readlatency  : 0x04 (14 cycles@Fixed)
    I (977) hex_psram: DriveStrength: 0x00 (1/1)
    I (983) MSPI DQS: tuning success, best phase id is 2
    I (1165) MSPI DQS: tuning success, best delayline id is 12
    I esp_psram: Found 32MB PSRAM device
    I esp_psram: Speed: 200MHz
    I (1166) mmu_psram: flash_drom_paddr_start: 0x10000
    I (1359) mmu_psram: flash_irom_paddr_start: 0x3e0000
    I (1406) hex_psram: psram CS IO is dedicated
    I (1406) cpu_start: Multicore app
    I (1755) esp_psram: SPI SRAM memory test OK
    W (1765) clk: esp_perip_clk_init() has not been implemented yet
    I (1772) cpu_start: Pro cpu start user code
    I (1772) cpu_start: cpu freq: 360000000 Hz
    I (1772) app_init: Application information:
    I (1775) app_init: Project name:     esp_brookesia_demo
    I (1781) app_init: App version:      bcf28556-dirty
    I (1786) app_init: Compile time:     Sep 27 2024 15:32:23
    I (1793) app_init: ELF file SHA256:  dea2628c2...
    I (1798) app_init: ESP-IDF:          v5.4-dev-2167-gdef35b1ca7-dirty
    I (1805) efuse_init: Min chip rev:     v0.1
    I (1810) efuse_init: Max chip rev:     v0.99 
    I (1815) efuse_init: Chip rev:         v0.1
    I (1820) heap_init: Initializing. RAM available for dynamic allocation:
    I (1827) heap_init: At 4FF21AE0 len 000194E0 (101 KiB): RAM
    I (1833) heap_init: At 4FF3AFC0 len 00004BF0 (18 KiB): RAM
    I (1839) heap_init: At 4FF40000 len 00040000 (256 KiB): RAM
    I (1846) heap_init: At 50108080 len 00007F80 (31 KiB): RTCRAM
    I (1852) heap_init: At 3010005C len 00001FA4 (7 KiB): TCM
    I (1858) esp_psram: Adding pool of 24000K of PSRAM memory to heap allocator
    I (1866) spi_flash: detected chip: generic
    I (1871) spi_flash: flash io: qio
    W (1875) i2c: This driver is an old driver, please migrate your application code to adapt `driver/i2c_master.h`
    ...
    ```

## Technical Support and Feedback

Please use the following feedback channels:

- For technical queries, go to the [esp32.com](https://esp32.com/viewforum.php?f=22) forum.
- For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-dev-kits/issues).

We will get back to you as soon as possible.
