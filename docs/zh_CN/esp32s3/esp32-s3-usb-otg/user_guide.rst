ESP32-S3-USB-OTG
=======================

:link_to_translation:`en:[English]`

ESP32-S3-USB-OTG 是一款侧重于 USB-OTG 功能验证和应用开发的开发板，基于 ESP32-S3 SoC，支持 Wi-Fi 和 BLE 5.0 无线功能，支持 USB 主机和 USB 从机功能。可用于开发无线存储设备、Wi-Fi 网卡、LTE MiFi、多媒体设备、虚拟键鼠等应用。该开发板具有以下特性：

-  板载 ESP32-S3-MINI-1-N8 模组，内置 8 MB flash
-  板载 USB Type-A 主机和从机接口，内置接口切换电路
-  板载 USB 转串口调试芯片（micro USB 接口）
-  板载 1.3 英寸 LCD 彩屏，支持 GUI
-  板载 SD 卡接口，兼容 SDIO 和 SPI 接口
-  板载充电 IC，可外接锂电池

.. figure:: ../../../_static/esp32-s3-usb-otg/pic_product_esp32_s3_otg.png
   :alt: ESP32-S3-USB-OTG (click to enlarge)
   :scale: 40%
   :figclass: align-center

   ESP32-S3-USB-OTG（点击图片放大）

**本指南包括：**

-  :ref:`Getting-started`：提供 ESP32-S3-USB-OTG 的简要概述及必须了解的硬件和软件信息。
-  :ref:`Hardware-reference`：提供 ESP32-S3-USB-OTG 的详细硬件信息。
-  :ref:`Related-documents`：提供相关文档的链接。

.. _Getting-started:

快速入门
-----------

本节介绍如何开始使用 ESP32-S3-USB-OTG。首先，介绍一些关于 ESP32-S3-USB-OTG 的基本信息，然后在应用程序开发章节介绍如何开始使用该开发板进行开发，最后介绍该开发板包装和零售信息。

组件介绍
~~~~~~~~

ESP32-S3-USB-OTG 开发板包括以下两部分：

-  **主板**：ESP32-S3-USB-OTG 主板是整个套件的核心，该主板集成了 ESP32-S3-MINI-1 模组，并提供一个与 1.3 英寸 LCD 屏连接的接口。

.. figure:: ../../../_static/esp32-s3-usb-otg/pic_board_top_lable.png
   :alt: ESP32-S3-USB-OTG（点击图片放大）
   :scale: 70%
   :figclass: align-center

   ESP32-S3-USB-OTG 正面图（点击图片放大）

下表将从左边的 USB_HOST 接口开始，以逆时针顺序介绍上图中的主要组件。

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - 主要组件
     - 描述
   * - USB_HOST 接口
     - USB Type-A 母口，用来连接其它 USB 设备。
   * - ESP32-S3-MINI-1 模组
     - ESP32-S3-MINI-1 是通用型 Wi-Fi + 低功耗蓝牙 MCU模组，具有丰富的外设接口、强大的神经网络运算能力和信号处理能力，专为人工智能和 AIoT 市场打造。ESP32-S3-MINI-1 采用 PCB 板载天线，与 ESP32-S2-MINI-1 pin-to-pin 兼容。
   * - MENU 按键
     - 菜单按键。
   * - Micro SD 卡槽
     - 可插入 Micro SD 卡。支持 4-线 SDIO 和 SPI 模式。
   * - USB Switch IC
     - 通过设置 USB_SEL 的电平，可以切换 USB 外设连接到 USB_DEV 或 USB_HOST 接口，默认连接到 USB_DEV。
   * - Reset 按钮
     - 用于重启系统。
   * - USB_DEV 接口
     - USB Type-A 公口，可连接其它 USB 主机，也作为锂电池充电。
   * - 电池供电开关
     - 拨向 ON 按键一侧，使用电池供电；拨向 GND 按键一侧，通过其它方式供电。
   * - Boot 按键
     - 按键 Boot 键并保持，同时按一下 Reset 键，进入“固件下载”模式，通过串口下载固件。正常使用中可以作为确认按钮。
   * - DW- 按键
     - 向下按键。
   * - 屏幕排座
     - 用于连接 1.3 英寸 LCD 屏。
   * - UP+ 按键
     - 向上按键。
   * - USB 转 UART 接口
     - Micro-USB 接口，可用作开发板的供电接口，可烧录固件至芯片，也可作为通信接口，通过板载 USB 转 UART 桥接器与芯片通信。


.. figure:: ../../../_static/esp32-s3-usb-otg/pic_board_bottom_lable.png
   :alt: ESP32-S3-USB-OTG (click to enlarge)
   :scale: 70%
   :figclass: align-center

   ESP32-S3-USB-OTG 背面图（点击图片放大）

下表将从左边的 USB_HOST 接口开始，以逆时针顺序介绍上图中的主要组件。

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - 主要组件
     - 描述
   * - 黄色指示灯
     - 设置 GPIO16 为高电平，指示灯亮。
   * - 绿色指示灯
     - 设置 GPIO15 为高电平，指示灯亮。
   * - 充电指示灯
     - 当为电池充电时，亮起红灯，充电完成红灯熄灭。
   * - 电池焊点
     - 可焊接 3.6 V 锂电池，为主板供电。
   * - 充电电路
     - 用于为锂电池充电。
   * - 空闲管脚
     - 可自定义的空闲管脚。
   * - USB 转 UART 桥接器
     - 单芯片 USB 至 UART 桥接器，可提供高达 3 Mbps 的传输速率。


-  **子板**：ESP32-S3-USB-OTG-SUB - 贴装 1.3 英寸 LCD 屏

.. figure:: ../../../_static/esp32-s3-usb-otg/pic_sub.png
   :alt: ESP32-S3-USB-OTG
   :scale: 50%
   :figclass: align-center

   ESP32-S3-USB-OTG 子板（点击图片放大）

应用程序开发
~~~~~~~~~~~~

ESP32-S3-USB-OTG 上电前，请首先确认开发板完好无损。

硬件准备
^^^^^^^^

-  ESP32-S3-USB-OTG
-  一根 USB 2.0 数据线（标准 A 转 Micro-B）
-  电脑（Windows、Linux 或 macOS）

软件设置
^^^^^^^^

请前往 `快速入门 <https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32s3/get-started/index.html>`__ 中 `详细安装步骤 <https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32s3/get-started/index.html#get-started-step-by-step>`__ 一节查看如何快速设置开发环境。

工程选项
^^^^^^^^

我们为 ESP32-S3-USB-OTG 开发板提供了应用示例，存放在 :project:`Examples <esp32-s3-usb-otg/examples>` 文件夹中。

您可以在示例目录下，输入 ``idf.py menuconfig`` 配置工程选项。


内容和包装
~~~~~~~~~~

零售订单
^^^^^^^^

每一个零售 ESP32-S3-USB-OTG 开发套件均有独立包装。

.. figure:: ../../../_static/esp32-s3-usb-otg/pic_product_package.png
   :alt: ESP32-S3-USB-OTG
   :scale: 50%
   :figclass: align-center

   ESP32-S3-USB-OTG 包装 (click to enlarge)


包含以下部分：

- 主板：

  - ESP32-S3-USB-OTG

- 子板：

  - ESP32-S3-USB-OTG_SUB

- 紧固件

  - 安装螺栓（x4）
  - 螺丝（x4）
  - 螺母（x4）

零售订单请前往 https://www.espressif.com/zh-hans/company/contact/buy-a-sample。

批量订单
^^^^^^^^

如批量购买，开发板将以大纸板箱包装。

批量订单请前往 https://www.espressif.com/zh-hans/contact-us/sales-questions。

.. _Hardware-reference:

硬件参考
--------

功能框图
~~~~~~~~

ESP32-S3-USB-OTG 的主要组件和连接方式如下图所示。

.. figure:: ../../../_static/esp32-s3-usb-otg/sch_function_block.png
   :alt: ESP32-S3-USB-OTG
   :scale: 40%
   :figclass: align-center

   ESP32-S3-USB-OTG 功能框图 (click to enlarge)

请注意，功能框图中的 ``USB_HOST D+ D-`` 信号对应的外部接口是 ``USB DEV``，是指 ESP32-S3 作为设备接收其它 USB 主机的信号。``USB_DEV D+ D-`` 信号对应的外部接口是 ``USB HOST``，是指 ESP32-S3 作为主机控制其它设备。

开发板供电选择
~~~~~~~~~~~~~~

开发板有三种供电方式：

1. 通过 ``Micro_USB`` 接口供电

   -  使用方法 1 供电时，应使用一根 USB 电缆（标准 A 转 Micro-B）将主控板连接至供电设备，将电源开关置于 OFF。请注意，该供电模式仅有主板和显示屏被供电。

2. 通过 ``USB_DEV`` 接口供电

   -  使用方法 2 供电时，应将 ``DEV_VBUS_EN`` 设置为高电平，并将电源开关设置为 OFF。该供电模式可同时向 ``USB HOST`` 接口供电，如已安装锂电池，会同时对锂电池进行充电。

3. 通过电池接口，使用外部锂电池供电

   -  使用方法 3 供电时，应将 ``BOOST_EN`` 设置为高电平，将电源开关设置为 ON，并将 1S 锂电池 (3.7 V ~ 4.2 V) 焊接于主控板背面预留的电源焊点。该供电模式可同时向 ``USB HOST`` 接口供电。电池接口说明如下图：

.. figure:: ../../../_static/esp32-s3-usb-otg/pic_board_battery_lable.png
   :alt: ESP32-S3-USB-OTG
   :scale: 65%
   :figclass: align-center

   电池连接图 (click to enlarge)

USB HOST 接口供电选择
~~~~~~~~~~~~~~~~~~~~~

``USB HOST`` 接口（Type-A 母口）可向已连接的 USB 设备供电，供电电压为 5 V，最大电流为 500 mA。

-  供电电源有以下两个：

   1. 通过 ``USB_DEV`` 接口供电，5 V 电源直接来自该接口连接的电源
   2. 通过锂电池接口供电，锂电池 3.6 V ~ 4.2 V 电压通过 Boost 电路升压到 5 V。Boost IC 的工作状态可通过 BOOST_EN/GPIO13 控制，GPIO13 为高电平是使能 Boost。

.. figure:: ../../../_static/esp32-s3-usb-otg/sch_boost_circuit.png
   :alt: ESP32-S3-USB-OTG
   :scale: 50%
   :figclass: align-center

   Boost 电路图 (click to enlarge)

-  供电电源选择：

.. list-table::
   :widths: 30 35 35
   :header-rows: 1

   * - BOOST_EN
     - DEV_VBUS_EN
     - Power Source
   * - 0
     - 1
     - USB_DEV
   * - 1
     - 0
     - Battery
   * - 0
     - 0
     - No output
   * - 1
     - 1
     - Undefined

.. figure:: ../../../_static/esp32-s3-usb-otg/sch_power_switch.png
   :alt: ESP32-S3-USB-OTG
   :scale: 65%
   :figclass: align-center

   供电切换电路图

-  500 mA 限流电路

   1. 限流 IC MIC2005A 可将 ``USB HOST`` 接口最大输出电流限制为 500 mA。必须设置 ``IDEV_LIMIT_EN`` (GPIO17) 为高电平，使能限流 IC，``USB HOST`` 接口才有电压输出。

.. figure:: ../../../_static/esp32-s3-usb-otg/sch_500ma_limit.png
   :alt: ESP32-S3-USB-OTG
   :scale: 40%
   :figclass: align-center

   500 mA 限流电路图

USB 接口选择电路
~~~~~~~~~~~~~~~~

.. figure:: ../../../_static/esp32-s3-usb-otg/sch_usb_switch.png
   :alt: ESP32-S3-USB-OTG
   :scale: 45%
   :figclass: align-center

   USB 接口切换电路图

-  当 **USB_SEL** (GPIO18) 引脚为高电平时，USB D+/D- (GPIO19, 20) 信号将连通到 ``USB_DEV D+ D-``，这时可通过 ``USB HOST`` 接口（Type-A 母口）连接其它 USB 设备；
-  当 **USB_SEL** (GPIO18) 引脚为低电平时，USB D+/D- (GPIO19, 20) 信号将连通到 ``USB_HOST D+ D-``，这时可通过 ``USB DEV`` 接口（Type-A 公口）连接到其它 USB 主机；
-  默认 **USB_SEL** 为低电平。

LCD 接口
~~~~~~~~

.. figure:: ../../../_static/esp32-s3-usb-otg/sch_interface_lcd.png
   :alt: ESP32-S3-USB-OTG
   :scale: 50%
   :figclass: align-center

   LCD 接口电路图

请注意，该接口支持连接 SPI 接口屏幕，该开发板使用的屏幕型号为 :project:` ST7789 <esp32-s3-usb-otg/datasheet/ST7789VW_datasheet.pdf>`，``LCD_BL`` (GPIO9) 可用于控制屏幕背光。

SD 卡接口
~~~~~~~~~

.. figure:: ../../../_static/esp32-s3-usb-otg/sch_micro_sd_slot.png
   :alt: ESP32-S3-USB-OTG
   :scale: 45%
   :figclass: align-center

   SD 卡接口电路图

请注意，SD 卡接口同时兼容 1-线，4-线 SDIO 模式和 SPI 模式。上电后，卡会处于 3.3 V signaling 模式下。发送第一个 CMD0 命令选择 bus 模式：SD 模式或者 SPI 模式。

充电电路
~~~~~~~~

.. figure:: ../../../_static/esp32-s3-usb-otg/sch_charge_circuit.png
   :alt: ESP32-S3-USB-OTG
   :scale: 40%
   :figclass: align-center

   充电电路图

请注意，可将 Type-A 公口接入输出为 5 V 的电源适配器，为电池充电时，充电电路上的红色指示灯亮，电池充电完成，红色指示灯熄灭。在使用充电电路时，请将电源开关置于 OFF。充电电流为 212.7 mA。

GPIO 分配
~~~~~~~~~

**功能引脚：**

.. list-table::
   :widths: 10 20 70
   :header-rows: 1

   * - No.
     - ESP32-S3-MINI-1 管脚
     - 说明
   * - 1
     - GPIO18
     - USB_SEL：用于切换 USB 接口，高电平时，USB_HOST 接口使能。低电平时（默认），USB_DEV 接口使能。
   * - 2
     - GPIO19
     - 与 USB_D- 接口相连。
   * - 3
     - GPIO20
     - 与 USB_D+ 接口相连。
   * - 4
     - GPIO15
     - LED_GREEN：用于控制绿色 LED 灯，高电平时，灯被点亮。
   * - 5
     - GPIO16
     - LED_YELLOW：用于控制黄色 LED 灯，高电平时，灯被点亮。
   * - 6
     - GPIO0
     - BUTTON_OK：OK 按键，按下为低电平。
   * - 7
     - GPIO11
     - BUTTON_DW：Down 按键，按下为低电平。
   * - 8
     - GPIO10
     - BUTTON_UP：UP 按键，按下为低电平。
   * - 9
     - GPIO14
     - BUTTON_MENU：MENU 按键，按下为低电平。
   * - 10
     - GPIO8
     - LCD_RET：用于复位 LCD，低电平时复位。
   * - 11
     - GPIO5
     - LCD_EN：用于使能 LCD，低电平时使能。
   * - 12
     - GPIO4
     - LCD_DC：用于切换数据和命令状态。
   * - 13
     - GPIO6
     - LCD_SCLK：LCD SPI 时钟信号。
   * - 14
     - GPIO7
     - LCD_SDA：LCD SPI MOSI 信号。
   * - 15
     - GPIO9
     - LCD_BL：LCD 背光控制信号。
   * - 16
     - GPIO36
     - SD_SCK：SD SPI CLK / SDIO CLK。
   * - 17
     - GPIO37
     - SD_DO：SD SPI MISO / SDIO  Data0。
   * - 18
     - GPIO38
     - SD_D1：SDIO Data1。
   * - 19
     - GPIO33
     - SD_D2：SDIO Data2。
   * - 20
     - GPIO34
     - SD_D3：SD SPI CS / SDIO Data3。
   * - 21
     - GPIO1
     - HOST_VOL：USB_DEV 电压监测，ADC1 通道 0。
   * - 22
     - GPIO2
     - BAT_VOL：电池电压监测，ADC1 通道 1。
   * - 23
     - GPIO17
     - LIMIT_EN：使能限流芯片，高电平使能。
   * - 24
     - GPIO21
     - 0VER_CURRENT：电流超限信号，高电平代表超限。
   * - 25
     - GPIO12
     - DEV_VBUS_EN：高电平选择 DEV_VBUS 电源。
   * - 26
     - GPIO13
     - BOOST_EN：高电平使能 Boost 升压电路。


**扩展功能引脚：**

.. list-table::
   :widths: 10 30 60
   :header-rows: 1

   * - No.
     - ESP32-S3-MINI-1 管脚
     - 说明
   * - 1
     - GPIO45
     - FREE_1：空闲，可自定义。
   * - 2
     - GPIO46
     - FREE_2：空闲，可自定义。
   * - 3
     - GPIO48
     - FREE_3：空闲，可自定义。
   * - 4
     - GPIO26
     - FREE_4：空闲，可自定义。
   * - 5
     - GPIO47
     - FREE_5：空闲，可自定义。
   * - 6
     - GPIO3
     - FREE_6：空闲，可自定义。


.. _Related-documents:

相关文档
------------

-  `ESP32-S3 技术规格书 <https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_cn.pdf>`__ (PDF)
-  `ESP32-S3-MINI-1/1U 技术规格书 <https://www.espressif.com/sites/default/files/documentation/esp32-s3-mini-1_mini-1u_datasheet_cn.pdf>`__ (PDF)
-  `乐鑫产品选型工具 <https://products.espressif.com/#/product-selector?names=>`__
-  `ESP32-S3-USB-OTG 原理图 <../../_static/esp32-s3-usb-otg/schematics/SCH_ESP32-S3_USB_OTG.pdf>`__ (PDF)
-  `ESP32-S3-USB-OTG PCB 布局图 <../../_static/esp32-s3-usb-otg/schematics/PCB_ESP32-S3_USB_OTG.pdf>`__ (PDF)
-  `ST7789VW 规格书 <../../_static/esp32-s3-usb-otg/datasheet/ST7789VW_datasheet.pdf>`_ (PDF)
