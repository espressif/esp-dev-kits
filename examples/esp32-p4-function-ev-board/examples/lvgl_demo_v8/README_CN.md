# LVGL Demo v8

[英文版本](./README.md)

该示例演示了如何移植 LVGL v8 并使用 LVGL 内置的示例进行性能测试。该示例使用了开发板的 MIPI-DSI 接口。基于此示例，可以开发基于 LVGL v8 的应用。

## 快速入门

### 准备工作

* 一块 ESP32-P4-Function-EV-Board 开发板。
* 一块由 [EK79007](https://dl.espressif.com/dl/schematics/display_driver_chip_EK79007AD_datasheet.pdf) 芯片驱动的 7 英寸 1024 x 600 LCD 屏幕，配有 32 针 FPC 连接 [适配板](https://dl.espressif.com/dl/schematics/esp32-p4-function-ev-board-lcd-subboard-schematics.pdf) ([LCD 规格](https://dl.espressif.com/dl/schematics/display_datasheet.pdf))。
* 用于供电和编程的 USB-C 电缆。
* 请参考以下步骤进行连接：
    * **步骤 1**. 根据下表，将屏幕适配板背面的引脚连接到开发板的相应引脚。

        | 屏幕适配板            | ESP32-P4-Function-EV-Board |
        | -------------------- | -------------------------- |
        | 5V（任意一个）        | 5V（任意一个）              |
        | GND（任意一个）       | GND（任意一个）             |
        | PWM                  | GPIO26                     |
        | LCD_RST              | GPIO27                     |

    * **步骤 2**. 通过 `MIPI_DSI` 接口连接 LCD 的 FPC。
    * **步骤 3**. 使用 USB-C 电缆将 `USB-UART` 端口连接到 PC（用于供电和查看串行输出）。
    * **步骤 4**. 打开开发板的电源开关。

### ESP-IDF 要求

- 此示例支持 ESP-IDF release/v5.3 及以上版本。默认情况下，在 ESP-IDF release/v5.3 上运行。
- 请参照 [ESP-IDF 编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html) 设置开发环境。**强烈推荐** 通过 [编译第一个工程](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html#id8) 来熟悉 ESP-IDF，并确保环境设置正确。

### 获取 esp-dev-kits 仓库

在编译 esp-dev-kits 仓库中的示例之前，请先在终端中运行以下命令，将该仓库克隆到本地：

```
git clone --recursive https://github.com/espressif/esp-dev-kits.git
```

### 配置


运行 ``idf.py menuconfig`` 并修改 ``Board Support Package(ESP32-P4)`` 配置：

```
menuconfig > Component config > Board Support Package
```

## 如何使用示例


### 编译和烧录示例

编译项目并将其烧录到开发板上，运行监视工具可查看串行端口输出（将 `PORT` 替换为所用开发板的串行端口名）：

```c
idf.py -p PORT flash monitor
```

输入``Ctrl-]`` 可退出串口监视。

有关配置和使用 ESP-IDF 来编译项目的完整步骤，请参阅 [ESP-IDF 快速入门指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html) 。

### 示例输出

- 完整日志如下所示：

    ```c
    I (25) boot: ESP-IDF v5.4-dev-2167-gdef35b1ca7-dirty 2nd stage bootloader
    I (26) boot: compile time Sep 27 2024 16:56:08
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
    I (105) esp_image: segment 0: paddr=00010020 vaddr=48060020 size=7573ch (481084) map
    I (189) esp_image: segment 1: paddr=00085764 vaddr=30100000 size=00020h (    32) load
    I (191) esp_image: segment 2: paddr=0008578c vaddr=30100020 size=0003ch (    60) load
    I (196) esp_image: segment 3: paddr=000857d0 vaddr=4ff00000 size=0a848h ( 43080) load
    I (212) esp_image: segment 4: paddr=00090020 vaddr=48000020 size=58208h (360968) map
    I (270) esp_image: segment 5: paddr=000e8230 vaddr=4ff0a848 size=0e2b8h ( 58040) load
    I (283) esp_image: segment 6: paddr=000f64f0 vaddr=4ff18b00 size=031b4h ( 12724) load
    I (291) boot: Loaded app from partition at offset 0x10000
    I (292) boot: Disabling RNG early entropy source...
    I (303) hex_psram: vendor id    : 0x0d (AP)
    I (304) hex_psram: Latency      : 0x01 (Fixed)
    I (304) hex_psram: DriveStr.    : 0x00 (25 Ohm)
    I (307) hex_psram: dev id       : 0x03 (generation 4)
    I (313) hex_psram: density      : 0x07 (256 Mbit)
    I (318) hex_psram: good-die     : 0x06 (Pass)
    I (323) hex_psram: SRF          : 0x02 (Slow Refresh)
    I (329) hex_psram: BurstType    : 0x00 ( Wrap)
    I (334) hex_psram: BurstLen     : 0x03 (2048 Byte)
    I (340) hex_psram: BitMode      : 0x01 (X16 Mode)
    I (345) hex_psram: Readlatency  : 0x04 (14 cycles@Fixed)
    I (351) hex_psram: DriveStrength: 0x00 (1/1)
    I (356) MSPI DQS: tuning success, best phase id is 2
    I (544) MSPI DQS: tuning success, best delayline id is 11
    I esp_psram: Found 32MB PSRAM device
    I esp_psram: Speed: 200MHz
    I (545) mmu_psram: flash_drom_paddr_start: 0x10000
    I (575) mmu_psram: flash_irom_paddr_start: 0x90000
    I (594) hex_psram: psram CS IO is dedicated
    I (594) cpu_start: Multicore app
    I (1050) esp_psram: SPI SRAM memory test OK
    W (1060) clk: esp_perip_clk_init() has not been implemented yet
    I (1067) cpu_start: Pro cpu start user code
    I (1067) cpu_start: cpu freq: 360000000 Hz
    I (1067) app_init: Application information:
    I (1070) app_init: Project name:     lvgl_demo_v8
    I (1076) app_init: App version:      7e53cd00-dirty
    I (1081) app_init: Compile time:     Sep 27 2024 16:55:57
    I (1087) app_init: ELF file SHA256:  f111a533d...
    I (1093) app_init: ESP-IDF:          v5.4-dev-2167-gdef35b1ca7-dirty
    I (1100) efuse_init: Min chip rev:     v0.1
    I (1104) efuse_init: Max chip rev:     v0.99 
    I (1109) efuse_init: Chip rev:         v0.1
    I (1114) heap_init: Initializing. RAM available for dynamic allocation:
    I (1122) heap_init: At 4FF1DCE0 len 0001D2E0 (116 KiB): RAM
    I (1128) heap_init: At 4FF3AFC0 len 00004BF0 (18 KiB): RAM
    I (1134) heap_init: At 4FF40000 len 00040000 (256 KiB): RAM
    I (1140) heap_init: At 50108080 len 00007F80 (31 KiB): RTCRAM
    I (1147) heap_init: At 3010005C len 00001FA4 (7 KiB): TCM
    I (1153) esp_psram: Adding pool of 31360K of PSRAM memory to heap allocator
    I (1161) spi_flash: detected chip: generic
    I (1165) spi_flash: flash io: qio
    W (1169) i2c: This driver is an old driver, please migrate your application code to adapt `driver/i2c_master.h`
    I (1180) main_task: Started on CPU0
    I (1203) esp_psram: Reserving pool of 32K of internal memory for DMA/internal allocations
    I (1203) main_task: Calling app_main()
    I (1205) LVGL: Starting LVGL task
    W (1209) ledc: GPIO 26 is not usable, maybe conflict with others
    I (1216) ESP32_P4_EV: MIPI DSI PHY Powered on
    I (1222) ESP32_P4_EV: Install MIPI DSI LCD control panel
    I (1227) ESP32_P4_EV: Install EK79007 LCD control panel
    I (1233) ek79007: version: 0.1.0
    I (1237) gpio: GPIO[27]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (1403) ESP32_P4_EV: Display initialized
    E (1404) lcd_panel: esp_lcd_panel_swap_xy(50): swap_xy is not supported by this panel
    W (1405) GT911: Unable to initialize the I2C address
    I (1411) GT911: TouchPad_ID:0x39,0x31,0x31
    I (1415) GT911: TouchPad_Config_Version:89
    I (1420) ESP32_P4_EV: Setting LCD backlight: 100%
    I (1489) main_task: Returned from app_main()
    ...
    ```

## 技术支持与反馈

请通过以下渠道进行反馈：

- 有关技术问题，请访问 [esp32.com](https://esp32.com/viewforum.php?f=22) 论坛。
- 有关功能请求或错误报告，请创建新的 [GitHub 问题](https://github.com/espressif/esp-dev-kits/issues)。


我们会尽快回复。