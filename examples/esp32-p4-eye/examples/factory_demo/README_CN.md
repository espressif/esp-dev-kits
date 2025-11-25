# Factory Demo

[英文版本](./README.md)

该示例基于 ESP32-P4-EYE 开发板，展示了一个功能全面的迷你相机应用，支持拍照、定时拍照、录像、相册预览、USB 挂载 SD 卡以及图像参数设置（可调节分辨率、饱和度、对比度、亮度和色度）。在此基础上，应用还集成了人脸检测、行人检测以及基于 YOLOv11 nano 模型的目标检测功能，增强了智能视觉识别能力。项目综合利用了开发板的多种外设资源，包括 MIPI-CSI 摄像头接口、SPI 接口 LCD 显示屏、USB High-Speed 接口、按键输入、旋转编码器和 SD 卡存储等。

## 快速入门

### 准备工作

* 一块 ESP32-P4-EYE 开发板。
* 用于供电和编程的 USB-C 电缆。
* 用于存储照片、录像的 SD 卡（可选）

### ESP-IDF 要求

此示例支持以下 ESP-IDF 版本：

- ESP-IDF release/v5.5 及所有 Bugfix 版本

请参照 [ESP-IDF 编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32p4/get-started/index.html) 设置开发环境。**强烈推荐** 通过 [编译第一个工程](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32p4/get-started/index.html#id8) 来熟悉 ESP-IDF，并确保环境设置正确。

### 获取 esp-dev-kits 仓库

在编译 esp-dev-kits 仓库中的示例之前，请先在终端中运行以下命令，将该仓库克隆到本地：

```
git clone --recursive https://github.com/espressif/esp-dev-kits.git
```

### 配置

运行 ``idf.py menuconfig`` 并修改 ``Board Support Package`` 配置：

```
menuconfig > Component config > Board Support Package
```

## 如何使用示例

### 应用补丁

* 当像素时钟设置为 80MHz 时，SPI 默认时钟源暂时可能无法满足时序需求。为此，请按照以下步骤应用补丁文件 `0004-fix-spi-default-clock-source.patch`：

1. 切换到 ESP-IDF 根目录并检出特定版本 (补丁针对 IDF release/v5.5 commit:98cd765953dfe0e7bb1c5df8367e1b54bd966cce 版本):

```bash
cd ~/esp-idf
git checkout release/v5.5
git checkout 98cd765953dfe0e7bb1c5df8367e1b54bd966cce
```

2. 将补丁文件复制到 ESP-IDF 根目录，例如：

```bash
cp 0004-fix-spi-default-clock-source.patch ~/esp-idf/
```

3. 切换到 ESP-IDF 根目录：

```bash
cd ~/esp-idf
```

4. 执行以下命令应用补丁

```bash
git apply 0004-fix-spi-default-clock-source.patch
```

* 若在拍照或录像过程中出现明显卡顿，可尝试应用补丁文件 `0004-fix-sdmmc-aligned-write-buffer.patch`，其使用方法同上。

### 编译和烧录示例

编译项目并将其烧录到开发板上，运行监视工具可查看串行端口输出（将 `PORT` 替换为所用开发板的串行端口名）：

```c
idf.py -p PORT flash monitor
```

输入``Ctrl-]`` 可退出串口监视。

有关配置和使用 ESP-IDF 来编译项目的完整步骤，请参阅 [ESP-IDF 快速入门指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html) 。

### 功能说明

| 示意图                                               | 功能     | 描述                                                         |
| ---------------------------------------------------- | -------- | ------------------------------------------------------------ |
| ![Camera](https://dl.espressif.com/AE/esp-dev-kits/esp32-p4-eye-capture.png)             | 拍照     | 用户可通过按下编码器按键进行拍照，同时支持通过旋转编码器或按键操作实现数码变焦功能。<br>**注意**：拍照功能需插入 SD 卡，照片将默认保存至 SD 卡中的 `esp32_p4_pic_save` 文件夹，并可通过相册功能进行预览 |
| ![interval_cam](https://dl.espressif.com/AE/esp-dev-kits/esp32_p4_eye_timed_capture.png) | 定时拍摄 | 用户可通过按键设置定时拍摄时间，并通过按下编码器按键启动定时拍摄模式。同时，可通过旋转编码器实现数码变焦。在定时拍摄过程中，按下任意按键即可停止拍摄。<br>**注意**：定时拍摄功能需插入 SD 卡，照片将默认保存至 SD 卡中的 `esp32_p4_pic_save` 文件夹，并可通过相册功能进行预览。 |
| ![video_mode](https://dl.espressif.com/AE/esp-dev-kits/esp32_p4_eye_video_record.png)     | 录像     | 用户可通过按下编码器按键开始录像，同时支持通过旋转编码器或按键操作进行数码变焦。<br>**注意**：录像功能需插入 SD 卡，视频将以 MP4 格式保存至 SD 卡中的 `esp32_p4_mp4_save` 文件夹。目前暂不支持通过相册功能预览 MP4 视频。 |
| ![ai_detect](https://dl.espressif.com/AE/esp-dev-kits/esp32_p4_eye_video_ai.png)     | AI 检测     | 检测功能支持人脸检测与行人检测，基于 esp-dl 推理框架实现，用户可通过上下翻按键在不同检测模式间切换。<br>**注意**：该模式下不可进行拍照。 |
| ![album](https://dl.espressif.com/AE/esp-dev-kits/esp32_p4_eye_album_preview.png)               | 相册     | 用户可在拍照模式或定时拍摄模式下预览已拍摄的照片，并通过按键进行上下翻页浏览。按下编码器按键可删除当前照片。若在设置中启用 YOLO 目标检测，浏览相册时将自动对照片进行目标检测。 |
| ![usb_disk](https://dl.espressif.com/AE/esp-dev-kits/esp32_p4_eye_usb_access.png)         | USB 挂载 | 将设备通过 USB 2.0 Device 接口连接至 PC 后，可直接访问 SD 卡中的文件内容。 |
| ![settings](https://dl.espressif.com/AE/esp-dev-kits/esp32_p4_eye_settings.png)         | 设置     | 用户可设置是否开启三轴加速度计旋转、是否开启 YOLO 目标检测、拍摄图像的分辨率、是否开启闪光灯，并可调节图像的饱和度、对比度、亮度和色度。 |

### 示例输出

- 完整日志如下所示：

    ```c
    I (27) boot: ESP-IDF v5.5-dev-2511-g2696c6bced8-dirt 2nd stage bootloader
    I (28) boot: compile time Apr  7 2025 20:58:06
    I (28) boot: Multicore bootloader
    I (31) boot: chip revision: v1.0
    I (33) boot: efuse block revision: v0.2
    I (36) boot.esp32p4: SPI Speed      : 80MHz
    I (40) boot.esp32p4: SPI Mode       : DIO
    I (44) boot.esp32p4: SPI Flash Size : 16MB
    I (48) boot: Enabling RNG early entropy source...
    I (52) boot: Partition Table:
    I (55) boot: ## Label            Usage          Type ST Offset   Length
    I (61) boot:  0 nvs              WiFi data        01 02 00009000 00006000
    I (68) boot:  1 phy_init         RF data          01 01 0000f000 00001000
    I (74) boot:  2 factory          factory app      00 00 00010000 00300000
    I (81) boot: End of partition table
    I (84) esp_image: segment 0: paddr=00010020 vaddr=480d0020 size=138a74h (1280628) map
    I (271) esp_image: segment 1: paddr=00148a9c vaddr=30100000 size=00424h (  1060) load
    I (273) esp_image: segment 2: paddr=00148ec8 vaddr=4ff00000 size=07150h ( 29008) load
    I (280) esp_image: segment 3: paddr=00150020 vaddr=48000020 size=c0638h (788024) map
    I (393) esp_image: segment 4: paddr=00210660 vaddr=4ff07150 size=13908h ( 80136) load
    I (407) esp_image: segment 5: paddr=00223f70 vaddr=4ff1aa80 size=03cf8h ( 15608) load
    I (412) esp_image: segment 6: paddr=00227c70 vaddr=50108080 size=0001ch (    28) load
    I (419) boot: Loaded app from partition at offset 0x10000
    I (419) boot: Disabling RNG early entropy source...
    I (433) hex_psram: vendor id    : 0x0d (AP)
    I (434) hex_psram: Latency      : 0x01 (Fixed)
    I (434) hex_psram: DriveStr.    : 0x00 (25 Ohm)
    I (435) hex_psram: dev id       : 0x03 (generation 4)
    I (439) hex_psram: density      : 0x07 (256 Mbit)
    I (443) hex_psram: good-die     : 0x06 (Pass)
    I (448) hex_psram: SRF          : 0x02 (Slow Refresh)
    I (452) hex_psram: BurstType    : 0x00 ( Wrap)
    I (456) hex_psram: BurstLen     : 0x03 (2048 Byte)
    I (461) hex_psram: BitMode      : 0x01 (X16 Mode)
    I (465) hex_psram: Readlatency  : 0x04 (14 cycles@Fixed)
    I (470) hex_psram: DriveStrength: 0x00 (1/1)
    I (475) MSPI DQS: tuning success, best phase id is 2
    I (657) MSPI DQS: tuning success, best delayline id is 10
    I esp_psram: Found 32MB PSRAM device
    I esp_psram: Speed: 200MHz
    I (752) mmu_psram: .rodata xip on psram
    I (813) mmu_psram: .text xip on psram
    I (814) hex_psram: psram CS IO is dedicated
    I (814) cpu_start: Multicore app
    I (825) cpu_start: Pro cpu start user code
    I (825) cpu_start: cpu freq: 400000000 Hz
    I (825) app_init: Application information:
    I (825) app_init: Project name:     p4_eye_factory_demo
    I (830) app_init: App version:      0.0.0
    I (833) app_init: Compile time:     Apr  7 2025 20:58:02
    I (838) app_init: ELF file SHA256:  2968383e9...
    I (843) app_init: ESP-IDF:          v5.5-dev-2511-g2696c6bced8-dirt
    I (849) efuse_init: Min chip rev:     v0.1
    I (853) efuse_init: Max chip rev:     v1.99 
    I (857) efuse_init: Chip rev:         v1.0
    I (860) heap_init: Initializing. RAM available for dynamic allocation:
    I (867) heap_init: At 4FF22310 len 00018CB0 (99 KiB): RAM
    I (872) heap_init: At 4FF3AFC0 len 00004BF0 (18 KiB): RAM
    I (877) heap_init: At 4FF40000 len 00040000 (256 KiB): RAM
    I (882) heap_init: At 5010809C len 00007F64 (31 KiB): RTCRAM
    I (887) heap_init: At 30100424 len 00001BDC (6 KiB): TCM
    I (893) esp_psram: Adding pool of 30656K of PSRAM memory to heap allocator
    I (899) spi_flash: detected chip: generic
    I (903) spi_flash: flash io: dio
    W (906) bsp_p4_eye: Auto-initializing ESP32-P4-EYE board using constructor attribute
    I (914) gpio: GPIO[46]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (922) gpio: GPIO[26]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (930) bsp_p4_eye: ESP32-P4-EYE board initialized successfully
    I (936) main_task: Started on CPU0
    I (939) esp_psram: Reserving pool of 32K of internal memory for DMA/internal allocations
    I (946) main_task: Calling app_main()
    I (954) gpio: GPIO[23]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (958) main: Initialize the display
    I (962) LVGL: Starting LVGL task
    I (965) gpio: GPIO[15]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (1275) app_storage: Settings loaded from NVS successfully
    I (1275) app_video_photo: Photo resolution set to 1920x1080
    I (1275) app_video_record: Photo resolution set to 640x480
    I (1280) app_storage: Settings saved to NVS successfully
    I (1285) app_storage: Settings saved to NVS successfully
    I (1290) app_storage: Settings saved to NVS successfully
    I (1295) app_storage: Camera settings loaded from NVS successfully
    W (1301) ui_extra: loaded_contrast: 53, loaded_saturation: 63, loaded_brightness: 59, loaded_hue: 5
    I (1327) main: Initialize the storage
    I (1327) app_storage: other wake up
    I (1327) app_storage: Interval state loaded: active=0, next_wake=0
    I (1329) app_storage: Interval state saved: active=0, next_wake=0
    I (1335) app_storage: Photo count saved: 0
    I (1354) app_video_stream: Interval photo stopped
    I (1354) app_storage: Photo count saved: 0
    I (1354) gpio: GPIO[45]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (1360) app_storage: SD card mounted
    W (1364) ldo: The voltage value 0 is out of the recommended range [500, 2700]
    I (1371) gpio: GPIO[42]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (1395) p4-eye: Setting LCD backlight: 100%
    I (1396) main: Initialize the application control module
    I (1396) gpio: GPIO[2]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (1402) gpio: GPIO[3]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (1424) gpio: GPIO[4]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (1424) gpio: GPIO[5]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (1432) button: IoT Button Version: 3.5.0
    I (1436) gpio: GPIO[3]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (1454) button: IoT Button Version: 3.5.0
    I (1454) gpio: GPIO[4]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (1461) app_album: SD card total space: 29970.00 MB
    I (1466) app_album: Calculated MAX_IMAGES: 20000 (based on 29970.00 MB total, 14985.00 MB allocated for storage)
    I (1556) app_album: Found 849 images in /sdcard/esp32_p4_pic_save (sorted by filename in descending order)
    I (1565) button: IoT Button Version: 3.5.0
    I (1565) gpio: GPIO[5]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (1567) button: IoT Button Version: 3.5.0
    I (1592) gpio: GPIO[2]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (1593) gpio: GPIO[48]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (1620) gpio: GPIO[47]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (1620) Knob: Iot Knob Config Succeed, encoder A:48, encoder B:47, direction:0, Version: 1.0.0
    I (1627) main: Initialize the I2C
    W (1650) i2c.master: Please check pull-up resistances whether be connected properly. Otherwise unexpected behavior would happen. For more detailed information, please read docs
    I (1680) main: Initialize the video streaming application
    I (1682) ov2710: Detected Camera sensor PID=0x2710
    I (1798) app_video: version: 0.9.0
    I (1799) app_video: driver:  MIPI-CSI
    I (1799) app_video: card:    MIPI-CSI
    I (1799) app_video: bus:     esp32p4:MIPI-CSI
    I (1801) app_video: width=1920 height=1080
    I (1804) app_album: Loaded image: /sdcard/esp32_p4_pic_save/pic_0849.jpg (485062 bytes)
    I (1814) app_storage: Camera settings loaded from NVS successfully
    I (1862) app_storage: Photo count loaded: 0
    I (1862) app_storage: Photo count saved: 0
    I (1862) p4-eye: bsp_microphone_init: 0
    W (1862) i2s_common: dma frame num is adjusted to 256 to align the dma buffer with 64, bufsize = 512
    I (1871) I2S_IF: channel mode 1 bits:16/16 channel:2 mask:3
    I (1876) Adev_Codec: Open codec device OK
    I (1880) app_storage: Album initialized
    I (1884) app_video: Video Stream Start
    I (1884) app_storage: Directory /sdcard/esp32_p4_pic_save already exists
    I (1896) main_task: Returned from app_main()
    I (1934) app_storage: Next picture number will be: 850
    I (1934) app_storage: Directory check completed
    I (1934) app_storage: USB MSC initialization
    W (1936) usb_phy: Using UTMI PHY instead of requested internal PHY
    W (1957) tusb_desc: No Device descriptor provided, using default.
    W (1957) tusb_desc: No FullSpeed configuration descriptor provided, using default.
    W (1963) tusb_desc: No HighSpeed configuration descriptor provided, using default.
    W (1970) tusb_desc: No Qulifier descriptor provided, using default.
    W (1976) tusb_desc: No String descriptors provided, using default.
    I (1997) tusb_desc: 
    ┌─────────────────────────────────┐
    │  USB Device Descriptor Summary  │
    ├───────────────────┬─────────────┤
    │bDeviceClass       │ 0           │
    ├───────────────────┼─────────────┤
    │bDeviceSubClass    │ 0           │
    ├───────────────────┼─────────────┤
    │bDeviceProtocol    │ 0           │
    ├───────────────────┼─────────────┤
    │bMaxPacketSize0    │ 64          │
    ├───────────────────┼─────────────┤
    │idVendor           │ 0x303a      │
    ├───────────────────┼─────────────┤
    │idProduct          │ 0x4002      │
    ├───────────────────┼─────────────┤
    │bcdDevice          │ 0x100       │
    ├───────────────────┼─────────────┤
    │iManufacturer      │ 0x1         │
    ├───────────────────┼─────────────┤
    │iProduct           │ 0x2         │
    ├───────────────────┼─────────────┤
    │iSerialNumber      │ 0x3         │
    ├───────────────────┼─────────────┤
    │bNumConfigurations │ 0x1         │
    └───────────────────┴─────────────┘
    I (2197) TinyUSB: TinyUSB Driver installed
    I (2197) app_storage: USB MSC initialization DONE
    ...
    ```

## 技术支持与反馈

请通过以下渠道进行反馈：

- 有关技术问题，请访问 [esp32.com](https://esp32.com/viewforum.php?f=22) 论坛。
- 有关功能请求或错误报告，请创建新的 [GitHub 问题](https://github.com/espressif/esp-dev-kits/issues)。


我们会尽快回复。