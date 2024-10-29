==========================
ESP32-P4-Function-EV-Board
==========================

:link_to_translation:`zh_CN:[中文]`

This user guide will help you get started with ESP32-P4-Function-EV-Board and will also provide more in-depth information.

ESP32-P4-Function-EV-Board is a multimedia development board based on the ESP32-P4 chip. ESP32-P4 chip features a dual-core 400 MHz RISC-V processor and supports up to 32 MB PSRAM. In addition, ESP32-P4 supports USB 2.0 specification, MIPI-CSI/DSI, H264 Encoder, and various other peripherals. With all of its outstanding features, the board is an ideal choice for developing low-cost, high-performance, low-power network-connected audio and video products.

The 2.4 GHz Wi-Fi 6 & Bluetooth 5 (LE) module ESP32-C6-MINI-1 serves as the Wi-Fi and Bluetooth module of the board. The board also includes a 7-inch capacitive touch screen with a resolution of 1024 x 600 and a 2MP camera with MIPI CSI, enriching the user interaction experience. The development board is suitable for prototyping a wide range of products, including visual doorbells, network cameras, smart home central control screens, LCD electronic price tags, two-wheel vehicle dashboards, etc.

Most of the I/O pins are broken out to the pin headers for easy interfacing. Developers can connect peripherals with jumper wires.

.. figure:: ../../../_static/esp32-p4-function-ev-board/esp32-p4-function-ev-board-isometric.png
    :align: center
    :alt: ESP32-P4-Function-EV-Board
    :figclass: align-center

    ESP32-P4-Function-EV-Board

The document consists of the following major sections:

- `Getting Started`_: Overview of ESP32-P4-Function-EV-Board and hardware/software setup instructions to get started.
- `Hardware Reference`_: More detailed information about the ESP32-P4-Function-EV-Board's hardware.
- `Hardware Revision Details`_: Revision history, known issues, and links to user guides for previous versions (if any) of ESP32-P4-Function-EV-Board.
- `Related Documents`_: Links to related documentation.

Getting Started
===============

This section provides a brief introduction to ESP32-P4-Function-EV-Board, instructions on how to do the initial hardware setup and how to flash firmware onto it.

Description of Components
-------------------------

.. _user-guide-esp32-p4-function-ev-board-front:

.. figure:: ../../../_static/esp32-p4-function-ev-board/esp32-p4-function-ev-board-annotated-photo-front.png
    :align: center
    :width: 100%
    :alt: ESP32-P4-Function-EV-Board - front (click to enlarge)
    :figclass: align-center

    ESP32-P4-Function-EV-Board - front (click to enlarge)

.. figure:: ../../../_static/esp32-p4-function-ev-board/esp32-p4-function-ev-board-annotated-photo-back.png
    :align: center
    :width: 100%
    :alt: ESP32-P4-Function-EV-Board - back (click to enlarge)
    :figclass: align-center

    ESP32-P4-Function-EV-Board - back (click to enlarge)

The key components of the board are described in a clockwise direction.

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Key Component
     - Description
   * - J1
     - All available GPIO pins are broken out to the header block J1 for easy interfacing. For more details, see :ref:`header-block`.
   * - ESP32-C6 Module Programming Connector
     - The connector can be used with ESP-Prog or other UART tools to flash firmware onto the ESP32-C6 module.
   * - ESP32-C6-MINI-1 Module
     - This module serves as the Wi-Fi and Bluetooth communication module for the board.
   * - Microphone
     - Onboard microphone connected to the interface of Audio Codec Chip.
   * - Reset Button
     - Resets the board.
   * - Audio Codec Chip
     - ES8311 is a low-power mono audio codec chip. It includes a single-channel ADC, a single-channel DAC, a low-noise pre-amplifier, a headphone driver, digital sound effects, analog mixing, and gain functions. It interfaces with the ESP32-P4 chip over I2S and I2C buses to provide hardware audio processing independent of the audio application.
   * - Speaker Output Port
     - This port is used to connect a speaker. The maximum output power can drive a 4 Ω, 3 W speaker. The pin spacing is 2.00 mm (0.08”).
   * - Audio PA Chip
     - NS4150B is an EMI-compliant, 3 W mono Class D audio power amplifier that amplifies audio signals from the audio codec chip to drive speakers.
   * - 5 V to 3.3 V LDO
     - A power regulator that converts a 5 V supply to a 3.3 V output.
   * - BOOT Button
     - The boot mode control button. Press the **Reset Button** while holding down the **Boot Button** to reset ESP32-P4 and enter firmware download mode. Firmware can then be downloaded to SPI flash via the USB-to-UART Port.
   * - Ethernet PHY IC
     - Ethernet PHY chip connected to the ESP32-P4 EMAC RMII interface and RJ45 Ethernet Port.
   * - Buck Converter
     - A buck DC-DC converter for the 3.3 V power supply.
   * - USB-to-UART Bridge Chip
     - CP2102N is a single USB-to-UART bridge chip connected to the ESP32-P4 UART0 interface, CHIP_PU, and GPIO35 (strapping pin). It provides transfer rates up to 3 Mbps for firmware downloading and debugging, supporting the automatic download functionality.
   * - 5 V Power-on LED
     - This LED lights up when the board is powered through any USB Type-C port.
   * - RJ45 Ethernet Port
     - An Ethernet Port supporting 10/100 Mbps adaptive.
   * - USB-to-UART Port
     - The USB Type-C port can be used to power the board, flash firmware to the chip, and communicate with the ESP32-P4 chip via the USB-to-UART Bridge Chip.
   * - USB Power-in Port
     - The USB Type-C port used to power the board.
   * - USB 2.0 Type-C Port
     - The USB 2.0 Type-C Port is connected to the USB 2.0 OTG High-Speed interface of ESP32-P4, compliant with the USB 2.0 specification. When communicating with other devices via this port, ESP32-P4 acts as a USB device connecting to a USB host. Please note that USB 2.0 Type-C Port and USB 2.0 Type-A Port cannot be used simultaneously. USB 2.0 Type-C Port can also be used for powering the board.
   * - USB 2.0 Type-A Port
     - The USB 2.0 Type-A Port is connected to the USB 2.0 OTG High-Speed interface of ESP32-P4, compliant with the USB 2.0 specification. When communicating with other devices via this port, ESP32-P4 acts as a USB host, providing up to 500 mA of current. Please note that USB 2.0 Type-C Port and USB 2.0 Type-A Port cannot be used simultaneously.
   * - Power Switch
     - Power On/Off Switch. Toggling toward the ON sign powers the board on (5 V), toggling away from the ON sign powers the board off.
   * - Switch
     - TPS2051C is a USB power switch that provides a 500 mA output current limit.
   * - MIPI CSI Connector
     - The FPC connector 1.0K-GT-15PB is used for connecting external camera modules to enable image transmission. For details, please refer to 1.0K-GT-15PB specification in Related Documents. FPC specifications: 1.0 mm pitch, 0.7 mm pin width, 0.3 mm thickness, 15 pins.
   * - Buck Converter
     - A buck DC-DC converter for VDD_HP power supply of ESP32-P4.
   * - ESP32-P4
     - A high-performance MCU with large internal memory and powerful image and voice processing capabilities.
   * - 40 MHz XTAL
     - An external precision 40 MHz crystal oscillator that serves as a clock for the system.
   * - 32.768 kHz XTAL
     - An external precision 32.768 kHz crystal oscillator that serves as a low-power clock while the chip is in deep-sleep mode.
   * - MIPI DSI Connector
     - The FPC connector 1.0K-GT-15PB is used for connecting displays. For details, please refer to 1.0K-GT-15PB Specification in Related Documents. FPC specifications: 1.0 mm pitch, 0.7 mm pin width, 0.3 mm thickness, 15 pins.
   * - SPI flash
     - The 16 MB flash is connected to the chip via the SPI interface.
   * - MicroSD Card Slot
     - The development board supports a MicroSD card in 4-bit mode and can store or play audio files from the MicroSD card.

Accessories
------------------

Optionally, the following accessories are included in the package:

- LCD and its accessories (optional)

  * 7-inch capacitive touch screen with a resolution of 1024 x 600
  * LCD adapter board
  * Accessories bag, including DuPont wires, ribbon cable for LCD, long standoffs (20 mm in length), and short standoffs (8 mm in length)

- Camera and its accessories (optional)

  * 2MP camera with MIPI CSI
  * Camera adapter board
  * Ribbon cable for camera

.. figure:: ../../../_static/esp32-p4-function-ev-board/ribbon_cable.png
    :align: center
    :scale: 30%
    :alt: Ribbon Cables in Forward and Reverse Directions
    :figclass: align-center

    Ribbon Cables in Forward and Reverse Directions

.. note::

  Please note that the ribbon cable in the **forward direction**, whose strips at the two ends are on the same side, should be used for the **camera**; the ribbon cable in the **reverse direction**, whose strips at the two ends are on different sides, should be used for the **LCD**.

Start Application Development
------------------------------------

Before powering up your ESP32-P4-Function-EV-Board, please make sure that it is in good condition with no obvious signs of damage.

Required Hardware
^^^^^^^^^^^^^^^^^

- ESP32-P4-Function-EV-Board
- USB cables
- Computer running Windows, Linux, or macOS

.. note::

  Be sure to use a good quality USB cable. Some cables are for charging only and do not provide the needed data lines nor work for programming the boards.

Optional Hardware
^^^^^^^^^^^^^^^^^

- MicroSD card

Hardware Setup
^^^^^^^^^^^^^^

Connect the ESP32-P4-Function-EV-Board to your computer using a USB cable. The board can be powered through any of the USB Type-C ports. The USB-to-UART Port is recommended for flashing firmware and debugging.

To connect the LCD, follow these steps:

1. Secure the development board to the LCD adapter board by attaching the short copper standoffs (8 mm in length) to the four standoff posts at the center of the LCD adapter board.
2. Connect the J3 header of the LCD adapter board to the MIPI DSI connector on the ESP32-P4-Function-EV-Board using the LCD ribbon cable (**reverse direction**). Note that the LCD adapter board is already connected to the LCD.
3. Use a DuPont wire to connect the RST_LCD pin of the J6 header of the LCD adapter board to the GPIO27 pin of the J1 header on the ESP32-P4-Function-EV-Board. The RST_LCD pin can be configured via software, with GPIO27 set as the default.
4. Use a DuPont wire to connect the PWM pin of the J6 header of the LCD adapter board to the GPIO26 pin of the J1 header on the ESP32-P4-Function-EV-Board. The PWM pin can be configured via software, with GPIO26 set as the default.
5. It is recommended to power the LCD by connecting a USB cable to the J1 header of the LCD adapter board. If this is not feasible, connect the 5V and GND pins of the LCD adapter board to corresponding pins on the J1 header of the ESP32-P4-Function-EV-Board, provided that the development board has sufficient power supply.
6. Attach the long copper standoffs (20 mm in length) to the four standoff posts on the periphery of the LCD adapter board to allow the LCD to stand upright.

In summary, the LCD adapter board and ESP32-P4-Function-EV-Board are connected via the following pins:

.. list-table::
  :widths: 20 20
  :header-rows: 1

  * - LCD Adapter Board
    - ESP32-P4-Function-EV
  * - J3 header
    - MIPI DSI connector
  * - RST_LCD pin of J6 header
    - GPIO27 pin of J1 header
  * - PWM pin of J6 header
    - GPIO26 pin of J1 header
  * - 5V pin of J6 header
    - 5V pin of J1 header
  * - GND pin of J6 header
    - GND pin of J1 header

.. note::

  - If you power the LCD adapter board by connecting a USB cable to its J1 header, you do not need to connect its 5V and GND pins to the corresponding pins on the development board.
  - To use the camera, connect the camera adapter board to the MIPI CSI connector on the development board using the camera ribbon cable (**forward direction**).

Software Setup
^^^^^^^^^^^^^^

To set up your development environment and flash an application example onto your board, please follow the instructions in `ESP-IDF Get Started <https://docs.espressif.com/projects/esp-idf/en/latest/esp32p4/get-started/index.html>`__.

You can find examples for ESP32-P4-Function-EV by accessing :project:`Examples <esp32-p4-function-ev-board/examples>`. To configure project options, enter ``idf.py menuconfig`` in the example directory.

Hardware Reference
==================

Block Diagram
-------------

The block diagram below shows the components of ESP32-P4-Function-EV-Board and their interconnections.

.. figure:: ../../../_static/esp32-p4-function-ev-board/esp32-p4-function-ev-board-block-diagram.png
    :align: center
    :width: 100%
    :alt: ESP32-P4-Function-EV-Board (click to enlarge)
    :figclass: align-center

    ESP32-P4-Function-EV-Board (click to enlarge)

.. _power-supply-options:

Power Supply Options
--------------------

Power can be supplied through any of the following ports:

- USB 2.0 Type-C Port
- USB Power-in Port
- USB-to-UART Port

If the USB cable used for debugging cannot provide enough current, you can connect the board to a power adapter via any available USB Type-C port.

.. _header-block:

Header Block
-------------

The tables below provide the **Name** and **Function** of the pin header J1 of the board. The pin header names are shown in Figure :ref:`user-guide-esp32-p4-function-ev-board-front`. The numbering is the same as in the `ESP32-P4-Function-EV-Board Schematic <../../_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-schematics.pdf>`_.

J1
^^^
===  =======  ==========  ==========================================
No.  Name     Type [1]_   Function
===  =======  ==========  ==========================================
1    3V3      P           3.3 V power supply
2    5V       P           5 V power supply
3    7        I/O/T       GPIO7
4    5V       P           5 V power supply
5    8        I/O/T       GPIO8
6    GND      GND         Ground
7    23       I/O/T       GPIO23
8    37       I/O/T       U0TXD, GPIO37
9    GND      GND         Ground
10   38       I/O/T       U0RXD, GPIO38
11   21       I/O/T       GPIO21
12   22       I/O/T       GPIO22
13   20       I/O/T       GPIO20
14   GND      GND         Ground
15   6        I/O/T       GPIO6
16   5        I/O/T       GPIO5
17   3V3      P           3.3 V power supply
18   4        I/O/T       GPIO4
19   3        I/O/T       GPIO3
20   GND      GND         Ground
21   2        I/O/T       GPIO2
22   NC(1)    I/O/T       GPIO1 [2]_
23   NC(0)    I/O/T       GPIO0 [2]_
24   36       I/O/T       GPIO36
25   GND      GND         Ground
26   32       I/O/T       GPIO32
27    24      I/O/T       GPIO24
28    25      I/O/T       GPIO25
29   33       I/O/T       GPIO33
30   GND      GND         Ground
31   26       I/O/T       GPIO26
32   54       I/O/T       GPIO54
33   48       I/O/T       GPIO48
34   GND      GND         Ground
35   53       I/O/T       GPIO53
36   46       I/O/T       GPIO46
37   47       I/O/T       GPIO47
38   27       I/O/T       GPIO27
39   GND      GND         Ground
40   NC(45)   I/O/T       GPIO45 [3]_
===  =======  ==========  ==========================================

.. [1] P: Power supply; I: Input; O: Output; T: High impedance.
.. [2] GPIO0 and GPIO1 can be enabled by disabling the XTAL_32K function, which can be achieved by moving R61 and R59 to R199 and R197, respectively.
.. [3] GPIO45 can be enabled by disabling the SD_PWRn function, which can be achieved by moving R231 to R100.

Hardware Revision Details
=========================

No previous versions available.

.. note::

  The current version of the development board is v1.4, and the next version, v1.5, is in the verification stage. `ESP32-P4-Function-EV-Board Schematic v1.5 <../../_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-schematics_v1.5.pdf>`_ has been uploaded for your reference.

Related Documents
=================

* `ESP32-P4-Function-EV-Board Schematic <../../_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-schematics.pdf>`_ (PDF)
* `ESP32-P4-Function-EV-Board Schematic v1.5 <../../_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-schematics_v1.5.pdf>`_ (PDF)
* `ESP32-P4-Function-EV-Board PCB Layout <../../_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-pcb-layout.pdf>`_ (PDF)
* `ESP32-P4-Function-EV-Board Dimensions <../../_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-dimensions.pdf>`_ (PDF)
* `ESP32-P4-Function-EV-Board Dimensions source file <../../_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-dimensions.dxf>`_ (DXF) - You can view it with `Autodesk Viewer <https://viewer.autodesk.com/>`_ online
* `1.0K-GT-15PB Specification <../../_static/esp32-p4-function-ev-board/schematics/1.0K-GT-15PB_specification.pdf>`_ (PDF)
* `Camera Datasheet <../../_static/esp32-p4-function-ev-board/camera_display_datasheet/camera_datasheet.pdf>`_ (PDF)
* `Display Datasheet <../../_static/esp32-p4-function-ev-board/camera_display_datasheet/display_datasheet.pdf>`_ (PDF)
* `Datasheet of display driver chip EK73217BCGA <../../_static/esp32-p4-function-ev-board/camera_display_datasheet/display_driver_chip_EK73217BCGA_datasheet.pdf>`_ (PDF)
* `Datasheet of display driver chip EK79007AD <../../_static/esp32-p4-function-ev-board/camera_display_datasheet/display_driver_chip_EK79007AD_datasheet.pdf>`_ (PDF)
* `LCD Adapter Board Schematic <../../_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-lcd-subboard-schematics.pdf>`_ (PDF)
* `LCD Adapter Board PCB Layout <../../_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-lcd-subboard-pcb-layout.pdf>`_ (PDF)
* `Camera Adapter Board Schematic <../../_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-camera-subboard-schematics.pdf>`_ (PDF)
* `Camera Adapter Board PCB Layout <../../_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-camera-subboard-pcb-layout.pdf>`_ (PDF)

For further design documentation for the board, please contact us at `sales@espressif.com <sales@espressif.com>`_.
