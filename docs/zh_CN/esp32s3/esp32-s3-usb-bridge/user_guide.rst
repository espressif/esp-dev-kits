===================
ESP32-S3-USB-Bridge
===================

:link_to_translation:`en:[English]`

本指南将帮助您快速上手 ESP32-S3-USB-Bridge，并提供该款开发板的详细信息。

本指南包括如下内容：

- `开发板概述`_：简要介绍了开发板的软件和硬件
- `应用程序开发`_：介绍了应用程序开发过程中的软硬件设置
- `硬件参考`_：详细介绍了开发板的硬件
- `硬件版本`_：暂无历史版本
- `样品获取`_：如何获取样品
- `相关文档`_：列出了相关文档的链接


开发板概述
============

ESP32-S3-USB-Bridge 是一款基于 ESP32-S3 芯片的开发板。

- 使用 `usb_wireless_bridge <https://github.com/espressif/esp-dev-kits/tree/master/esp32-s3-usb-bridge/examples/usb_wireless_bridge>`_ 例程可在计算机和目标芯片之间建立桥接。它可以模拟 USB 复合设备，支持多种功能：

  - **USB 转 UART 桥接**：通过 USB 转 UART 桥接，实现计算机与目标芯片的串口数据收发。
  - **JTAG 适配器**：通过 JTAG 桥接，实现计算机与目标芯片之间双向传输 JTAG 通信。
  - **MSC 存储设备**：通过将 UF2 固件文件拖放到开发板的 USB 存储设备中，实现固件升级。
  - **无线桥接**：通过 ESP-NOW，实现无线烧录以及无线串口数据收发。

.. figure:: ../../../_static/esp32-s3-usb-bridge/esp32-s3-usb-bridge.png
    :align: center
    :scale: 35%
    :alt: ESP32-S3-USB-Bridge Type-C 连接

    ESP32-S3-USB-Bridge Type-C 连接

此外，开发板还支持 USB Type-A 接口，更换方便。

.. figure:: ../../../_static/esp32-s3-usb-bridge/esp32-s3-usb-bridge-typea.png
    :align: center
    :scale: 25%
    :alt: ESP32-S3-USB-Bridge Type-A 连接

    ESP32-S3-USB-Bridge Type-A 连接

特性列表
----------------

该开发板具有以下特性：

-  **嵌入式模组**：板载 ESP32-S3-MINI-1 模组，内置 4 MB flash 以及 2 MB PSRAM
-  **指示灯**：板载一颗 WS2812 指示灯，以及两颗串口数据指示灯
-  **USB**：板载 USB 转 UART 桥接器及 JTAG 适配器，支持 USB Type-C 接口下载调试


功能框图
-------------

ESP32-S3-USB-Bridge 的主要组件和连接方式如下图所示。

.. figure:: ../../../_static/esp32-s3-usb-bridge/esp32-s3-usb-bridge-block-diagram.png
    :align: center
    :scale: 90%
    :alt: ESP32-S3-USB-Bridge 功能框图（点击放大）

    ESP32-S3-USB-Bridge 功能框图（点击放大）


组件介绍
-----------

**ESP32-S3-USB-Bridge** 开发板较为小巧，尺寸为 23.3 mm * 31.5 mm。其集成了 ESP32-S3-MINI-1 模组，并提供了 12 个外接接口。

.. figure:: ../../../_static/esp32-s3-usb-bridge/esp32-s3-usb-bridge-front-instruction.png
    :align: center
    :scale: 60%
    :alt: ESP32-S3-USB-Bridge - 正面（点击放大）

    ESP32-S3-USB-Bridge - 正面（点击放大）


以下按照顺时针顺序依次介绍开发板上的主要组件。

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - 主要组件
     - 介绍
   * - ESP32-S3-MINI-1-N4R2 模组
     - ESP32-S3-MINI-1-N4R2 模组是一款通用型 Wi-Fi + 低功耗蓝牙 MCU 模组，搭载 ESP32-S3 系列芯片，内置 4 MB flash 以及 2 MB PSRAM。除具有丰富的外设接口外，模组还拥有强大的神经网络运算能力和信号处理能力，适用于 AIoT 领域的多种应用场景。
   * - TX/RX 指示灯
     - 用于指示串口数据的收发状态。
   * - 扩展连接器
     - 可供连接的 JTAG 管脚、串口管脚、TX/RX 管脚、Boot 管脚、Reset 管脚以及系统电压管脚。
   * - Reset 按键
     - 此按键连接目标芯片的 Reset 按键，与模组的 IO8 相连。单独按下此按钮，可以复位目标芯片。
   * - USB 转 USB 接口
     - 为整个系统提供电源。该端口用于 PC 端与 ESP32-S3-MINI-1 模组的 USB 通信。
   * - Boot 按键
     - 此按键连接目标芯片的 Boot 按键，与模组的 IO9 相连。长按 Boot 键时，再按 Reset 键可启动固件上传模式，然后便可通过串口或 USB 上传固件。

.. figure:: ../../../_static/esp32-s3-usb-bridge/esp32-s3-usb-bridge-back-instruction.png
    :align: center
    :scale: 55%
    :alt: ESP32-S3-USB-Bridge - 背面（点击放大）

    ESP32-S3-USB-Bridge - 背面（点击放大）

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - 主要组件
     - 介绍
   * - 5 V 转 3.3 V
     - 用于将 USB 电压转换为 3.3 V 电压，为 ESP32-S3-MINI-1 模组供电。
   * - 模组 Boot 按键
     - 此按键连接模组的 IO0 按键，长按此按键再重新给开发板上电，即可让开发板处于下载模式，上传新固件。
   * - WS2812
     - 与模组的 IO42 相连，用于指示开发板当前的状态。


软件支持
----------------

ESP32-S3-USB-Bridge 的开发框架为 `ESP-IDF <https://github.com/espressif/esp-idf>`_。ESP-IDF 是基于 FreeRTOS 的乐鑫 SoC 开发框架，具有众多组件，包括 LCD、ADC、RMT、SPI 等。开发板应用示例存放在 :project:`Examples <esp32-s3-usb-bridge/examples>` 中，在示例目录下输入 ``idf.py menuconfig`` 可以配置工程选项。

.. note::

  - 目前支持的 ESP-IDF 版本为 release/5.0。
  - 请不要按住模组自身的 Boot 按键后上下电，防止默认固件被替换。


应用程序开发
======================

本节介绍硬件和软件的设置方法，以及烧录固件至开发板以开发应用程序的说明。


必备硬件
--------

- 1 x ESP32-S3-USB-Bridge
- 1 x LCD 子板
- 1 x USB 2.0 数据线（标准 A 型转 Type-C 型）
- 1 x 电脑（Windows、Linux 或 macOS）

.. note::

  请确保使用适当的 USB 数据线。部分数据线仅可用于充电，无法用于数据传输和程序烧录。


硬件设置
--------

准备开发板，加载第一个示例应用程序：

1. 插入 USB 数据线，分别连接 PC 与开发板 USB 口。
2. 确保开发板处于下载模式。
3. 指示灯亮起，烧录完成。

硬件设置完成，接下来可以进行软件设置。


软件设置
--------

请前往 `快速入门 <https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html>`_ 的 `详细安装步骤 <https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html#get-started-how-to-get-esp-idf>`_ 小节查看如何快速设置开发环境。

了解开发应用程序的更多软件信息，请查看 `软件支持`_。


硬件参考
========

本节提供关于开发板硬件的更多信息。


GPIO 分配列表
-------------

下表为 ESP32-S3-MINI-1 模组管脚以及外接接口的 GPIO 分配列表，用于控制开发板以及外接目标芯片的特定组件或功能。

.. list-table:: ESP32-S3-MINI-1 管脚及外接接口 GPIO 分配
   :header-rows: 1
   :widths: 20 20 50

   * - 管脚
     - 管脚名称
     - 功能
   * - 1
     - GND
     - 接地
   * - 2
     - 3V3
     - 供电
   * - 3
     - IO0
     - 模组 Boot 按键，用于进入下载模式，以及作为按键输入管脚
   * - 4
     - IO2
     - JTAG 管脚 TDO，用于测试数据输出
   * - 5
     - IO3
     - JTAG 管脚 TDI，用于测试数据输入
   * - 6
     - IO4
     - JTAG 管脚 TCK，用于同步测试数据传输
   * - 7
     - IO5
     - JTAG 管脚 TMS，用于测试模式选择
   * - 8
     - IO8
     - 连接目标芯片的 Reset 管脚，按下为低电平
   * - 9
     - IO9
     - 连接目标芯片的 Boot 管脚，按下为低电平
   * - 10
     - IO19
     - 与 USB_D- 接口相连
   * - 11
     - IO20
     - 与 USB_D+ 接口相连
   * - 12
     - IO40
     - RX，用于连接目标芯片的 UART TX 管脚
   * - 13
     - IO41
     - TX，用于连接目标芯片的 UART RX 管脚
   * - 14
     - IO42
     - WS2812 控制管脚

.. note::

  管脚 3-14 为开发板提供的外接接口。除上表所列内容外，所有引出 IO 均可作为其他用处，其中 GPIO5 和 GPIO8 与外部按键相连。


供电说明
--------

USB 供电
^^^^^^^^

开发板有两种 USB 供电方式：

- 通过 ``Type-A`` 端口供电

.. figure:: ../../../_static/esp32-s3-usb-bridge/esp32-s3-usb-bridge-usb-typea.png
    :align: center
    :scale: 35%
    :alt: ESP32-S3-USB-Bridge -Type-A 电源供电

    ESP32-S3-USB-Bridge - Type-A 电源供电

- 通过 ``Type-C`` 端口供电

.. figure:: ../../../_static/esp32-s3-usb-bridge/esp32-s3-usb-bridge-usb-typec.png
    :align: center
    :scale: 35%
    :alt: ESP32-S3-USB-Bridge - Type-C 电源供电

    ESP32-S3-USB-Bridge - Type-C 电源供电


电压转换电路
^^^^^^^^^^^^^^

ESP32-S3-USB-Bridge 可以将 5 V 转化为 3.3 V 供模组使用。

.. figure:: ../../../_static/esp32-s3-usb-bridge/esp32-s3-usb-bridge-power.png
    :align: center
    :scale: 100%
    :alt: ESP32-S3-USB-Bridge - 电压转换

    ESP32-S3-USB-Bridge - 电压转换


硬件设置选项
------------

自动下载
^^^^^^^^

按下模组 Boot 按键后重新上电，随后松开模组 Boot 按键，即可使开发板进入下载模式。


硬件版本
================

无历史版本。


样品获取
================

此开发板已开源至 `立创开源硬件平台 <https://oshwhub.com/esp-college/esp32s3_usb_flash_tool>`_。如有需要，请自行打样。


相关文档
========

-  `ESP32-S3 技术规格书 <https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_cn.pdf>`__
-  `ESP32-S3-MINI-1 技术规格书 <https://www.espressif.com/sites/default/files/documentation/esp32-s3-mini-1_mini-1u_datasheet_cn.pdf>`__
-  `乐鑫产品选型工具 <https://products.espressif.com/#/product-selector?names=>`__
-  `ESP32-S3-USB-Bridge PCB 布局图 <../../_static/esp32-s3-usb-bridge/schematics/PCB_ESP32-S3-USB-Bridge-MB_V2.1_20230601.pdf>`__
-  `ESP32-S3-USB-Bridge 原理图 <../../_static/esp32-s3-usb-bridge/schematics/SCH_ESP32-S3-USB-Bridge-MB_V2.1_20230601.pdf>`__

有关本开发板的更多设计文档，请联系我们的商务部门 `sales@espressif.com <sales@espressif.com>`_。
