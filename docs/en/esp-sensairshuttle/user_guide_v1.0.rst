==========================
ESP-SensairShuttle v1.0
==========================

:link_to_translation:`zh_CN:[中文]`

.. note::

  Please check the silkscreen version number on the mainboard (in the white circle at the top right corner of the front or back of the mainboard) to confirm your development board version. For v1.0 development boards, please refer to this user guide.

This guide will help you get started with ESP-SensairShuttle quickly and provide detailed information about this development board.

**ESP-SensairShuttle** is a development board jointly launched by Espressif and **Bosch Sensortec** for **motion sensing** and **large language model human-computer interaction** scenarios, dedicated to promoting the deep integration of multimodal sensing and intelligent interaction technologies. The platform covers typical application scenarios such as **AI toys, smart homes, sports health, and smart offices**, supporting a complete technical chain from environmental sensing, behavior understanding to intelligent feedback, providing a more natural, real-time, and intelligent interaction experience for next-generation intelligent terminals.

ESP-SensairShuttle uses Espressif's **ESP32-C5-WROOM-1-N16R8** module as the main controller, featuring dual-band Wi-Fi 6 (802.11ax) at 2.4 & 5 GHz, Bluetooth® 5 (LE), Zigbee, and Thread (802.15.4) wireless communication capabilities. In addition, the mainboard provides rich peripheral interfaces, including `Bosch Sensortec Shuttle Board <https://www.digikey.sg/en/products/filter/evaluation-boards/expansion-boards-daughter-cards/797?s=N4IgjCBcoLQdIDGUBmBDANgZwKYBoQB7KAbRAA4AmckAXQF96DLSQsALAVwBduMcABACNCaAE4ATAQGYAdAAY6BAKxRQAByhgC6zZErLGQA>`_ (only supports shuttle board 3.0 version) interface, **microphone and speaker interfaces**, and **battery power interface**. Users can flexibly achieve multi-dimensional sensing such as **air quality, gesture actions, attitude direction, and magnetic field information** by replacing different Shuttle sensor daughterboards (Espressif officially supports **BME690** and **BMI270 & BMM350** daughterboards), suitable for teaching demonstrations, algorithm verification, and multi-scenario prototype development.

In terms of audio, ESP-SensairShuttle supports external microphones and speakers, which can seamlessly connect to various **large language models** to achieve natural and smooth AI voice interaction capabilities, suitable for voice interaction products that require large model empowerment such as **AI toys, smart speakers, and smart control panels**.

This guide includes the following content:

- `Getting Started`_: Briefly introduces the development board and hardware and software setup guides.
- `Hardware Reference`_: Details the hardware of the development board.
- `Hardware Revision History`_: Introduces hardware revision history and known issues (if any).
- `Related Documents`_: Lists links to related documents.

.. _Getting-started_esp-sensairshuttle:

Getting Started
===============

This section will briefly introduce ESP-SensairShuttle and explain how to flash firmware on ESP-SensairShuttle and related preparation work.

Component Overview
------------------
.. figure:: ../../_static/esp-sensairshuttle/esp-sensairshuttle-mainboard-front.png
   :alt: SensairShuttle-Mainboard PCB Front View (Click to enlarge)
   :scale: 70%
   :figclass: align-center

   SensairShuttle-Mainboard PCB Front View (Click to enlarge)

.. figure:: ../../_static/esp-sensairshuttle/esp-sensairshuttle-bme690-front.png
   :alt: ShuttleBoard-BME690 PCB Front View (Click to enlarge)
   :scale: 60%
   :figclass: align-center

   ShuttleBoard-BME690 PCB Front View (Click to enlarge)

.. figure:: ../../_static/esp-sensairshuttle/esp-sensairshuttle-bmi270&bmm350-front.png
   :alt: ShuttleBoard-BMI270&BMM350 PCB Front View (Click to enlarge)
   :scale: 60%
   :figclass: align-center

   ShuttleBoard-BMI270&BMM350 PCB Front View (Click to enlarge)

The main components on the front PCB are introduced in clockwise order below.

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Main Component
     - Description
   * - :strong:`MainBoard`
     -
   * - External Pin Interface
     - 4-pin external pin interface, from top to bottom: GPIO5, GPIO4, VDD, GND. Note: GPIO5 is not available by default. To use it, please install the R14 resistor.
   * - I2C Interface (External I2C Interface)
     - 4-pin external I2C interface that can connect to devices supporting I2C protocol communication.
   * - RGB Interface (External RGB Strip Interface)
     - 3-pin external RGB strip interface that can connect to WS2812 and other RGB strips.
   * - ESP32-C5-WROOM-1-N16R8
     - Main control module, integrated with 16 MB Flash and 8 MB PSRAM, featuring dual-band Wi-Fi 6 (802.11ax) at 2.4 & 5 GHz, Bluetooth® 5 (LE), Zigbee, and Thread (802.15.4) wireless communication capabilities.
   * - LCD Connector
     - Used to connect LCD screen with a resolution of 284 x 240.
   * - Boot Button
     - Used to manually enter download mode, can also be used as a regular function button.
   * - Power Indicator LED
     - Used to indicate device power status. For indicator status details, please refer to the `Power Options`_ section.
   * - Power Switch
     - Used to control device power on/off. Click the power switch to toggle the power state.
   * - :strong:`ShuttleBoard-BME690 (BME690 Sensor Daughterboard)`
     -
   * - BME690 Sensor
     - Bosch BME690 gas sensor that can detect air quality, including temperature, humidity, pressure, and gas resistance, supporting I2C and SPI protocol communication.
   * - :strong:`ShuttleBoard-BMI270&BMM350 (BMI270&BMM350 Sensor Daughterboard)`
     -
   * - BMI270 Sensor
     - Bosch BMI270 inertial measurement unit that can detect three-axis acceleration and three-axis angular velocity, supporting I2C and SPI protocol communication.
   * - BMM350 Sensor
     - Bosch BMM350 magnetometer that can detect three-axis magnetic field strength, supporting I2C protocol communication.

.. figure:: ../../_static/esp-sensairshuttle/esp-sensairshuttle-mainboard-back.png
   :alt: SensairShuttle-Mainboard PCB Back View (Click to enlarge)
   :scale: 70%
   :figclass: align-center

   SensairShuttle-Mainboard PCB Back View (Click to enlarge)

.. figure:: ../../_static/esp-sensairshuttle/esp-sensairshuttle-bme690-back.png
   :alt: ShuttleBoard-BME690 PCB Back View (Click to enlarge)
   :scale: 70%
   :figclass: align-center

   ShuttleBoard-BME690 PCB Back View (Click to enlarge)

.. figure:: ../../_static/esp-sensairshuttle/esp-sensairshuttle-bmi270&bmm350-back.png
   :alt: ShuttleBoard-BMI270&BMM350 PCB Back View (Click to enlarge)
   :scale: 70%
   :figclass: align-center

   ShuttleBoard-BMI270&BMM350 PCB Back View (Click to enlarge)

The main components on the back PCB are introduced in clockwise order below.

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Main Component
     - Description
   * - :strong:`MainBoard`
     -
   * - Battery Connector
     - Battery connector that can connect to an external 3.7V lithium battery, using HC-1.25-2P wire-to-board connector.
   * - Mic Connector
     - 2-wire microphone connector that can connect to an external analog microphone, using HC-1.25-2P wire-to-board connector.
   * - Shuttle Board Connector
     - 9+7 pin 1.27mm female header connector that can connect to ShuttleBoard-BME690, ShuttleBoard-BMI270&BMM350 and other sensor daughterboards.
   * - Speaker Connector
     - 2-wire speaker connector that can connect to an external speaker, using HC-1.25-2P wire-to-board connector.
   * - Type-C Port (USB-C Interface)
     - USB-C interface for power supply, program flashing, and debugging, supporting lithium battery charging.
   * - :strong:`ShuttleBoard-BME690 (BME690 Sensor Daughterboard)`
     - Pin definitions for the sensor daughterboard are marked in the figure.
   * - :strong:`ShuttleBoard-BMI270&BMM350 (BMI270&BMM350 Sensor Daughterboard)`
     - Pin definitions for the sensor daughterboard are marked in the figure.

Start Development
-----------------

Before powering on, please ensure that ESP-SensairShuttle is intact.

Required Hardware
~~~~~~~~~~~~~~~~~

- ESP-SensairShuttle mainboard, ShuttleBoard-BME690 daughterboard, ShuttleBoard-BMI270&BMM350 daughterboard
- USB cable
- Computer (Windows, Linux, or macOS)

.. note::

  Please ensure you use an appropriate USB cable. Some cables can only be used for charging and cannot be used for data transmission and programming.

Hardware Setup
~~~~~~~~~~~~~~

Connect ESP-SensairShuttle to your computer using a USB cable, and flash firmware, debug, and power supply through the ``Type-C (USB-C Interface)``.

Software Setup
~~~~~~~~~~~~~~

Please visit the `ESP-IDF Getting Started <https://docs.espressif.com/projects/esp-idf/en/latest/esp32c5/get-started/index.html>`__ section to learn how to quickly set up the development environment and flash applications to your development board.

.. note::

  The development board uses a USB port to communicate with the computer. Most operating systems (Windows, Linux, macOS) have the required drivers pre-installed, and the development board can be automatically recognized after insertion. If the device cannot be recognized or a serial connection cannot be established, please refer to `Establish Serial Connection <https://docs.espressif.com/projects/esp-idf/en/latest/esp32c5/get-started/establish-serial-connection.html>`__ for detailed steps on installing drivers.

Hardware Reference
==================

Function Block Diagram
----------------------

The main components and connection methods of ESP-SensairShuttle are shown in the figure below.

.. figure:: ../../_static/esp-sensairshuttle/esp-sensairshuttle-sch-function-block_v1_0.png
   :alt: ESP-SensairShuttle Function Block Diagram (Click to enlarge)
   :scale: 32%
   :figclass: align-center

   ESP-SensairShuttle Function Block Diagram (Click to enlarge)

Power Options
-------------

The development board can be powered by the following methods:

1. Power via ``Type-C (USB-C Interface)``

   When using this method, connect the Type-C interface on the device using a USB Type-C cable.
   If no lithium battery is installed, the power indicator LED will be green. If a lithium battery is installed, press the ``POWER`` button to turn on the device. At this time, the power indicator LED will be yellow (battery is charging) or green (battery is fully charged).

2. Power via ``Battery``

   The device can connect to an external 3.7V lithium battery. Press the ``POWER`` button to power the device. The power indicator LED will be green when the device is on, and off when the device is off.

Type-C Interface
-----------------

.. figure:: ../../_static/esp-sensairshuttle/esp-sensairshuttle-mainboard-sch-type-c-v1_0.png
   :alt: Type-C Interface Circuit Diagram (Click to enlarge)
   :scale: 50%
   :figclass: align-center

   Type-C Interface Circuit Diagram (Click to enlarge)

LCD Interface
-------------

.. figure:: ../../_static/esp-sensairshuttle/esp-sensairshuttle-mainboard-sch-lcd-v1_0.png
   :alt: LCD Interface Circuit Diagram (Click to enlarge)
   :scale: 50%
   :figclass: align-center

   LCD Interface Circuit Diagram (Click to enlarge)

The X1 interface is the LCD screen interface in use. The screen model used on this development board is `ST7789P3 <https://dl.espressif.com/AE/esp-dev-kits/UE018HV-RB39-A002A%20%20V1.0%20SPEC.pdf>`_,
LCD: 1.83", 240(H)x284(V), ST7789P3, 4-line SPI Interface. ``PWR_CTRL`` (GPIO5) can be used to control the screen power.

Power Switch Circuit
-------------------------

.. figure:: ../../_static/esp-sensairshuttle/esp-sensairshuttle-mainboard-sch-powerswitch-v1_0.png
   :alt: Power Switch Circuit Diagram (Click to enlarge)
   :scale: 40%
   :figclass: align-center

   Power Switch Circuit Diagram (Click to enlarge)

Battery Charging Circuit
-----------------------------

.. figure:: ../../_static/esp-sensairshuttle/esp-sensairshuttle-mainboard-sch-battery-charge-v1_0.png
   :alt: Battery Charging Circuit Diagram (Click to enlarge)
   :scale: 50%
   :figclass: align-center

   Battery Charging Circuit Diagram (Click to enlarge)

Shuttle Board Interface Circuit
-------------------------------------

.. figure:: ../../_static/esp-sensairshuttle/esp-sensairshuttle-mainboard-sch-shuttle-board-connector-v1_0.png
   :alt: Shuttle Board Interface Circuit Diagram (Click to enlarge)
   :scale: 50%
   :figclass: align-center

   Shuttle Board Interface Circuit Diagram (Click to enlarge)

I2C/RGB/External Pin Interface
-----------------------------------

.. figure:: ../../_static/esp-sensairshuttle/esp-sensairshuttle-mainboard-sch-external-interface-v1_0.png
   :alt: I2C/RGB/External Pin Interface Circuit Diagram (Click to enlarge)
   :scale: 45%
   :figclass: align-center

   I2C/RGB/External Pin Interface Circuit Diagram (Click to enlarge)

Hardware Revision History
==============================

No revision history.

.. _Related-documents_esp-sensairshuttle:

Related Documents
=====================

-  `ESP32-C5 Datasheet`_ (PDF)
-  `ESP32-C5-WROOM-1 Datasheet`_ (PDF)
-  `Espressif Product Selector`_ (PDF)
-  `ESP-SensairShuttle-Mainboard V1.0 Schematic`_ (PDF)
-  `ESP-SensairShuttle-Mainboard V1.0 PCB Layout`_ (PDF)
-  `ESP-SensairShuttle-ShuttleBoard-BME690 V1.0 Schematic`_ (PDF)
-  `ESP-SensairShuttle-ShuttleBoard-BME690 V1.0 PCB Layout`_ (PDF)
-  `ESP-SensairShuttle-ShuttleBoard-BMI270&BMM350 V1.0 Schematic`_ (PDF)
-  `ESP-SensairShuttle-ShuttleBoard-BMI270&BMM350 V1.0 PCB Layout`_ (PDF)
-  `Display Specification`_ (PDF)

.. _ESP32-C5 Datasheet: https://documentation.espressif.com/esp32-c5_datasheet_en.pdf
.. _ESP32-C5-WROOM-1 Datasheet: https://documentation.espressif.com/esp32-c5-wroom-1_wroom-1u_datasheet_en.html
.. _Espressif Product Selector: https://products.espressif.com/#/product-selector?names=
.. _ESP-SensairShuttle-Mainboard V1.0 Schematic: https://dl.espressif.com/AE/esp-dev-kits/SCH_SCH-ESP-SensairShuttle-MainBoard-V1_0_2025-12-16.pdf
.. _ESP-SensairShuttle-Mainboard V1.0 PCB Layout: https://dl.espressif.com/AE/esp-dev-kits/PCB_PCB-ESP-SensairShuttle-MainBoard-V1_0_2025-12-16.pdf
.. _ESP-SensairShuttle-ShuttleBoard-BME690 V1.0 Schematic: https://dl.espressif.com/AE/esp-dev-kits/SCH_SCH-ShuttleBoard-BME690-V1_0_2025-12-16.pdf
.. _ESP-SensairShuttle-ShuttleBoard-BME690 V1.0 PCB Layout: https://dl.espressif.com/AE/esp-dev-kits/PCB_PCB-ShuttleBoard-BME690-V1_0_2025-12-16.pdf
.. _ESP-SensairShuttle-ShuttleBoard-BMI270&BMM350 V1.0 Schematic: https://dl.espressif.com/AE/esp-dev-kits/SCH_SCH-ShuttleBoard-BMI270&BMM350-V1_1_2025-12-16.pdf
.. _ESP-SensairShuttle-ShuttleBoard-BMI270&BMM350 V1.0 PCB Layout: https://dl.espressif.com/AE/esp-dev-kits/PCB_PCB-ShuttleBoard-BMI270&BMM350-V1_1_2025-12-16.pdf
.. _Display Specification: https://dl.espressif.com/AE/esp-dev-kits/1.83-inch-LCD-P183B001-V4-CTP.pdf

