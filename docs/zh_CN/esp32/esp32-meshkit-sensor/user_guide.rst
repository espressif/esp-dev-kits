ESP32-MeshKit-Sense 
====================

:link_to_translation:`en:[English]`

产品概述
----------

ESP32-MeshKit-Sense 是一款以乐鑫 ESP32 模组为核心的开发板，集成了温湿度传感器、环境亮度传感器等外设，并且可外接屏幕，主要用于检测模组在正常工作或睡眠状态下，连接各个外设时的电流情况。

关于 ESP32 详细信息，请参考文档 `《ESP32 技术规格书》 <https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_cn.pdf>`__。

.. figure:: ../../../_static/esp32-meshkit-sensor/ESP32_BATTERY_EB.png
   :align: center
   :alt: ESP32-MeshKit-Sense
   :figclass: align-center
 
   ESP32-MeshKit-Sense

电路设计说明
-------------

系统框图
++++++++

ESP32 的系统框图如下图所示。

.. figure:: ../../../_static/esp32-meshkit-sensor/blockdiagram.png
   :align: center
   :alt: ESP32-MeshKit-Sense
   :figclass: align-center
 
   ESP32 系统框图


PCB 布局
++++++++++

PCB 布局如下图所示。

.. figure:: ../../../_static/esp32-meshkit-sensor/EBPCB.png
   :align: center
   :alt: ESP32-MeshKit-Sense
   :figclass: align-center
 
   PCB 布局


PCB 部件功能说明如下表所示：

+-----------------+------------------------------------------------------------------------------+
| PCB 部件        | 说明                                                                         |
+=================+==============================================================================+
| EXT5V           | USB 输入的 5 V                                                               |
+-----------------+------------------------------------------------------------------------------+
| CH5V            | 充电管理芯片的输入                                                           |
+-----------------+------------------------------------------------------------------------------+
| CHVBA           | 充电管理芯片的输出                                                           |
+-----------------+------------------------------------------------------------------------------+
| VBA             | 接至电源的正极                                                               |
+-----------------+------------------------------------------------------------------------------+
| SUFVCC          | 当开关处于“ON”档位时，与电源输入接通，当开关处于“OFF”档位时，与电源输入断开  |
+-----------------+------------------------------------------------------------------------------+
| DCVCC           | 电源管理芯片 DC-DC 的输入                                                    |
+-----------------+------------------------------------------------------------------------------+
| 3.3V            | 电源管理芯片的输出, 即总路 3.3 V                                             |
+-----------------+------------------------------------------------------------------------------+
| 3.3V\_PER       | 为所有外设供电的 3.3V\_Perip                                                 |
+-----------------+------------------------------------------------------------------------------+
| 3.3V\_ESP       | 为 ESP32 模组模块供电的 3.3V_ESP                                             |
+-----------------+------------------------------------------------------------------------------+
| 3.3V\_SEN       | 为三款传感器供电的 3.3V-Perip_Sensor                                         |
+-----------------+------------------------------------------------------------------------------+
| 3.3V\_SCR       | 为外接屏幕供电的 3.3V-Perip_Screen                                           |
+-----------------+------------------------------------------------------------------------------+
| Charge          | 电池充电指示灯，D5 为红灯，表示正在充电；D6 为绿灯，表示充电完成             |
+-----------------+------------------------------------------------------------------------------+
| Sensor          | 电源指示灯，表示 3.3V-Perip_Sensor 已接通                                    |
+-----------------+------------------------------------------------------------------------------+
| Screen          | 电源指示灯，表示 3.3V-Perip_Screen 已接通                                    |
+-----------------+------------------------------------------------------------------------------+
| WiFi / IO15     | 信号指示灯，表示 Wi-Fi 正常工作                                              |
+-----------------+------------------------------------------------------------------------------+
| Network / IO4   | 信号指示灯，表示与服务器连接正常                                             |
+-----------------+------------------------------------------------------------------------------+

硬件模块
----------

本章主要介绍各个功能模块（接口）的硬件实现，以及对这些模块的描述。

电源管理
++++++++++

USB/BAT 供电管理模块
~~~~~~~~~~~~~~~~~~~~~~~~~~

开发板支持电池供电，电源管理芯片 AP5056 可对电池进行充电。AP5056 是一款单片锂离子电池恒流/恒压线性电源管理芯片。高达 1 A 的可编程充电电流，预设充电电压为 4.2 V。

而当 USB 供电与电池供电同时存在时，系统的选择会如下图所示: VBUS 为高，Q4 处于截止状态，VBAT（电池电源）自动与系统电源切断，USB 成为了系统的供电之选。

USB/电池供电管理模块电路图如下图所示。

.. figure:: ../../../_static/esp32-meshkit-sensor/BATTERY-POWER.png
   :align: center
   :alt: ESP32-MeshKit-Sense
   :figclass: align-center
 
   USB/电池供电管理模块

外设电源管理模块
~~~~~~~~~~~~~~~~~~~~~~

首先，源自 USB 或 BAT 的输入需要通过电源管理芯片生成电路所需的 3.3 V。开发板采用了 ETA3425，其输出电压为 3.3 V，最大输出电流为 600 mA。

外设电池电路如下图所示。

.. figure:: ../../../_static/esp32-meshkit-sensor/peripheral.png
   :align: center
   :alt: ESP32-MeshKit-Sense
   :figclass: align-center
 
   外设电池电路

总路 VDD33 有两路分支：

-  专为 ESP32 模组模块供电的 ESP32_VDD33
-  专为所有外设供电的 VDD33_PeriP

二者的连接与否都可以通过排针及跳线帽进行控制，ESP32_VDD33 原理图如下图所示。

.. figure:: ../../../_static/esp32-meshkit-sensor/VDD33.png
   :align: center
   :alt: ESP32-MeshKit-Sense
   :figclass: align-center
 
   ESP32_VDD33 原理图

其中，VDD33-PeriP 也有两路分支：

- 专为外接屏供电的 VDD33_PeriP_Screen
- 专为三款传感器供电的 VDD33_PeriP_Sensor

二者的连接与否都可以通过模组 GPIO+MOS 管进行控制。VDD33_PeriP 原理图如下图所示。

.. figure:: ../../../_static/esp32-meshkit-sensor/PeriP.png
   :align: center
   :alt: ESP32-MeshKit-Sense
   :figclass: align-center
 
   VDD33_PeriP 原理图

Boot & UART 功能
++++++++++++++++++++

开发板采用一款插座 PROG Header，可通过排线连接至另一款 ESP-PROG 开发板上，然后再将 ESP-PROG 开发板中的 micro USB 接口与 PC 机相连，即可利用 PC 机对此开发板进行下载及调试。

Boot & UART 电路原理图如下图所示。

.. figure:: ../../../_static/esp32-meshkit-sensor/UART.png
   :align: center
   :alt: ESP32-MeshKit-Sense
   :figclass: align-center
 
   Boot & UART 电路原理图

睡眠唤醒模块
++++++++++++++++

开发板采用了一个实体按键，IO34 是一个 RTC 域中的管脚。当芯片处于睡眠时，可以利用此按键的操作来实现芯片的唤醒。

睡眠唤醒模块电路原理图如下图所示。

.. figure:: ../../../_static/esp32-meshkit-sensor/wakeup.png
   :align: center
   :alt: ESP32-MeshKit-Sense
   :figclass: align-center
 
   睡眠唤醒模块电路原理图

外接屏幕
++++++++++++

开发板采用一款可以外接屏幕的连接插座，利用排线可以将不同屏幕接至开发板上，以实现 ESP32 模组对屏幕的操作。

外接屏幕电路原理图如下图所示。

.. figure:: ../../../_static/esp32-meshkit-sensor/screen.png
   :align: center
   :alt: ESP32-MeshKit-Sense
   :figclass: align-center
 
   外接屏幕电路原理图

传感器
++++++++

湿温度传感器
~~~~~~~~~~~~~~~~~~

HTS221 是一种超小型相对湿度和温度传感器。开发板采用 3.3 V 供电，以及 I2C 的接口方式。

温湿度传感器电路原理图如下图所示。

.. figure:: ../../../_static/esp32-meshkit-sensor/THsensor.png
   :align: center
   :alt: ESP32-MeshKit-Sense
   :figclass: align-center
 
   温湿度传感器电路原理图


环境光传感器
~~~~~~~~~~~~~~~~~~

BH1750FVI 是一款数字的环境光传感器。开发板采用 3.3 V 供电，以及 I2C 的接口方式。

环境光传感器电路原理图如下图所示。

.. figure:: ../../../_static/esp32-meshkit-sensor/ambientlightsensor.png
   :align: center
   :alt: ESP32-MeshKit-Sense
   :figclass: align-center
 
   环境光传感器电路原理图

环境亮度传感器
~~~~~~~~~~~~~~~~~~~~

APDS-9960 是一款集成 ALS、红外 LED 和接近检测器的光学模块和环境亮度感测的环境亮度传感器。开发板 采用 3.3 V 供电，以及 I2C 的接口方式。需说明的是，此款传感器当前设计中默认为不上件的状态。

环境亮度传感器电路原理图如下图所示。

.. figure:: ../../../_static/esp32-meshkit-sensor/proximity.png
   :align: center
   :alt: ESP32-MeshKit-Sense
   :figclass: align-center
 
   环境亮度传感器电路原理图

示例程序
-----------

见 `esp-mdf/examples/development_kit/sense <https://github.com/espressif/esp-mdf/tree/master/examples/development_kit/sense>`__。


相关文档
-----------

- `ESP32-MeshKit-Sense 原理图 <../../_static/schematics/esp32-meshkit-sense/ESP32-MESHKIT-SENSE_V1_1-0917A.pdf>`_
- `ESP32-MeshKit-Sense PCB 布局 <../../_static/schematics/esp32-meshkit-sense/ESP32-MeshKit-Sense_V1.1.pdf>`_