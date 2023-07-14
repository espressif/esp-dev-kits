=========================
ESP-Module-Prog-SUB-01&04
=========================

:link_to_translation:`zh_CN:[中文]`

This user guide will help you get started with ESP-Module-Prog-SUB-01&04 and will also provide more in-depth information. For detailed information about the mainboard and the other subboard, please click the links below and check the corresponding user guides.

  - :doc:`../esp-module-prog-1-r/user_guide`
  - :doc:`../esp-module-prog-sub-02/user_guide`

.. figure:: ../../../_static/esp-module-prog-sub-01&04/esp-module-prog-sub-0104.png
    :align: center
    :scale: 50%
    :alt: ESP-Module-Prog-SUB-01&04

    ESP-Module-Prog-SUB-01&04

This user guide consists of the following sections:

- `Board Overview`_: Overview of the board hardware/software.
- `Start Application Development`_: How to set up hardware/software to develop applications.
- `Hardware Reference`_: More detailed information about the board's hardware.
- `Sample Request`_: How to get a sample board.
- `Related Documents`_: Links to related documentation.


Board Overview
==============

ESP-Module-Prog-SUB-01&04 is an Espressif flashing subboard designed specifically for modules. It can be used to flash modules without soldering the module to the power supply and signal lines. With a module mounted, ESP-Module-Prog-SUB-01&04 can also be used as a mini development board like ESP32-DevKitC. Note that as a subboard, ESP-Module-Prog-SUB-01&04 cannot be used alone, but must be used together with the ESP-Module-Prog-1 or ESP-Module-Prog-1R mainboard.

.. _fitting-modules-of-prog-0104:

- ESP-Module-Prog-SUB-01&04 fits the following Espressif modules:
   - ESP8685-WROOM-01
   - ESP8684-WROOM-01C
   - ESP8685-WROOM-04
   - ESP8684-WROOM-04C

For information about the above modules, please refer to `Espressif Series Modules <https://www.espressif.com/en/products/modules?id=ESP32>`_.


Description of Components
-------------------------

.. _user-guide-esp-module-prog-sub-0104-front:

.. figure:: ../../../_static/esp-module-prog-sub-01&04/esp-module-prog-0104-front.png
    :align: center
    :scale: 40%
    :alt: ESP-Module-Prog-SUB-01&04 - Front

    ESP-Module-Prog-SUB-01&04 - Front

The key components of the board are described in a clockwise direction.

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Key Component
     - Description
   * - Spring Pins
     - Pins that fit into the module's castellated holes for attaching and securing the module.
   * - Pin Headers
     - 2.54 mm pin headers that are connected to the pins of the module mounted in this board and to the mainboard. For detailed information, please refer to Section `Pin Descriptions`_.
   * - J4
     - Configures the strapping pin. For details, please refer to Section `Strapping Pin Configuration`_.
   * - ESP-Module-Prog-1(R)
     - ESP-Module-Prog-1 and ESP-Module-Prog-1R (R stands for WROVER) are two flashing mainboards produced by Espressif. For detailed information about the mainboard, please refer to :doc:`../esp-module-prog-1-r/user_guide`.


Start Application Development
=============================

Before powering up your ESP-Module-Prog-SUB-01&04, please make sure that it is in good condition with no obvious signs of damage.

Required Hardware
-----------------

- Any one of the above-mentioned Espressif modules
- USB-A to USB-C cable
- PC (Windows, Linux, or macOS)
- ESP-Module-Prog-1 or ESP-Module-Prog-1R mainboard

.. note::

  Please make sure to use the appropriate USB cable. Some cables can only be used for charging, not for data transfer or program flashing.

Hardware Setup
--------------

Mount the module to the ESP-Module-Prog-SUB-01&04 subboard as follows:

1. Place the module lightly on ESP-Module-Prog-SUB-01&04, making sure the castellated holes on the module are aligned with the spring pins on the board.
2. Press the module inward until you hear a "click", which indicates that the module has been successfully mounted.
3. Check if all the spring pins are clicked into the castellated holes. If there is a misalignment, you can use tweezers to poke the spring pins into the castellated holes.
4. Mount the subboard to the mainboard.

Now the board is ready for software setup.


Software Setup
--------------

Recommended Approach
^^^^^^^^^^^^^^^^^^^^

It is recommended to use the ESP-IDF development framework to flash the binary file (\*.bin) to ESP-Module-Prog-SUB-01&04. Please refer to `ESP-IDF Get Started <https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html>`__ to get a quick overview of setting up your development environment and flashing applications.

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

The block diagram below shows the components of ESP-Module-Prog-SUB-01&04 and their interconnections.

.. figure:: ../../../_static/esp-module-prog-sub-01&04/esp-module-prog-sub-0104-block-diagram-v1.0.png
    :align: center
    :alt: ESP-Module-Prog-SUB-01&04

    ESP-Module-Prog-SUB-01&04


Strapping Pin Configuration
---------------------------

J4 on ESP-Module-Prog-SUB-01&04 is a 2-Pin header, with one end being P14 and one end being High (i.e. pull-up):

- For some modules, the corresponding pin of P14 is not a strapping pin and no pull-up is needed for downloading. In this case, a jump cap is not required for J4.
- For some modules, the corresponding pin of P14 is a strapping pin and a pull-up is needed for downloading. In this case, a jump cap is required for J4.

.. note::

  1. For the corresponding pin of P14 on Espressif modules, please refer to `ESP-Module-Prog-SUB-01&04 GPIO Map <https://dl.espressif.com/dl/schematics/GPIO_MAP_ESP-Module-Prog-SUB-01&04_V1.0_EN_20230308.xls>`__.
  2. ESP-Module-Prog-SUB-01&04 is currently fitted to ESP8684 series modules, where P14 needs to be pulled up. So, J4 requires a jump cap by default.

Pin Descriptions
----------------

The two tables below provide the **Name** and **Signal** of pin headers on both sides of the board (J2 and J3). The pin names are shown in Figure :ref:`user-guide-esp-module-prog-sub-0104-front`. The numbering is the same as in the `ESP-Module-Prog-SUB-01&04 Schematics <https://dl.espressif.com/dl/schematics/esp_idf/esp-module-prog-sub-0104-schematics.pdf>`_ (PDF). For the corresponding pin of each signal on Espressif modules, please refer to `ESP-Module-Prog-SUB-01&04 GPIO Map <https://dl.espressif.com/dl/schematics/GPIO_MAP_ESP-Module-Prog-SUB-01&04_V1.0_EN_20230308.xls>`__.

J2
^^^
=======  ================  ================================
No.      Name              Signal
=======  ================  ================================
1        3V3               3.3 V power supply
2        EN                CHIP_EN (High: enables the chip; Low: disables the chip. Pulled up by default.)
3        4                 P4
4        5                 P5
5        6                 P6
6        13                P13
7        NC                No connection
8        NC                No connection
9        14                P14
10       7                 P7
11       8                 P8
12       9                 P9
13       10                P10
14       NC                No connection
=======  ================  ================================


J3
^^^
=======  ================  ================================
No.      Name              Signal
=======  ================  ================================
1        G                 Ground
2        1                 P1
3        2                 P2
4        TX                TXD0
5        RX                RXD0
6        3                 P3
7        17                P17
8        16                P16
9        NC                No connection
10       NC                No connection
11       12                P12
12       11                P11
13       NC                No connection
14       15                P15
=======  ================  ================================


Sample Request
==============

Retail Orders
-------------

If you order a few samples, each ESP-Module-Prog-SUB-01&04 comes in an individual package in either antistatic bag or any packaging depending on your retailer.

For retail orders, please go to https://www.espressif.com/en/company/contact/buy-a-sample.

Wholesale Orders
----------------

If you order in bulk, the boards come in large cardboard boxes.

For wholesale orders, please go to https://www.espressif.com/en/contact-us/sales-questions.


Related Documents
=================

- `ESP-Module-Prog-SUB-01&04 GPIO Map <https://dl.espressif.com/dl/schematics/GPIO_MAP_ESP-Module-Prog-SUB-01&04_V1.0_EN_20230308.xls>`__ (XLS)
- `ESP-Module-Prog-SUB-01&04 Schematics <https://dl.espressif.com/dl/schematics/esp_idf/esp-module-prog-sub-0104-schematics.pdf>`_ (PDF)
- `ESP-Module-Prog-SUB-01&04 PCB Layout <https://dl.espressif.com/dl/schematics/esp_idf/PCB_ESP-Module-Prog-SUB-01&04_V1.0_20230308.pdf>`_ (PDF)
- `ESP-Module-Prog-SUB-01&04 Dimensions <https://dl.espressif.com/dl/schematics/esp_idf/Dimension_ESP-Module-Prog-SUB-01&04_V1.0_20230523.pdf>`_ (PDF)
- `ESP-Module-Prog-SUB-01&04 Dimensions source file <https://dl.espressif.com/dl/schematics/esp_idf/Dimension_ESP-Module-Prog-SUB-01&04_V1.0_20230523.dxf>`_ (DXF) - You can view it with `Autodesk Viewer <https://viewer.autodesk.com/>`_ online
- `Espressif Modules Datasheet <https://www.espressif.com/en/support/documents/technical-documents?keys=&field_type_tid%5B%5D=1133&field_type_tid%5B%5D=838&field_type_tid%5B%5D=839&field_type_tid%5B%5D=1181&field_type_tid%5B%5D=682&field_type_tid%5B%5D=268&field_type_tid%5B%5D=266&field_type_tid%5B%5D=54&field_type_tid%5B%5D=400>`__
- `Espressif Product Selector <https://products.espressif.com/#/product-selector?names=>`__
