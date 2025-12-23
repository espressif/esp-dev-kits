==========
ESP-Prog-2
==========

:link_to_translation:`zh_CN:[中文]`

This user guide helps you get started with ESP-Prog-2, and provides a detailed introduction to its features and capabilities.

ESP-Prog-2 is a development and debugging tool released by Espressif. It supports function such as automatic firmware downloading, serial communication, and JTAG online debugging. Automatic downloading and serial communication are compatible with ESP8266, ESP32, ESP32-S2, ESP32-S3, and ESP32-C3, while JTAG online debugging is supported on ESP32, ESP32-S2, ESP32-S3, and ESP32-C3.

ESP-Prog-2 is simple in design and easy to use—only one USB cable is needed to connect it to a computer. The computer automatically recognizes two ports, one for downloading and the other for JTAG debugging, based on the port numbers.

Considering that different target boards may operate at different voltages, both ESP-Prog-2 interfaces support either 5 V or 3.3 V power supply voltage. The voltage can be selected via jumper pins, providing compatibility with a variety of boards. However, while the power supply can switch between 3.3 V and 5 V, the RX/TX and JTAG signal levels are fixed at 3.3 V.

.. figure:: ../../_static/esp-prog-2/esp-prog-2.png
    :align: center
    :scale: 25%
    :alt: ESP-Prog-2

    ESP-Prog-2 (click to enlarge)

The document consists of the following content:

- `Getting started`_: Board overview, and hardware/software setup instructions to get started.
- `Hardware Reference`_: More detailed information about the board's hardware.
- `Related Documents`_: Links to related documentation.


Getting Started
===============

This section introduces how to set up ESP-Prog-2 hardware for the first time.


Description of Components
-------------------------

.. figure:: ../../_static/esp-prog-2/prog-2-modules.png
    :align: center
    :scale: 50%
    :alt: ESP-Prog-2 - front

    ESP-Prog-2 - front (click to enlarge)

The key components of the board are described in clockwise order.

.. list-table:: Key Components on ESP-Prog-2
   :widths: 25 75
   :header-rows: 1

   * - Key Component
     - Description
   * - ESP32-S3-MINI-1
     - The ESP32-S3-MINI-1 is a versatile Wi-Fi and Bluetooth Low Energy (BLE) MCU module. It features a variety of peripheral interfaces and comes with a PCB-mounted antenna. The core of the module is the ESP32-S3 chip.
   * - Reset Button
     - Used to restart the target system being debugged.
   * - Boot Button
     - Target system download button. Press and hold the **Boot** button, then briefly press the **Reset** button to enter the Firmware Download mode, allowing you to download the firmware to the target system via the serial port for debugging. It can be used as a confirmation button during normal operation.
   * - Boot On/Off
     - Header pin used to configure the status of the IO0 Strapping pin.
   * - PROG PWR SEL
     - Header pin used to select the program interface power supply input voltage (3.3 V or 5 V).
   * - PROG 2.54 mm
     - Program interface with 2.54 mm (0.1") pin spacing.
   * - PROG 1.27 mm
     - Program interface with 1.27 mm (0.05") pin spacing.
   * - USB Type-C Port
     - The USB Type-C interface on the ESP32-S3 chip supports USB 2.0 Full-Speed mode, with a data transfer rate of up to 12 Mbps (Note that the interface does not support 480 Mbps high-speed transfer mode). This interface can be used as a power supply interface for the board, and supports communication with ESP32-S3 via USB protocol.
   * - JTAG 1.27 mm
     - JTAG interface with 1.27 mm (0.05") pin spacing.
   * - JTAG 2.54 mm
     - JTAG interface with 2.54 mm (0.1") pin spacing.
   * - JTAG PWR SEL
     - Header pin used to select the JTAG interface power input voltage (3.3 V or 5 V).
   * - LED indicator
     - Displays the status of ESP-Prog-2. There are three LED modes: red, green, and blue.  The red LED is on when the system’s 3.3 V power is active; the green LED is on when the automatic firmware downloading and serial communication functions are in use; the blue LED is on when the JTAG online debugging function is active.
   * - ESP32-S3 Boot Button
     - The ESP32-S3 download button for ESP32-S3 firmware downloading. To enter firmware download mode, power on the ESP-Prog-2 again while pressing the ESP32-S3 Boot button.
   * - Extension Connector
     - Extension IO connector used to bring out the idle IOs of ESP32-S3. You can customize pins to extend the additional functionality of ESP-Prog-2.


Start Application Development
-----------------------------

Before powering on, ensure the board is in good condition.

Required Hardware
^^^^^^^^^^^^^^^^^

- ESP-Prog-2
- USB-A to USB-C conversion cable
- A computer (Windows, Linux, or macOS)
- Dupont wires or the ribbon cable provided by Espressif to connect the development board and ESP-Prog-2

.. note::

    Be sure to use an appropriate USB cable. Some cables can only be used for charging, and cannot be used for data transmission or programming.

Hardware Setup
^^^^^^^^^^^^^^

1. Connect the ESP-Prog-2 board to the PC USB port using a USB cable.
2. Use the PROG PWR SEL or JTAG PWR SEL header pin to select the power supply output voltage for the Program interface or JTAG interface.
3. Use the ribbon cable provided by Espressif to connect the ESP-Prog-2 debugging board to the Espressif target board.
4. Use the Espressif official software tools or scripts to enable automatic downloading and JTAG debugging functions.

Firmware Installation
^^^^^^^^^^^^^^^^^^^^^

Before starting software development, it is recommended to update the built-in factory firmware of the ESP-Prog-2. This ensures the device run the latest features and bug fixes.

1. **Update firmware using online tool**: Update ESP-Prog-2 firmware via Espressif's online flashing tool `ESP Launchpad <https://espressif.github.io/esp-launchpad/?flashConfigURL=https://espressif.github.io/esp-usb-bridge/launchpad.toml>`__ following the steps below:

  a. Use a Chromium-based browser, such as Chrome.
  b. **Press and hold** the BOOT button closest to the module, and then connect the USB cable to put ESP-Prog-2 into firmware download mode.
  c. Open the  `ESP Launchpad <https://espressif.github.io/esp-launchpad/?flashConfigURL=https://espressif.github.io/esp-usb-bridge/launchpad.toml>`__ page in your browser, and follow the instructions on the screen to connect and flash the firmware.
  
2. **Obtain the firmware source code**: To understand the firmware implementation, check the firmware source code of ESP-Prog-2 at `esp-usb-bridge <https://github.com/espressif/esp-usb-bridge>`__.

Software Setup
^^^^^^^^^^^^^^

To set up the development environment, please refer to Section `Installation <https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html#get-started-step-by-step>`__ in `Get Started with ESP-IDF <https://idf.espressif.com/index.html>`__.


Contents and Packaging
----------------------

Retail Orders
^^^^^^^^^^^^^

Each ESP-Prog-2 board comes in an individual package:

.. figure:: ../../_static/esp-prog-2/esp-prog-2-package.png
   :align: center
   :scale: 20%
   :alt: ESP-Prog-2 Package Contents

   ESP-Prog-2 Package Contents (click to enlarge)

The package contents are as follows:

- Development board

  ESP-Prog-2

- Data cable

  One cable is used for the PROG 2.54 mm interface, connecting the 2x5-PIN header.
  The other cable is used for the PROG 1.27 mm interface, connecting the 2x3-PIN header.

If you order a few samples, each board comes in an individual package in either antistatic bag or any packaging depending on your retailer.

For retail orders, please go to `Get Samples <https://www.espressif.com/en/company/contact/buy-a-sample>`_.


Wholesale Orders
^^^^^^^^^^^^^^^^

If you order in bulk, the boards come in large cardboard boxes.

For wholesale orders, please go to `Contact Sales <https://www.espressif.com/en/contact-us/sales-questions>`_.


Hardware Reference
==================

Block Diagram
-------------

The block diagram below shows the components of ESP-Prog-2 and their interconnections.

.. figure:: ../../_static/esp-prog-2/esp-prog-2-block.png
    :align: center
    :scale: 50%
    :alt: ESP-Prog-2 Block Diagram

    ESP-Prog-2 Block Diagram (click to enlarge)


Power Supply Options
^^^^^^^^^^^^^^^^^^^^

The development board has three incompatible power supply options:

- Default power supply via the ESP-Prog-2 USB port (recommended)
- Power supply via the 5 V and G (GND) pins
- Power supply via the 3.3 V and G (GND) pins


Header Block
-------------

The tables below provide the **Name** and **Function** of the pins on both sides of the board (Program Interface and JTAG Interface), and on the expansion connector. The pin names are shown in the front view of the ESP-Prog-2 board. The numbering is the same as in the `ESP-Prog-2 Schematic <https://dl.espressif.com/schematics/SCH_ESP32-PROG-2_20250403.pdf>`__ (PDF).

Program Interface
^^^^^^^^^^^^^^^^^^

.. list-table::
   :widths: 30 35 35
   :header-rows: 1

   * - No.
     - Name
     - Function
   * - 1
     - ESP_EN
     - Enable signal
   * - 2
     - VDD
     - Power supply
   * - 3
     - ESP_TXD
     - TX pin
   * - 4
     - GND
     - Ground
   * - 5
     - ESP_RXD
     - RX pin
   * - 6
     - ESP_IO0
     - Strapping pin


JTAG Interface
^^^^^^^^^^^^^^^

.. list-table::
   :widths: 30 30 40
   :header-rows: 1

   * - Number
     - Name
     - Function
   * - 1
     - VDD
     - Power supply
   * - 2
     - ESP_TMS
     - JTAG TMS pin, mode selection
   * - 3
     - GND
     - Ground
   * - 4
     - ESP_TCK
     - JTAG TCK pin, clock input
   * - 5
     - GND
     - Ground
   * - 6
     - ESP_TDO
     - JTAG TDO pin
   * - 7
     - GND
     - Ground
   * - 8
     - ESP_TDI
     - JTAG TDI pin
   * - 9
     - GND
     - Ground
   * - 10
     - NC
     - No connection

Extend Connector
^^^^^^^^^^^^^^^^

.. list-table::
   :widths: 30 40 40
   :header-rows: 1

   * - Number
     - Name
     - Function
   * - 1
     - I2C_SDA/OPTN_IO
     - Configurable IO
   * - 2
     - I2C_SCL/OPTN_IO
     - Configurable IO
   * - 3
     - GND
     - Ground
   * - 4
     - SPI_HD/OPTN_IO
     - Configurable IO
   * - 5
     - SPI_D/OPTN_IO
     - Configurable IO
   * - 6
     - SPI_CS/OPTN_IO
     - Configurable IO
   * - 7
     - GND
     - Ground
   * - 8
     - SPI_CLK/OPTN_IO
     - Configurable IO
   * - 9
     - SPI_WD/OPTN_IO
     - Configurable IO
   * - 10
     - SPI_Q/OPTN_IO
     - Configurable IO
   * - 11
     - VCC_5V
     - 5 V Power supply
   * - 12
     - VCC_3V3
     - 3V3 Power supply


Related Documents
=================

* `ESP-Prog-2 Schematic`_ (PDF)
* `ESP-Prog-2 PCB Layout`_ (PDF)
* `ESP-Prog-2 Dimensions`_ (PDF)

.. _ESP-Prog-2 Schematic: https://dl.espressif.com/schematics/SCH_ESP32-PROG-2_20250403.pdf
.. _ESP-Prog-2 PCB Layout: https://dl.espressif.com/schematics/PCB_ESP32-PROG-2_20250403.pdf
.. _ESP-Prog-2 Dimensions: https://dl.espressif.com/schematics/DIM_ESP32-PROG-2_20250403.pdf

For further design documentation for the board, please contact us at `sales@espressif.com <sales@espressif.com>`_.
