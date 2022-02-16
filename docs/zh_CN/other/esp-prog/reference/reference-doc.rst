参考文档
=======================

:link_to_translation:`en:[English]`


功能介绍
~~~~~~~~~~~~~

USB 桥接器的工作模式
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

ESP-Prog 采用 FTDI 公司的 FT2232HL 作为 USB 桥接控制器芯片，可通过配置将 USB 2.0 接口转换为支持多种工业标准的串行和并行接口。ESP-Prog 使用 FT2232HL 默认的双异步串行接口模式，用户只需在电脑上安装相应的 `FT2232HL 芯片驱动 <http://www.ftdichip.com/Drivers/VCP.htm>`__ 即可使用。

.. 注解:: 在电脑端识别出的两个端口中，序号大的是 Program 接口，序号小的是 JTAG 接口。

通讯接口
^^^^^^^^^^^^^^^

ESP-Prog 可以通过 Program 接口和 JTAG 接口连接 ESP32 产品板。

-  **Program 接口**

   Program 接口有 6 个管脚，包括了 UART 接口 (TXD, RXD)、启动模式选择管脚 (ESP\_IO0) 和复位管脚 (ESP\_EN)。用户板上 Program 接口管脚设计应如下图。

.. figure:: ../../../../_static/esp-prog/program_pin.png
   :align: center
   :scale: 80%
   :alt: Program 接口（点击放大）

   Program 接口（点击放大）

-  **JTAG 接口**

   用户板上 JTAG 接口管脚设计应如下图。

.. figure:: ../../../../_static/esp-prog/JTAG_pin.png
   :align: center
   :scale: 65%
   :alt: JTAG 接口（点击放大）

   JTAG 接口（点击放大）

-  **防呆设计**

   ESP-Prog 接口使用牛角座连接器 (DC3-6P/DC3-10P)，具有防反接功能。建议用户使用同类型的连接器，如 ``FTSH-105-01-S-DV-*`` 或 ``DC3-*P``。

.. 注解:: 由于排线具有方向性，ESP-Prog 的每个接口都与排线一一对应，如果使用不配套的排线会导致接错接口，所以请使用官方提供的排线。

自动下载功能
^^^^^^^^^^^^^^^^^^^

ESP-Prog 支持自动下载功能。连接 Program 接口到用户板之后，下载程序会控制复位管脚和启动选择模式管脚的状态，执行下载命令后即可实现设备的自动下载和运行，无需用户手动重启和选择下载模式。ESP-Prog 上的两个按键能实现手动复位设备和控制设备的启动模式。

自动下载电路的原理图如下。

.. figure:: ../../../../_static/esp-prog/Auto_program.png
   :align: center
   :scale: 70%
   :alt: 自动下载电路（点击放大）

   自动下载电路（点击放大）

延时电路
^^^^^^^^^^^^^^^

延时电路包括了总线缓存、反相器、MOS 管、一阶 RC 电路等器件。延时电路可以保证 ESP32 芯片在上电或复位过程中，先完成自身的上电启动或复位，再接通 JTAG 信号，以免受到 JTAG 影响。

.. figure:: ../../../../_static/esp-prog/delay_circuit.png
   :align: center
   :scale: 60%
   :alt: ESP-Prog 延时电路（点击放大）

   ESP-Prog 延时电路（点击放大）

LED 状态指示灯
^^^^^^^^^^^^^^^^^^^

-  当系统的 3.3 V 电源通电时，红色 LED 灯亮起；
-  当下载板向 ESP32 发送数据时，绿色 LED 灯亮起；
-  当下载板接收来自 ESP32 的数据时，蓝色 LED 灯亮起。

.. figure:: ../../../../_static/esp-prog/prog_led.jpg
   :align: center
   :scale: 70%
   :alt: LED 状态（点击放大）

   LED 状态（点击放大）

排针
^^^^^^^^^^^^^^^^^^^

可以通过排针选择 Program 和 JTAG 接口中的参考电源，如下图所示。

-  **接口电源选择排针**

   中间的排针是每个接口的电源输入管脚。与 5 V 连接时，接口的电源输出为 5 V；与 3.3 V 连接时，接口电源输出则为 3.3 V。

-  **IO0 On/Off 排针**

   IO0 是 ESP8266 和 ESP32 的 Boot 模式选择管脚，芯片上电后，IO0 可作为正常 GPIO 使用。为了不让 ESP-Prog 自动下载电路影响用户板上 IO0 的正常使用，用户可手动控制 IO0 的通断。

.. figure:: ../../../../_static/esp-prog/prog_power_sel.jpg
   :align: center
   :scale: 80%
   :alt: 排针（点击放大）

   排针（点击放大）


有关本开发板的更多设计文档，请联系我们的商务部门 `sales@espressif.com <sales@espressif.com>`_。
