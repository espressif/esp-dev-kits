============================
ESP-Mosaico 多功能交互子板
============================

:link_to_translation:`en:[English]`

本指南介绍 ESP-Mosaico **多功能交互子板** （Interaction Subboard）的硬件接口、安装方式与软件使用要点。该子板通过左右 ``2 × 10P`` 模块插槽接入，提供环境光、人体感应、红外发射、RGB LED 与按键 / 触摸输入。

.. note::

  多功能交互子板可插入左侧（``H2``，EEPROM 0x50）或右侧（``H1``，EEPROM 0x51）插槽。右侧插槽相对左侧旋转 180° 安装。主板版本请参阅 :doc:`user_guide` 或 :doc:`user_guide_v1.0`。

子板集成 GL5506 光敏电阻、AS312 人体感应、经 AO3400A 驱动的红外发射管、6 颗 WS2812（顶部 4 颗、按键下方 2 颗），以及两路可配置为机械按键或电容触摸的焊盘。板载 AT24C02 用于插槽发现，板型为 0x16。

本指南包括如下内容：

- `入门指南`_：安装、供电与使用。
- `硬件参考`_：左右插槽管脚与外设说明。
- `硬件版本`_：硬件历史版本（如有）。
- `相关文档`_：相关文档链接。
- `免责声明和版权公告`_：免责声明和版权公告。

.. _Getting-started_esp-mosaico-interact:

入门指南
========

组件介绍
--------

下文先介绍正面视图及其主要组件，再介绍背面视图及其主要组件。

正面
^^^^

.. figure:: ../../_static/esp-mosaico/esp-mosaico-interact-front.png
   :alt: 多功能交互子板正面（点击放大）
   :width: 80%
   :figclass: align-center

   多功能交互子板正面（点击放大）

以下按图中标号，沿顺时针方向介绍正面主要组件。

.. list-table::
   :widths: 8 28 64
   :header-rows: 1

   * - 序号
     - 主要组件
     - 描述
   * - 1
     - Touch_R
     - 右侧触摸焊盘，可作电容触摸通道。
   * - 2
     - key_R
     - 右侧机械按键，低电平有效。
   * - 3
     - LDR
     - GL5506 光敏电阻，经 ADC 采集环境光。
   * - 4
     - PIR Sensor
     - AS312 人体感应 (PIR) 传感器，输出运动检测电平。
   * - 5
     - IR Emitter
     - 红外发射管，经 AO3400A 驱动，默认 NEC、38 kHz 载波。
   * - 6
     - Key_L
     - 左侧机械按键，低电平有效。
   * - 7
     - Touch_L
     - 左侧触摸焊盘，可作电容触摸通道。
   * - 8
     - WS2812 × 4
     - 顶部 LED0–LED3，共 4 颗可编程 RGB LED，单线控制。

背面
^^^^

.. figure:: ../../_static/esp-mosaico/esp-mosaico-interact-back.png
   :alt: 多功能交互子板背面（点击放大）
   :width: 80%
   :figclass: align-center

   多功能交互子板背面（点击放大）

以下按图中标号介绍背面主要组件。

.. list-table::
   :widths: 8 28 64
   :header-rows: 1

   * - 序号
     - 主要组件
     - 描述
   * - 1
     - 2 × 10-Pin Header（模块排针）
     - 2 × 10P、2.54 mm 间距，对接 BaseBoard 左右模块插槽。
   * - 2
     - WS2812
     - 图中右侧的可编程 RGB LED。
   * - 3
     - EEPROM
     - AT24C02 模块 EEPROM。左槽地址 0x50，右槽地址 0x51，板型 0x16。
   * - 4
     - WS2812
     - 图中左侧的可编程 RGB LED。

开始使用
--------

必备硬件
^^^^^^^^

- ESP-Mosaico（CoreBoard V1.0 或 V1.2）
- ESP-Mosaico 多功能交互子板
- USB 数据线（支持数据传输）
- 电脑（Windows、Linux 或 macOS）

安装说明
^^^^^^^^

.. figure:: ../../_static/esp-mosaico/esp-mosaico-interact-connection.png
   :alt: 多功能交互子板与主机的连接方向（点击放大）
   :width: 80%
   :figclass: align-center

   多功能交互子板与主机的连接方向（点击放大）

1. 确认主机已关机或已关闭对外 3.3 V / 5 V 输出后，将子板插入左侧或右侧模块插槽。插入右侧时请按 180° 方向安装。
2. 通过 Type-C 为 ESP-Mosaico 供电并开机。扩展 3.3 V / 5 V 仅在 GPIO60 置低时输出。
3. 开机后，主机通过板载 EEPROM 识别交互子板（板型 0x16）。未写入该信息的子板不会被识别。

使用说明
^^^^^^^^

识别成功后，屏幕会为每个已插入的插槽提供功能入口，可在环境光 (LDR)、人体感应 (PIR)、红外发射 (IR)、RGB 灯 (LED)、按键 (Key) 与触摸 (Touch) 之间切换。左右插槽可以同时使用，功能互不影响。

.. _Hardware-reference_esp-mosaico-interact:

硬件参考
========

插槽与发现
----------

ESP-Mosaico 通过 EEPROM 地址区分左右插槽：

.. list-table::
   :widths: 20 20 60
   :header-rows: 1

   * - 插槽
     - EEPROM 地址
     - 说明
   * - Left（``H2``）
     - 0x50
     - 地址选择 GPIO14，低电平
   * - Right（``H1``）
     - 0x51
     - 地址选择 GPIO39，高电平；相对左侧旋转 180° 安装

模块 I2C 使用扩展接口 SDA / SCL（GPIO0 / GPIO1）。CoreBoard **V1.2** 上该总线为对外 I2C1；**V1.0** 上与板载器件共享同一路 I2C。

左右插槽管脚
------------

下表给出多功能交互子板在左右插槽上的信号对应关系。

.. list-table::
   :widths: 22 20 20 38
   :header-rows: 1

   * - 信号
     - 左侧 GPIO
     - 右侧 GPIO
     - 说明
   * - LDR
     - GPIO53
     - GPIO46
     - GL5506 光敏电阻 ADC
   * - IR
     - GPIO48
     - GPIO47
     - 红外发射驱动（AO3400A）
   * - KEY_L / Touch
     - GPIO13
     - GPIO11
     - 左键或触摸通道
   * - KEY_R / Touch
     - GPIO12
     - GPIO10
     - 右键或触摸通道
   * - PIR
     - GPIO4
     - GPIO5
     - AS312 人体感应输入
   * - WS2812
     - GPIO15
     - GPIO38
     - 6 颗 RGB LED 数据线
   * - SDA
     - GPIO0
     - GPIO0
     - 模块 I2C / EEPROM
   * - SCL
     - GPIO1
     - GPIO1
     - 模块 I2C / EEPROM
   * - VCC_3V3
     - 插槽 Pin19
     - 插槽 Pin19
     - 3.3 V 供电（由 GPIO60 控制）
   * - 5V / GND
     - 插槽 Pin17 / 18 / 20
     - 插槽 Pin17 / 18 / 20
     - 5 V 输入输出与地

主机侧连接器定义见 :doc:`user_guide`。

.. figure:: ../../_static/esp-mosaico/esp-mosaico-expansion-interface-left.png
   :alt: 左侧模块接口电路图（点击放大）
   :scale: 45%
   :figclass: align-center

   左侧模块接口电路图（点击放大）

.. figure:: ../../_static/esp-mosaico/esp-mosaico-expansion-interface-right.png
   :alt: 右侧模块接口电路图（点击放大）
   :scale: 45%
   :figclass: align-center

   右侧模块接口电路图（点击放大）

外设说明
--------

光敏电阻 (LDR)
^^^^^^^^^^^^^^

GL5506 经 ADC 采样，读数以 0–100 表示环境明暗，可用于指示或调节 LED 亮度。

人体感应 (PIR)
^^^^^^^^^^^^^^

AS312 输出运动检测电平。检测到人体活动时，界面对应状态会点亮。

红外发射 (IR)
^^^^^^^^^^^^^

红外 LED 经 AO3400A 驱动，发送 NEC 红外码（8-bit 地址与命令），载波 38 kHz。

WS2812
^^^^^^

子板共 **6** 颗 WS2812，可单独或一起设置颜色，并限制亮度。

按键与触摸
^^^^^^^^^^

``KEY_L`` / ``KEY_R`` 共用同一对焊盘，可作按键或触摸：

- **按键**：机械按下，对地有效。
- **触摸**：手指触摸焊盘即可触发，按下按键同样会触发。

电源
----

子板由主机扩展接口的 ``VCC_3V3`` 供电，该电源仅在 GPIO60 置低时输出。CoreBoard V1.2 的对外 3.3 V 由独立 DCDC 产生，与内部 MCU 供电隔离。

硬件版本
========

无历史版本。

.. _Related-documents_esp-mosaico-interact:

相关文档
========

-  `ESP-Mosaico 多功能交互子板原理图`_ (PDF)

.. _ESP-Mosaico 多功能交互子板原理图: https://dl.espressif.com/AE/Mosaico/SCH_SCH_ESP-Mosaico_Interaction_V1_0_2026-09-24.pdf

免责声明和版权公告
==================

请参阅 :doc:`免责声明和版权公告 <../disclaimer-and-copyright>`。
