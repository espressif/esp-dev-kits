# LVGL Demo v9

[中文版本](./README_CN.md)

This example demonstrates how to port LVGL v9 and conduct performance testing using LVGL's built-in demos. The example utilizes the development board's MIPI-DSI interface. Based on this example, applications based on LVGL v9 can be developed.


## Getting Started


### Prerequisites

* An ESP32-P4-Function-EV-Board.
* A 7-inch 1024 x 600 LCD screen powered by the [EK79007](https://dl.espressif.com/dl/schematics/display_driver_chip_EK79007AD_datasheet.pdf) IC, accompanied by a 32-pin FPC connection [adapter board](https://dl.espressif.com/dl/schematics/esp32-p4-function-ev-board-lcd-subboard-schematics.pdf) ([LCD Specifications](https://dl.espressif.com/dl/schematics/display_datasheet.pdf)).
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
    I (26) boot: compile time Sep 27 2024 17:00:31
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
    I (85) boot:  2 factory          factory app      00 00 00010000 00800000
    I (93) boot:  3 storage          Unknown data     01 82 00810000 00700000
    I (101) boot: End of partition table
    I (105) esp_image: segment 0: paddr=00010020 vaddr=48060020 size=bacdch (765148) map
    I (233) esp_image: segment 1: paddr=000cad04 vaddr=30100000 size=00020h (    32) load
    I (235) esp_image: segment 2: paddr=000cad2c vaddr=30100020 size=0003ch (    60) load
    I (240) esp_image: segment 3: paddr=000cad70 vaddr=4ff00000 size=052a8h ( 21160) load
    I (253) esp_image: segment 4: paddr=000d0020 vaddr=48000020 size=5b534h (374068) map
    I (315) esp_image: segment 5: paddr=0012b55c vaddr=4ff052a8 size=1caa0h (117408) load
    I (338) esp_image: segment 6: paddr=00148004 vaddr=4ff21d80 size=03074h ( 12404) load
    I (348) boot: Loaded app from partition at offset 0x10000
    I (349) boot: Disabling RNG early entropy source...
    I (360) hex_psram: vendor id    : 0x0d (AP)
    I (361) hex_psram: Latency      : 0x01 (Fixed)
    I (361) hex_psram: DriveStr.    : 0x00 (25 Ohm)
    I (364) hex_psram: dev id       : 0x03 (generation 4)
    I (370) hex_psram: density      : 0x07 (256 Mbit)
    I (375) hex_psram: good-die     : 0x06 (Pass)
    I (380) hex_psram: SRF          : 0x02 (Slow Refresh)
    I (386) hex_psram: BurstType    : 0x00 ( Wrap)
    I (391) hex_psram: BurstLen     : 0x03 (2048 Byte)
    I (397) hex_psram: BitMode      : 0x01 (X16 Mode)
    I (402) hex_psram: Readlatency  : 0x04 (14 cycles@Fixed)
    I (408) hex_psram: DriveStrength: 0x00 (1/1)
    I (413) MSPI DQS: tuning success, best phase id is 2
    I (597) MSPI DQS: tuning success, best delayline id is 11
    I esp_psram: Found 32MB PSRAM device
    I esp_psram: Speed: 200MHz
    I (597) mmu_psram: flash_drom_paddr_start: 0x10000
    I (640) mmu_psram: flash_irom_paddr_start: 0xd0000
    I (659) hex_psram: psram CS IO is dedicated
    I (659) cpu_start: Multicore app
    I (1091) esp_psram: SPI SRAM memory test OK
    W (1101) clk: esp_perip_clk_init() has not been implemented yet
    I (1108) cpu_start: Pro cpu start user code
    I (1108) cpu_start: cpu freq: 360000000 Hz
    I (1108) app_init: Application information:
    I (1111) app_init: Project name:     lvgl_demo_v9
    I (1117) app_init: App version:      7e53cd00-dirty
    I (1122) app_init: Compile time:     Sep 27 2024 17:00:22
    I (1128) app_init: ELF file SHA256:  506da7290...
    I (1134) app_init: ESP-IDF:          v5.4-dev-2167-gdef35b1ca7-dirty
    I (1141) efuse_init: Min chip rev:     v0.1
    I (1146) efuse_init: Max chip rev:     v0.99 
    I (1151) efuse_init: Chip rev:         v0.1
    I (1155) heap_init: Initializing. RAM available for dynamic allocation:
    I (1163) heap_init: At 4FF26B50 len 00014470 (81 KiB): RAM
    I (1169) heap_init: At 4FF3AFC0 len 00004BF0 (18 KiB): RAM
    I (1175) heap_init: At 4FF40000 len 00040000 (256 KiB): RAM
    I (1182) heap_init: At 50108080 len 00007F80 (31 KiB): RTCRAM
    I (1188) heap_init: At 3010005C len 00001FA4 (7 KiB): TCM
    I (1194) esp_psram: Adding pool of 30848K of PSRAM memory to heap allocator
    I (1202) spi_flash: detected chip: generic
    I (1206) spi_flash: flash io: qio
    W (1210) i2c: This driver is an old driver, please migrate your application code to adapt `driver/i2c_master.h`
    I (1221) main_task: Started on CPU0
    I (1244) esp_psram: Reserving pool of 32K of internal memory for DMA/internal allocations
    I (1244) main_task: Calling app_main()
    I (1246) LVGL: Starting LVGL task
    W (1250) ledc: GPIO 26 is not usable, maybe conflict with others
    I (1257) ESP32_P4_EV: MIPI DSI PHY Powered on
    I (1263) ESP32_P4_EV: Install MIPI DSI LCD control panel
    I (1268) ESP32_P4_EV: Install EK79007 LCD control panel
    I (1274) ek79007: version: 0.1.0
    I (1278) gpio: GPIO[27]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (1444) ESP32_P4_EV: Display initialized
    E (1446) lcd_panel: esp_lcd_panel_swap_xy(50): swap_xy is not supported by this panel
    W (1446) GT911: Unable to initialize the I2C address
    I (1452) GT911: TouchPad_ID:0x39,0x31,0x31
    I (1456) GT911: TouchPad_Config_Version:89
    I (1461) ESP32_P4_EV: Setting LCD backlight: 100%
    I (1542) main_task: Returned from app_main()
    ...
    ```

## Technical Support and Feedback

Please use the following feedback channels:

- For technical queries, go to the [esp32.com](https://esp32.com/viewforum.php?f=22) forum.
- For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-dev-kits/issues).

We will get back to you as soon as possible.
