===================
ESP32-H2-DevKitM-1
===================

:link_to_translation:`zh_CN:[中文]`

This user guide will help you get started with ESP32-H2-DevKitM-1 and will also provide more in-depth information.

ESP32-H2-DevKitM-1 is an entry-level development board based on Bluetooth® Low Energy and IEEE 802.15.4 combo module ESP32-H2-MINI-1 or ESP32-H2-MINI-1U.

Most of the I/O pins on the ESP32-H2-MINI-1/1U module are broken out to the pin headers on both sides of this board for easy interfacing. Developers can either connect peripherals with jumper wires or mount ESP32-H2-DevKitM-1 on a breadboard.

.. figure:: ../../../_static/esp32-h2-devkitm-1/esp32-h2-devkitm-1-45.png
    :align: center
    :alt: ESP32-H2-DevKitM-1 (with ESP32-H2-MINI-1 on board)

    ESP32-H2-DevKitM-1 (with ESP32-H2-MINI-1 on board)

The document consists of the following major sections:

- `Getting Started`_: Overview of ESP32-H2-DevKitM-1 and hardware/software setup instructions to get started.
- `Hardware Reference`_: More detailed information about the ESP32-H2-DevKitM-1's hardware.
- `Hardware Revision Details`_: Revision history, known issues, and links to user guides for previous versions (if any) of ESP32-H2-DevKitM-1.
- `Related Documents`_: Links to related documentation.


Getting Started
===============

This section provides a brief introduction of ESP32-H2-DevKitM-1, instructions on how to do the initial hardware setup and how to flash firmware onto it.


Description of Components
-------------------------

.. _user-guide-H2-devkitm-1-board-front:

.. figure:: ../../../_static/esp32-h2-devkitm-1/esp32-h2-devkitm-1_v1.2_callouts.png
    :align: center
    :alt: ESP32-H2-DevKitM-1 - Front

    ESP32-H2-DevKitM-1 - Front

The description of components starts from the ESP32-H2-MINI-1/1U module on the left side and then goes clockwise.

.. list-table::
   :widths: 20 80
   :header-rows: 1

   * - Key Component
     - Description
   * - ESP32-H2-MINI-1 or ESP32-H2-MINI-1U
     - ESP32-H2-MINI-1/1U, with ESP32-H2 inside which integrates a 2.4 GHz transceiver compliant with Bluetooth ® Low Energy and IEEE 802.15.4-based technologies, supporting Bluetooth 5 (LE), Bluetooth mesh, Thread, Matter, and Zigbee. This module is specially designed for all kinds of low-power IoT applications.
   * - Pin Headers
     - All available GPIO pins (except for the SPI bus for flash) are broken out to the pin headers on the board. For details, please see `Header Block`_.
   * - 3.3 V Power On LED
     - Turns on when the USB power is connected to the board.
   * - 5 V to 3.3 V LDO
     - Power regulator that converts a 5 V supply into a 3.3 V output.
   * - USB-to-UART Bridge
     - Single USB-UART bridge chip provides transfer rates up to 3 Mbps.
   * - ESP32-H2 USB Type-C Port
     - The USB Type-C port on the ESP32-H2 chip compliant with USB 2.0 full speed. It is capable of up to 12 Mbps transfer speed (Note that this port does not support the faster 480 Mbps high-speed transfer mode).
   * - Boot Button
     - Download button. Holding down **Boot** and then pressing **Reset** initiates Firmware Download mode for downloading firmware through the serial port.
   * - Reset Button
     - Press this button to restart the system.
   * - USB Type-C to UART Port
     - Power supply for the board as well as the communication interface between a computer and the ESP32-H2 chip via USB-to-UART bridge.
   * - RGB LED
     - Addressable RGB LED, driven by GPIO8.
   * - J5
     - Used for current measurement. See details in Section :ref:`user-guide-h2-devkitm-1-current`.
   * - 32.768 kHz Crystal [A]_
     - ESP32-H2 supports an external 32.768 kHz crystal to act as the Low-Power clock. This external Low-Power clock source enhances timing accuracy and consequently decreases average power consumption, without impacting functionality.   

.. [A] Boards with the PW number before PW-2024-02-0362 (before February 2024) does not populate a 32.768 kHz crystal by default.

Start Application Development
-----------------------------

Before powering up your ESP32-H2-DevKitM-1, please make sure that it is in good condition with no obvious signs of damage.


Required Hardware
^^^^^^^^^^^^^^^^^

- ESP32-H2-DevKitM-1
- USB-A to USB-C (Type C) cable
- Computer running Windows, Linux, or macOS

.. note::

  Some USB cables can only be used for charging, not data transmission and programming. Please choose accordingly.


Software Setup
^^^^^^^^^^^^^^

Please proceed to `Get Started <https://docs.espressif.com/projects/esp-idf/en/latest/esp32h2/get-started/index.html>`_, where Section `Installation <https://docs.espressif.com/projects/esp-idf/en/latest/esp32h2/get-started/index.html#get-started-step-by-step>`_ will quickly help you set up the development environment and then flash an application example onto your ESP32-H2-DevKitM-1.


Contents and Packaging
----------------------

Ordering Information
^^^^^^^^^^^^^^^^^^^^

The development board has a variety of variants to choose from, as shown in the table below.

.. list-table::
   :header-rows: 1
   :widths: 41 24 9 8 18

   * - Ordering Code
     - On-board Module
     - Flash [B]_
     - PSRAM
     - Antenna
   * - ESP32-H2-DevKitM-1-N4
     - ESP32-H2-MINI-1
     - 4 MB
     - 0 MB
     - PCB on-board antenna
   * - ESP32-H2-DevKitM-1U-N4
     - ESP32-H2-MINI-1U
     - 4 MB
     - 0 MB
     - External antenna connector

.. [B] The flash is integrated in the chip's package.

Retail Orders
^^^^^^^^^^^^^

If you order one or several samples, each ESP32-H2-DevKitM-1 comes in an individual package in either antistatic bag or any packaging depending on your retailer.

For retail orders, please go to https://www.espressif.com/en/company/contact/buy-a-sample.


Wholesale Orders
^^^^^^^^^^^^^^^^

If you order in bulk, the boards come in large cardboard boxes.

For wholesale orders, please go to https://www.espressif.com/en/contact-us/sales-questions.


Hardware Reference
==================

Block Diagram
-------------

The block diagram below shows the components of ESP32-H2-DevKitM-1 and their interconnections.

.. figure:: ../../../_static/esp32-h2-devkitm-1/esp32-h2-devkitm-1_v1.0_systemblock.png
    :align: center
    :alt: ESP32-H2-DevKitM-1
    :width: 650

    ESP32-H2-DevKitM-1


Power Supply Options
^^^^^^^^^^^^^^^^^^^^

There are three mutually exclusive ways to provide power to the board:

- USB Type-C to UART port, default power supply
- 5V and GND pin headers
- 3V3 and GND pin headers

.. _user-guide-h2-devkitm-1-current:

Current Measurement
-------------------

The J5 headers on ESP32-H2-DevKitM-1 (see J5 in Figure :ref:`user-guide-H2-devkitm-1-board-front`) can be used for measuring the current drawn by the ESP32-H2-MINI-1/1U module:

- Remove the jumper: Power supply between the module and peripherals on the board is cut off. To measure the module's current, connect the board with an ammeter via J5 headers.
- Apply the jumper (factory default): Restore the board's normal functionality.

.. note::

  When using 3V3 and GND pin headers to power the board, please remove the J5 jumper, and connect an ammeter in series to the external circuit to measure the module's current.

Header Block
------------

The two tables below provide the **Name** and **Function** of the pin headers on both sides of the board (J1 and J3). The pin header names are shown in `Pin Layout`_. The numbering is the same as in the ESP32-H2-DevKitM-1 Schematic. (see attached PDF).

J1
^^^
====  ====  ==========  ===============================================================================
No.   Name  Type [1]_   Function
====  ====  ==========  ===============================================================================
1     3V3     P         3.3 V power supply
2     RST     I         High: enables the chip; Low: the chip powers off; connected to the internal pull-up resistor by default
3     0       I/O/T     GPIO0, FSPIQ
4     1       I/O/T     GPIO1, FSPICS0, ADC1_CH0
5     2       I/O/T     GPIO2, FSPIWP, ADC1_CH1, MTMS
6     3       I/O/T     GPIO3, FSPIHD, ADC1_CH2, MTDO
7     13/N    I/O/T     GPIO13, XTAL_32K_P [2]_
8     14/N    I/O/T     GPIO14, XTAL_32K_N [3]_
9     4       I/O/T     GPIO4, FSPICLK, ADC1_CH3, MTCK
10    5       I/O/T     GPIO5, FSPID, ADC1_CH4, MTDI
11    NC      --        NC
12    VBAT    P         3.3 V power supply or battery
13    G       P         Ground
14    5V      P         5 V power supply
15    G       P         Ground
====  ====  ==========  ===============================================================================


J3
^^^
====  ====  ==========  ================================
No.   Name  Type [1]_   Function
====  ====  ==========  ================================
1     G     P           Ground
2     TX    I/O/T       GPIO24, FSPICS2, U0TXD
3     RX    I/O/T       GPIO23, FSPICS1, U0RXD
4     10    I/O/T       GPIO10, ZCD0
5     11    I/O/T       GPIO11, ZCD1
6     25    I/O/T       GPIO25, FSPICS3
7     12    I/O/T       GPIO12
8     8     I/O/T       GPIO8 [4]_, LOG
9     22    I/O/T       GPIO22
10    G     P           Ground
11    9     I/O/T       GPIO9, BOOT
12    G     P           Ground
13    27    I/O/T       GPIO27, FSPICS5, USB_D+
14    26    I/O/T       GPIO26, FSPICS4, USB_D-
15    G     P           Ground
====  ====  ==========  ================================

.. [1] P: Power supply; I: Input; O: Output; T: High impedance.
.. [2] When connected to XTAL_32K_P inside the module, this pin cannot be used for other purpose.
.. [3] When connected to XTAL_32K_N inside the module, this pin cannot be used for other purpose.
.. [4] Used for driving RGB LED inside the module.

For more information about pin description, please see `ESP32-H2 Datasheet`_.


Pin Layout
^^^^^^^^^^^

.. figure:: ../../../_static/esp32-h2-devkitm-1/esp32-h2-devkitm-1-v1.2_pinlayout.png
    :align: center
    :scale: 42%
    :alt: ESP32-H2-DevKitM-1

    ESP32-H2-DevKitM-1 Pin Layout


Hardware Revision Details
=========================

- For boards with the PW number of and after PW-2024-02-0362 (on and after February 2024), the 32.768 kHz crystal is populated by default, while the series resistor connected here to the surrounding pins is updated to not populated. To optimize the circuit, the series resistor R7 on the UART_RXD has been updated to 470 Ω.

.. note::

  The PW number can be found in the product label on the large cardboard boxes for wholesale orders.

Related Documents
=================

- `ESP32-H2 Datasheet <https://www.espressif.com/sites/default/files/documentation/esp32-h2_datasheet_en.pdf>`_ (PDF)
- `ESP32-H2-MINI-1/1U Datasheet <https://www.espressif.com/sites/default/files/documentation/esp32-h2-mini-1_mini-1u_datasheet_en.pdf>`_ (PDF)
- `ESP32-H2-DevKitM-1 Schematics v1.3 (Applies to boards of and after PW-2024-02-0362) <../../_static/esp32-h2-devkitm-1/esp32-h2-devkitm-1_v1.3_schematics.pdf>`_ (PDF)
- `ESP32-H2-DevKitM-1 Schematics v1.2 (Applies to boards before PW-2024-02-0362) <../../_static/esp32-h2-devkitm-1/esp32-h2-devkitm-1_v1.2_schematics.pdf>`_ (PDF)
- `ESP32-H2-DevKitM-1 PCB Layout <../../_static/esp32-h2-devkitm-1/esp32-h2-devkitm-1_v1.2_pcb_layout.pdf>`_ (PDF)
- `ESP32-H2-DevKitM-1 Dimensions <../../_static/esp32-h2-devkitm-1/esp32-h2-devkitm-1_v1.2_dimension.pdf>`_ (PDF)
- `ESP32-H2-DevKitM-1 Dimensions source file <../../_static/esp32-h2-devkitm-1/esp32-h2-devkitm-1_v1.2_dimension.dxf>`_ (DXF)

For further design documentation for the board, please contact us at `sales@espressif.com <sales@espressif.com>`_.