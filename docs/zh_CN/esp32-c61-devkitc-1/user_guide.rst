===========================
ESP32-C61-DevKitC-1 v2.0
===========================

:link_to_translation:`en: [English]`

更早版本：:doc:`user_guide_v1.0`

本指南将帮助您快速上手 ESP32-C61-DevKitC-1，并提供该款开发板的详细信息。

ESP32-C61-DevKitC-1 是一款入门级开发板，使用通用型模组 `ESP32-C61-WROOM-1 <https://www.espressif.com/sites/default/files/documentation/esp32-c61-wroom-1_wroom-1u_datasheet_cn.pdf>`_。该开发板搭载的模组配置了高达 8 MB SPI flash 和 2 MB PSRAM。该款开发板具备完整的 Wi-Fi 和低功耗蓝牙功能。

板上模组大部分管脚均已引出至两侧排针，开发人员可根据实际需求，轻松通过跳线连接多种外围设备，同时也可将开发板插在面包板上使用。

.. figure:: ../../_static/esp32-c61-devkitc-1/esp32-c61-devkitc-1-isometric.png
    :align: center
    :scale: 20%
    :alt: ESP32-C61-DevKitC-1（板载 ESP32-C61-WROOM-1 模组）
    :figclass: align-center

    ESP32-C61-DevKitC-1（点击放大）

本指南包括如下内容：

- `入门指南`_：简要介绍了开发板和硬件、软件设置指南。
- `硬件参考`_：详细介绍了开发板的硬件。
- `硬件版本`_：介绍硬件历史版本和已知问题（如有）。
- `相关文档`_：列出了相关文档的链接。
- `免责声明和版权公告`_: 链接到免责声明和版权公告。

入门指南
========

本小节将简要介绍 ESP32-C61-DevKitC-1，说明如何在 ESP32-C61-DevKitC-1 上烧录固件及相关准备工作。

组件介绍
--------

.. _user-guide-C61-devkitc-1-v2-board-front:

.. figure:: ../../_static/esp32-c61-devkitc-1/esp32-c61-devkitc-1-v1-annotated-photo.png
    :align: center
    :alt: ESP32-C61-DevKitC-1 - 正面
    :figclass: align-center

    ESP32-C61-DevKitC-1 - 正面

以下按照顺时针的顺序依次介绍开发板上的主要组件。

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - 主要组件
     - 介绍
   * - ESP32-C61-WROOM-1
     - ESP32-C61-WROOM-1 是一款通用型模组，支持 2.4 GHz Wi-Fi 6 和 蓝牙 5。该款模组内置 ESP32-C61HR2 芯片，采用 PCB 板载天线，配置高达 8 MB SPI flash 和 2 MB PSRAM。
   * - Pin Header（排针）
     - 所有可用 GPIO 管脚（除 flash 和 PSRAM 的 SPI 总线）均已引出至开发板的排针。
   * - 5 V to 3.3 V LDO（5 V 转 3.3 V LDO）
     - 电源转换器，输入 5 V，输出 3.3 V。
   * - 3.3 V Power On LED（3.3 V 电源指示灯）
     - 开发板连接 USB 电源后，该指示灯亮起。
   * - USB-to-UART Bridge（USB 转 UART 桥接器）
     - 单芯片 USB 转 UART 桥接器，可提供高达 3 Mbps 的传输速率。
   * - ESP32-C61 USB Type-C Port（ESP32-C61 USB Type-C 接口）
     - ESP32-C61 芯片的 USB Type-C 接口，支持 USB 2.0 全速模式，数据传输速率最高为 12 Mbps（注意，该接口不支持 480 Mbps 的高速传输模式）。该接口可用作开发板的供电接口，可烧录固件至芯片，可通过 USB 协议与芯片通信，也可用于 JTAG 调试。
   * - Boot Button（Boot 键）
     - 下载按键。按住 **Boot** 键的同时按一下 **Reset** 键进入“固件下载”模式，通过串口下载固件。
   * - Reset Button（Reset 键）
     - 复位按键。
   * - USB Type-C to UART Port（USB Type-C 转 UART 接口）
     - 可用作开发板的供电接口，可烧录固件至芯片，也可作为通信接口，通过板载 USB 转 UART 桥接器与 ESP32-C61 芯片通信。
   * - RGB LED
     - 可寻址 RGB 发光二极管，由 GPIO8 驱动。
   * - J5
     - 用于测量电流。详见章节 :ref:`user-guide-C61-devkitc-1-v2-current`。


开始开发应用
------------

通电前，请确保开发板完好无损。

必备硬件
^^^^^^^^

- ESP32-C61-DevKitC-1
- USB-A 转 USB-C 数据线
- 电脑（Windows、Linux 或 macOS）

.. 注解::

  请确保使用优质 USB 数据线。部分数据线仅可用于充电，无法用于数据传输和编程。

硬件设置
^^^^^^^^

请使用 **USB Type-C 转 UART 接口** 将开发板连接至电脑。**ESP32-C61 USB Type-C 接口** 的软件支持尚未完全实现。在后续步骤中，默认使用 **USB Type-C 转 UART 接口**。


软件设置
^^^^^^^^

请前往 `ESP-IDF 快速入门 <https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html>`__ 小节查看如何快速设置开发环境，将应用程序烧录至您的开发板。

.. 注解::

  开发板使用 USB 端口与电脑通信。大多数操作系统（Windows、Linux、macOS）已预装所需驱动，开发板插入后可自动识别。如无法识别设备或无法建立串口连接，请参考 `如何建立串口连接 <https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32c61/get-started/establish-serial-connection.html>`__ 获取安装驱动的详细步骤。

乐鑫为多种开发板提供了板级支持包 (BSP)，可帮助您更轻松、高效地初始化和使用板载的主要外设，如 LCD 显示屏、音频芯片、按键和 LED 等。请访问 `esp-bsp <https://github.com/espressif/esp-bsp>`__ 查询支持的所有开发板。

.. 其他开发框架选项
.. ^^^^^^^^^^^^^^^^^^^^

.. .. ----------------------------------------------------------------------------
.. .. - Semi-fixed content, depending on whether the chip series supports the frameworks
.. .. ----------------------------------------------------------------------------

.. 除了 ESP-IDF 开发框架外，本开发板还支持以下其他开发框架，为不同用户需求和应用场景提供了更多灵活选择：

.. `ESP-AT <https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/index.html>`__：通过 UART 使用 AT 指令控制开发板，无需编写嵌入式代码。

.. `Arduino-ESP32 <https://docs.espressif.com/projects/arduino-esp32/en/latest/#>`__：基于 ESP-IDF 的 Arduino 内核，提供简化的 API 并兼容 Arduino 生态。

.. `ESP RainMaker <https://docs.rainmaker.espressif.com/zh-Hans/docs/product_overview/technical_overview/introduction/>`__：ESP RainMaker 是高度可定制的物联网平台，提供设备固件、手机应用、云端后台、语音助手集成和设备管理控制台。

内含组件和包装
--------------

.. include:: ordering-info.inc

如需了解更多可选型号，请访问 `乐鑫产品选型工具 <https://products.espressif.com/zh/product-selector>`__，选择 **开发板** 标签页。如果在本表或乐鑫产品选型工具中未找到所需型号，或有特殊定制需求，请联系 `乐鑫商务 <https://www.espressif.com/zh-hans/contact-us/sales-questions>`__。

零售订单
^^^^^^^^

如购买样品，每个开发板将以防静电袋或零售商选择的其他方式包装。

零售订单请前往 https://www.espressif.com/zh-hans/company/contact/buy-a-sample。


批量订单
^^^^^^^^

如批量购买，开发板将以大纸板箱包装。

批量订单请前往 https://www.espressif.com/zh-hans/contact-us/sales-questions。

硬件参考
========

功能框图
--------

ESP32-C61-DevKitC-1 的主要组件和连接方式如下图所示。

.. figure:: ../../_static/esp32-c61-devkitc-1/esp32-c61-devkitc-1-v1-block-diagram.png
    :align: center
    :scale: 60%
    :alt: ESP32-C61-DevKitC-1
    :figclass: align-center

    ESP32-C61-DevKitC-1（点击放大）

电源选项
--------

您可从以下三种供电方式中任选其一给 ESP32-C61-DevKitC-1 供电：

- USB Type-C 转 UART 接口或 ESP32-C61 USB Type-C 接口供电（选择其一或同时供电），默认供电方式（推荐）
- 5V 和 GND 排针供电
- 3V3 和 GND 排针供电

.. 注解::

  该开发板工作电压为 5 V，所需最小电流为 0.5 A。如果应用所需电流超过 0.5 A，建议通过 USB 集线器给开发板供电，以确保其稳定运行。


.. _user-guide-C61-devkitc-1-v2-current:

测量电流
--------

开发板上的 J5 排针（见图 :ref:`user-guide-C61-devkitc-1-v2-board-front` 中的 J5）可用于测量 ESP32-C61-WROOM-1 模组的电流：

- 移除 J5 跳帽：此时开发板上外设和模组电源断开，J5 排针接入电流表后可测量模组电流。
- 安装 J5 跳帽（出厂时默认）：开发板恢复正常功能。

.. 注解::

  使用 3V3 和 GND 排针给开发板供电时，需移除 J5 跳帽，在外部电路上串联接入电流表，才可测量模组的电流。

排针
----

下表列出了开发板两侧排针（J1 和 J3）的 **名称** 和 **功能**，排针的名称如图 :ref:`user-guide-C61-devkitc-1-v2-board-front` 所示，排针的序号与 `ESP32-C61-DevKitC-1 原理图`_ (PDF) 一致。

J1
^^^
====  =======  ==========  =================================================
序号  名称     类型 [1]_    功能
====  =======  ==========  =================================================
1     3V3       P          3.3 V 电源
2     RST       I          高电平：芯片使能；低电平：芯片关闭。
3     4         I/O/T      MTDI, GPIO4, LP_GPIO4, ADC1_CH2, FSPIWP
4     5         I/O/T      MTCK, GPIO5, LP_GPIO5, ADC1_CH3
5     6         I/O/T      MTDO, GPIO6, LP_GPIO6, FSPICLK
6     7         I/O/T      GPIO7 [3]_, FSPID
7     0         I/O/T      GPIO0, XTAL_32K_P, LP_GPIO0
8     1         I/O/T      GPIO1, XTAL_32K_N, LP_GPIO1, ADC1_CH0
9     8         I/O/T      GPIO8 [2]_ [3]_, ZCD0, FSPICS0
10    29        I/O/T      GPIO29
11    NC/14     I/O/T      空管脚/GPIO14 [4]_
12    2         I/O/T      GPIO2, LP_GPIO2, FSPIQ
13    3         I/O/T      MTMS, GPIO3, LP_GPIO3, ADC1_CH1, FSPIHD
14    5V        P          5 V 电源
15    G         G          接地
16    NC        --         空管脚
====  =======  ==========  =================================================


J3
^^^
====  ==========  ======  ==========================================
序号  名称         类型     功能
====  ==========  ======  ==========================================
1      G          G       接地
2      TX         I/O/T   U0TXD, GPIO11
3      RX         I/O/T   U0RXD, GPIO10
4      24         I/O/T   GPIO24
5      23         I/O/T   GPIO23, SDIO_DATA3
6      22         I/O/T   GPIO22, SDIO_DATA2
7      28         I/O/T   GPIO28, SDIO_DATA1
8      27         I/O/T   GPIO27, SDIO_DATA0
9      26         I/O/T   GPIO26, SDIO_CLK
10     25         I/O/T   GPIO25, SDIO_CMD
11     9          I/O/T   GPIO9 [3]_, ZCD1
12     G          G       接地
13     13         I/O/T   GPIO13, USB_D+
14     12         I/O/T   GPIO12, USB_D-
15     G          G       接地
16     NC         --      空管脚
====  ==========  ======  ==========================================

.. [1] P：电源；I：输入；O：输出；T：可设置为高阻。
.. [2] 用于驱动 RGB LED。
.. [3] GPIO7、GPIO8 和 GPIO9为 ESP32-C61 芯片的 Strapping 管脚。在芯片上电和系统复位过程中，Strapping 管脚根据管脚的二进制电压值控制芯片功能。Strapping 管脚的具体描述和应用，请参考 `ESP32-C61 技术规格书`_ > 启动配置项章节。
.. [4] 在集成封装内 SPI PSRAM 的模组中，此管脚已用作 SPI PSRAM 的 SPICS1，不可再用于其他功能; 在未集成封装内 SPI PSRAM 的模组中，此管脚可用作 GPIO14。


管脚布局
^^^^^^^^
.. figure:: ../../_static/esp32-c61-devkitc-1/esp32-c61-devkitc-1-pin-layout-v2.png
    :align: center
    :scale: 40%
    :alt: ESP32-C61-DevKitC-1 管脚布局
    :figclass: align-center

    ESP32-C61-DevKitC-1 管脚布局（点击放大）

硬件版本
==========

改版历史
--------

- ESP32-C61-DevKitC-1 v2.0：生产工单号为 PW-2025-05-0781 及之后的开发板，J1 和 J3 排针功能更新，详见章节 `排针`_。

- :doc:`ESP32-C61-DevKitC-1 v1.0 <user_guide_v1.0>` 为首次发布。

.. 注解::

  生产工单号可在批量订单大纸板箱包装的物料标签中找到。


相关文档
========

.. only:: latex

   请前往 `esp-dev-kits 文档 HTML 网页版本 <https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/{IDF_TARGET_PATH_NAME}/index.html>`_ 下载以下文档。

* `ESP32-C61 技术规格书`_ (PDF)
* `ESP32-C61-WROOM-1 技术规格书`_ (PDF)

* `ESP32-C61-DevKitC-1 原理图`_ (PDF)
* `ESP32-C61-DevKitC-1 PCB 布局图`_ (PDF)
* `ESP32-C61-DevKitC-1 尺寸图`_ (PDF)
* `ESP32-C61-DevKitC-1 尺寸图源文件`_ (DXF) - 可使用 `Autodesk Viewer <https://viewer.autodesk.com/>`_ 查看

有关本开发板的更多设计文档，请联系我们的商务部门 `sales@espressif.com <sales@espressif.com>`_。

.. _ESP32-C61 技术规格书: https://www.espressif.com/sites/default/files/documentation/esp32-c61_datasheet_cn.pdf
.. _ESP32-C61-WROOM-1 技术规格书: https://www.espressif.com/sites/default/files/documentation/esp32-c61-wroom-1_wroom-1u_datasheet_cn.pdf
.. _ESP32-C61-DevKitC-1 原理图: https://dl.espressif.com/dl/schematics/esp32-c61-devkitc-1-schematics_v2.0.pdf
.. _ESP32-C61-DevKitC-1 PCB 布局图: https://dl.espressif.com/dl/schematics/esp32-c61-devkitc-1-pcb-layout_v2.0.pdf
.. _ESP32-C61-DevKitC-1 尺寸图: https://dl.espressif.com/dl/schematics/esp32-c61-devkitc-1-dimensions_v2.0.pdf
.. _ESP32-C61-DevKitC-1 尺寸图源文件: https://dl.espressif.com/dl/schematics/ESP32-C61-DevKitC-1-dimensions_v2.0.dxf

.. toctree::
    :hidden:

    user_guide_v1.0

免责声明和版权公告
==================

请参阅 :doc:`免责声明和版权公告 <../disclaimer-and-copyright>`。

