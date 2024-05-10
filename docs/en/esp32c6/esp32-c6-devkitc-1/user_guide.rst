=======================
ESP32-C6-DevKitC-1 v1.2
=======================

:link_to_translation:`zh_CN:[中文]`

The older version: :doc:`user_guide_v1.1`

This user guide will help you get started with ESP32-C6-DevKitC-1 and will also provide more in-depth information.

ESP32-C6-DevKitC-1 is an entry-level development board based on `ESP32-C6-WROOM-1(U) <https://www.espressif.com/sites/default/files/documentation/esp32-c6-wroom-1_datasheet_en.pdf>`_, a general-purpose module with a 8 MB SPI flash. This board integrates complete Wi-Fi, Bluetooth LE, Zigbee, and Thread functions.

Most of the I/O pins are broken out to the pin headers on both sides for easy interfacing. Developers can either connect peripherals with jumper wires or mount ESP32-C6-DevKitC-1 on a breadboard.

.. figure:: ../../../_static/esp32-c6-devkitc-1/esp32-c6-devkitc-1-isometric_v1.2.png
    :align: center
    :alt: ESP32-C6-DevKitC-1
    :figclass: align-center

    ESP32-C6-DevKitC-1

The document consists of the following major sections:

- `Getting Started`_: Overview of ESP32-C6-DevKitC-1 and hardware/software setup instructions to get started.
- `Hardware Reference`_: More detailed information about the ESP32-C6-DevKitC-1's hardware.
- `Hardware Revision Details`_: Revision history, known issues, and links to user guides for previous versions (if any) of ESP32-C6-DevKitC-1.
- `Related Documents`_: Links to related documentation.

Getting Started
===============

This section provides a brief introduction of ESP32-C6-DevKitC-1, instructions on how to do the initial hardware setup and how to flash firmware onto it.

Description of Components
-------------------------

.. _user-guide-c6-devkitc-1-v1.2-board-front:

.. figure:: ../../../_static/esp32-c6-devkitc-1/esp32-c6-devkitc-1-v1.2-annotated-photo.png
    :align: center
    :alt: ESP32-C6-DevKitC-1 - front
    :figclass: align-center

    ESP32-C6-DevKitC-1 - front

The key components of the board are described in a clockwise direction. 

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Key Component
     - Description
   * - ESP32-C6-WROOM-1 or ESP32-C6-WROOM-1U
     - ESP32-C6-WROOM-1 and ESP32-C6-WROOM-1U are general-purpose modules supporting Wi-Fi 6 in 2.4 GHz band, Bluetooth 5, and IEEE 802.15.4 (Zigbee 3.0 and Thread 1.3). They are built around the ESP32-C6 chip, and comes with a 8 MB SPI flash. ESP32-C6-WROOM-1 uses on-board PCB antenna, whereas ESP32-C6-WROOM-1U uses external antenna connector. For more information, see `ESP32-C6-WROOM-1 Datasheet`_.
   * - Pin Header
     - All available GPIO pins (except for the SPI bus for flash) are broken out to the pin headers on the board.   
   * - 5 V to 3.3 V LDO
     - Power regulator that converts a 5 V supply into a 3.3 V output.
   * - 3.3 V Power On LED
     - Turns on when the USB power is connected to the board.
   * - USB-to-UART Bridge
     - Single USB-to-UART bridge chip provides transfer rates up to 3 Mbps.     
   * - ESP32-C6 USB Type-C Port
     - The USB Type-C port on the ESP32-C6 chip compliant with USB 2.0 full speed. It is capable of up to 12 Mbps transfer speed (Note that this port does not support the faster 480 Mbps high-speed transfer mode). This port is used for power supply to the board, for flashing applications to the chip, for communication with the chip using USB protocols, as well as for JTAG debugging.
   * - Boot Button
     - Download button. Holding down **Boot** and then pressing **Reset** initiates Firmware Download mode for downloading firmware through the serial port.
   * - Reset Button
     - Press this button to restart the system.
   * - USB Type-C to UART Port
     - Used for power supply to the board, for flashing applications to the chip, as well as the communication with the ESP32-C6 chip via the on-board USB-to-UART bridge.   
   * - RGB LED
     - Addressable RGB LED, driven by GPIO8.
   * - J5
     - Used for current measurement. See details in Section :ref:`user-guide-c6-devkitc-1-v1.2-current`.


Start Application Development
-----------------------------

Before powering up your ESP32-C6-DevKitC-1, please make sure that it is in good condition with no obvious signs of damage.

Required Hardware
^^^^^^^^^^^^^^^^^

- ESP32-C6-DevKitC-1
- USB-A to USB-C cable
- Computer running Windows, Linux, or macOS

.. note::

  Be sure to use a good quality USB cable. Some cables are for charging only and do not provide the needed data lines nor work for programming the boards.

Software Setup
^^^^^^^^^^^^^^

Please proceed to `ESP-IDF Get Started <https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/get-started/index.html>`__, which will quickly help you set up the development environment then flash an application example onto your board.

.. ESP-AT Support
.. ^^^^^^^^^^^^^^^^^^^^^^

.. The ESP32-C6-DevKitC-1 supports ESP-AT software that provides a set of AT commands with which you can quickly integrate wireless connectivity features into your product without a need for embedded application development of the module on this development board.

.. The software is available as a pre-built binary that can be downloaded from `ESP-AT repository <https://github.com/espressif/esp-at/tags>`_.

.. For more information about using ESP-AT, including information on how to customize pre-built binaries, please refer to `ESP-AT User Guide <https://docs.espressif.com/projects/esp-at/en/latest/>`_.

Contents and Packaging
----------------------

Retail orders
^^^^^^^^^^^^^

If you order a few samples, each ESP32-C6-DevKitC-1 comes in an individual package in either antistatic bag or any packaging depending on your retailer.

For retail orders, please go to https://www.espressif.com/en/company/contact/buy-a-sample.

Wholesale Orders
^^^^^^^^^^^^^^^^

If you order in bulk, the boards come in large cardboard boxes.

For wholesale orders, please check `Espressif Product Ordering Information <https://www.espressif.com/sites/default/files/documentation/espressif_products_ordering_information_en.pdf>`_ (PDF)

Hardware Reference
==================

Block Diagram
-------------

The block diagram below shows the components of ESP32-C6-DevKitC-1 and their interconnections.

.. figure:: ../../../_static/esp32-c6-devkitc-1/esp32-c6-devkitc-1-v1.2-block-diagram.png
    :align: center
    :scale: 70%
    :alt: ESP32-C6-DevKitC-1 (click to enlarge)
    :figclass: align-center

    ESP32-C6-DevKitC-1 (click to enlarge)

Power Supply Options
--------------------

There are three mutually exclusive ways to provide power to the board:

- USB Type-C to UART Port and ESP32-C6 USB Type-C Port (either one or both), default power supply (recommended)
- 5V and GND pin headers
- 3V3 and GND pin headers

.. _user-guide-c6-devkitc-1-v1.2-current:

Current Measurement
-------------------

The J5 headers on ESP32-C6-DevKitC-1 (see J5 in Figure :ref:`user-guide-c6-devkitc-1-v1.2-board-front`) can be used for measuring the current drawn by the ESP32-C6-WROOM-1(U) module:

- Remove the jumper: Power supply between the module and peripherals on the board is cut off. To measure the module's current, connect the board with an ammeter via J5 headers.
- Apply the jumper (factory default): Restore the board's normal functionality.

.. note::

  When using 3V3 and GND pin headers to power the board, please remove the J5 jumper, and connect an ammeter in series to the external circuit to measure the module's current.

Header Block 
------------- 

The two tables below provide the **Name** and **Function** of the pin headers on both sides of the board (J1 and J3). The pin header names are shown in Figure :ref:`user-guide-c6-devkitc-1-v1.2-board-front`. The numbering is the same as in the `ESP32-C6-DevKitC-1 Schematic <../../_static/esp32-c6-devkitc-1/schematics/esp32-c6-devkitc-1-schematics_v1.2.pdf>`_ (PDF). 

J1 
^^^ 
===  =======  ==========  ================================================= 
No.  Name     Type [1]_    Function 
===  =======  ==========  ================================================= 
1    3V3       P          3.3 V power supply
2    RST       I          High: enables the chip; Low: disables the chip.
3    4         I/O/T      MTMS [3]_, GPIO4, LP_GPIO4, LP_UART_RXD, ADC1_CH4, FSPIHD
4    5         I/O/T      MTDI [3]_, GPIO5, LP_GPIO5, LP_UART_TXD, ADC1_CH5, FSPIWP
5    6         I/O/T      MTCK, GPIO6, LP_GPIO6, LP_I2C_SDA, ADC1_CH6, FSPICLK
6    7         I/O/T      MTDO, GPIO7, LP_GPIO7, LP_I2C_SCL, FSPID
7    0         I/O/T      GPIO0, XTAL_32K_P, LP_GPIO0, LP_UART_DTRN, ADC1_CH0
8    1         I/O/T      GPIO1, XTAL_32K_N, LP_GPIO1, LP_UART_DSRN, ADC1_CH1
9    8         I/O/T      GPIO8 [2]_ [3]_
10   10        I/O/T      GPIO10
11   11        I/O/T      GPIO11
12   2         I/O/T      GPIO2, LP_GPIO2, LP_UART_RTSN, ADC1_CH2, FSPIQ
13   3         I/O/T      GPIO3, LP_GPIO3, LP_UART_CTSN, ADC1_CH3
14   5V        P          5 V power supply
15   G         G          Ground
16   NC        –          No connection
===  =======  ==========  ================================================= 


J3 
^^^ 
===  ==========  ======  ========================================== 
No.   Name       Type    Function 
===  ==========  ======  ==========================================
1     G          G       Ground
2     TX         I/O/T   U0TXD, GPIO16, FSPICS0
3     RX         I/O/T   U0RXD, GPIO17, FSPICS1
4     15         I/O/T   GPIO15 [3]_
5     23         I/O/T   GPIO23, SDIO_DATA3
6     22         I/O/T   GPIO22, SDIO_DATA2
7     21         I/O/T   GPIO21, SDIO_DATA1, FSPICS5
8     20         I/O/T   GPIO20, SDIO_DATA0, FSPICS4
9     19         I/O/T   GPIO19, SDIO_CLK, FSPICS3
10    18         I/O/T   GPIO18, SDIO_CMD, FSPICS2
11    9          I/O/T   GPIO9 [3]_
12    G          G       Ground
13    13         I/O/T   GPIO13, USB_D+
14    12         I/O/T   GPIO12, USB_D-
15    G          G       Ground
16    NC         –       No connection
===  ==========  ======  ==========================================

.. [1] P: Power supply; I: Input; O: Output; T: High impedance. 
.. [2] Used to drive the RGB LED.
.. [3] MTMS, MTDI, GPIO8, GPIO9, and GPIO15 are strapping pins of the ESP32-C6 chip. These pins are used to control several chip functions depending on binary voltage values applied to the pins during chip power-up or system reset. For description and application of the strapping pins, please refer to `ESP32-C6 Datasheet`_ > Section *Strapping Pins*. 

Pin Layout
^^^^^^^^^^^
.. figure:: ../../../_static/esp32-c6-devkitc-1/esp32-c6-devkitc-1-pin-layout.png
    :align: center
    :scale: 40%
    :alt: ESP32-C6-DevKitC-1 Pin Layout (click to enlarge)
    :figclass: align-center

    ESP32-C6-DevKitC-1 Pin Layout (click to enlarge)
    
Hardware Revision Details
=========================

ESP32-C6-DevKitC-1 v1.2
-----------------------

- For boards with the PW number of and after PW-2023-02-0139 (on and after February 2023), J5 is changed from straight headers to curved headers.
- For boards with the PW number of and after PW-2023-07-XXXX (on and after July 2023), multi-point calibration is performed on ADC instead of two-point calibration, and the measurement range and accuracy are illustrated in `ESP32-C6 Datasheet`_ > Section ADC Characteristics. For boards with eariler PW number, please `ask our sales team <https://www.espressif.com/en/contact-us/sales-questions>`_ to provide the actual range and accuracy according to batch.
- For boards with the PW number of and after PW-2023-07-0440 (on and after July 2023), to optimize the WS2812 driving circuit, the resistance of R29 is updated from 4.7 kΩ to 10 kΩ, and the resistance of R6 is updated from 10 kΩ to 3.3 kΩ. For details, see `ESP32-C6-DevKitC-1 Schematic v1.3`_.
- For boards with the PW number of and after PW-2024-03-0595 and PW-2024-03-0921 (on and after March 2024), to optimize the circuit, the resistance of R7 on UART_RXD is updated from 0 Ω to 470 Ω. For details, see `ESP32-C6-DevKitC-1 Schematic v1.4`_.

.. note::

  The PW number can be found in the product label on the large cardboard boxes for wholesale orders.

ESP32-C6-DevKitC-1 v1.1
-----------------------

:doc:`Initial release <user_guide_v1.1>`

Related Documents
=================
* `ESP32-C6 Datasheet`_ (PDF)
* `ESP32-C6-WROOM-1 Datasheet`_ (PDF)
* `ESP32-C6-DevKitC-1 Schematic v1.4`_ (PDF) - Applies to boards of and after PW-2024-03-0595 and PW-2024-03-0921
* `ESP32-C6-DevKitC-1 Schematic v1.3`_ (PDF) - Applies to boards of and after PW-2023-07-0440
* `ESP32-C6-DevKitC-1 Schematic v1.2`_ (PDF) - Applies to boards before PW-2023-07-0440
* `ESP32-C6-DevKitC-1 PCB Layout <../../_static/esp32-c6-devkitc-1/schematics/esp32-c6-devkitc-1-pcb-layout_v1.2.pdf>`_ (PDF)
* `ESP32-C6-DevKitC-1 Dimensions <../../_static/esp32-c6-devkitc-1/schematics/esp32-c6-devkitc-1-dimensions_v1.2.pdf>`_ (PDF)
* `ESP32-C6-DevKitC-1 Dimensions source file <../../_static/esp32-c6-devkitc-1/schematics/esp32-c6-devkitc-1-dimensions_v1.2.dxf>`_ (DXF) - You can view it with `Autodesk Viewer <https://viewer.autodesk.com/>`_ online

For further design documentation for the board, please contact us at `sales@espressif.com <sales@espressif.com>`_.

.. _ESP32-C6 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32-c6_datasheet_en.pdf
.. _ESP32-C6-WROOM-1 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32-c6-wroom-1_datasheet_en.pdf
.. _ESP32-C6-DevKitC-1 Schematic v1.4: ../../_static/esp32-c6-devkitc-1/schematics/esp32-c6-devkitc-1-schematics_v1.4.pdf
.. _ESP32-C6-DevKitC-1 Schematic v1.3: ../../_static/esp32-c6-devkitc-1/schematics/esp32-c6-devkitc-1-schematics_v1.3.pdf
.. _ESP32-C6-DevKitC-1 Schematic v1.2: ../../_static/esp32-c6-devkitc-1/schematics/esp32-c6-devkitc-1-schematics_v1.2.pdf

.. toctree::
    :hidden:

    user_guide_v1.1
    