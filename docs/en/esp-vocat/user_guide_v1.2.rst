================
ESP-VoCat v1.2
================

:link_to_translation:`zh_CN:[中文]`

.. note::

  Please check the silkscreen version number on the mainboard to confirm your development board version. For v1.0 version development boards, please refer to :doc:`user_guide_v1.0`; For v1.2 version development boards, please refer to the current user guide.

.. tip::

   For quick-start procedures after powering on ESP-VoCat, refer to the `ESP-VoCat Built-in Firmware User Guide`_.

This guide will help you get started with ESP-VoCat quickly and provide detailed information about this development board.

ESP-VoCat is an intelligent AI development kit. It is suitable for voice interaction products that require large model capabilities, such as toys, smart speakers, and smart central control systems. The device is equipped with an ESP32-S3-WROOM-1 module, a 1.85-inch QSPI circular touch screen, dual microphone array, and supports offline voice wake-up and sound source localization algorithms. Combined with the large model capabilities provided by Amazon Nova, OpenAI, Xiaozhi AI, Gemini, etc. ESP-VoCat can achieve full-duplex voice interaction, multimodal recognition, and intelligent agent control, providing a solid foundation for developers to create complete edge-side AI application experiences. 

ESP-VoCat's main controller uses the Espressif ESP32-S3-WROOM-1-N16R16VA module, supporting 2.4 GHz Wi-Fi and Bluetooth 5 (LE) wireless connectivity. For storage, the entire device has 16MB PSRAM and 16MB Flash storage space, and is also equipped with a microSD card slot that can support up to 32GB, meeting the needs of voice interaction and multimedia processing. It features a 1.85-inch circular touch screen (360 × 360 resolution) with ESP32-S3 native touch sensors, providing an intuitive and rich interactive experience.

For audio, ESP-VoCat has a built-in 3W speaker and dual microphone array, supporting local voice wake-up and sound source localization. The power system is compatible with 5 V DC and 3.7 V lithium battery power supply. Additionally, it integrates a USB-C interface for power supply and programming download, while reserving a Pogopin interface for convenient functional expansion.

.. figure:: ../../_static/esp-vocat/esp-vocat-black-front_v1.0.png
   :alt: ESP-VoCat Front View (Click to enlarge)
   :scale: 18%
   :figclass: align-center

   ESP-VoCat Front View (Click to enlarge)

.. figure:: ../../_static/esp-vocat/esp-vocat-black-back_v1.0.png
   :alt: ESP-VoCat Back View (Click to enlarge)
   :scale: 18%
   :figclass: align-center

   ESP-VoCat Back View (Click to enlarge)

The document consists of the following major sections:

- `Getting Started`_: Overview of the board and hardware/software setup instructions to get started.
- `Hardware Reference`_: More detailed information about the board's hardware.
- `Hardware Revision Details`_: Hardware revision history and known issues (if any) of the board.
- `Related Documents`_: Links to related documentation.
- `Disclaimer and Copyright Notice`_: Link to the disclaimer and copyright notice.

.. _Getting-started_esp-vocat_3:

Getting Started
======================

This section provides a brief introduction of ESP-VoCat, instructions on how to do the initial hardware setup and how to flash firmware onto it.

Description of Components
------------------------------

.. figure:: ../../_static/esp-vocat/esp-vocat-coreboard-front-annotated-photo_v1.2.png
   :alt: ESP-VoCat CoreBoard PCB Front View (Click to enlarge)
   :scale: 70%
   :figclass: align-center

   ESP-VoCat CoreBoard PCB Front View (Click to enlarge)

.. figure:: ../../_static/esp-vocat/esp-vocat-micboard-front-annotated-photo_v1.2.png
   :alt: ESP-VoCat MicBoard PCB Front View (Click to enlarge)
   :scale: 70%
   :figclass: align-center

   ESP-VoCat MicBoard PCB Front View (Click to enlarge)

.. figure:: ../../_static/esp-vocat/esp-vocat-baseboard-front-annotated-photo_v1.2.png
   :alt: ESP-VoCat BaseBoard PCB Front View (Click to enlarge)
   :scale: 70%
   :figclass: align-center

   ESP-VoCat BaseBoard PCB Front View (Click to enlarge)

.. figure:: ../../_static/esp-vocat/esp-vocat-black-front-annotated-photo_v1.0.png
   :alt: ESP-VoCat Front View (Click to enlarge)
   :scale: 50%
   :figclass: align-center

   ESP-VoCat Front View (Click to enlarge)

The following introduces the main components on the front PCB in clockwise order. For user convenience, we have also labeled these components or interfaces on the ESP-VoCat housing.

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Main Components
     - Description
   * - :strong:`CoreBoard`
     -
   * - ESP32-S3-WROOM-1-N16R16VA
     - Main controller chip, integrates 16 MB Flash and 16 MB PSRAM, supports 2.4 GHz Wi-Fi and Bluetooth 5 (LE) wireless connectivity.
   * - Battery Connector
     - Battery connector for connecting 3.7 V lithium battery, top is positive, bottom is negative.
   * - LCD FPC Connector
     - For connecting 1.85-inch circular LCD screen with 360 x 360 resolution. For detailed parameters, please refer to the `Display Specification`_.
   * - MicBoard Connector
     - MicBoard PCB connector, connects dual microphone array and status indicator LED.
   * - Touch Connector
     - Touch connector for connecting touch copper foil to achieve touch interaction functionality.
   * - Speaker Connector
     - 2-wire speaker connector for connecting built-in 3 W speaker.
   * - :strong:`MicBoard`
     -
   * - Green LED
     - Green LED indicator.
   * - MIC (Microphone Array)
     - Dual LMA3729T381-OY3S microphone array, supports local voice wake-up and sound source localization functionality.
   * - :strong:`BaseBoard`
     -
   * - BQ27220 (Battery Management Chip)
     - Battery management chip for battery level detection, charging management, and power status monitoring.
   * - TP4057 (Lithium Battery Charging Chip)
     - Lithium battery charging chip for charging lithium batteries with 250 mA charging current.
   * - CoreBoard Connector
     - CoreBoard PCB connector, connects core board with base board.
   * - TlV62569 (DCDC Chip)
     - Buck converter chip for converting 5 V power to 3.3 V power, providing stable power supply for the system.
   * - SAM8108 (Power Control Chip)
     - Power control chip for controlling device power on/off. Single click of POWER button can toggle power on/off status.
   * - Type-C (USB-C Interface)
     - USB-C interface for power supply, programming download, and debugging, supports charging lithium batteries.

.. figure:: ../../_static/esp-vocat/esp-vocat-coreboard-back-annotated-photo_v1.2.png
   :alt: ESP-VoCat CoreBoard PCB Back View (Click to enlarge)
   :scale: 70%
   :figclass: align-center

   ESP-VoCat CoreBoard PCB Back View (Click to enlarge)

.. figure:: ../../_static/esp-vocat/esp-vocat-micboard-back-annotated-photo_v1.2.png
   :alt: ESP-VoCat MicBoard PCB Back View (Click to enlarge)
   :scale: 70%
   :figclass: align-center

   ESP-VoCat MicBoard PCB Back View (Click to enlarge)

.. figure:: ../../_static/esp-vocat/esp-vocat-baseboard-back-annotated-photo_v1.2.png
   :alt: ESP-VoCat BaseBoard PCB Back View (Click to enlarge)
   :scale: 70%
   :figclass: align-center

   ESP-VoCat BaseBoard PCB Back View (Click to enlarge)

.. figure:: ../../_static/esp-vocat/esp-vocat-black-back-annotated-photo_v1.0.png
   :alt: ESP-VoCat Back View (Click to enlarge)
   :scale: 50%
   :figclass: align-center

   ESP-VoCat Back View (Click to enlarge)

The following introduces the main components on the back PCB in clockwise order. For user convenience, we have also labeled these components or interfaces on the ESP-VoCat housing.

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Main Components
     - Description
   * - :strong:`CoreBoard`
     -
   * - NS4150B (Class D Amplifier)
     - Ultra-low EMI, filterless, 3W mono Class D audio amplifier.
   * - ES7210 (Audio Decoder Chip)
     - High-performance 4-channel audio decoder chip, supports I2S/PDM/TDM data ports.
   * - SY8088AAC (DCDC chip)  
     - Buck converter chip used to convert a 5 V power supply to a 3.3 V power supply, providing stable power for the Codec.
   * - BaseBoard Connector
     - Base board connector for connecting base board with CoreBoard.
   * - BMI270 (IMU - Inertial Measurement Unit)
     - 6-axis intelligent low-power inertial measurement unit for high-performance applications.
   * - ES8311 (Audio Codec Chip)
     - Low-power mono audio codec with high-performance multi-bit Delta-Sigma audio ADC and DAC.
   * - :strong:`MicBoard`
     -
   * - FPC Connector
     - Connects MicBoard with CoreBoard FPC connector.
   * - :strong:`BaseBoard`
     -
   * - M1.6 Welded Nut
     - For fixing main board to housing.
   * - RST Button (Reset Button)
     - For resetting the main board.
   * - Magnetic Connector
     - For functional expansion, provides a serial port and 5 V power interface, can connect to rotating base and other devices.
   * - POWER Switch
     - For controlling device power on/off. Single click of POWER button can toggle power on/off status.
   * - BOOT Button
     - When powering on, hold this button to enter download mode.
   * - SD Card Slot
     - Supports up to 32 GB microSD card slot for storing audio, images, videos, and other data.

Start Application Development
-----------------------------

Before powering on, please ensure ESP-VoCat is intact and undamaged.

Required Hardware
^^^^^^^^^^^^^^^^^^^^^^

- ESP-VoCat
- USB data cable
- Computer (Windows, Linux, or macOS)

.. note::

  Be sure to use an appropriate USB cable. Some cables are for charging only and do not provide the needed data lines nor work for programming the boards.

Hardware Setup
^^^^^^^^^^^^^^^^^^^^^^

Use a USB data cable to connect ESP-VoCat to your computer through the ``Type-C (USB-C Interface)`` for firmware burning, debugging, and power supply.

Software Setup
^^^^^^^^^^^^^^^^^^^^^^

Please proceed to `ESP-IDF Get Started <https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/index.html>`__ to set up the development environment and flash an application example onto your board.

.. note::

  In most cases, USB drivers required to operate the board are already included in Windows, Linux, and macOS operating systems. Some additional port access or security configuration may be required depending on your OS. In case of issues, please check documentation on `how to establish serial connection <https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/establish-serial-connection.html>`__ with the board. The documentation also includes links to USB drivers applicable to boards produced by Espressif.

Espressif provides Board Support Packages (BSPs) for various Espressif boards that help you initialize and use key onboard peripherals, such as LCD displays, audio chips, buttons, and LEDs, more easily and efficiently. For a complete list of supported boards, please visit `esp-bsp <https://github.com/espressif/esp-bsp>`__.

Development board application examples are stored in the `esp-brookesia <https://github.com/espressif/esp-brookesia/tree/release/v0.6/products/speaker>`_ project.

Contents and Packaging
----------------------

.. include:: ordering-info.inc

For more information about available variants, visit the `ESP Product Selector <https://products.espressif.com/#/product-selector?names=>`__ and select the **Development Board** tab. If you cannot find the variant you need in the table or Product Selector, or if you have special customization requirements, contact `Espressif Sales <https://www.espressif.com/en/contact-us/sales-questions>`__.

Retail Orders
^^^^^^^^^^^^^

If you order a few samples, each board comes in an individual package in either antistatic bag or any packaging depending on your retailer.

For retail orders, please go to https://www.espressif.com/en/company/contact/buy-a-sample.

Wholesale Orders
^^^^^^^^^^^^^^^^

If you order in bulk, the boards come in large cardboard boxes.

For wholesale orders, please go to https://www.espressif.com/en/contact-us/sales-questions.

.. _Hardware-reference_esp-vocat_3:

Hardware Reference
======================

Block Diagram
-----------------------------

The block diagram below shows the components of ESP-VoCat and their interconnections.

.. figure:: ../../_static/esp-vocat/esp-vocat-sch-function-block_v1.2.png
   :alt: ESP-VoCat Block Diagram (Click to enlarge)
   :scale: 40%
   :figclass: align-center

   ESP-VoCat Block Diagram (Click to enlarge)

Power Supply Options
-----------------------------

There are three mutually exclusive ways to provide power to the board:

1. Power through ``Type-C (USB-C Interface)``

   When using this method, connect the device's Type-C interface using a USB Type-C data cable. If a lithium battery is installed, you need to press the ``POWER`` button to enable power supply to the device.

2. Power through ``Magnetic Connector``

   When using this method, connect the device to the corresponding magnetic connector base to power the device. If a lithium battery is installed, you need to press the ``POWER`` button to enable power supply to the device.

  .. figure:: ../../_static/esp-vocat/esp-vocat-magnetic-connector-annotated-photo_v1.2.png
   :alt: Magnetic Connector (Click to enlarge)
   :scale: 70%
   :figclass: align-center

   Magnetic Connector (Click to enlarge)

3. Power through ``Battery``

   The device has an integrated 3.7 V lithium battery. Press the ``POWER`` button to power the device.

Any of the above external power supply methods can charge the internal lithium battery.

Type-C Interface
-----------------------------

.. figure:: ../../_static/esp-vocat/esp-vocat-type-c-v1.0.png
   :alt: Type-C Interface Circuit Diagram (Click to enlarge)
   :scale: 40%
   :figclass: align-center

   Type-C Interface Circuit Diagram (Click to enlarge)

Magnetic Connector Interface
-----------------------------

.. figure:: ../../_static/esp-vocat/esp-vocat-sch-magnetic-connector_v1.0.png
   :alt: Magnetic Connector Interface Circuit Diagram (Click to enlarge)
   :scale: 40%
   :figclass: align-center

   Magnetic Connector Interface Circuit Diagram (Click to enlarge)

LCD Interface
-----------------------------

.. figure:: ../../_static/esp-vocat/esp-vocat-sch-lcd_v1.0.png
   :alt: LCD Interface Circuit Diagram (Click to enlarge)
   :scale: 50%
   :figclass: align-center

   LCD Interface Circuit Diagram (Click to enlarge)

Please note that the CN3 interface is reserved for other screen compatibility and is currently not enabled.

The U2 interface is the officially used LCD screen interface. The screen model used by this development board is `ST77916 <https://dl.espressif.com/AE/esp-dev-kits/UE018HV-RB39-A002A%20%20V1.0%20SPEC.pdf>`_, LCD: 1.85", 360 x 360, ST77916, QSPI Interface. ``LCD_BLK`` (GPIO44) can be used to control screen backlight.

SD Card Interface
-----------------------------

.. figure:: ../../_static/esp-vocat/esp-vocat-sch-sd-card_v1.0.png
   :alt: SD Card Interface Circuit Diagram (Click to enlarge)
   :scale: 50%
   :figclass: align-center

   SD Card Interface Circuit Diagram (Click to enlarge)

**Please note that the SD card interface supports:**

* 1-wire SD bus configuration
* Communication through SDIO protocol

Power Switch Circuit
-----------------------------

.. figure:: ../../_static/esp-vocat/esp-vocat-sch-powerswitch_v1.0.png
   :alt: Power Switch Circuit Diagram (Click to enlarge)
   :scale: 50%
   :figclass: align-center

   Power Switch Circuit Diagram (Click to enlarge)

Charging Circuit
-----------------------------

.. figure:: ../../_static/esp-vocat/esp-vocat-sch-battery-charge_v1.0.png
   :alt: Charging Circuit Diagram (Click to enlarge)
   :scale: 50%
   :figclass: align-center

   Charging Circuit Diagram (Click to enlarge)

.. figure:: ../../_static/esp-vocat/esp-vocat-sch-battery-management_v1.0.png
   :alt: Battery Management Circuit Diagram (Click to enlarge)
   :scale: 50%
   :figclass: align-center

   Battery Management Circuit Diagram (Click to enlarge)

Microphone Interface
-----------------------------

.. figure:: ../../_static/esp-vocat/esp-vocat-sch-micboard-connector_v1.0.png
   :alt: Microphone Interface Circuit Diagram (Click to enlarge)
   :scale: 50%
   :figclass: align-center

   Microphone Interface Circuit Diagram (Click to enlarge)

Hardware Revision Details
=========================

ESP-VoCat v1.2
--------------------------

- The following pins have been reassigned:

  - ``U1RXD``: Changed from ``IO5`` to ``IO4``
  - ``U1TXD``: Changed from ``IO6`` to ``IO5``
  - ``I2S_DI``: Changed from ``IO15`` to ``IO3``
  - ``PA_CTRL``: Changed from ``IO4`` to ``IO15``
  - ``LCD_RST_CTRL``: Changed from ``IO3`` to ``IO47``

- TOUCH interface increased from 1 to 2 for sliding touch sensing:

  - ``TOUCH_PAD1``: ``IO6``
  - ``TOUCH_PAD2``: ``IO7``

- Added ``CODEC`` power control:

  - ``CODEC_PWR_CTRL``: ``IO48``

- Added ``IIC`` expansion interface:

  - Added ``HC-1.25-4PLT`` interface: Exposes ``VIN``, ``I2C_SDA``, ``I2C_SCL``, and ``GND``

ESP-VoCat v1.0
--------------------------

- :doc:`Initial Release <user_guide_v1.0>`

.. _Related-documents_esp-vocat_3:

Related Documents
======================

-  `ESP-VoCat V1.2 Schematic`_ (PDF)
-  `ESP-VoCat V1.2 PCB Layout`_ (PDF)
-  `Display Specification`_ (PDF)
-  `ESP-VoCat Built-in Firmware User Guide`_ (HTML)

.. _ESP-VoCat V1.2 Schematic: https://dl.espressif.com/AE/esp-dev-kits/ESP-VoCat_SCH_V1_2.pdf
.. _ESP-VoCat V1.2 PCB Layout: https://dl.espressif.com/AE/esp-dev-kits/ESP-VoCat_pcb_V1_2.zip
.. _Display Specification: https://dl.espressif.com/AE/esp-dev-kits/UE018HV-RB39-A002A%20%20V1.0%20SPEC.pdf
.. _ESP-VoCat Built-in Firmware User Guide: https://espressif.craft.me/CI2XAhb4Ix7fZk

.. toctree::
    :hidden:

    user_guide_v1.0

Disclaimer and Copyright Notice
===============================

See :doc:`Disclaimer and Copyright Notice <../disclaimer-and-copyright>`.

