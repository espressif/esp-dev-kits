ESP32-S2-Kaluga
===============

ESP32-S2-kaluga-1 是乐鑫发布的一款针对人机交互产品的开发套件，该套件由 1
个主板和 4 个主要功能的外接子板构成，可实现多子板的插接和拓展，实现
Audio 音频播放、 Touch 面板控制、 LCD 图像显示和 Camera
图像采集这四类功能，使用户通过感官通道以及肢体语言与机器进行多维交互。

**Touch 面板控制**

-  支持 14 个非常敏感的触摸传感器，其中有3个支持距离感应
-  支持 3 mm 亚克力面板
-  支持湿手操作
-  防误触设计：若多个触摸按键同时被水滴大面积覆盖，ESP32-S2
   会启动自锁功能（擦掉水滴后恢复正常）

**Audio 音频播放**

-  连接音箱可播放音频
-  通过和 Touch 面板的共用，可进行音频播放开关、调整声音大小。

**LCD 图像显示**

-  支持 LCD 接口（8-bit 并口 RGB、8080、6800 接口）
-  支持 ST7789 和 ILI9341 驱动的显示屏
-  支持将摄像头数据实时显示在 LCD 屏上

**Camera 图像采集**

-  支持 8/16 bit DVP 图像传感器接口
-  支持 OV3660 和 OV2640 驱动的摄像头
-  最高时钟频率支持到 40 MHz
-  DMA 传输带宽优化，高分辨率图像传输更轻松

将以上4类功能任意排列组合，可以满足用户对产品的不同需求。例如以下常见场景：

-  智能家居：从最简单的智能照明、智能门锁、智能插座，到白色家电、厨房电器、OTT
   设备和安全摄像头等视频流设备；
-  电池供电设备：Wi-Fi 联网传感器、Wi-Fi
   联网玩具、可穿戴设备和健康管理设备；
-  工业自动化设备：无线控制与机器人技术、智能照明、HVAC 控制设备等；
-  零售及餐饮业应用：POS 机和服务型机器人。

.. figure:: ../../../../docs/_static/esp32-s2-kaluga-1/ESP32-S2-Kaluga_V1.0_body.png
   :alt: kaluga 正面图


1. 内容
=======

1.1 目录
--------

::

    ├── components
    ├── docs
    │   ├── datasheet
    │   ├── schematic
    │   └── _static
    ├── examples
    │   ├── lcd
    │   ├── touch
    │   ├── camera
    │   ├── web_tts
    │   ├── touch_audio
    │   ├── README.md
    │   └── ...
    └── README_cn.md

-  **components**: 通用的程序组件
-  **docs**: 各部分的原理图及数据手册
-  datasheet : 各硬件的数据手册
-  schematic ：各硬件的原理图
-  \_static : 工程内各 ``README.md`` 调用的图片
-  **examples**:各种 ESP32-S2-Kaluga 相关示例程序

   -  `lcd <examples/lcd>`__ : LCD 屏显示本地图片和 RGB 三色。
   -  `touch <examples/touch>`__ : 触摸板控制 LED 灯显示不同颜色。
   -  `camera <examples/camera>`__ : 摄像头捕获图像后显示在 LCD 屏上。
   -  `adc\_button <examples/adc_button>`__ : 通过 Audio
      子版上的按键控制 LED 灯。
   -  `touch\_audio <examples/touch_audio>`__ :
      通过触摸板来控制音频播放音乐。
   -  `web\_tts <example/web_tts>`__ : 通过 Web 给 kaluga
      发送中文句子，Kaluga 将会通过本地合成语音进行播放。
   -  `Audio\_passthru <https://github.com/espressif/esp-adf/tree/master/examples/audio_processing/pipeline_passthru>`__
      : 实现录音放音的功能。（ESP-ADF）

2. ESP32-S2-Kaluga 快速入门
===========================

2.1 硬件准备
------------

.. raw:: html

   <table>

::

    <tr>
        <td ><img src="docs/_static/ESP32-S2-Kaluga_V1.0_mainbody.png" width="300" ><p align=center>ESP32-S2-Kaluga-1 开发板</p></td>
        <td ><img src="docs/_static/ESP-LyraT-8311A-V1.0.png" width="300"><p align=center>ESP-LyraT-8311A 音频板</p></td>
        <td ><img src="docs/_static/ESP-LyraP-LCD32_V1.0.png" width="300"><p align=center>EESP-LyraP-LCD32 显示屏</p></td>
        <td ><img src="docs/_static/ESP-LyraP-TOUCHA_V1.0.png" width="300"><p align=center>ESP-LyraP-TOUCHA 触摸板</p></td>
        <td ><img src="docs/_static/ESP-LyraP-CAM_V1.0.png" width="300"><p align=center>ESP-LyraP-CAM 摄像头</p></td>
    </tr>

.. raw:: html

   </table>

-  两根 USB 2.0 数据线（标准 A 型转 Micro-B 型）
-  电脑（Windows、Linux 或 macOS）

有关如何为您的应用程序配置 ESP32-S2-Kaluga 模块，请参考每个
`examples <examples>`__ 的 ``README.md``.

2.2 软件准备
------------

::

    git clone --recursive https://github.com/espressif/esp32-s2-kaluga.git 

如果克隆不带\ ``--recursive``\ 标志的项目，请在执行任何操作之前转到\ ``esp32-s2-kaluga``\ 目录并运行命令\ ``git submodule update --init``\ 。

2.2.1 ESP-IDF
~~~~~~~~~~~~~

有关设置 ESP-IDF 的详细说明，请参阅设置指南：

-  `ESP-IDF v4.2
   入门指南 <https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/index.html>`__

我们将 `ESP-IDF
v4.2 <https://github.com/espressif/esp-idf/tree/release/v4.2>`__
作为默认版本。

2.2.2 Examples
~~~~~~~~~~~~~~

`examples <examples>`__ 文件夹包含了 ESP32-S2-Kaluga
各功能展示的示例程序。

以一个 `显示屏 <examples/lcd>`__ 程序为例： 1. 按照如上方式下载
ESP-IDF，并配置 IDF 的环境变量。

2. 进入一个示例文件夹：\ ``esp32-s2-kaluga/examples/lcd/``\ 。

::

    cd esp32-s2-kaluga/examples/lcd/

3. 编译并烧录工程

::

    idf.py set-target esp32s2 build
    idf.py flash -p [PORT] monitor

4. 高级用户可以使用命令修改某些选项 ``idf.py menuconfig``\ 。

检查每个示例的 ``README.md`` 以获取更多详细信息。

3. 附录
=======

3.1 原理图
----------

-  `ESP32-S2-Kaluga-1 v1.2
   主板 <docs/schematic/SCH_ESP32-S2-KALUGA-1_V1_2_20200325A.pdf>`__
-  `ESP-LyraT-8311A v1.2
   音频板 <docs/schematic/SCH_ESP-LYRAT-8311A_V1_2_20200324A.pdf>`__
-  `ESP-LyraP-LCD32 v1.2
   显示屏 <docs/schematic/SCH_ESP-LYRAP-LCD32_V1_1_20200324A.pdf>`__
-  `ESP-LyraP-TOUCHA v1.2
   触摸板 <docs/schematic/SCH_ESP-LYRAP-TOUCHA_V1.1_20200325A.pdf>`__
-  `ESP-LyraP-CAM v1.2
   摄像头 <docs/schematic/SCH_ESP-LYRAP-CAM_V1_20200302.pdf>`__

3.2 数据手册
------------

-  `ESP32-S2中文手册 <docs/datasheet/esp32-s2_datasheet_cn.pdf>`__
-  `LCD ST7789 <docs/datasheet/LCD_ST7789.pdf>`__
-  `Camera OV2640 <docs/datasheet/Camera_OV2640.pdf>`__
-  `Audio ES8311 <docs/datasheet/Audio_ES8311.pdf>`__
