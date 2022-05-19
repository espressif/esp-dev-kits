ESP32-S3-LCD-EV-BOARD
=======================


本指南将帮助您快速上手 ESP32-S3-LCD-EV-BOARD，并提供该款开发板的详细信息。

ESP32-S3-LCD-EV-BOARD 是用于评估和验证 ESP32-S3 屏幕交互应用的开发板，具有触摸屏交互和语音交互的功能，该开发板具有以下特性：

-  板载 ESP32-S3-WROOM-1 模组，内置 16 MB flash + 8MB PSRAM
-  板载音频 Codec + 音频功放
-  板载双麦克风拾音
-  可搭配不同屏幕子板使用，支持 ``RGB``, ``8080``, ``SPI``, ``I2C`` 接口屏幕
-  USB Type-C 接口下载调试

.. figure:: ../../../_static/esp32-s3-lcd-ev-board/esp32-s3-lcd-ev-board.png
   :align: center
   :alt: ESP32-S3-LCD-EV-BOARD
   :scale: 20%
   :figclass: align-center

   ESP32-S3-LCD-EV-BOARD

**本指南包括：**

-  :ref:`Getting-Started-esp32-s3-lcd-ev-board`：提供 ESP32-S3-LCD-EV-BOARD 必须了解的硬件资源和开发前的软件准备。
-  :ref:`Related-Documents-esp32-s3-lcd-ev-board`：提供相关文档的链接。

.. _Getting-Started-esp32-s3-lcd-ev-board:

快速入门
-----------

本节介绍如何开始使用 ESP32-S3-LCD-EV-BOARD。首先，介绍一些关于 ESP32-S3-LCD-EV-BOARD 的基本信息。

组件介绍
~~~~~~~~

ESP32-S3-LCD-EV-BOARD 开发板包括以下两部分：

-  **主板**：ESP32-S3-LCD-EV-BOARD 主板集成了 ESP32-S3-WROOM-1 模组，语音输入输出设备，屏幕子板连接接口以及拓展接口。下图展示了 ESP32-S3-LCD-EV-BOARD 主要板载资源。

.. figure:: ../../../_static/esp32-s3-lcd-ev-board/board_resource.png
   :align: center
   :alt: ESP32-S3-LCD-EV-BOARD-MB
   :scale: 20%
   :figclass: align-center

   ESP32-S3-LCD-EV-BOARD-MB 正面图

-  **子板2**：ESP32-S3-LCD-EV-BOARD-SUB2 子板提供了两种屏幕接口，分别支持连接一块 RGB 接口屏或者一块 8080 并口屏。当前子板贴装了一块 3.95 英寸 RGB565 接口的触摸屏，该屏使用的驱动芯片型号为 GC9503CV。

.. figure:: ../../../_static/esp32-s3-lcd-ev-board/ESP32-S3-LCD-EV-Board_SUB2.png
   :align: center
   :alt: ESP32-S3-LCD-EV-BOARD-SUB2
   :scale: 20%
   :figclass: align-center

   ESP32-S3-LCD-EV-BOARD-SUB2 背面图

.. _Related-Documents-esp32-s3-lcd-ev-board:

相关文档
-----------

-  `ESP32-S3 技术规格书 <https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_cn.pdf>`__ (PDF)
-  `ESP32-S3-WROOM-1 技术规格书 <https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf>`__ (PDF)
-  `乐鑫产品选型工具 <https://products.espressif.com/#/product-selector?names=>`__
-  `ESP32-S3-LCD-EV-BOARD-MB 原理图 <../../_static/schematics/esp32-s3-lcd-ev-board/SCH_ESP32-S3-LCD_EV_Board_MB_V1.1_20220713.pdf>`__ (PDF)
-  `ESP32-S3-LCD-EV-BOARD-MB PCB 布局图 <../../_static/schematics/esp32-s3-lcd-ev-board/PCB_ESP32-S3-LCD_Ev_Board_MB_V1.0_20220610.pdf>`__ (PDF)
-  `ESP32-S3-LCD-EV-BOARD-SUB1 原理图 <../../_static/schematics/esp32-s3-lcd-ev-board/SCH_ESP32-S3-LCD_Ev_Board_SUB1_V1.0_20220617.pdf>`__ (PDF)
-  `ESP32-S3-LCD-EV-BOARD-SUB1 PCB 布局图 <../../_static/schematics/esp32-s3-lcd-ev-board/PCB_ESP32-S3-LCD_Ev_Board_SUB1_V1.0_20220617.pdf>`__ (PDF)
-  `ESP32-S3-LCD-EV-BOARD-SUB2 原理图 <../../_static/schematics/esp32-s3-lcd-ev-board/SCH_ESP32-S3-LCD_Ev_Board_SUB2_V1.0_20220615.pdf>`__ (PDF)
-  `ESP32-S3-LCD-EV-BOARD-SUB2 PCB 布局图 <../../_static/schematics/esp32-s3-lcd-ev-board/PCB_ESP32-S3-LCD_Ev_Board_SUB2_V1.1_20220708.pdf>`__ (PDF)
-  `ESP32-S3-LCD-EV-BOARD-SUB3 原理图 <../../_static/schematics/esp32-s3-lcd-ev-board/SCH_ESP32-S3-LCD_Ev_Board_SUB3_V1.0_20220617.pdf>`__ (PDF)
-  `ESP32-S3-LCD-EV-BOARD-SUB3 PCB 布局图 <../../_static/schematics/esp32-s3-lcd-ev-board/PCB_ESP32-S3-LCD_Ev_Board_SUB3_V1.0_20220617.pdf>`__ (PDF)
-  `GC9503CV 规格书 <../../_static/esp32-s3-lcd-ev-board/datasheets/3.95_480x480_SmartDisplay/GC9503NP_DataSheet_V1.7.pdf>`_ (PDF)
