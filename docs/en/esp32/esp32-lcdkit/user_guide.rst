ESP32-LCDKit
============

:link_to_translation:`zh_CN:[中文]`

Overview
----------

ESP32-LCDKit is an HMI (Human Machine Interface) development board with the ESP32-DevKitC at its core. ESP32-LCDKit is integrated with such peripherals as SD-Card, DAC-Audio, and can be connected to an external display. The board is mainly used for HMI-related development and evaluation.Development board reserved screen interface type: SPI serial interface, 8-bit parallel interface, 16-bit parallel interface.

You may find HMI-related examples running with ESP32-LCDKit in `HMI Example <https://github.com/espressif/esp-iot-solution/tree/release/v1.1/examples/hmi>`__.

For more information on ESP32, please refer to `ESP32 Series Datasheet <https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf>`__.

.. figure:: ../../../_static/esp32-lcdkit/esp32_lcdkit.jpg
   :align: center
   :alt: ESP32-LCDKit
   :figclass: align-center
 
   ESP32-LCDKit 

Block Diagram and PCB Layout
---------------------------------

Block Diagram
^^^^^^^^^^^^^^^^^

The figure below shows the block diagram for ESP32-LCDKit.

.. figure:: ../../../_static/esp32-lcdkit/esp32_lcdkit_block.jpg
   :align: center
   :alt: ESP32-LCDKit
   :figclass: align-center
 
   ESP32-LCDKit Block Diagram

PCB Layout
^^^^^^^^^^^^^^^^

The figure below shows the layout of ESP32-LCDKit PCB.

.. figure:: ../../../_static/esp32-lcdkit/esp32_lcdkit_pcb.jpg
   :align: center
   :alt: ESP32-LCDKit
   :figclass: align-center
 
   ESP32-LCDKit PCB Layout

Descriptions of PCB components are shown in the following table:

+-----------------------------------+-----------------------------------+
| Components                        | Description                       |
+===================================+===================================+
| Display connection module         | Allows to connect serial or       |
|                                   | parallel LCD displays (8/16 bit)  |
+-----------------------------------+-----------------------------------+
| ESP32 DevKitC connection module   | Offers connection to an ESP32     |
|                                   | DevKitC development board         |
+-----------------------------------+-----------------------------------+
| SD-Card module                    | Provides an SD-Card slot for      |
|                                   | memory expansion                  |
+-----------------------------------+-----------------------------------+
| DAC-Audio module                  | Features an audio power amplifier |
|                                   | and two output ports for external |
|                                   | speakers                          |
+-----------------------------------+-----------------------------------+

Functional Modules
------------------------

This section introduces the functional modules (interfaces) of
ESP32-LCDKit and their hardware schematics.

- `Schematic <../../_static/schematics/esp32-lcdkit/SCH_ESP32-LCDKit_V1.1_20190218.pdf>`__

- `PCB Layout <../../_static/schematics/esp32-lcdkit/PCB_ESP32-LCDKit_V1.1_20190218.pdf>`__

ESP32 DevKitC Connection Module
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For the HMI-related development with ESP32-LCDKit, you also need the `ESP32 DevKitC <https://docs.espressif.com/projects/esp-idf/en/stable/hw-reference/modules-and-boards.html#esp32-devkitc-v4>`__ development board.

The figure below shows the schematics for the ESP32 DevKitC connection module.

.. figure:: ../../../_static/esp32-lcdkit/coreboard_module.jpg
   :align: center
   :alt: ESP32-LCDKit
   :figclass: align-center
 
   ESP32 DevKitC Connection Module

Power Supply Management Module
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The figure below shows the schematics for the USB power supply management module.

.. figure:: ../../../_static/esp32-lcdkit/power_module.jpg
   :align: center
   :alt: ESP32-LCDKit
   :figclass: align-center
 
   ESP32-LCDKit Power Supply Module

Display Connection Module
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The display connection module supports the following interfaces:

-  SPI serial interface
-  8-bit parallel interface
-  16-bit parallel interface

With this module, you can connect ESP32-LCDKit to an external display and interact with the pre-programmed GUI if the display has a touchscreen.

The figure below shows the schematics for this module.

.. figure:: ../../../_static/esp32-lcdkit/serial_screen_module.jpg
   :align: center
   :alt: ESP32-LCDKit
   :figclass: align-center
 
   ESP32-LCDKit Display Connection Module

SD-Card and DAC-Audio Modules
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The SD-Card module provides an SD Card slot for memory expansion. The DAC-Audio module features the MIX3006 power amplifier and two output ports for connection of external speakers.

The figure below shows the schematics for the SD-Card and DAC-Audio modules.

.. figure:: ../../../_static/esp32-lcdkit/sd_card_dac_module.jpg
   :align: center
   :alt: ESP32-LCDKit
   :figclass: align-center
 
   SD-Card and DAC-Audio Modules


Related Documents
---------------------

- `ESP32-LCDKit Schematic <../../_static/schematics/esp32-lcdkit/SCH_ESP32-LCDKit_V1.1_20190218.pdf>`_
- `ESP32-LCDKit PCB Layout <../../_static/schematics/esp32-lcdkit/PCB_ESP32-LCDKit_V1.1_20190218.pdf>`_