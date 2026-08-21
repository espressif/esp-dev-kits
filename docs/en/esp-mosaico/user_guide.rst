===================
ESP-Mosaico V1.0
===================

:link_to_translation:`zh_CN:[中文]`

.. note::

  This document covers CoreBoard **V1.0** hardware. Please check the silkscreen version on the mainboard to confirm your board revision.

This guide helps you get started with ESP-Mosaico and provides detailed information about the board.

ESP-Mosaico is an expandable smart interaction development kit based on ESP32-S31. It targets magnetic expansion toys, motion sensing, square touch display HMI, and edge voice/multimedia applications. The device integrates a 480 × 480 QSPI square touch display, ES8311 audio codec, NS4150B amplifier, BMI270 6-axis IMU, dual BMM150 magnetometers, SPI NAND flash, and left/right 2 × 10P, 2.54 mm pitch module headers.

The main controller is Espressif ESP32-S31, supporting 2.4 GHz Wi-Fi 6 (IEEE 802.11b/g/n/ax), Bluetooth® 5.4 (LE + BR/EDR), and IEEE 802.15.4 (Thread / Zigbee). For storage, the board provides 16 MB in-package PSRAM, onboard 16 MB SPI NOR flash, and onboard 1 Gbit SPI NAND flash for graphics, multimedia, and asset caching. The display uses a CO5300-driven 480 × 480 QSPI square touch display with a CST9220-series touch controller.

For audio, ESP-Mosaico integrates ES8311 and NS4150B Class-D PA. The power system supports USB Type-C 5 V and a 3.7 V / 65 mAh Li-ion battery, with TP4057 charging, BQ27220 fuel gauge, and SAM8108 power-switch control. Both the Type-C port and the left/right module interfaces support power input and output. A USB 2.0 High-Speed OTG port supports power, debug, and communication, and the board also reserves UART expansion capability and left/right module interfaces.

.. figure:: ../../_static/esp-mosaico/esp-mosaico-isometric.png
   :alt: ESP-Mosaico Appearance (Click to enlarge)
   :width: 60%
   :figclass: align-center

   ESP-Mosaico Appearance (Click to enlarge)

The document consists of the following major sections:

- `Getting Started`_: Overview of the board and hardware/software setup instructions.
- `Hardware Reference`_: More detailed information about the board hardware.
- `Hardware Revision Details`_: Hardware revision history and known issues (if any).
- `Related Documents`_: Links to related documentation.
- `Disclaimer and Copyright Notice`_: Link to the disclaimer and copyright notice.

.. _Getting-started_esp-mosaico_en:

Getting Started
===============

This section briefly introduces ESP-Mosaico and describes how to flash firmware and prepare the board.

Description of Components
-------------------------

ESP-Mosaico consists of a CoreBoard and a BaseBoard. The front views and their main components are shown first, followed by the back views and their main components.

Front
^^^^^

.. figure:: ../../_static/esp-mosaico/esp-mosaico-front.png
   :alt: ESP-Mosaico Front (Click to enlarge)
   :width: 60%
   :figclass: align-center

   ESP-Mosaico Front (Click to enlarge)

.. figure:: ../../_static/esp-mosaico/esp-mosaico-coreboard-front.png
   :alt: ESP-Mosaico CoreBoard Front (Click to enlarge)
   :width: 80%
   :figclass: align-center

   ESP-Mosaico CoreBoard Front (Click to enlarge)

.. figure:: ../../_static/esp-mosaico/esp-mosaico-baseboard-front.png
   :alt: ESP-Mosaico BaseBoard Front (Click to enlarge)
   :width: 80%
   :figclass: align-center

   ESP-Mosaico BaseBoard Front (Click to enlarge)

The following describes the main components on the front PCB in clockwise order.

.. list-table::
   :widths: 8 28 64
   :header-rows: 1

   * - No.
     - Key Component
     - Description
   * -
     - :strong:`CoreBoard`
     - Core board integrating the SoC, RF antenna, power management, display connector, audio codec, and sensors.
   * - 1
     - ESP32-S31NRV16
     - ESP32-S31 SoC, RISC-V 32-bit dual-core up to 320 MHz with 16 MB in-package PSRAM; supports 2.4 GHz Wi-Fi 6, Bluetooth 5.4 (LE + BR/EDR), and IEEE 802.15.4.
   * - 2
     - KH-3216-A35 Ceramic Antenna
     - 2.4 GHz chip ceramic antenna for Wi-Fi and Bluetooth RF.
   * - 3
     - Orange LED
     - Programmable mono status LED on GPIO3, active-low.
   * - 4
     - TLV62569
     - Buck (DCDC) converter that converts the input supply to the system 3.3 V rail.
   * - 5
     - BMM150
     - 3-axis magnetometer (BMM150 #2), I2C address 0x11.
   * - 6
     - SY7088
     - Boost (BOOST) converter for rails higher than the battery voltage.
   * - 7
     - Display Connector
     - Connects the 480 × 480, 2.16-inch QSPI touch display (CO5300 driver + CST9220 touch).
   * - 8
     - TP4057
     - Li-ion battery charger for the onboard battery.
   * - 9
     - BQ27220
     - Fuel gauge at I2C address 0x55 for voltage, current, state of charge (SOC), and related status.
   * - 10
     - 3V3 Power LED
     - Lights when the 3.3 V rail is present after power-on; turns off when the device is powered off.
   * - 11
     - HUSB320
     - USB Type-C interface controller that handles CC detection and controls the USB power direction.
   * - 12
     - Charge LED
     - Charge status indicator: red while charging, green when fully charged.
   * - 13
     - SAM8108
     - Power on/off controller used with the POWER button.
   * - 14
     - HE9073A33
     - LDO providing a dedicated supply to the audio codec to reduce noise.
   * - 15
     - NS4150B
     - Low-EMI, filterless 3 W mono Class-D amplifier for the speaker.
   * - 16
     - ES8311
     - Low-power mono audio codec, I2C address 0x19, connected to the SoC over I2S.
   * - 17
     - BMM150
     - 3-axis magnetometer (BMM150 #3), I2C address 0x12.
   * - 18
     - BMI270
     - 6-axis IMU (accelerometer + gyroscope), I2C address 0x69.
   * - 19
     - GD25Q128EWIGR
     - 16 MB (128 Mbit) SPI NOR flash for program and data storage.
   * - 20
     - GD5F1GM7UEYIGR
     - 1 Gbit (128 MB) SPI NAND flash for large-capacity asset storage.
   * -
     - :strong:`BaseBoard`
     - Base board integrating the speaker, battery, and left/right 2 × 10P module headers, mated to the CoreBoard through a board-to-board connector.
   * - 21
     - Speaker
     - Onboard speaker driven by the NS4150B amplifier.
   * - 22
     - BTB Connector
     - 60-pin 0.5 mm board-to-board connector linking the CoreBoard and BaseBoard.
   * - 23
     - Expansion Header 1
     - 2 × 10P dual-row, 2.54 mm pitch header exposing GPIOs and power for expansion.
   * - 24
     - Battery
     - 3.7 V / 65 mAh Li-ion battery for portable power.
   * - 25
     - Expansion Header 2
     - Additional header exposing the remaining GPIOs and power.

Back
^^^^

.. figure:: ../../_static/esp-mosaico/esp-mosaico-back.png
   :alt: ESP-Mosaico Back (Click to enlarge)
   :width: 80%
   :figclass: align-center

   ESP-Mosaico Back (Click to enlarge)

.. figure:: ../../_static/esp-mosaico/esp-mosaico-coreboard-back.png
   :alt: ESP-Mosaico CoreBoard Back (Click to enlarge)
   :width: 80%
   :figclass: align-center

   ESP-Mosaico CoreBoard Back (Click to enlarge)

.. figure:: ../../_static/esp-mosaico/esp-mosaico-baseboard-back.png
   :alt: ESP-Mosaico BaseBoard Back (Click to enlarge)
   :width: 55%
   :figclass: align-center

   ESP-Mosaico BaseBoard Back (Click to enlarge)

The following describes the main components on the back PCB in clockwise order.

.. list-table::
   :widths: 8 28 64
   :header-rows: 1

   * - No.
     - Key Component
     - Description
   * -
     - :strong:`CoreBoard`
     - Core board back side, exposing the buttons, Type-C connector, microphone, and vibration motor.
   * - 1
     - BTB Connector
     - Board-to-board connector mating with the BaseBoard (back side).
   * - 2
     - Vibration Motor
     - Vibration motor controlled by GPIO8 (active-high) for haptic feedback.
   * - 3
     - BOOT Button
     - Hold during power-up to enter firmware download mode.
   * - 4
     - LMA2718B331-OE1
     - Onboard microphone connected to the ES8311 for voice capture.
   * - 5
     - Type-C Connector
     - USB 2.0 High-Speed interface for power, debug/download, and USB communication, with Li-ion charging support.
   * - 6
     - POWER Button
     - Single press toggles the device power on/off.
   * - 7
     - Function Button
     - Application button (AI button) on GPIO7, active-low.
   * -
     - :strong:`BaseBoard`
     - Base board back side, providing debug pads and back expansion power pads.
   * - 8
     - Debug Pads
     - Test points exposing GND / TX / RX / BOOT / EN / 3V3 debug signals.
   * - 9
     - Back Expansion Pads
     - Back-side expansion pads exposing I2C and power interfaces for supplying the device.

Start Application Development
-----------------------------

Before powering up, make sure ESP-Mosaico is in good condition.

Required Hardware
^^^^^^^^^^^^^^^^^

- ESP-Mosaico
- USB cable (data-capable)
- Computer (Windows, Linux, or macOS)

.. note::

  Be sure to use a proper USB data cable. Some cables are charge-only and cannot be used for data transfer or programming.

Hardware Notes
^^^^^^^^^^^^^^

1. Flashing

   Connect ESP-Mosaico to the computer with a USB cable through the onboard Type-C port. This port is USB 2.0 High-Speed and supports both power delivery and USB data communication. The native ESP32-S31 chip only supports firmware download through this port in download mode, and does not provide auto-download or log output by itself. The factory firmware integrates a CDC virtual serial port and supports auto-download and serial log output. If flashing fails, try holding the BOOT button while the device is powered off, then power it on to enter download mode. In addition, the left and right module interfaces provide USB Serial/JTAG (left) and a TX/RX UART (right), both of which can be used for debugging and firmware download.

2. Power

   The Type-C port can power the device. The onboard battery is charged at a rated current of 65 mA. When a USB sink device is attached, the Type-C port automatically switches to source mode, with a maximum output of 5 V / 500 mA. If the load exceeds 1 A, the port shuts off automatically and recovers after a period of time.
   The left and right module interfaces also provide outward 3.3 V and 5 V rails (``3V`` / ``5V``). These rails are output only when GPIO60 is driven low. ``VIN`` is an input supply pin that can also charge the onboard battery.

Software Setup
^^^^^^^^^^^^^^

Follow the `ESP-IDF Get Started <https://docs.espressif.com/projects/esp-idf/en/latest/esp32s31/get-started/index.html>`__ guide to set up the development environment. The target chip for this board is ``esp32s31``.

.. note::

  The board communicates with the PC over USB. Most operating systems (Windows, Linux, macOS) already include the required drivers. If the device is not recognized or the serial connection fails, see `Establish Serial Connection with ESP32-S31 <https://docs.espressif.com/projects/esp-idf/en/latest/esp32s31/get-started/establish-serial-connection.html>`__ for driver installation details.

BSP and Examples
^^^^^^^^^^^^^^^^

Espressif provides a board support package (BSP) for ESP-Mosaico to simplify initialization of the LCD, touch, audio, sensors, battery, NAND flash, module resources, and USB console.

- BSP and examples: ``esp-mosaico``
- Recommended application framework: `ESP-Claw <https://esp-claw.com/en/>`__.
- After the first successful flash, use the BSP USB CDC console and auto-download support. Typical commands:

.. code-block:: bash

   idf.py set-target esp32s31
   idf.py -p /dev/ttyACM0 flash monitor

.. note::

  - The first flash may require manually entering ROM download mode.
  - The USB device briefly disconnects while the chip resets; IDF Monitor usually reconnects automatically.
  - Use ``monitor --no-reset`` when attaching without restarting the application.
  - Related options: ``CONFIG_BSP_USB_CONSOLE``, ``CONFIG_BSP_USB_CONSOLE_AUTO_INIT``, ``CONFIG_BSP_USB_AUTO_DOWNLOAD``.

.. _Hardware-reference_esp-mosaico_en:

Hardware Reference
==================

Block Diagram
-------------

The main components and connections of ESP-Mosaico are shown below.

.. figure:: ../../_static/esp-mosaico/esp-mosaico-sch-function-block_v0.3.png
   :alt: ESP-Mosaico Block Diagram (Click to enlarge)
   :width: 90%
   :figclass: align-center

   ESP-Mosaico Block Diagram (Click to enlarge)

Power Supply Options
--------------------

You can power the board in the following ways:

1. Through the ``Type-C (USB-C)`` port

   Connect a USB Type-C cable to the Type-C port.

2. Through the ``back expansion pads``

   Supply external power through the back expansion pads on the BaseBoard.

3. Through the ``battery``

   The board supports a 3.7 V / 65 mAh Li-ion battery.

Regardless of the power source, use the POWER button to turn the device on or off. External power sources above can also charge the onboard battery.

Type-C Interface
----------------

.. figure:: ../../_static/esp-mosaico/esp-mosaico-type-c.png
   :alt: Type-C Interface Schematic (Click to enlarge)
   :scale: 45%
   :figclass: align-center

   Type-C Interface Schematic (Click to enlarge)

The Type-C port connects to ESP32-S31 USB 2.0 High-Speed OTG for power, debug, and application communication. Onboard USB Serial/JTAG signals are used for download and debugging. The BSP USB CDC console and auto-download feature are the recommended download and log path.

DCDC 3.3 V Circuit
------------------

.. figure:: ../../_static/esp-mosaico/esp-mosaico-DCDC-3V3.png
   :alt: DCDC 3.3 V Schematic (Click to enlarge)
   :scale: 45%
   :figclass: align-center

   DCDC 3.3 V Schematic (Click to enlarge)

TLV62569 steps ``VDD`` down to ``MCU_3V3``, which is then switched to ``VCC_3V3`` by the ``VCC_PW`` load switch. ``U7`` is the 3.3 V power LED.

Power-control GPIOs defined by the BSP:

.. list-table::
   :widths: 30 20 50
   :header-rows: 1

   * - Signal
     - GPIO
     - Description
   * - VCC_3V3 / VCC_PW
     - GPIO60
     - System 3.3 V rail control, **active-low**; BSP applies a soft-start ramp when enabling.
   * - CODEC_3V3
     - GPIO56
     - Codec power control, **active-high**.
   * - POWER_SWITCH
     - GPIO57
     - Whole-device power-off request; kept high-Z during normal BSP operation, driven open-drain low to request shutdown.

BOOST Circuit
-------------

.. figure:: ../../_static/esp-mosaico/esp-mosaico-boost.png
   :alt: BOOST Schematic (Click to enlarge)
   :scale: 45%
   :figclass: align-center

   BOOST Schematic (Click to enlarge)

The onboard SY7088 boost converter is enabled by ``VCC_3V3`` and outputs ``VOUT_BOOST`` for rails higher than the battery voltage.

Power On/Off Circuit
--------------------

.. figure:: ../../_static/esp-mosaico/esp-mosaico-powerswitch.png
   :alt: Power On/Off Schematic (Click to enlarge)
   :scale: 45%
   :figclass: align-center

   Power On/Off Schematic (Click to enlarge)

SAM8108 works with the POWER button for whole-device power control. The ``POWER_SWITCH`` signal can also be monitored or driven by software to request shutdown.

MCU and Pin Assignment
----------------------

The following table lists the main pin assignments used by the ESP-Mosaico BSP, grouped by function.

.. container:: wide-table-scroll

   .. list-table:: ESP-Mosaico Main Pin Assignment
      :header-rows: 1
      :widths: 14 18 12 56

      * - Category
        - Signal
        - GPIO
        - Description
      * - I2C / Sensors
        - I2C0_SDA
        - GPIO0
        - Shared I2C for touch, ES8311, BMI270, BMM150, BQ27220, and module EEPROM
      * -
        - I2C0_SCL
        - GPIO1
        - Shared I2C clock
      * -
        - SENSOR_INT
        - GPIO2
        - IMU / magnetometer interrupt or signal
      * -
        - TOUCH_INT
        - GPIO6
        - Touch interrupt
      * - HMI
        - STATUS_LED
        - GPIO3
        - Orange status LED, software-controlled, active-low
      * -
        - AI_BUTTON
        - GPIO7
        - Application button, active-low
      * -
        - MOTOR
        - GPIO8
        - Vibration motor, active-high
      * - LCD
        - LCD_DATA3
        - GPIO9
        - CO5300 QSPI DATA3
      * -
        - LCD_DATA2
        - GPIO35
        - CO5300 QSPI DATA2
      * -
        - LCD_DATA0
        - GPIO36
        - CO5300 QSPI DATA0
      * -
        - LCD_RST
        - GPIO42
        - LCD reset
      * -
        - LCD_TE
        - GPIO43
        - LCD_TE anti-tearing sync
      * -
        - LCD_SCL
        - GPIO44
        - QSPI clock
      * -
        - LCD_CS
        - GPIO50
        - LCD chip select
      * -
        - LCD_DATA1
        - GPIO51
        - CO5300 QSPI DATA1
      * - Audio
        - I2S_BCK
        - GPIO37
        - Audio bit clock
      * -
        - I2S_DOUT
        - GPIO40
        - Audio data out (DAC)
      * -
        - PA_CTRL
        - GPIO45
        - Amplifier enable
      * -
        - I2S_WS
        - GPIO49
        - Audio word select
      * -
        - I2S_DIN
        - GPIO52
        - Audio data in (ADC)
      * -
        - I2S_MCLK
        - GPIO54
        - Audio master clock
      * -
        - CODEC_PW
        - GPIO56
        - Codec 3.3 V power control
      * - Power
        - POWER_SWITCH
        - GPIO57
        - Power on/off request
      * -
        - VCC_3V3_CTRL
        - GPIO60
        - System 3.3 V power control
      * - NAND Flash
        - NAND_CLK
        - GPIO20
        - SPI NAND (SD_D0)
      * -
        - NAND_D
        - GPIO21
        - SPI NAND (SD_D1 / SIO0)
      * -
        - NAND_Q
        - GPIO22
        - SPI NAND (SD_D2 / SIO1)
      * -
        - NAND_CS
        - GPIO23
        - SPI NAND (SD_D3)
      * -
        - NAND_HOLD
        - GPIO24
        - SPI NAND (SD_CLK / SIO3)
      * -
        - NAND_WP
        - GPIO25
        - SPI NAND (SD_CMD / SIO2)

I2C Device Addresses
--------------------

The 7-bit addresses on the shared I2C bus (``I2C0_SDA`` / ``I2C0_SCL``) are listed below. Onboard devices are on the CoreBoard / BaseBoard. **Module EEPROMs are on the attached modules, not on the mainboard**, and appear only when a module with EEPROM is plugged into the corresponding slot.

.. list-table::
   :widths: 20 40 40
   :header-rows: 1

   * - I2C Address
     - Device
     - Description
   * - 0x11
     - BMM150 #2
     - Onboard 3-axis magnetometer
   * - 0x12
     - BMM150 #3
     - Onboard 3-axis magnetometer
   * - 0x19
     - ES8311
     - Onboard audio codec
   * - 0x50
     - Module EEPROM (Left)
     - On the left module, not the mainboard; selected when GPIO14 is driven low
   * - 0x51
     - Module EEPROM (Right)
     - On the right module, not the mainboard; selected when GPIO39 is driven high
   * - 0x55
     - BQ27220
     - Onboard fuel gauge
   * - 0x5A
     - CST9220
     - Onboard touch controller
   * - 0x69
     - BMI270
     - Onboard 6-axis IMU

LCD Interface
-------------

.. figure:: ../../_static/esp-mosaico/esp-mosaico-lcd.png
   :alt: LCD Interface Schematic (Click to enlarge)
   :scale: 45%
   :figclass: align-center

   LCD Interface Schematic (Click to enlarge)

- LCD driver: CO5300, QSPI interface, 480 × 480 resolution.
- Touch controller: CST9220 series, I2C interface (``TP_SCL`` / ``TP_SDA`` / ``TP_RST`` / ``TP_INT``).

Vibration Motor Interface
-------------------------

.. figure:: ../../_static/esp-mosaico/esp-mosaico-motor.png
   :alt: Vibration Motor Schematic (Click to enlarge)
   :scale: 45%
   :figclass: align-center

   Vibration Motor Schematic (Click to enlarge)

The vibration motor is driven by ``MOTOR`` (GPIO8) through a low-side MOSFET, active-high.

Module Interface
----------------

The BaseBoard provides two ``2 × 10P``, 2.54 mm pitch expansion headers (``H2`` left and ``H1`` right) that expose GPIOs, UART, USB Serial/JTAG, power, and ground for secondary development. Except for power and ground, **all expansion signal pins support GPIO and can be flexibly remapped through the GPIO matrix**. Labels such as ADC / TOUCH / COMP / DAC in the tables below mark the pin's analog capabilities and do not prevent using it as a general-purpose GPIO.

ESP-Mosaico provides left and right module slots. The BSP discovers them through EEPROM address selection:

.. list-table::
   :widths: 20 20 60
   :header-rows: 1

   * - Slot
     - EEPROM Address
     - Description
   * - Left (``H2``)
     - 0x50
     - Address-select GPIO14, driven low
   * - Right (``H1``)
     - 0x51
     - Address-select GPIO39, driven high; mounted 180° relative to the left slot

Left Module Slot (``H2``) Pins
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. figure:: ../../_static/esp-mosaico/esp-mosaico-expansion-interface-left.png
   :alt: Left Module Interface Schematic (Click to enlarge)
   :scale: 45%
   :figclass: align-center

   Left Module Interface Schematic (Click to enlarge)

Pins are listed clockwise around the connector, starting from Pin1 (odd-numbered row first, then even-numbered row back).

.. list-table::
   :widths: 12 28 60
   :header-rows: 1

   * - Pin
     - Signal
     - Description
   * - 1
     - GPIO55
     - ADC
   * - 3
     - GPIO19
     - ADC
   * - 5
     - GPIO18
     - TOUCH
   * - 7
     - GPIO17
     - TOUCH
   * - 9
     - GPIO16
     - TOUCH
   * - 11
     - GPIO15
     - TOUCH
   * - 13
     - USJ_DN (GPIO33)
     - USB Serial/JTAG D-
   * - 15
     - USJ_DP (GPIO34)
     - USB Serial/JTAG D+
   * - 17
     - 5V_IN
     - External 5 V input (can power and charge the device)
   * - 19
     - VCC_3V3
     - 3.3 V output (controlled by GPIO60)
   * - 20
     - GND
     - Ground
   * - 18
     - 5V_OUT
     - 5 V output (controlled by GPIO60)
   * - 16
     - GPIO0
     - SDA (shared onboard I2C)
   * - 14
     - GPIO1
     - SCL (shared onboard I2C)
   * - 12
     - GPIO4
     - DAC
   * - 10
     - GPIO14
     - TOUCH; EEPROM address select
   * - 8
     - GPIO12
     - TOUCH
   * - 6
     - GPIO13
     - TOUCH
   * - 4
     - GPIO48
     - ADC
   * - 2
     - GPIO53
     - ADC

Right Module Slot (``H1``) Pins
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. figure:: ../../_static/esp-mosaico/esp-mosaico-expansion-interface-right.png
   :alt: Right Module Interface Schematic (Click to enlarge)
   :scale: 45%
   :figclass: align-center

   Right Module Interface Schematic (Click to enlarge)

Pins are listed clockwise around the connector, starting from Pin1 (odd-numbered row first, then even-numbered row back).

.. list-table::
   :widths: 12 28 60
   :header-rows: 1

   * - Pin
     - Signal
     - Description
   * - 1
     - GPIO49 \*
     - ADC; multiplexed with codec ``I2S_WS``
   * - 3
     - GPIO52 \*
     - ADC; multiplexed with codec ``I2S_DIN``
   * - 5
     - GPIO54 \*
     - ADC; multiplexed with codec ``I2S_MCLK``
   * - 7
     - GPIO37 \*
     - COMP; multiplexed with codec ``I2S_BCK``
   * - 9
     - GPIO40 \*
     - COMP; multiplexed with codec ``I2S_DOUT``
   * - 11
     - GPIO38
     - COMP
   * - 13
     - RX0 (GPIO59)
     - UART RX
   * - 15
     - TX0 (GPIO58)
     - UART TX
   * - 17
     - 5V_IN
     - External 5 V input (can power and charge the device)
   * - 19
     - VCC_3V3
     - 3.3 V output (controlled by GPIO60)
   * - 20
     - GND
     - Ground
   * - 18
     - 5V_OUT
     - 5 V output (controlled by GPIO60)
   * - 16
     - GPIO0
     - SDA (shared onboard I2C)
   * - 14
     - GPIO1
     - SCL (shared onboard I2C)
   * - 12
     - GPIO5
     - DAC
   * - 10
     - GPIO39
     - TOUCH; EEPROM address select
   * - 8
     - GPIO10
     - TOUCH
   * - 6
     - GPIO11
     - TOUCH
   * - 4
     - GPIO47
     - ADC
   * - 2
     - GPIO46
     - ADC

.. important::

  Pins marked with ``*`` (**5** in total) are multiplexed with the onboard audio codec I2S bus: ``GPIO49`` (``I2S_WS``), ``GPIO52`` (``I2S_DIN``), ``GPIO54`` (``I2S_MCLK``), ``GPIO37`` (``I2S_BCK``), and ``GPIO40`` (``I2S_DOUT``). These pins can be used for module expansion **only when codec audio is not required**. Do not use them for expansion if the application still needs the onboard ES8311 / speaker / microphone path.

.. note::

  When the left camera module claims DVP pins, GPIO14 (EEPROM address-select) is reused as DVP D4 and EEPROM access to that slot is suspended until the camera resource is released.

Hardware Revision Details
==========================

No previous versions.

.. _Related-documents_esp-mosaico_en:

Related Documents
=================

-  `ESP-Mosaico CoreBoard V1.0 Schematic`_ (PDF)
-  `2.16-inch 480 × 480 QSPI AMOLED Display Datasheet`_ (PDF)
-  `ESP32-S31 Series Datasheet`_ (HTML)
-  ESP-Mosaico BSP and examples: ``esp-mosaico`` repository
-  `ESP-Claw`_ (HTML)

.. _ESP-Mosaico CoreBoard V1.0 Schematic: https://dl.espressif.com/AE/SCH_SCH_ESP-Mosaico_CoreBoard_V1_0_2026-08-18.pdf
.. _2.16-inch 480 × 480 QSPI AMOLED Display Datasheet: https://dl.espressif.com/AE/H0216F002AMT004-1%20V1%E8%A7%84%E6%A0%BC%E4%B9%A62.16%E5%AF%B8%20480X480%20QSPI%20MIPI%20%20AMOLED.pdf
.. _ESP32-S31 Series Datasheet: https://documentation.espressif.com/esp32-s31_datasheet_en.html
.. _ESP-Claw: https://esp-claw.com/en/

Disclaimer and Copyright Notice
===============================

Please refer to the :doc:`Disclaimer and Copyright Notice <../disclaimer-and-copyright>`.
