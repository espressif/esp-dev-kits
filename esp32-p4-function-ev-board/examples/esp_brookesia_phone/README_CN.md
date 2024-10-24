# ESP_Brookesia Phone

[英文版本](./README.md)

该示例基于 [ESP_Brookesia](https://github.com/espressif/esp-brookesia)，展示了一个类似 Android 的界面，其中包含许多不同的应用程序。该示例使用了开发板的 MIPI-DSI 接口、MIPI-CSI 接口、ESP32-C6、SD 卡和音频接口。基于此示例，可以基于 ESP_Brookesia 创建一个使用案例，从而高效开发多媒体应用程序。

## 快速入门

### 准备工作

* 一块 ESP32-P4-Function-EV-Board 开发板。
* 一块由 [EK79007](https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/_static/esp32-p4-function-ev-board/camera_display_datasheet/display_driver_chip_EK79007AD_datasheet.pdf) 芯片驱动的 7 英寸 1024 x 600 LCD 屏幕，配有 32 针 FPC 连接 [适配板](https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-lcd-subboard-schematics.pdf) ([LCD 规格](https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/_static/esp32-p4-function-ev-board/camera_display_datasheet/display_datasheet.pdf))。
* 一款由 SC2336 芯片驱动的 MIPI-CSI 摄像头，配有 32 针 FPC 连接的 [适配板](https://docs.espressif.com/projects/esp-dev-kits/en/latest/_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-camera-subboard-schematics.pdf)([摄像头规格](https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/_static/esp32-p4-function-ev-board/camera_display_datasheet/camera_datasheet.pdf))。
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

- 此示例支持 ESP-IDF release/v5.4 及以上版本。默认情况下，在 ESP-IDF release/v5.4 上运行。
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

若要使用 SD 卡并启用 "Video Player" APP，请运行 ``idf.py menuconfig`` 然后选择 ``Example Configurations`` > ``Enable SD Card``

**注意**

若要体验视频播放功能，请将 MJPEG 格式的视频保存在 SD 卡上，并将 SD 卡插入 SD 卡槽。**目前仅支持 MJPEG 格式的视频**。视频格式转换方法如下：

* 安装 ffmpeg.
```
    sudo apt update
    sudo apt install ffmpeg
```
* 使用 ffmpeg 转换视频.
```
   ffmpeg -i YOUR_INPUT_FILE_NAME.mp4 -vcodec mjpeg -q:v 2 -vf "scale=1024:600" -acodec copy YOUR_OUTPUT_FILE_NAME.mjpeg
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

## 技术支持与反馈

请通过以下渠道进行反馈：

- 有关技术问题，请访问 [esp32.com](https://esp32.com/viewforum.php?f=22) 论坛。
- 有关功能请求或错误报告，请创建新的 [GitHub 问题](https://github.com/espressif/esp-dev-kits/issues)。


我们会尽快回复。