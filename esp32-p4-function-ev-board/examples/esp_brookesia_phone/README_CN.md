# ESP_Brookesia Phone

[英文版本](./README.md)

该示例基于 [ESP_Brookesia](https://github.com/espressif/esp-brookesia)，展示了一个类似 Android 的界面，其中包含许多不同的应用程序。该示例使用了开发板的 MIPI-DSI 接口、ESP32-C6 和音频接口。基于此示例，可以基于 ESP_Brookesia 创建一个使用案例，从而高效开发 HMI 应用程序。

## 快速入门

### 准备工作

* 一块 ESP32-P4-Function-EV-Board。
* 一块由 [EK79007](../../docs/_static/esp32-p4-function-ev-board/camera_display_datasheet/display_driver_chip_EK79007AD_datasheet.pdf) 芯片驱动的 7 英寸 1024 x 600 LCD 屏幕，配有 32 针 FPC 连接 [适配板](../../docs/_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-lcd-subboard-schematics.pdf) ([LCD 规格](../../docs/_static/esp32-p4-function-ev-board/camera_display_datasheet/display_datasheet.pdf))。
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

## 技术支持与反馈

请通过以下渠道进行反馈：

- 有关技术问题，请访问 [esp32.com](https://esp32.com/viewforum.php?f=22) 论坛。
- 有关功能请求或错误报告，请创建新的 [GitHub 问题](https://github.com/espressif/esp-dev-kits/issues)。


我们会尽快回复。