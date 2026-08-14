======================
ESP32-S31-Korvo-1 V1.1
======================

:link_to_translation:`zh_CN:[中文]`

This user guide will help you get started with ESP32-S31-Korvo-1 V1.1 and provides detailed information about this development board.

The ESP32-S31-Korvo-1 V1.1 is a multimedia development board based on the ESP32-S31-WROOM-3 module for smart audio and HMI applications. It features a dual-microphone array, speaker output, a microSD card slot, and an onboard DVP camera; supports speech recognition and voice wake-up; and comes with a 4.3-inch LCD subboard (ESP32-S3-LCD-EV-Board-SUB3).


.. figure:: ../../_static/esp32-s31-korvo-1/esp32-s31-korvo-1-isometric.png
    :align: center
    :width: 85%
    :alt: ESP32-S31-Korvo-1 V1.1 (with ESP32-S31-WROOM-3 module on board)
    :figclass: align-center

    ESP32-S31-Korvo-1 V1.1 (with ESP32-S31-WROOM-3 module on board)


The ESP32-S31-Korvo-1 V1.1 main board can work with the LCD subboard ESP32-S3-LCD-EV-Board-SUB3. This document mainly introduces the ESP32-S31-Korvo-1 V1.1 main board. For more information about the LCD subboard, please refer to the `ESP32-S3-LCD-EV-Board User Guide <https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html>`__.


The document consists of the following major sections:

- `Getting Started`_: Overview of ESP32-S31-Korvo-1 V1.1 and hardware/software setup instructions to get started.
- `Hardware Reference`_: More detailed information about the ESP32-S31-Korvo-1 V1.1 hardware.
- `Hardware Revision Details`_: Information about revision history, known issues, and links to user guides for previous versions.
- `Related Documents`_: Links to related documentation.
- `Disclaimer and Copyright Notice`_: Link to the disclaimer and copyright notice.


Getting Started
===============

This section briefly introduces ESP32-S31-Korvo-1 V1.1 and explains how to perform the initial hardware setup and how to flash firmware onto the board.


Description of Components
-------------------------

.. figure:: ../../_static/esp32-s31-korvo-1/esp32-s31-korvo-1-callouts.png
    :align: center
    :width: 100%
    :alt: ESP32-S31-Korvo-1 V1.1 (click to enlarge)
    :figclass: align-center

    ESP32-S31-Korvo-1 V1.1 (click to enlarge)

The following list describes the key components on the board in a clockwise direction.

.. list-table::
   :widths: 10 20 70
   :header-rows: 1

   * - No.
     - Component
     - Description
   * - 1
     - USB Type-C Port (Power)
     - Power input only; no data communication.
   * - 2
     - USB Type-C Port (UART)
     - Can power the board, flash firmware to the chip, and communicate with the ESP32-S31 via the onboard USB-to-UART bridge.
   * - 3
     - USB-to-UART Bridge
     - Single-chip USB-to-UART bridge supporting up to 3 Mbps.
   * - 4
     - Power Switch
     - Slide toward ON to apply 5 V power; slide away from ON to disconnect 5 V power.
   * - 5
     - USB 2.0 Type-A Port
     - Connected to the ESP32-S31 USB 2.0 OTG High-Speed interface and supports the USB 2.0 standard. When used for USB communication, ESP32-S31 acts as a USB Host connected to downstream USB devices, providing up to 500 mA output current.
   * - 6
     - Buck Converter
     - Buck DC-DC converter for 3.3 V system power.
   * - 7
     - 5 V Power-on LED
     - Lights when USB power is connected to the board.
   * - 8
     - Switch
     - TPS2051C USB power switch with 500 mA current limit.
   * - 9
     - Right Speaker Output Port
     - Right-channel speaker output; can drive a 4 Ω, 3 W speaker. The pin spacing is 2.00 mm (0.08").
   * - 10
     - Right Microphone
     - Onboard right analog microphone routed to the audio codec.
   * - 11
     - 5 V to 3.3 V LDO
     - Converts 5 V to 3.3 V for the audio circuitry.
   * - 12
     - Right Audio PA Chip
     - NS4150B low-EMI 3 W mono class-D amplifier driving the right speaker from the codec output.
   * - 13
     - Function Buttons
     - Four buttons: PLAY, SET, VOL-, and VOL+, connected to ESP32-S31-WROOM-3 for UI control and audio application testing via dedicated APIs.
   * - 14
     - Audio Codec Chip
     - ES8389 low-power stereo codec with dual ADC/DAC, low-noise preamp, headphone driver, digital effects, analog mixing, and gain control. Connected to the ESP32-S31 over I2S and I2C for hardware audio processing independent of application software.
   * - 15
     - Left Audio PA Chip
     - NS4150B low-EMI 3 W mono class-D amplifier driving the left speaker from the codec output.
   * - 16
     - Left Microphone
     - Onboard left analog microphone routed to the audio codec.
   * - 17
     - Left Speaker Output Port
     - Left-channel speaker output; can drive a 4 Ω, 3 W speaker. The pin spacing is 2.00 mm (0.08").
   * - 18
     - RGB LED
     - Addressable RGB LED driven by GPIO37.

.. list-table::
   :widths: 10 20 70
   :header-rows: 1

   * - No.
     - Component
     - Description
   * - 19
     - 3.3 V to 1.8 V LDO (NC)
     - Converts 3.3 V to 1.8 V for 1.8 V SPI NAND flash. Not populated by default (NC).
   * - 20
     - SPI NAND Flash (NC)
     - Quad SPI NAND flash sharing ESP32-S31-WROOM-3 signals with the microSD interface. Not populated by default (NC).
   * - 21
     - LCD Connector
     - Connector for the LCD subboard.
   * - 22
     - ESP32-S31-WROOM-3
     - ESP32-S31-WROOM-3 is a general-purpose module supporting 2.4 GHz Wi-Fi 6, Bluetooth 5.4, Bluetooth Classic, and IEEE 802.15.4 (Zigbee 3.0 and Thread 1.4). The module integrates ESP32-S31, 16 MB SPI flash, and 16 MB PSRAM, and uses an onboard PCB antenna.
   * - 23
     - microSD Card Slot
     - Supports 4-bit microSD for audio storage and playback. SDIO 3.0 capable.
   * - 24
     - 3.3 V to 2.8 V LDO
     - Converts 3.3 V to 2.8 V for the camera module.
   * - 25
     - 3.3 V to 1.5 V LDO
     - Converts 3.3 V to 1.5 V for the camera module.
   * - 26
     - Camera Connector
     - Connector for the onboard camera module for image transfer.
   * - 27
     - Reset Button
     - System reset.
   * - 28
     - Boot Button
     - Firmware download: hold Boot, press and release Reset to enter download mode; flash over the serial port.


Start Application Development
-----------------------------

Before powering up the board, please make sure that it is in good condition with no obvious sign of damage.

Required Hardware
^^^^^^^^^^^^^^^^^

- ESP32-S31-Korvo-1 V1.1
- LCD subboard: ESP32-S3-LCD-EV-Board-SUB3
- One or two speakers
- Two USB 2.0 cables (Standard-A to Type-C)
- Computer running Windows, Linux, or macOS

.. note::

  Be sure to use a good-quality USB cable. Some cables are for charging only and do not provide the needed data lines nor work for programming the board.


Optional Hardware
^^^^^^^^^^^^^^^^^

- microSD card


Hardware Setup
^^^^^^^^^^^^^^

1. Connect the LCD subboard to the **LCD Connector**.
2. Connect one or two speakers to the **speaker output** port(s).
3. Plug in two USB cables from the PC to both USB ports on the board.
4. Turn on the **power switch**.
5. The red power LED should light.


.. _esp32-s31-korvo-1-software-setup:

Software Setup
^^^^^^^^^^^^^^

Please proceed to `ESP-IDF Get Started <https://docs.espressif.com/projects/esp-idf/en/latest/esp32s31/get-started/index.html>`__, which will help you set up the development environment quickly and then flash an application onto your board.

To validate the board peripherals, you can also flash the :project:`factory demo <examples/esp32-s31-korvo/examples/factory_demo>`.

.. note::

  The board uses USB ports to communicate with the computer. Most operating systems (Windows, Linux, macOS) come with the required drivers pre-installed and the board is recognized automatically once plugged in. If the device cannot be recognized or a serial connection cannot be established, refer to `Establish Serial Connection with ESP32-S31 <https://docs.espressif.com/projects/esp-idf/en/latest/esp32s31/get-started/establish-serial-connection.html>`__ for detailed driver installation steps.

Espressif provides board peripheral management components for many development boards to help you initialize and use key onboard peripherals such as LCD, audio codec, keys, and LEDs more easily and efficiently. Please visit the `esp_board_manager component on the ESP Component Registry <https://components.espressif.com/components/espressif/esp_board_manager>`__ for support coverage.


Other Development Framework Options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In addition to ESP-IDF, this board supports the following frameworks and options for different user needs:

- Espressif Bluetooth LE ecosystem: Develop Bluetooth LE applications using solutions such as ESP-BLE-MESH and ESP-BLE-AUDIO to accelerate time-to-market and mass production.
- `ESP-Brookesia <https://github.com/espressif/esp-brookesia>`__: Human-machine interaction framework for AIoT devices for graphical UI and smart display applications.
- `ESP-GMF <https://github.com/espressif/esp-gmf>`__: Espressif general multimedia framework with audio/video processing components for multimedia applications.

  - `Bluetooth audio <https://github.com/espressif/esp-gmf/tree/main/packages/esp_bt_audio>`__: Unified Bluetooth audio APIs supporting Bluetooth Classic and LE Audio.

- `ESP Video Components <https://github.com/espressif/esp-video-components>`__: Camera, video streaming, and video processing components for image capture and video applications.
- `ESP-Matter <https://github.com/espressif/esp-matter>`__: Build Matter and Thread devices suited to low-power and battery-powered scenarios.


Contents and Packaging
----------------------

The package contains:

- ESP32-S31-Korvo-1 V1.1 main board (with the camera module installed)
- LCD subboard: ESP32-S3-LCD-EV-Board-SUB3


Retail Orders
^^^^^^^^^^^^^

If you order a few samples, each board comes in an individual package in either an antistatic bag or any other packaging depending on your retailer.

For retail orders, please go to `Get Samples <https://www.espressif.com/en/company/contact/buy-a-sample>`__.


Wholesale Orders
^^^^^^^^^^^^^^^^

If you order in bulk, the boards come in large cardboard boxes.

For wholesale orders, please go to `Contact Us <https://www.espressif.com/en/contact-us/sales-questions>`__.


Hardware Reference
==================

Block Diagram
-------------

The main components of ESP32-S31-Korvo-1 V1.1 and their interconnections are shown in the block diagram below.

.. figure:: ../../_static/esp32-s31-korvo-1/esp32-s31-korvo-1-block-diagram.png
    :align: center
    :width: 70%
    :alt: ESP32-S31-Korvo-1 V1.1 electrical block diagram (click to enlarge)
    :figclass: align-center

    ESP32-S31-Korvo-1 V1.1 electrical block diagram (click to enlarge)


Power Supply Description
------------------------

USB Power
^^^^^^^^^^^^^^^^^^^^^

Both USB Type-C ports can power the board: the Power port is power-only, while the UART port can supply power and carry data. When driving high-power speakers and using the USB Type-A port to power external devices at the same time, ensure the total input current to the board meets 3 A. Use a dedicated cable for USB power delivery, separate from the USB cable used for flashing applications.

.. figure:: ../../_static/esp32-s31-korvo-1/esp32-s31-korvo-1-usb-ps.png
    :align: center
    :width: 85%
    :alt: ESP32-S31-Korvo-1 V1.1 - USB power supply (click to enlarge)
    :figclass: align-center

    ESP32-S31-Korvo-1 V1.1 - USB power supply (click to enlarge)


Separate Audio Power
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

ESP32-S31-Korvo-1 V1.1 provides independent power supplies for audio components to reduce digital noise in audio signals and improve overall performance.

.. figure:: ../../_static/esp32-s31-korvo-1/esp32-s31-korvo-1-audio-ps.png
    :align: center
    :width: 85%
    :alt: ESP32-S31-Korvo-1 V1.1 - audio power supply (click to enlarge)
    :figclass: align-center

    ESP32-S31-Korvo-1 V1.1 - audio power supply (click to enlarge)


ES8389 Audio Codec Notes
--------------------------

ESP32-S31-Korvo-1 V1.1 uses the ES8389 audio codec. When transferring audio data with the SoC, note the following:

- Both DAC and ADC signals must be sent to the SoC in **TDM (Time Division Multiplexing)** format.
- **Pin 4 (MCLK)** of the ES8389 can be muxed to **TDMIN** via register configuration.

.. figure:: ../../_static/esp32-s31-korvo-1/esp32-s31-korvo-1-es8389-schematic.png
    :align: center
    :width: 85%
    :alt: ESP32-S31-Korvo-1 V1.1 - ES8389 audio codec schematic (click to enlarge)
    :figclass: align-center

    ESP32-S31-Korvo-1 V1.1 - ES8389 audio codec schematic (click to enlarge)


microSD Card and SPI NAND Flash
---------------------------------

The microSD card and SPI NAND flash functions share GPIO20–GPIO25 on the ESP32-S31-WROOM-3 module. The board uses the microSD card function by default. To switch to the SPI NAND flash function, perform hardware rework: remove R7, R65, R66, R67, R68, and R69, and populate R22, R23, R1, R2, R3, R4, C6, R20, and U4. Note that ESP32-S31 supports both 1.8 V and 3.3 V NAND flash. For **1.8 V** NAND flash, you must also populate R134, C66, C80, R100, U1, C82, and C67; for **3.3 V** NAND flash, populate R135.

.. figure:: ../../_static/esp32-s31-korvo-1/esp32-s31-korvo-1-microsd.png
    :align: center
    :width: 85%
    :alt: ESP32-S31-Korvo-1 V1.1 - microSD card function (click to enlarge)
    :figclass: align-center

    ESP32-S31-Korvo-1 V1.1 - microSD card function (click to enlarge)

.. figure:: ../../_static/esp32-s31-korvo-1/esp32-s31-korvo-1-spi-nand-flash.png
    :align: center
    :width: 85%
    :alt: ESP32-S31-Korvo-1 V1.1 - SPI NAND flash function (click to enlarge)
    :figclass: align-center

    ESP32-S31-Korvo-1 V1.1 - SPI NAND flash function (click to enlarge)


Pin Assignment Table
----------------------

The table below lists the ESP32-S31-WROOM-3 module signals routed to onboard peripherals, grouped by peripheral.

.. list-table:: ESP32-S31-WROOM-3 pin assignment
   :header-rows: 1
   :widths: 22 20 16 42

   * - Peripheral
     - Signal
     - GPIO
     - Description
   * - Audio I2S
     - I2S_MCLK
     - GPIO2
     - Master clock
   * -
     - I2S_SCLK
     - GPIO3
     - Serial clock
   * -
     - I2S_LRCLK
     - GPIO4
     - Frame clock (left/right channel select)
   * -
     - I2S_SDOUT
     - GPIO5
     - Audio output data line (DAC playback)
   * -
     - I2S_DSIN
     - GPIO6
     - Audio input data line (ADC recording)
   * - Audio I2C
     - I2C_SDA
     - GPIO0
     - I2C data line for ES8389
   * -
     - I2C_SCL
     - GPIO1
     - I2C clock line for ES8389
   * - System
     - PA_CTRL
     - GPIO7
     - Enable control for left/right audio PA chips
   * -
     - WS2812_CTRL
     - GPIO37
     - Addressable RGB LED control
   * -
     - ADC BUTTON
     - GPIO42
     - Function buttons (PLAY/SET/VOL-/VOL+) via resistor divider ADC
   * - UART0
     - U0TXD
     - GPIO58
     - UART0 transmit (USB-to-UART bridge)
   * -
     - U0RXD
     - GPIO59
     - UART0 receive (USB-to-UART bridge)
   * - microSD / SPI NAND
     - SDIO_DATA0
     - GPIO20
     - Default microSD; SPI2_CLK when switched to SPI NAND
   * -
     - SDIO_DATA1
     - GPIO21
     - Default microSD; SPI2_D when switched to SPI NAND
   * -
     - SDIO_DATA2
     - GPIO22
     - Default microSD; SPI2_Q when switched to SPI NAND
   * -
     - SDIO_DATA3
     - GPIO23
     - Default microSD; SPI2_CS when switched to SPI NAND
   * -
     - SDIO_CLK
     - GPIO24
     - Default microSD; SPI2_HOLD when switched to SPI NAND
   * -
     - SDIO_CMD
     - GPIO25
     - Default microSD; SPI2_WP when switched to SPI NAND
   * -
     - SD_CTRL
     - GPIO39
     - microSD power/switch control; also Boot Mode 1
   * - USB 2.0
     - USB2.0_DP
     - USB_DP
     - USB 2.0 D+
   * -
     - USB2.0_DM
     - USB_DM
     - USB 2.0 D-
   * - RGB LCD
     - DB0 (B3)
     - GPIO8
     - LCD data bit 0
   * -
     - DB1 (B4)
     - GPIO9
     - LCD data bit 1
   * -
     - DB2 (B5)
     - GPIO10
     - LCD data bit 2
   * -
     - DB3 (B6)
     - GPIO11
     - LCD data bit 3
   * -
     - DB4 (B7)
     - GPIO12
     - LCD data bit 4
   * -
     - DB5 (G2)
     - GPIO13
     - LCD data bit 5
   * -
     - DB6 (G3)
     - GPIO14
     - LCD data bit 6
   * -
     - DB7 (G4)
     - GPIO15
     - LCD data bit 7
   * -
     - DB8 (G5)
     - GPIO16
     - LCD data bit 8
   * -
     - DB9 (G6)
     - GPIO17
     - LCD data bit 9
   * -
     - DB10 (G7)
     - GPIO18
     - LCD data bit 10
   * -
     - DB11 (R3)
     - GPIO19
     - LCD data bit 11
   * -
     - DB12 (R4)
     - GPIO33
     - LCD data bit 12
   * -
     - DB13 (R5)
     - GPIO34
     - LCD data bit 13
   * -
     - DB14 (R6)
     - GPIO35
     - LCD data bit 14
   * -
     - DB15 (R7)
     - GPIO36
     - LCD data bit 15
   * -
     - LCD_CS
     - GPIO38
     - LCD chip select; also Boot Mode 0 and GM_FK
   * -
     - LCD_PCLK
     - GPIO40
     - LCD pixel clock; also Boot Mode 2
   * -
     - LCD_H_EN
     - GPIO43
     - LCD data enable
   * -
     - LCD_H_SYNC
     - GPIO44
     - LCD horizontal sync
   * -
     - LCD_V_SYNC
     - GPIO45
     - LCD vertical sync
   * -
     - LCD_MOSI
     - GPIO60
     - LCD SPI MOSI; also Boot Mode 3
   * -
     - LCD_SCK
     - GPIO61
     - LCD SPI clock; also Boot Mode 4
   * - DVP camera
     - CAM_D0
     - GPIO46
     - Camera data bit 0
   * -
     - CAM_D1
     - GPIO47
     - Camera data bit 1
   * -
     - CAM_D2
     - GPIO48
     - Camera data bit 2
   * -
     - CAM_D3
     - GPIO49
     - Camera data bit 3
   * -
     - CAM_D4
     - GPIO50
     - Camera data bit 4
   * -
     - CAM_D5
     - GPIO51
     - Camera data bit 5
   * -
     - CAM_D6
     - GPIO52
     - Camera data bit 6
   * -
     - CAM_D7
     - GPIO53
     - Camera data bit 7
   * -
     - CAM_PCLK
     - GPIO54
     - Camera pixel clock
   * -
     - CAM_XCLK
     - GPIO55
     - Camera master clock
   * -
     - CAM_V_SYNC
     - GPIO56
     - Camera vertical sync
   * -
     - CAM_H_SYNC
     - GPIO57
     - Camera horizontal sync
   * -
     - GM_FK
     - GPIO38
     - Camera GM_FK; shared with LCD_CS and Boot Mode 0


Hardware Setup Options
----------------------

Automatic Download
^^^^^^^^^^^^^^^^^^

There are two ways to put the ESP development board into download mode:

- Manually press the Boot and RST buttons, then release RST first and Boot afterwards.
- Let software perform automatic download using the serial port DTR and RTS signals to control EN and GPIO61. For details, see the `ESP32-S31-Korvo-1 V1.1 schematic`_ (PDF).


Hardware Revision Details
=========================

  - ESP32-S31-Korvo-1 V1.1:

    Matte black solder mask; larger PCB footprint; LCD subboard stacks on top of the main board when assembled. GPIO mapping is unchanged from V1.0.

  - ESP32-S31-Korvo-1 V1.0:

    First revision with green solder mask; LCD subboard extended beyond the board edge so onboard functions remained exposed for debugging.


Related Documents
=================

.. only:: latex

   Please download the following documents from the `HTML version of esp-dev-kits Documentation <https://docs.espressif.com/projects/esp-dev-kits/en/latest/{IDF_TARGET_PATH_NAME}/index.html>`_.


- `ESP32-S31 Datasheet`_ (PDF)
- `ESP32-S31-WROOM-3 Datasheet`_ (PDF)
- `ESP32-S31-Korvo-1 V1.1 schematic`_ (PDF)
- `ESP32-S31-Korvo-1 V1.1 PCB layout`_ (PDF)
- `ESP32-S31-Korvo-1 V1.1 dimensions`_ (PDF)
- `ESP32-S31-Korvo-1 V1.1 dimensions source file`_ (DXF) - You can view it with `Autodesk Viewer <https://viewer.autodesk.com/>`_ online
- `ESP32-S31-Korvo-1 V1.1 3D Printed Case Files`_ (STL & STEP) - 3D printable case files are available here
- `ESP32-S31-Korvo-1 Reference Design`_ (ZIP)

.. _ESP32-S31 Datasheet: https://documentation.espressif.com/esp32-s31_datasheet_en.pdf
.. _ESP32-S31-WROOM-3 Datasheet: https://documentation.espressif.com/esp32-s31-wroom-3_datasheet_en.pdf
.. _ESP32-S31-Korvo-1 V1.1 schematic: https://dl.espressif.com/schematics/esp32-s31-korvo-1-schematics.pdf
.. _ESP32-S31-Korvo-1 V1.1 PCB layout: https://dl.espressif.com/schematics/esp32-s31-korvo-1-pcb-layout.pdf
.. _ESP32-S31-Korvo-1 V1.1 dimensions: https://dl.espressif.com/schematics/esp32-s31-korvo-1-dimensions.pdf
.. _ESP32-S31-Korvo-1 V1.1 dimensions source file: https://dl.espressif.com/schematics/esp32-s31-korvo-1-dimensions.dxf
.. _ESP32-S31-Korvo-1 V1.1 3D Printed Case Files: https://github.com/espressif/esp-dev-kits/tree/master/examples/esp32-s31-korvo
.. _ESP32-S31-Korvo-1 Reference Design: https://documentation.espressif.com/ESP32-S31-Korvo-1_Reference_Design_EN.zip


For further design documentation for the board, please contact us at `sales@espressif.com <sales@espressif.com>`_.

Disclaimer and Copyright Notice
===============================

See :doc:`Disclaimer and Copyright Notice <../disclaimer-and-copyright>`.

