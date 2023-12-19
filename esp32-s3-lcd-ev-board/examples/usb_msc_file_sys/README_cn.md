# U 盘文件系统

本例程展示了 esp32s2/s3 使用 USB HOST 读写 U 盘的操作，可以使用开发板 `ESP32-S3-LCD-EV-Board` 或者 `ESP32-S3-LCD-EV-Board-2` 连接 U 盘，通过屏幕显示 U 盘中的文件.

目前支持预览的文件：
* JPG
* PNG
* BMP
* GIF
* MP3（需要外接扬声器）

## 如何使用例程

请首先阅读 `ESP32-S3-LCD-EV-Board` 的[用户指南](https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html#esp32-s3-lcd-ev-board)，了解有关其软件和硬件信息的内容。

* idf_version: >= release/v5.1

### 硬件需求

* `ESP32-S3-LCD-EV-Board（480x480）`或 `ESP32-S3-LCD-EV-Board-2（800x480）`开发板。
* 一个小于 32GB 的 U 盘。
* 一根 USB Type-C 电缆用于供电和编程（请连接到 UART 端口，而不是 USB 端口）

### 硬件连接

| USB_DP | USB_DM |
| ------ | ------ |
| GPIO20 | GPIO19 |

**注意：** 如果 U盘 连接到USB端口，需要短接二极管D1。有关详细信息，请查看[电源供应通过USB](https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html#power-supply-over-usb)部分的原理图。

### 构建和烧录


1. 默认情况下，该项目将 PSRAM 配置为 80M Octal 模式。**只有搭载 ESP32-S3-WROOM-1-N16R8 模组的开发板可以通过以下命令启用 PSRAM 120M DDR（Octal）功能**，详细信息请参见[此处](../../README.md#psram-120m-ddr)。
    ```
    rm -rf build sdkconfig sdkconfig.old
    idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.test.psram_120m_ddr" reconfigure
    ```
2. 运行 `idf.py -p PORT flash monitor` 来构建、烧录和监视项目。**请注意，必须连接到 UART 端口而不是 USB 端口。**

（要退出串行监视器，请键入 "Ctrl-]"。）

请参阅[入门指南](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html)了解配置和使用 ESP-IDF 构建项目的完整步骤。

## 示例输出

下面的动画是开发板运行示例的实际效果。

![usb_msc_file_sys](https://dl.espressif.com/AE/esp-dev-kits/s3-lcd-ev-board_examples_usb_msc_file_sys_5.gif)

## 故障排除

* 程序构建失败
    * 出现 `error: static assertion failed: "FLASH and PSRAM Mode configuration are not supported"` 错误消息：请修改 Flash （在`Serial flasher config`）和 PSRAM （在`SPI RAM config`）的组合配置，如下所示。
        |   Flash   |    PSRAM    |  IDF Version   |
        | :-------: | :---------: | -------------- |
        | QIO, 120M | Octal, 120M | >=release/v5,1 |
    * 出现 `error: 'esp_lcd_rgb_panel_config_t' has no member named 'num_fbs'` 错误消息：请更新ESP-IDF的分支（release/v5.0或master）。
* 程序上传失败
    * 硬件连接不正确：运行 `idf.py -p PORT monitor`，然后重启开发板，查看是否有输出日志。
    * 下载波特率过高：在 `menuconfig` 菜单中降低波特率，然后重试。
    * 出现 `A fatal error occurred: Could not open /dev/ttyACM0, the port doesn't exist` 错误消息：请先确保连接了开发板，然后按以下步骤将板子设置为 “Download Boot”：
        1. 持续按住 "BOOT(SW2)" 按钮。
        2. 短按一下 "RST(SW1)" 按钮。
        3. 松开 "BOOT(SW2)" 按钮。
        4. 上传程序并重启。
* 程序运行失败
    * 子板2屏幕（480x480）显示异常，背光亮起但没有显示图像：如果日志级别配置为"Debug"或更低，请同时增加日志输出的波特率（例如，2000000）。
    * 带有 `W (xxx) lcd_panel.io.3wire_spi: Delete but keep CS line inactive` 的警告消息：这是正常的消息，请忽略。
    * 引导过程中卡住：只有搭载 `ESP32-S3-WROOM-1-N16R8` 芯片的开发板可以启用 PSRAM 120M DDR（Octal）功能。当使用搭载 `ESP32-S3-WROOM-1-N16R16V` 芯片的开发板时，请在 menuconfig 中将 PSRAM 配置设置为 80M DDR（Octal）。

## 技术支持和反馈

请使用以下反馈渠道：

* 对于技术问题，请访问[esp32.com](https://esp32.com/)论坛。
* 对于功能请求或错误报告，请创建[GitHub问题](https://github.com/espressif/esp-dev-kits/issues)。

我们会尽快回复您。