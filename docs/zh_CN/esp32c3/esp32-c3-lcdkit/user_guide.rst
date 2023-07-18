===============
ESP32-C3-LCDkit
===============

:link_to_translation:`en:[English]`


本指南将帮助您快速上手 ESP32-C3-LCDkit，并提供该款开发板的详细信息。

本指南包括如下内容：

- `开发板概述`_：简要介绍了开发板的软件和硬件。
- `应用程序开发`_：介绍了应用程序开发过程中的软硬件设置。
- `硬件参考`_：详细介绍了开发板的硬件。
- `硬件版本`_：暂无历史版本。
- `样品获取`_：如何获取样品。
- `相关文档`_：列出了相关文档的链接。


开发板概述
===================

ESP32-C3-LCDkit 是一款基于 ESP32-C3 芯片和 SPI 接口显示屏的评估开发板，不仅通过旋转编码器开关实现屏幕交互，还具有音频播放和红外无线控制功能。由于 ESP32-C3 具有成本低、功耗低、性能强的特点，能够满足用户基本的 GUI 交互需求，其在小尺寸屏幕的应用场景中占据优势。

.. figure:: ../../../_static/esp32-c3-lcdkit/esp32-c3-lcdkit-isometric-raw.png
    :align: center
    :scale: 60%
    :alt: 搭载 ESP32-C3-MINI-1 模组的 ESP32-C3-LCDkit

    搭载 ESP32-C3-MINI-1 模组的 ESP32-C3-LCDkit


特性列表
----------------

该开发板具有以下特性：

-  **嵌入式模组**：板载 ESP32-C3-MINI-1 模组，内置 4 MB flash 以及 400 KB SRAM
-  **屏幕**：可搭配不同屏幕子板使用，支持 ``I2C`` 和 ``SPI`` 接口屏幕，请查看 `LCD 子板`_ 了解更多信息
-  **旋转编码器开关**：支持按键开关以及进行 360° 的旋转，用于实现对屏幕 GUI 的控制操作
-  **红外模块**：支持红外发射器和红外接收器，用于实现红外无线控制
-  **音频**：板载音频功放和扬声器，支持音频播放
-  **USB**：支持 USB Type-C 接口下载调试


功能框图
-------------

ESP32-C3-LCDkit 的主要组件和连接方式如下图所示。

.. figure:: ../../../_static/esp32-c3-lcdkit/esp32-c3-lcdkit-block-diagram.png
    :align: center
    :scale: 80%
    :alt: ESP32-C3-LCDkit 功能框图（点击放大）

    ESP32-C3-LCDkit 功能框图（点击放大）


组件介绍
-----------

ESP32-C3-LCDkit 是一款物种保护系列开发板，由主板和子板组成。

主板
^^^^^^

**ESP32-C3-LCDkit_MB** 主板是整个套件的核心，该主板集成了 ESP32-C3-MINI-1 模组，并提供与 LCD 子板连接的端口。

.. figure:: ../../../_static/esp32-c3-lcdkit/esp32-c3-lcdkit-layout-front.png
    :align: center
    :scale: 80%
    :alt: ESP32-C3-LCDkit - 正面（点击放大）

    ESP32-C3-LCDkit - 正面（点击放大）


以下按照顺时针顺序依次介绍开发板上的主要组件。

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - 主要组件
     - 介绍
   * - ESP32-C3-MINI-1 模组
     - ESP32-C3-MINI-1 模组是一款通用型 Wi-Fi + 低功耗蓝牙 MCU 模组，搭载 ESP32-C3 系列芯片，内置 4 MB flash 以及 400 KB SRAM。
   * - UART & RGB 扩展 I/O
     - 通过 2.54 mm 间距的排针连接系统电源管脚以及 UART 和 RGB 的数据引脚。
   * - RGB LED
     - 具有 RGB 三色显示功能的 LED，可供用户配置用来做状态行为指示。
   * - 扬声器
     - 可通过音频功率放大器的支持，实现扬声器播放功能。
   * - LCD 屏幕连接器
     - 通过 2.54 mm 间距的排母连接 1.28 英寸 LCD 子板。
   * - USB 电源端口
     - 为整个系统提供电源。建议使用至少 5V/2A 电源适配器供电，保证供电稳定。该端口用于 PC 端与 ESP32-C3-MINI-1 模组的 USB 通信。
   * - Reset 按键
     - 单独按下此按键会重置系统。
   * - 5V-to-3.3V LDO
     - 低压差线型稳压器 (LDO)。
   * - 3.3 V 电源指示灯
     - 用于指示系统供电的状态。
   * - EC11 旋转编码器开关
     - 同时具有 360° 旋转编码器和按键开关的功能，用于实现对屏幕 GUI 的控制操作。
   * - 音频功率放大芯片
     - 实现扬声器播放功能。
   * - 红外接收器
     - 用于接收外界发送的红外信号。
   * - 红外功能选择端口
     - 通过 2.54 mm 间距的排针和跳线帽选择使用红外发射或者接收的功能。
   * - 红外发射器
     - 用于向外界发送红外信号。


LCD 子板
^^^^^^^^

**ESP32-C3-LCDkit_DB** 子板支持 1.28 英寸、SPI 接口、分辨率为 240x240 的 LCD 屏，该屏使用的驱动芯片型号为 GC9A01。

.. figure:: ../../../_static/esp32-c3-lcdkit/esp32-c3-lcdkit-sub-front.png
    :align: center
    :scale: 60%
    :alt: ESP32-C3-LCDkit_DB - 正面（点击放大）

    ESP32-C3-LCDkit_DB - 正面（点击放大）

.. figure:: ../../../_static/esp32-c3-lcdkit/esp32-c3-lcdkit-sub-back.png
    :align: center
    :scale: 65%
    :alt: ESP32-C3-LCDkit_DB - 反面（点击放大）

    ESP32-C3-LCDkit_DB - 反面（点击放大）


软件支持
----------------

ESP32-C3-LCDkit 的开发框架为 `ESP-IDF <https://github.com/espressif/esp-idf>`_。ESP-IDF 是基于 FreeRTOS 的乐鑫 SoC 开发框架，具有众多组件，包括 LCD、ADC、RMT、SPI 等。开发板应用示例存放在 :project:`Examples <esp32-c3-lcdkit/examples>` 中，在示例目录下输入 ``idf.py menuconfig`` 可以配置工程选项。


应用程序开发
======================

本节介绍硬件和软件的设置方法，以及烧录固件至开发板以开发应用程序的说明。

必备硬件
--------

- 1 x ESP32-C3-LCDkit_MB
- 1 x LCD 子板
- 1 x USB 2.0 数据线（标准 A 型转 Type-C 型）
- 1 x 电脑（Windows、Linux 或 macOS）

.. note::

  请确保使用适当的 USB 数据线。部分数据线仅可用于充电，无法用于数据传输和程序烧录。

硬件设置
--------

准备开发板，加载第一个示例应用程序：

1. 连接 LCD 子板至 **LCD 屏幕连接器**。
2. 插入 USB 数据线，分别连接 PC 与开发板的 USB 端口。
3. LCD 屏幕亮起，可以使用旋转编码器开关控制 GUI。

硬件设置完成，接下来可以进行软件设置。

软件设置
--------

了解如何快速设置开发环境，请前往 `快速入门 <https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32c3/get-started/index.html>`__ > `安装 <https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32c3/get-started/index.html#get-started-step-by-step>`__。

了解开发应用程序的更多软件信息，请查看 `软件支持`_。


硬件参考
========

本节提供关于开发板硬件的更多信息。

GPIO 分配列表
-------------

下表为 ESP32-C3-MINI-1 模组管脚的 GPIO 分配列表，用于控制开发板的特定组件或功能。

.. list-table:: ESP32-C3-MINI-1 GPIO 分配
   :header-rows: 1
   :widths: 20 20 50

   * - 管脚
     - 管脚名称
     - 功能
   * - 1
     - GND
     - 接地
   * - 2
     - GND
     - 接地
   * - 3
     - 3V3
     - 供电
   * - 4
     - NC
     - 未连接
   * - 5
     - IO2
     - LCD_D/C
   * - 6
     - IO3
     - AUDIO_PA
   * - 7
     - NC
     - 未连接
   * - 8
     - EN
     - Reset
   * - 9
     - NC
     - 未连接
   * - 10
     - NC
     - 未连接
   * - 11
     - GND
     - 接地
   * - 12
     - IO0
     - LCD_SDA
   * - 13
     - IO1
     - LCD_SCL
   * - 14
     - GND
     - 接地
   * - 15
     - NC
     - 未连接
   * - 16
     - IO10
     - ENCODER_A
   * - 17
     - NC
     - 未连接
   * - 18
     - IO4
     - IR_RX/IR_TX
   * - 19
     - IO5
     - LCD_BL_CTRL
   * - 20
     - IO6
     - ENCODER_A
   * - 21
     - IO7
     - LCD_CS
   * - 22
     - IO8
     - RGB_LED
   * - 23
     - IO9
     - ENCODER_SW
   * - 24
     - NC
     - 未连接
   * - 25
     - NC
     - 未连接
   * - 26
     - IO18
     - USB_DN
   * - 27
     - IO19
     - USB_DP
   * - 28
     - NC
     - 未连接
   * - 29
     - NC
     - 未连接
   * - 30
     - RXD0
     - 预留
   * - 31
     - TXD0
     - 预留
   * - 32-35
     - NC
     - 未连接
   * - 36-53
     - GND
     - 接地


供电说明
--------

开发板通过 USB 端口供电：

.. figure:: ../../../_static/esp32-c3-lcdkit/esp32-c3-lcdkit-usb-ps.png
    :align: center
    :scale: 60%
    :alt: ESP32-C3-LCDkit - USB 电源供电

    ESP32-C3-LCDkit - USB 电源供电

输出系统电源供电：

.. figure:: ../../../_static/esp32-c3-lcdkit/esp32-c3-lcdkit-system-ps.png
    :align: center
    :scale: 80%
    :alt: ESP32-C3-LCDkit - 系统电源供电

    ESP32-C3-LCDkit - 系统电源供电

红外发射/接收选择
-----------------

由于红外发射模块和红外接收模块共用芯片上同一根信号线，需要通过跳线帽短接 ``红外功能选择端口`` 中特定的引脚，来选择使用红外发射或红外接收的功能：

.. figure:: ../../../_static/esp32-c3-lcdkit/esp32-c3-lcdkit-ir-receiver.png
    :align: center
    :scale: 80%
    :alt: ESP32-C3-LCDkit - 红外接收模块

    ESP32-C3-LCDkit - 红外接收模块

.. figure:: ../../../_static/esp32-c3-lcdkit/esp32-c3-lcdkit-ir-transmitter.png
    :align: center
    :scale: 80%
    :alt: ESP32-C3-LCDkit - 红外发射模块

    ESP32-C3-LCDkit - 红外发射模块

C6 模组兼容性设计
-----------------

**ESP32-C3-LCDkit** 开发板默认使用 ESP32-C3-MINI-1 模组，此时主板上连接了电阻 R2、R4、R5 和 R35：

.. figure:: ../../../_static/esp32-c3-lcdkit/esp32-c3-lcdkit-c6-design.png
    :align: center
    :scale: 80%
    :alt: ESP32-C3-LCDkit - C3/C6 电阻设置

    ESP32-C3-LCDkit - C3/C6 电阻设置

开发板在设计上还兼容了 ESP32-C6-MINI-1 模组，此时需要移除电阻 R2、R4、R5 和 R35，并连接上电阻 R13 和 R14。

硬件设置选项
------------

自动下载
^^^^^^^^

开发板上电后，手动按下旋转编码器开关和 Reset 键，然后先松开 Reset，再松开编码器，即可使 ESP 开发板进入下载模式。


硬件版本
================

- ESP32-C3-LCD-Ev-Board：该版本为工程样品，乐鑫有限维护。有关此版本开发板的历史文档，请参见 :doc:`ESP32-C3-LCD-Ev-Board 用户指南 <user_guide_c3_lcd_ev_board>`。


样品获取
================

ESP32-C3 具有成本优势和行业领先的低功耗性能，适用于构建由 SPI 接口驱动的旋钮屏或小尺寸显示屏。有采购需求请邮件联系 sales@espressif.com。


相关文档
========

-  `ESP32-C3 技术规格书 <https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_cn.pdf>`__
-  `ESP32-C3-MINI-1 技术规格书 <https://www.espressif.com/sites/default/files/documentation/esp32-c3-mini-1_datasheet_cn.pdf>`__
-  `乐鑫产品选型工具 <https://products.espressif.com/#/product-selector?names=>`__
-  `ESP32-C3-LCDkit_MB 原理图 <../../_static/esp32-c3-lcdkit/schematics/SCH_ESP32-C3-C6-LCDkit-MB_V1.1_20230417.pdf>`__
-  `ESP32-C3-LCDkit_MB PCB 布局图 <../../_static/esp32-c3-lcdkit/schematics/PCB_ESP32-C3-C6-LCDkit-MB_V1.1_20230418.pdf>`__
-  `ESP32-C3-LCDkit_DB 原理图 <../../_static/esp32-c3-lcdkit/schematics/SCH_ESP32-C3-LCDkit-DB_V1.0_20230329.pdf>`__
-  `ESP32-C3-LCDkit_DB PCB 布局图 <../../_static/esp32-c3-lcdkit/schematics/PCB_ESP32-C3-LCDkit-DB_V1.0_20230329.pdf>`__
-  `ESP32-C6-LCDkit_DB 原理图 <../../_static/esp32-c3-lcdkit/schematics/SCH_ESP32-C6-LCDkit-DB_V1.0_20230403.pdf>`__
-  `ESP32-C6-LCDkit_DB PCB 布局图 <../../_static/esp32-c3-lcdkit/schematics/PCB_ESP32-C6-LCDkit-DB_V1.0_20230403.pdf>`__
-  `1.28_TFT_240x240_SPI_屏 <../../_static/esp32-c3-lcdkit/datasheets/1.28_TFT_240x240_SPI_屏.pdf>`__
-  `红外发射器（IR67-21CTR8） <../../_static/esp32-c3-lcdkit/datasheets/EVERLIGHT(亿光)_IR67-21CTR8.PDF>`__
-  `红外接收器（IRM-H638TTR2） <../../_static/esp32-c3-lcdkit/datasheets/EVERLIGHT(亿光)_IRM-H638TTR2.PDF>`__
-  `音频功放（NS4150） <../../_static/esp32-c3-lcdkit/datasheets/NS4150B.pdf>`__
-  `RGB LED（WS2812B） <../../_static/esp32-c3-lcdkit/datasheets/WS2812B-Mini-V3.pdf>`__
-  `2415 音腔喇叭 <../../_static/esp32-c3-lcdkit/datasheets/方形2415音腔喇叭规格书-2P1.25-20MM.pdf>`__

有关本开发板的更多设计文档，请联系我们的商务部门 `sales@espressif.com <sales@espressif.com>`_。

.. toctree::
    :hidden:

    user_guide_c3_lcd_ev_board