=====================
ESP32-S2-HMI-DevKit-1
=====================

:link_to_translation:`en: [English]`

本指南将帮助您快速上手 ESP32-S2-HMI-DevKit-1，并提供该款开发板的详细信息。


组件介绍
-----------

该开发板是基于 ESP32-S2 的 HMI 开发板，其板载资源如下图所示。

.. figure:: ../../../_static/esp32-s2-hmi-devkit-1/board_func.jpg
   :align: center
   :alt: ESP32-S2-HMI-DevKit-0（点击放大）
   :figclass: align-center

   ESP32-S2-HMI-DevKit-0（点击放大）

该开发板使用 ESP32-S2-WROVER 模组，模组内置 4 MB flash 和 2 MB PSRAM。开发板搭载了一块使用 16 位 8080 并口的 4.3 寸 TFT-LCD，分辨率为 480×800，配有 256 级硬件 DC 背光调节电路（V1.1 版本开发板暂未配备该功能）。并配备了电容式触摸面板，使用 I2C 接口进行通讯。

开发板上搭载了带有三轴加速计和三轴陀螺仪的惯性传感器、环境光传感器、温湿度传感器、IO 扩展器、I2C ADC、可编程 RGB LED 灯、麦克风、音频功放，SD 卡接口等。

该开发板提供了多个扩展接口方便用户进行二次开发，包括 5 V 与 3.3 V 电源接口、程序下载/UART 接口、I2C 接口、SPI 接口、USB 接口（支持 USB OTG）、TWAI（兼容 CAN 2.0）接口等。

开发板配备了一块 1950 mAh 的单芯锂离子电池，并配有充电 IC 可对电池进行充电。


开始开发应用
-------------

通电前，请确保开发板完好无损。


必备硬件
^^^^^^^^^^

.-  1 x 装有 Windows、Mac OS 或 Linux 系统的 PC（推荐使用 Linux 操作系统）
-  1 x ESP32-S2-HMI-DevKit-1
-  1 x USB-C 数据线（如果想同时评估 MCU 的 USB 功能，推荐准备 2 根 USB-C 数据线）
-  1 x 扬声器(8 Ohm, 2 W)
-  1 x Micro-SD 卡（部分示例可能需要大容量存储）

.. note::

  请确保使用适当的 USB 数据线。部分数据线仅可用于充电，无法用于数据传输和编程。


硬件设置
^^^^^^^^^^

为了方便您对所有例程进行快速评估，请按照以下步骤设置开发板：

1. 将 Micro-SD 卡插入卡槽。请确保您的重要资料已经备份，若分区不为 FAT 格式，该 Micro-SD 可能会被格式化。
2. 如果需要评估音频播放功能，请将开发板下方 USB 口附近的扬声器焊盘连接至附赠的扬声器，或者连接至其它相近规格的扬声器 (8 Ohm, 2 W)。


软件设置
^^^^^^^^^^

首先，请确保您已经正确完成 IDF 环境配置。为确保这一点，请在终端中输入 ``idf.py --version``，如果输出结果类似于 ``ESP-IDF v4.2-dev-2084-g98d5b5dfd``，则代表安装成功。详细的安装和配置说明请参考 `快速入门文档 <https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32s2/get-started/index.html>`_。

配置完成后，请切换到仓库所在的目录。所有的示例代码都被放置在 examples 目录下。您可以运行 ``idf.py build`` 对工程进行编译。


工程选项
^^^^^^^^^^

您可以在各示例目录下，输入 ``idf.py menuconfig`` 配置工程选项。

在 menuconfig 中，请确保正确配置以下选项：

-  ``(Top) > HMI Board Config > HMI board``：开发板版本选择，默认请选择 ``ESP32-S2-HMI-DevKit-V2``；
-  ``(Top) > HMI Board Config > Audio HAL``：音频输出接口选择，使用 PWM 或 DAC；
-  ``(Top) > HMI Board Config > LCD Drivers``：LCD 显示 IC 型号选择，ESP32-S2-HMI-DevKit-1 使用的显示 IC 为 RM68120；
-  ``(Top) > Component config > ESP32S2-specific`` 中，请进入 ``Support for external, SPI-connected RAM`` 选项：

   -  在 ``SPI RAM config > Set RAM clock speed`` 中，将 PSRAM 的时钟设置为 ``80 MHz clock speed``；

-  ``(Top) -> Component config -> FreeRTOS``：``Tick rate (Hz)`` 设置为 1000。

在每个示例下，我们都提供了名为 ``sdkconfig.defaults`` 的默认配置文件，已将上述选项配置完成。


IDF 版本依赖
^^^^^^^^^^^^^

``examples/storage/usb_msc`` 需要在 IDF v4.3 下进行编译，其它示例程序可以在 IDF v4.2 及以后版本进行编译。


内含组件和包装
----------------


零售订单
^^^^^^^^^^

如购买样品，每个开发板将以防静电袋或零售商选择的其他方式包装。

零售订单请前往 https://www.espressif.com/zh-hans/company/contact/buy-a-sample。


批量订单
^^^^^^^^^^

如批量购买，开发板将以大纸板箱包装。

批量订单请前往 https://www.espressif.com/zh-hans/contact-us/sales-questions。
