====================
ESP-Module-Prog-1(R)
====================

:link_to_translation:`zh_CN:[中文]`

This user guide will help you get started with ESP-Module-Prog-1(R) and will also provide more in-depth information.

ESP-Module-Prog-1 and ESP-Module-Prog-1R (R stands for WROVER) are two flashing mainboards produced by Espressif. They can be used to flash modules without soldering the module to the power supply and signal lines. The mainboard can be used independently or in combination with a subboard. Espressif supports two subboards: ESP-Module-Prog-SUB-02 and ESP-Module-Prog-SUB-01&04, which cannot be used independently and must be used in conjunction with any of the above-mentioned mainboards.

This user guide will introduce **ESP-Module-Prog-1(R) Mainboard**. For detailed information about the subboards, please click the links below and check the corresponding user guides.

  - :doc:`../esp-module-prog-sub-01&04/user_guide`
  - :doc:`../esp-module-prog-sub-02/user_guide`

.. figure:: ../../../_static/esp-module-prog-1-r/esp-module-prog-1.png
    :align: center
    :scale: 70%
    :alt: ESP-Module-Prog-1

    ESP-Module-Prog-1

.. figure:: ../../../_static/esp-module-prog-1-r/esp-module-prog-1r.png
    :align: center
    :scale: 60%
    :alt: ESP-Module-Prog-1R

    ESP-Module-Prog-1R

This user guide consists of the following sections:

- `Board Overview`_: Overview of the board hardware/software.
- `Start Application Development`_: How to set up hardware/software to develop applications.
- `Hardware Reference`_: More detailed information about the board's hardware.
- `Sample Request`_: How to get a sample board.
- `Related Documents`_: Links to related documentation.


Board Overview
==============

ESP-Module-Prog-1(R) is an Espressif flashing mainboard designed specifically for modules. With a module mounted, it can also be used as a mini development board like ESP32-DevKitC. The only difference between ESP-Module-Prog-1 and ESP-Module-Prog-1R lies in the layout of the spring pins, which are used to fit different modules.

.. _fitting-modules-of-prog-1:

.. list-table::
   :widths: 40 60
   :header-rows: 1

   * - Mainboard
     - Fitting Module
   * - ESP-Module-Prog-1
     - * ESP32-WROOM-32
       * ESP32-WROOM-32D
       * ESP32-WROOM-32U
       * ESP32-SOLO-1
       * ESP32-WROOM-32E
       * ESP32-WROOM-32UE
       * ESP32-S2-SOLO
       * ESP32-S2-SOLO-U
       * ESP32-S2-SOLO-2
       * ESP32-S2-SOLO-2U
       * ESP32-S3-WROOM-1
       * ESP32-S3-WROOM-1U
       * ESP32-S3-WROOM-2
       * ESP32-C6-WROOM-1
       * ESP32-C6-WROOM-1U
   * - ESP-Module-Prog-1R:
     - * ESP32-WROVER-B
       * ESP32-WROVER-IB
       * ESP32-WROVER-E
       * ESP32-WROVER-IE

For information about the above modules, please refer to `Espressif Series Modules <https://www.espressif.com/zh-hans/products/modules?id=ESP32>`_.


Description of Components
-------------------------

.. _user-guide-esp-module-prog-1-front:

.. figure:: ../../../_static/esp-module-prog-1-r/esp-module-prog-1-front.png
    :align: center
    :alt: ESP-Module-Prog-1 - Front

    ESP-Module-Prog-1 - Front

.. figure:: ../../../_static/esp-module-prog-1-r/esp-module-prog-1r-front.png
    :align: center
    :alt: ESP-Module-Prog-1R - Front

    ESP-Module-Prog-1R - Front

The key components of the board are described in a clockwise direction.

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Key Component
     - Description
   * - Spring Pins
     - Pins that fit into the module's castellated holes for attaching and securing the module.
   * - Female Headers
     - 2.54 mm female headers that are connected to the pins of the module mounted in this board. For detailed information, please refer to Section `Pin Descriptions`_.
   * - J4
     - Configures the strapping pin. For details, please refer to Section `Strapping Pin Configuration`_.
   * - 3.3 V Power On LED
     - Lights up when the board is powered on with USB or power supply.
   * - USB-to-UART Bridge
     - Single-chip USB to UART bridge that provides transfer rates of up to 3 Mbps.
   * - Boot Button
     - Download button. Holding down **Boot** and then pressing **EN** initiates Firmware Download mode for downloading firmware through the serial port.
   * - USB Type-C to UART Interface
     - Serves either as a power supply interface for the development board or as a communication interface to the chip via an on-board USB to UART bridge.
   * - Reset Button
     - Reset button.
   * - 5V-to-3.3V LDO
     - Low Dropout Regulator (LDO).
   * - J5
     - Measures the current. For details, please refer to Section `Measuring Current`_.


Start Application Development
=============================

Before powering up your ESP-Module-Prog-1(R), please make sure that it is in good condition with no obvious signs of damage.

Required Hardware
-----------------

- Any one of the above-mentioned Espressif modules
- USB-A to USB-C cable
- PC (Windows, Linux, or macOS)

.. note::

  Please make sure to use the appropriate USB cable. Some cables can only be used for charging, not for data transfer or program flashing.

Hardware Setup
--------------

Mount the module to ESP-Module-Prog-1(R) as follows:

1. Place the module lightly on ESP-Module-Prog-1(R), making sure the castellated holes on the module are aligned with the spring pins on the board.
2. Press the module inward until you hear a "click", which indicates that the module has been successfully mounted.
3. Check if all the spring pins are clicked into the castellated holes. If there is a misalignment, you can use tweezers to poke the spring pins into the castellated holes.

Now the board is ready for software setup.


Software Setup
--------------

Recommended Approach
^^^^^^^^^^^^^^^^^^^^

It is recommended to use the ESP-IDF development framework to flash the binary file (\*.bin) to ESP-Module-Prog-1(R). Please refer to `ESP-IDF Get Started <https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html>`__ to get a quick overview of setting up your development environment and flashing applications.

Alternative Approach
^^^^^^^^^^^^^^^^^^^^

For Windows systems, you can also use the `Flash Download Tool <https://www.espressif.com/en/support/download/other-tools?keys=FLASH+>`_ to flash the binary files.

.. note::

  1. To flash binary files, the chip should be set to Firmware Download mode. This can be done either by the flash tool automatically, or by holding down the Boot button and tapping the Reset button.
  2. After flashing binary files, the Flash Download Tool restarts your module and boots the flashed application by default.


Hardware Reference
==================

This section provides more detailed information about the board’s hardware.

Block Diagram
-------------

The block diagram below shows the components of ESP-Module-Prog-1(R) and their interconnections.

.. figure:: ../../../_static/esp-module-prog-1-r/esp-module-prog-1-block-diagram-v1.1.png
    :align: center
    :alt: ESP-Module-Prog-1(R)

    ESP-Module-Prog-1(R)


Power Source Select
-------------------

There are three mutually exclusive ways to provide power to the board:

- USB Type-C to UART port (default and recommended)
- 5V and GND header pins
- 3V3 and GND header pins

Strapping Pin Configuration
---------------------------

J4 on ESP-Module-Prog-1(R) is a 2-Pin header, with one end being P7 and one end being High (i.e. pull-up):

- For some modules, the corresponding pin of P7 is not a strapping pin and no pull-up is needed for downloading. In this case, a jump cap is not required for J4.
- For some modules, the corresponding pin of P7 is a strapping pin and a pull-up is needed for downloading. In this case, a jump cap is required for J4.

.. note::

  For the corresponding pin of P7 on Espressif modules, please refer to `ESP-Module-Prog-1(R) GPIO Map <https://dl.espressif.com/dl/schematics/GPIO_MAP_ESP-Module-Prog-1_V1.1_EN_20230523.xls>`__.

Measuring Current
-----------------

J5 on ESP-Module-Prog-1(R) can be used to measure the current of the module.

- Remove the J5 jumper cap: At this time, the peripherals and modules on the board are disconnected from the power supply, and the module current can be measured after the J5 pin is connected to the ammeter.
- Install the J5 jump cap (factory default): the board functions normally.

.. note::

  When using the 3V3 and GND header pins to power up the board, you need to remove the J5 jumper cap and connect an ammeter in series with the external circuit to measure the current of the module.


Pin Descriptions
----------------

The two tables below provide the **Name** and **Signal** of female headers on both sides of the board (J2 and J3). The pin names are shown in Figure :ref:`user-guide-esp-module-prog-1-front`. The numbering is the same as in the `ESP-Module-Prog-1(R) Schematics <https://dl.espressif.com/dl/schematics/esp_idf/esp-module-prog-1-schematics.pdf>`_ (PDF). For the corresponding pin of each signal on Espressif modules, please refer to `ESP-Module-Prog-1(R) GPIO Map <https://dl.espressif.com/dl/schematics/GPIO_MAP_ESP-Module-Prog-1_V1.1_EN_20230523.xls>`__.

J2
^^^
=======  ================  ================================
No.       Name              Signal
=======  ================  ================================
1         3V3               3.3 V power supply
2         EN                CHIP_EN (High: enables the chip; Low: disables the chip. Pulled up by default.)
3         1                 P1
4         2                 P2
5         3                 P3
6         4                 P4
7         5                 P5
8         6                 P6
9         7                 P7
10        8                 P8
11        9                 P9
12        10                P10
13        11                P11
14        12                P12
15        13                P13
16        14                P14
17        15                P15
18        16                P16
19        17                P17
20        18                P18
21        19                P19
22        20                P20
23        21                P21
24        22                P22
25        5V                5 V power supply
=======  ================  ================================


J3
^^^
=======  ================  ================================
No.      Name              Signal
=======  ================  ================================
1        G                 Ground
2        44                P44
3        43                P43
4        TX                TXD0
5        RX                RXD0
6        42                P42
7        41                P41
8        40                P40
9        39                P39
10       38                P38
11       37                P37
12       36                P36
13       35                P35
14       34                P34
15       33                P33
16       32                P32
17       31                P31
18       30                P30
19       29                P29
20       28                P28
21       27                P27
22       26                P26
23       25                P25
24       24                P24
25       23                P23
=======  ================  ================================


Sample Request
==============

Retail Orders
-------------

If you order a few samples, each ESP-Module-Prog-1(R) comes in an individual package in either antistatic bag or any packaging depending on your retailer.

For retail orders, please go to https://www.espressif.com/en/company/contact/buy-a-sample.

Wholesale Orders
----------------

If you order in bulk, the boards come in large cardboard boxes.

For wholesale orders, please go to https://www.espressif.com/en/contact-us/sales-questions.


Related Documents
=================

- `ESP-Module-Prog-1(R) GPIO Map <https://dl.espressif.com/dl/schematics/GPIO_MAP_ESP-Module-Prog-1_V1.1_EN_20230523.xls>`__ (XLS)
- `ESP-Module-Prog-1(R) Schematics <https://dl.espressif.com/dl/schematics/esp_idf/esp-module-prog-1-schematics.pdf>`_ (PDF)
- `ESP-Module-Prog-1(R) PCB Layout <https://dl.espressif.com/dl/schematics/esp_idf/PCB_ESP-Module-Prog-1_V1.1_20230113.pdf>`_ (PDF)
- `ESP-Module-Prog-1(R) Dimensions <https://dl.espressif.com/dl/schematics/esp_idf/Dimension_ESP-Module-Prog-1_V1.1_20230523.pdf>`_ (PDF)
- `ESP-Module-Prog-1(R) Dimensions source file <https://dl.espressif.com/dl/schematics/esp_idf/Dimension_ESP-Module-Prog-1_V1.1_20230523.dxf>`_ (DXF) - You can view it with `Autodesk Viewer <https://viewer.autodesk.com/>`_ online
- `Espressif Modules Datasheet <https://www.espressif.com/en/support/documents/technical-documents?keys=&field_type_tid%5B%5D=1133&field_type_tid%5B%5D=838&field_type_tid%5B%5D=839&field_type_tid%5B%5D=1181&field_type_tid%5B%5D=682&field_type_tid%5B%5D=268&field_type_tid%5B%5D=266&field_type_tid%5B%5D=54&field_type_tid%5B%5D=400>`__
- `Espressif Product Selector <https://products.espressif.com/#/product-selector?names=>`__
