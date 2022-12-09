========
ESP-Prog
========

:link_to_translation:`en:[English]`


本指南将帮助您快速上手 ESP-Prog，并提供该款开发板的详细信息。

ESP-Prog 是一款乐鑫推出的开发调试工具，具有自动下载固件、串口通信、JTAG 在线调试等功能。自动下载固件和串口通信功能适用于 ESP8266、ESP32、ESP32-S2、ESP32-S3 和 ESP32-C3；JTAG 在线调试功能适用于 ESP32、ESP32-S2、ESP32-S3 和 ESP32-C3。

ESP-Prog 使用简单方便，仅用一根 USB 线即可实现与电脑的连接。电脑端可以通过端口号识别出下载功能和 JTAG 功能对应的两个端口。

考虑到不同用户板的电源电压可能不同，ESP-Prog 的两个接口均可通过排针选择 5 V 或者 3.3 V 供电，具备较强的电源兼容性。不过，尽管 ESP-Prog 的电源可以在 3.3 V 和 5 V 之间切换，但 RX/TX 和 JTAG 信号将恒定在 3.3 V。

.. figure:: ../../../_static/esp-prog/three_dimension.png
    :align: center
    :scale: 70%
    :alt: ESP-Prog（点击放大）

    ESP-Prog（点击放大）

本指南包括如下内容：

- `入门指南`_：简要介绍了开发板和硬件、软件设置指南。
- `硬件参考`_：详细介绍了开发板的硬件。
- `相关文档`_：列出了相关文档的链接。


入门指南
========

本小节将简要介绍 ESP-Prog，说明如何在 ESP-Prog 进行初始硬件设置。


组件介绍
--------

.. figure:: ../../../_static/esp-prog/modules.png
    :align: center
    :scale: 70%
    :alt: ESP-Prog - 正面（点击放大）

    ESP-Prog - 正面（点击放大）

以下按照顺时针的顺序依次介绍开发板上的主要组件。

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - 主要组件
     - 介绍
   * - Micro USB
     - 电脑端与 ESP-Prog 的接口。
   * - Boot 按键
     - 下载按键。按下 Boot 键并保持，同时按一下 Reset 键，进入“固件下载”模式，通过串口下载固件。正常使用中可以作为确认按钮。
   * - IO0 On/Off
     - 用于配置 GPIO0 Strapping 管脚状态的排针。
   * - PROG PWR SEL
     - 用于选择 Program 接口电源输入的排针。
   * - PROG 2.54 mm
     - 管脚间距为 2.54 mm (0.1") 的 Program 接口。
   * - PROG 1.27 mm
     - 管脚间距为 1.27 mm (0.05") 的 Program 接口。
   * - JTAG 1.27 mm
     - 管脚间距为 1.27 mm (0.05") 的 JTAG 接口。
   * - JTAG 2.54 mm
     - 管脚间距为 2.54 mm (0.1") 的 JTAG 接口。
   * - JTAG PWR SEL
     - 用于选择 JTAG 接口电源输入的排针。
   * - LED 指示灯
     - 显示 ESP-Prog 的状态。共有三种 LED 模式：红色、绿色和蓝色。当系统的 3.3 V 电源通电时，红色 LED 灯亮起；当下载板发送数据时，绿色 LED 灯亮起；当下载板接收数据时，蓝色 LED 灯亮起。
   * - USB 桥接器
     - 单芯片 USB 至 UART 桥接器，可提供高达 3 Mbps 的传输速率。
   * - Reset 按钮
     - 用于重启系统。


开始开发应用
-------------

通电前，请确保开发板完好无损。


必备硬件
^^^^^^^^

- ESP-Prog
- 一根 USB 2.0 数据线（标准 A 转 Micro-B）
- 装有 Windows、macOS 或 Linux 系统的电脑
- 杜邦线或是乐鑫提供的排线，用于连接开发板和 ESP-Prog

.. 注解::

  请确保使用适当的 USB 数据线。部分数据线仅可用于充电，无法用于数据传输和编程。


硬件设置
^^^^^^^^^^^^^^

1. 通过 USB 线连接 ESP-Prog 调试板和电脑端的 USB 口。
2. 在电脑端安装 `FT2232HL 芯片驱动 <http://www.ftdichip.com/Drivers/VCP.htm>`_。若电脑端识别到两个端口，则表明驱动已安装成功。
3. 使用 PROG PWR SEL/JTAG PWR SEL 排针选择 Program/JTAG 接口上的电源输出电压。
4. 使用乐鑫提供的排线连接 ESP-Prog 调试板和 ESP 产品板。
5. 使用乐鑫官方软件工具或脚本，实现自动下载和 JTAG 调试功能。


软件设置
^^^^^^^^^^^^^^

请前往 `ESP-IDF 快速入门 <https://idf.espressif.com/zh-cn/index.html>`__ 中 `详细安装步骤 <https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html#get-started-step-by-step>`__ 一节查看如何快速设置开发环境。


内容和包装
----------------------

零售订单
^^^^^^^^^^^^^

每一个零售 ESP-Prog 开发板均有独立包装。

.. figure:: ../../../_static/esp-prog/package.png
   :align: center
   :scale: 120%
   :alt: ESP-Prog 包装内容

   ESP-Prog 包装内容

包含以下部分：

- 开发板

  ESP-Prog

- 数据线

  两根排线：
    - 一根用于 PROG 2.54 mm 接口，连接 2*5-PIN 的牛角座。
    - 一根用于 PROG 1.27 mm 接口，连接 2*3-PIN 的牛角座。

如果您订购了一批样品，根据零售商的不同，每块板子的独立包装会有所差异。

零售订单请前往 https://www.espressif.com/zh-hans/company/contact/buy-a-sample。


批量订单
^^^^^^^^^^^^^^^^

If you order in bulk, the boards come in large cardboard boxes.

批量订单请前往 https://www.espressif.com/zh-hans/contact-us/sales-questions。


硬件参考
==================

功能框图
-------------

ESP-Prog 的主要组件和连接方式如下图所示。

.. figure:: ../../../_static/esp-prog/block.png
    :align: center
    :scale: 80%
    :alt: ESP-Prog 功能框图（点击放大）

    ESP-Prog 功能框图（点击放大）


电源选项
^^^^^^^^^^^^^^^^^^^^^^^

开发板有三种互不兼容的供电方式：

- 默认使用 ESP-Prog USB 端口供电（推荐供电方式）
- 5 V 和 G (GND) 管脚供电
- 3.3 V 和 G (GND) 管脚供电


排针
-------------

下列的两个表格提供了开发板两侧管脚（Program 接口和 JTAG 接口）的 **名称** 和 **功能**。管脚名称如 ESP-Prog 正面图所示，管脚编号与 `ESP-Prog 原理图 <../../_static/esp-prog/schematics/SCH_ESP32-PROG_V2.1_20190709.pdf>`_ (PDF) 中一致。


Program 接口
^^^^^^^^^^^^^^^^^^

.. list-table::
   :widths: 30 35 35
   :header-rows: 1

   * - 编号
     - 名称
     - 功能
   * - 1
     - ESP_EN
     - 使能信号
   * - 2
     - VDD
     - 供电
   * - 3
     - ESP_TXD
     - TX 管脚
   * - 4
     - GND
     - 地线
   * - 5
     - ESP_RXD
     - RX 管脚
   * - 6
     - ESP_IO0
     - Strapping 管脚


JTAG 接口
^^^^^^^^^^^^^^^

.. list-table::
   :widths: 30 30 40
   :header-rows: 1

   * - 编号
     - 名称
     - 功能
   * - 1
     - VDD
     - 供电
   * - 2
     - ESP_TMS
     - JTAG TMS 管脚，选择模式
   * - 3
     - GND
     - 地线
   * - 4
     - ESP_TCK
     - JTAG TCK 管脚，时钟输入
   * - 5
     - GND
     - 地线
   * - 6
     - ESP_TDO
     - JTAG TDO 管脚
   * - 7
     - GND
     - 地线
   * - 8
     - ESP_TDI
     - JTAG TDI 管脚
   * - 9
     - GND
     - 地线
   * - 10
     - NC
     - 无


相关文档
=================

- `ESP-Prog 原理图 <../../_static/esp-prog/schematics/SCH_ESP32-PROG_V2.1_20190709.pdf>`_ (PDF)
- `ESP-Prog PCB 布局图 <../../_static/esp-prog/schematics/PCB_ESP32-PROG_V2.1_20190709.pdf>`_ (PDF)
- `ESP-Prog 尺寸图 <../../_static/esp-prog/schematics/DIM_ESP32-PROG_V2.1_20190709.pdf>`_ (PDF)


有关本开发板的更多设计文档，请联系我们的商务部门 `sales@espressif.com <sales@espressif.com>`_。

