============
ESP-DualKey
============

:link_to_translation:`zh_CN:[中文]`

.. note::

  Please check the silkscreen version number on the main board to confirm your development board version.

This guide will help you get started with ESP-DualKey quickly and provide detailed information about this development board.

ESP-DualKey is an intelligent dual-key controller development board developed by Espressif based on the ESP32-S3 chip, integrating smart home control, Bluetooth keyboard, USB keyboard and other functions. It can switch between different working modes through physical switches. The development board is equipped with dual buttons, RGB lighting effects, battery management, power monitoring and other functions, providing developers with a complete intelligent button solution.

The ESP-DualKey main controller uses the Espressif ESP32-S3 chip, supporting 2.4 GHz Wi-Fi and Bluetooth 5 (LE) wireless connectivity. In terms of storage, the entire device has 8 MB Flash storage space to meet the needs of intelligent button applications. It is equipped with a dual-button matrix and WS2812 RGB lighting effects, providing an intuitive and rich interactive experience.

In terms of power supply, ESP-DualKey supports USB-C power supply and built-in lithium battery power supply, integrating battery management chips and power monitoring functions. In addition, it also integrates a USB-C interface that supports power supply and programming download, while providing multiple working mode switching functions.

.. figure:: ../../_static/esp-dualkey/esp-dualkey-front.jpg
   :alt: ESP-DualKey Front View (Click to enlarge)
   :scale: 30%
   :figclass: align-center

   ESP-DualKey Front View (Click to enlarge)

This guide includes the following content:

- `Getting Started`_: Briefly introduces the development board and hardware, software setup guide.
- `Hardware Reference`_: Detailed introduction to the development board hardware.
- `Hardware Versions`_: Introduces hardware historical versions and known issues (if any).
- `Related Documents`_: Lists links to related documents.

.. note::

  For instructions on using the factory firmware, please refer to: 'ESP-DualKey User Guide <https://espressif.craft.me/DiTh53ddFXD94L>'


.. _Getting-started_esp_dualkey:

Getting Started
===============

This section will briefly introduce ESP-DualKey and explain how to flash firmware on ESP-DualKey and related preparations.

Component Introduction
----------------------

The following introduces the main components on the PCB in clockwise order. For user convenience, we have also marked these components or interfaces on the ESP-DualKey housing.

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Main Component
     - Description
   * - :strong:`Main Controller`
     -
   * - ESP32-S3
     - Main controller chip supporting 2.4 GHz Wi-Fi and Bluetooth 5 (LE) connectivity, and featuring 8 MB of integrated Flash storage.
   * - :strong:`Interaction`
     -
   * - Dual Buttons
     - 2x1 button matrix, supports independent key detection and combination key functions.
   * - Mode Switch
     - Three-position switch for toggling between operating modes (Bluetooth keyboard / Wi-Fi Mode / USB keyboard).
   * - :strong:`RGB Lighting`
     -
   * - WS2812 RGB LED
     - Programmable RGB LED supporting diverse lighting effects, e.g., heat map, breathing effect, flowing effect.
   * - :strong:`Power Management`
     -
   * - Lithium Battery Charging Chip
     - TP4057 lithium battery charging management, supports USB-C charging.
   * - :strong:`Interfaces`
     -
   * - USB-C Port
     - USB-C for power, programming/download, debugging, and battery charging.
   * - 2 × HY2.0-4P Ports
     - Support auxiliary power output for connecting external sensors.
   * - :strong:`Other Functions`
     -
   * - Battery Voltage Monitoring
     - Real-time battery voltage and charging status monitoring.
   * - VBUS Monitoring
     - Monitors USB power status.
   * - Deep Sleep
     - Supports deep sleep mode for energy saving.

Start Developing Applications
-----------------------------

Required Hardware
^^^^^^^^^^^^^^^^^

- ESP-DualKey
- USB data cable
- Computer (Windows, Linux or macOS)

.. note::

  Please ensure you use an appropriate USB data cable. Some cables can only be used for charging and cannot be used for data transmission and programming.

Hardware Setup
^^^^^^^^^^^^^^

Connect ESP-DualKey to your computer using a USB data cable through the ``USB-C interface`` for firmware flashing, debugging and power supply.

Software Setup
^^^^^^^^^^^^^^

Please refer to the `ESP-IDF Quick Start <https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/index.html>`__ section to see how to quickly set up the development environment and flash applications to your development board.

.. note::

  The development board uses USB ports to communicate with the computer. Most operating systems (Windows, Linux, macOS) come with the required drivers pre-installed, and the development board can be automatically recognized after insertion. If the device cannot be recognized or a serial connection cannot be established, please refer to `How to Establish Serial Connection <https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/establish-serial-connection.html>`__ for detailed steps on driver installation.

Espressif provides Board Support Packages (BSP) for various development boards, which can help you more easily and efficiently initialize and use the main peripherals on the board, such as buttons, LEDs and power management. Please visit `esp-bsp <https://github.com/espressif/esp-bsp>`__ to check all supported development boards.

Development board application examples are stored in the :project:`Examples <examples/esp-dualkey/examples>`.

.. _Hardware-reference_esp_dualkey:

Hardware Reference
==================

Functional Block Diagram
------------------------

The main components and connection methods of ESP-DualKey are shown in the following figure.

.. figure:: ../../_static/esp-dualkey/esp-dualkey-block-diagram.png
   :alt: ESP-DualKey Functional Block Diagram (Click to enlarge)
   :scale: 60%
   :figclass: align-center

   ESP-DualKey Functional Block Diagram (Click to enlarge)

Power Options
-------------

The development board can be powered by the following methods:

1. Through ``USB-C interface`` power supply

   When using this method for power supply, use a USB Type-C data cable to connect to the USB-C interface on the device.

2. Through ``built-in lithium battery`` power supply

  The device has an integrated lithium battery inside, supporting independent power supply operation.

**Button Matrix Configuration:**

* 2x1 matrix configuration
* Supports independent button detection
* Supports combination button functions

**RGB LED Features:**

* WS2812 programmable RGB LED
* Supports multiple lighting effect modes
* Supports brightness, hue, and saturation adjustment

Mode Switch
-----------

.. figure:: ../../_static/esp-dualkey/esp-dualkey-switch-schematic.png
   :alt: Mode Switch Circuit Diagram (Click to enlarge)
   :scale: 50%
   :figclass: align-center

   Mode Switch Circuit Diagram (Click to enlarge)

**Switch Position Description:**

* **Left Position**: Bluetooth keyboard mode
* **Middle Position**: USB keyboard mode
* **Right Position**: Smart home mode (Rainmaker)

Hardware Versions
=================

ESP-DualKey 
--------------------------

- :doc:`First Release <user_guide>`

.. _Related-documents_esp_dualkey:

Related Documents
=================

-  `ESP-DualKey Schematic`_ (PDF)
-  `User Guide`_ (HTML)

.. _ESP-DualKey Schematic:  https://dl.espressif.com/AE/esp-dev-kits/SCH-ESP-Dualkey-MainBoard-V1_2.pdf
.. _User Guide: https://espressif.craft.me/DiTh53ddFXD94L
