# Factory Demo

[英文版本](./README.md)

该示例基于 [ESP_Brookesia](https://github.com/espressif/esp-brookesia)，展示了一个类似于安卓的界面，其中包含了许多不同的应用程序。该示例利用了开发板的 MIPI-DSI 接口、MIPI-CSI 接口、SD 卡槽、ESP32-C6 和音频接口。基于此示例，可以使用 ESP32-P4-Function-EV-Board 来体验 ESP32-P4 的性能。

## 快速入门

### 准备工作

* 一块 ESP32-P4-Function-EV-Board 开发板。
* 一块由 [EK79007](../../docs/_static/esp32-p4-function-ev-board/camera_display_datasheet/display_driver_chip_EK79007AD_datasheet.pdf) 芯片驱动的 7 英寸 1024 x 600 LCD 屏幕，配有 32 针 FPC 连接 [适配板](../../docs/_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-lcd-subboard-schematics.pdf) ([LCD 规格](../../docs/_static/esp32-p4-function-ev-board/camera_display_datasheet/display_datasheet.pdf))。
* 一款由 SC2336 芯片驱动的 MIPI-CSI 摄像头，配有 32 针 FPC 连接的 [适配板]((../../docs/_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-camera-subboard-schematics.pdf)) ([摄像头规格](../../docs/_static/esp32-p4-function-ev-board/camera_display_datasheet/camera_datasheet.pdf))。
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
    * **步骤 3**. 通过 `MIPI_CSI` 接口连接 Camera 的 FPC。
    * **步骤 4**. 使用 USB-C 电缆将 `USB-UART` 端口连接到 PC（用于供电和查看串行输出）。
    * **步骤 5**. 打开开发板的电源开关。

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

运行 `esptool.py --port PORT write_flash 0 p4_factory_v14_031.bin` 烧录项目.

输入``Ctrl-]`` 可退出串口监视。

有关配置和使用 ESP-IDF 来编译项目的完整步骤，请参阅 [ESP-IDF 快速入门指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html) 。

### 注意
要体验视频播放功能，请将 MJPEG 格式的视频保存在 SD 卡上，并将 SD 卡插入 SD 卡槽。**目前仅支持 MJPEG 格式的视频**。插入 SD 卡后，视频播放应用将自动出现在界面上

#### 视频格式转换
* 安装 ffmpeg.
```
    sudo apt update
    sudo apt install ffmpeg
```
* 使用 ffmpeg 转换视频.
```
   ffmpeg -i YOUR_INPUT_FILE_NAME.mp4 -vcodec mjpeg -q:v 2 -vf "scale=1024:600" -acodec copy YOUR_OUTPUT_FILE_NAME.mjpeg
```

### 示例输出

- 完整日志如下所示：

    ```c
    I (27) boot: ESP-IDF v5.4-dev-2166-g4eebfc4165 2nd stage bootloader
    I (28) boot: compile time Aug 13 2024 10:21:24
    I (28) boot: Multicore bootloader
    I (33) boot: chip revision: v0.1
    I (36) qio_mode: Enabling default flash chip QIO
    I (41) boot.esp32p4: SPI Speed      : 40MHz
    I (46) boot.esp32p4: SPI Mode       : QIO
    I (51) boot.esp32p4: SPI Flash Size : 16MB
    I (56) boot: Enabling RNG early entropy source...
    I (61) boot: Partition Table:
    I (65) boot: ## Label            Usage          Type ST Offset   Length
    I (72) boot:  0 nvs              WiFi data        01 02 00009000 00006000
    I (79) boot:  1 phy_init         RF data          01 01 0000f000 00001000
    I (87) boot:  2 factory          factory app      00 00 00010000 00900000
    I (94) boot:  3 storage          Unknown data     01 82 00910000 00400000
    I (102) boot:  4 model            Unknown data     01 82 00d10000 00200000
    I (109) boot:  5 fr               Unknown data     01 06 00f10000 00020000
    I (118) boot: End of partition table
    I (121) esp_image: segment 0: paddr=00010020 vaddr=48110020 size=42bb6ch (4373356) map
    I (881) esp_image: segment 1: paddr=0043bb94 vaddr=30100000 size=00020h (    32) load
    I (883) esp_image: segment 2: paddr=0043bbbc vaddr=30100020 size=0003ch (    60) load
    I (888) esp_image: segment 3: paddr=0043bc00 vaddr=4ff00000 size=04418h ( 17432) load
    I (900) esp_image: segment 4: paddr=00440020 vaddr=48000020 size=105730h (1070896) map
    I (1088) esp_image: segment 5: paddr=00545758 vaddr=4ff04418 size=1722ch ( 94764) load
    I (1109) esp_image: segment 6: paddr=0055c98c vaddr=4ff1b680 size=037f4h ( 14324) load
    I (1119) boot: Loaded app from partition at offset 0x10000
    I (1119) boot: Disabling RNG early entropy source...
    I (1131) hex_psram: vendor id    : 0x0d (AP)
    I (1131) hex_psram: Latency      : 0x01 (Fixed)
    I (1132) hex_psram: DriveStr.    : 0x00 (25 Ohm)
    I (1135) hex_psram: dev id       : 0x03 (generation 4)
    I (1141) hex_psram: density      : 0x07 (256 Mbit)
    I (1146) hex_psram: good-die     : 0x06 (Pass)
    I (1151) hex_psram: SRF          : 0x02 (Slow Refresh)
    I (1157) hex_psram: BurstType    : 0x00 ( Wrap)
    I (1163) hex_psram: BurstLen     : 0x03 (2048 Byte)
    I (1168) hex_psram: BitMode      : 0x01 (X16 Mode)
    I (1174) hex_psram: Readlatency  : 0x04 (14 cycles@Fixed)
    I (1180) hex_psram: DriveStrength: 0x00 (1/1)
    I (1185) MSPI DQS: tuning success, best phase id is 2
    I (1373) MSPI DQS: tuning success, best delayline id is 12
    I esp_psram: Found 32MB PSRAM device
    I esp_psram: Speed: 200MHz
    I (1374) mmu_psram: flash_drom_paddr_start: 0x10000
    I (1704) mmu_psram: flash_irom_paddr_start: 0x440000
    I (1787) hex_psram: psram CS IO is dedicated
    I (1788) cpu_start: Multicore app
    I (2112) esp_psram: SPI SRAM memory test OK
    W (2122) clk: esp_perip_clk_init() has not been implemented yet
    I (2128) cpu_start: Pro cpu start user code
    I (2129) cpu_start: cpu freq: 360000000 Hz
    I (2129) app_init: Application information:
    I (2132) app_init: Project name:     eui_demo
    I (2137) app_init: App version:      2f78502e-dirty
    I (2142) app_init: Compile time:     Aug 13 2024 10:21:13
    I (2148) app_init: ELF file SHA256:  7443ef054...
    --- Warning: Checksum mismatch between flashed and built applications. Checksum of built application is dea2628c204a0aed070c5d54306be1ab34e311fa13a058d4d045fe8d28559d3a
    I (2154) app_init: ESP-IDF:          v5.4-dev-2166-g4eebfc4165
    I (2160) efuse_init: Min chip rev:     v0.0
    I (2165) efuse_init: Max chip rev:     v0.99 
    I (2170) efuse_init: Chip rev:         v0.1
    I (2175) heap_init: Initializing. RAM available for dynamic allocation:
    I (2182) heap_init: At 4FF22CD0 len 000182F0 (96 KiB): RAM
    I (2188) heap_init: At 4FF3AFC0 len 00004BF0 (18 KiB): RAM
    I (2195) heap_init: At 4FF40000 len 00040000 (256 KiB): RAM
    I (2201) heap_init: At 50108080 len 00007F80 (31 KiB): RTCRAM
    I (2207) heap_init: At 3010005C len 00001FA4 (7 KiB): TCM
    I (2213) esp_psram: Adding pool of 23104K of PSRAM memory to heap allocator
    I (2221) spi_flash: detected chip: generic
    I (2226) spi_flash: flash io: qio
    I (2230) host_init: ESP Hosted : Host chip_ip[18]
    I (2263) H_API: ESP-Hosted starting. Hosted_Tasks: prio:23, stack: 5120 RPC_task_stack: 5120
    sdio_mempool_create free:23784316 min-free:23784316 lfb-def:23592960 lfb-8bit:23592960

    I (2269) gpio: GPIO[18]| InputEn: 0| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (2279) gpio: GPIO[19]| InputEn: 0| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (2288) gpio: GPIO[14]| InputEn: 0| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (2297) gpio: GPIO[15]| InputEn: 0| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (2307) gpio: GPIO[16]| InputEn: 0| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (2316) gpio: GPIO[17]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (2326) H_API: ** add_esp_wifi_remote_channels **
    I (2331) transport: Add ESP-Hosted channel IF[1]: S[0] Tx[0x4800d500] Rx[0x4801a032]
    ...
    ```
## 故障排除

- 屏幕背光暗淡：可能是由于 R12 电阻焊接不良造成的。
- 屏幕不亮：可能是杜邦线连接不良。
- 屏幕逐渐变白：这表明屏幕排线存在问题。
- 无法打开相机：这可能是由于相机 FPC 连接不当造成的

## 技术支持与反馈

请通过以下渠道进行反馈：

- 有关技术问题，请访问 [esp32.com](https://esp32.com/viewforum.php?f=22) 论坛。
- 有关功能请求或错误报告，请创建新的 [GitHub 问题](https://github.com/espressif/esp-dev-kits/issues)。


我们会尽快回复。