esp-dev-kits Documentation
==========================

:link_to_translation:`zh_CN:[中文]`

.. only:: not other

   .. only:: html

      This document provides detailed user guides and examples for {IDF_TARGET_NAME} series development boards. To switch to a different SoC target, choose target from the dropdown in the upper left.

   .. only:: latex

      This document provides detailed user guides and examples for {IDF_TARGET_NAME} series development boards.

.. only:: other

   .. only:: html

      This document provides detailed user guides for Espressif debugging and flashing boards that fit different SoCs. To switch to a specific SoC target, choose target from the dropdown in the upper left.

   .. only:: latex

      This document provides detailed user guides for Espressif debugging and flashing boards that fit different SoCs.

.. only:: esp32c2

   The {IDF_TARGET_NAME} SoC currently consists of only one series, the ESP8684, so the references of {IDF_TARGET_NAME} in this document refers to the ESP8684 Series SoCs.

.. note::

   For the full list of Espressif development boards, please go to `ESP DevKits <https://www.espressif.com/en/products/devkits>`_.

.. only:: esp32p4

   =============================   ===========================
         **{IDF_TARGET_NAME} Development Boards**
   -----------------------------------------------------------
   |ESP32-P4-Function-EV-Board|_
   -----------------------------   ---------------------------
   `ESP32-P4-Function-EV-Board`_
   =============================   ===========================

.. only:: esp32c5

   ============================   ===========================
         **{IDF_TARGET_NAME} Development Boards**
   ----------------------------------------------------------
   |ESP32-C5-DevKitC-1|_
   ----------------------------   ---------------------------
   `ESP32-C5-DevKitC-1`_
   ============================   ===========================

.. only:: esp32h2

   ============================   ===========================
         **{IDF_TARGET_NAME} Development Boards**
   ----------------------------------------------------------
   |ESP32-H2-DevKitM-1|_
   ----------------------------   ---------------------------
   `ESP32-H2-DevKitM-1`_
   ============================   ===========================

.. only:: esp32c3

   ============================   ===========================
         **{IDF_TARGET_NAME} Development Boards**
   ----------------------------------------------------------
   |ESP32-C3-DevKitC-02|_         |ESP32-C3-DevKitM-1|_
   ----------------------------   ---------------------------
   `ESP32-C3-DevKitC-02`_          `ESP32-C3-DevKitM-1`_
   ----------------------------   ---------------------------
   |ESP32-C3-LCDkit|_
   ----------------------------   ---------------------------
   `ESP32-C3-LCDkit`_
   ============================   ===========================

.. only:: esp32c6

   ============================   ===========================
         **{IDF_TARGET_NAME} Development Boards**
   ----------------------------------------------------------
   |ESP32-C6-DevKitC-1|_          |ESP32-C6-DevKitM-1|_
   ----------------------------   ---------------------------
   `ESP32-C6-DevKitC-1`_          `ESP32-C6-DevKitM-1`_
   ============================   ===========================

.. only:: esp32c61

   ============================   ===========================
         **{IDF_TARGET_NAME} Development Boards**
   ----------------------------------------------------------
   |ESP32-C61-DevKitC-1|_
   ----------------------------   ---------------------------
   `ESP32-C61-DevKitC-1`_
   ============================   ===========================

.. only:: esp32c2

   ============================   ===========================
         **{IDF_TARGET_NAME} Development Boards**
   ----------------------------------------------------------
   |ESP8684-DevKitM-1|_            |ESP8684-DevKitC-02|_
   ----------------------------   ---------------------------
   `ESP8684-DevKitM-1`_            `ESP8684-DevKitC-02`_
   ============================   ===========================

.. only:: esp32s3

   ============================   ===========================
         **{IDF_TARGET_NAME} Development Boards**
   ----------------------------------------------------------
   |ESP32-S3-DevKitC-1|_            |ESP32-S3-DevKitM-1|_
   ----------------------------   ---------------------------
   `ESP32-S3-DevKitC-1`_            `ESP32-S3-DevKitM-1`_
   ----------------------------   ---------------------------
   |ESP32-S3-USB-OTG|_            |ESP32-S3-LCD-EV-Board|_
   ----------------------------   ---------------------------
   `ESP32-S3-USB-OTG`_            `ESP32-S3-LCD-EV-Board`_
   ----------------------------   ---------------------------
   |ESP32-S3-USB-Bridge|_
   ----------------------------   ---------------------------
   `ESP32-S3-USB-Bridge`_
   ============================   ===========================

.. only:: esp32s2

   ============================   ===========================
         **{IDF_TARGET_NAME} Development Boards**
   ----------------------------------------------------------
   |ESP32-S2-DevKitC-1|_            |ESP32-S2-DevKitM-1|_
   ----------------------------   ---------------------------
   `ESP32-S2-DevKitC-1`_            `ESP32-S2-DevKitM-1`_
   ============================   ===========================

.. only:: esp32

   ============================   ===========================
         **{IDF_TARGET_NAME} Development Boards**
   ----------------------------------------------------------
   |ESP32-DevKitC|_                |ESP32-DevKitM-1|_
   ----------------------------   ---------------------------
   `ESP32-DevKitC`_                `ESP32-DevKitM-1`_
   ----------------------------   ---------------------------
   |ESP32-PICO-KIT-1|_             |ESP32-PICO-DevKitM-2|_
   ----------------------------   ---------------------------
   `ESP32-PICO-KIT-1`_             `ESP32-PICO-DevKitM-2`_
   ----------------------------   ---------------------------
   |ESP32-LCDKit|_                 |ESP32-Ethernet-Kit|_
   ----------------------------   ---------------------------
   `ESP32-LCDKit`_                 `ESP32-Ethernet-Kit`_
   ----------------------------   ---------------------------
   |ESP32-PICO-KIT|_
   ----------------------------   ---------------------------
   `ESP32-PICO-KIT`_
   ============================   ===========================

.. only:: other

   ============================   ===========================
         **Debugging and Flashing Development Boards**
   ----------------------------------------------------------
   |ESP-Prog|_                     |ESP-Module-Prog-1|_
   ----------------------------   ---------------------------
   `ESP-Prog`_                     `ESP-Module-Prog-1`_  
   ----------------------------   ---------------------------
   |ESP-Module-Prog-1R|_           |ESP-Module-Prog-SUB-02|_
   ----------------------------   ---------------------------
   `ESP-Module-Prog-1R`_           `ESP-Module-Prog-SUB-02`_ 
   ----------------------------   ---------------------------
   |ESP-Module-Prog-SUB-01&04|_
   ----------------------------   ---------------------------
   `ESP-Module-Prog-SUB-01&04`_
   ============================   ===========================

.. |ESP32-S2-DevKitC-1| image:: ../_static/esp32-s2-devkitc-1/esp32-s2-devkitc-1-v1-isometric.png
.. _ESP32-S2-DevKitC-1: esp32s2/esp32-s2-devkitc-1/index.html

.. |ESP32-S2-DevKitM-1| image:: ../_static/esp32-s2-devkitm-1/esp32-s2-devkitm-1-v1-isometric.png
.. _ESP32-S2-DevKitM-1: esp32s2/esp32-s2-devkitm-1/index.html

.. |ESP32-LCDKit| image:: ../_static/esp32-lcdkit/esp32-lcdkit-board.png
                  :height: 230px
                  :width: 400px
.. _ESP32-LCDKit: esp32/esp32-lcdkit/index.html

.. |ESP32-DevKitC| image:: ../_static/esp32-devkitc/esp32-devkitc-board.png
                  :height: 230px
                  :width: 400px
.. _ESP32-DevKitC: esp32/esp32-devkitc/index.html

.. |ESP32-S3-DevKitC-1| image:: ../_static/esp32-s3-devkitc-1/esp32-s3-devkitc-1-v1-isometric.png
.. _ESP32-S3-DevKitC-1: esp32s3/esp32-s3-devkitc-1/index.html

.. |ESP32-S3-DevKitM-1| image:: ../_static/esp32-s3-devkitm-1/esp32-s3-devkitm-1-v1-isometric.png
.. _ESP32-S3-DevKitM-1: esp32s3/esp32-s3-devkitm-1/index.html

.. |ESP32-S3-USB-OTG| image:: ../_static/esp32-s3-usb-otg/pic_product_esp32_s3_otg.png
                        :height: 230px
                        :width: 400px
.. _ESP32-S3-USB-OTG: esp32s3/esp32-s3-usb-otg/index.html

.. |ESP32-S3-LCD-EV-Board| image:: ../_static/esp32-s3-lcd-ev-board/ESP32-S3-LCD-EV-Board_480x480.png
.. _ESP32-S3-LCD-EV-Board: esp32s3/esp32-s3-lcd-ev-board/index.html

.. |ESP32-S3-USB-Bridge| image:: ../_static/esp32-s3-usb-bridge/esp32-s3-usb-bridge.png
.. _ESP32-S3-USB-Bridge: esp32s3/esp32-s3-usb-bridge/index.html

.. |ESP8684-DevKitM-1| image:: ../_static/esp8684-devkitm-1/esp8684-devkitm-1-v0.1-isometric.png
.. _ESP8684-DevKitM-1: esp8684/esp8684-devkitm-1/index.html

.. |ESP8684-DevKitC-02| image:: ../_static/esp8684-devkitc-02/esp8684-devkitc-02-v1.0-isometric.png
.. _ESP8684-DevKitC-02: esp8684/esp8684-devkitc-02/index.html

.. |ESP32-C3-DevKitC-02| image:: ../_static/esp32-c3-devkitc-02/esp32-c3-devkitc-02-v1-isometric.png
.. _ESP32-C3-DevKitC-02: esp32c3/esp32-c3-devkitc-02/index.html

.. |ESP32-C3-DevKitM-1| image:: ../_static/esp32-c3-devkitm-1/esp32-c3-devkitm-1-v1-isometric.png
.. _ESP32-C3-DevKitM-1: esp32c3/esp32-c3-devkitm-1/index.html

.. |ESP32-C3-LCDkit| image:: ../_static/esp32-c3-lcdkit/esp32-c3-lcdkit-isometric-raw.png
.. _ESP32-C3-LCDkit: esp32c3/esp32-c3-lcdkit/index.html

.. |ESP32-C6-DevKitC-1| image:: ../_static/esp32-c6-devkitc-1/esp32-c6-devkitc-1-isometric_v1.2.png
.. _ESP32-C6-DevKitC-1: esp32c6/esp32-c6-devkitc-1/index.html

.. |ESP32-C6-DevKitM-1| image:: ../_static/esp32-c6-devkitm-1/esp32-c6-devkitm-1-isometric.png
.. _ESP32-C6-DevKitM-1: esp32c6/esp32-c6-devkitm-1/index.html

.. |ESP32-C61-DevKitC-1| image:: ../_static/esp32-c61-devkitc-1/esp32-c61-devkitc-1-isometric.png
.. _ESP32-C61-DevKitC-1: esp32c61/esp32-c61-devkitc-1/index.html

.. |ESP-Prog| image:: ../_static/esp-prog/three_dimension.png
.. _ESP-Prog: other/esp-prog/index.html

.. |ESP32-H2-DevKitM-1| image:: ../_static/esp32-h2-devkitm-1/esp32-h2-devkitm-1-45.png
.. _ESP32-H2-DevKitM-1: esp32h2/esp32-h2-devkitm-1/index.html

.. |ESP32-P4-Function-EV-Board| image:: ../_static/esp32-p4-function-ev-board/esp32-p4-function-ev-board-isometric.png
.. _ESP32-P4-Function-EV-Board: esp32p4/esp32-p4-function-ev-board/index.html

.. |ESP32-C5-DevKitC-1| image:: ../_static/esp32-c5-devkitc-1/esp32-c5-devkitc-1-isometric_v1.1.png
.. _ESP32-C5-DevKitC-1: esp32c5/esp32-c5-devkitc-1/index.html

.. |ESP-Module-Prog-1| image:: ../_static/esp-module-prog-1-r/esp-module-prog-1.png
.. _ESP-Module-Prog-1: other/esp-module-prog-1-r/index.html

.. |ESP-Module-Prog-1R| image:: ../_static/esp-module-prog-1-r/esp-module-prog-1r.png
.. _ESP-Module-Prog-1R: other/esp-module-prog-1-r/index.html

.. |ESP-Module-Prog-SUB-01&04| image:: ../_static/esp-module-prog-sub-01&04/esp-module-prog-sub-0104.png
.. _ESP-Module-Prog-SUB-01&04: other/esp-module-prog-sub-01&04/index.html

.. |ESP-Module-Prog-SUB-02| image:: ../_static/esp-module-prog-sub-02/esp-module-prog-sub-02.png
.. _ESP-Module-Prog-SUB-02: other/esp-module-prog-sub-02/index.html

.. |ESP32-PICO-DevKitM-2| image:: ../_static/esp32-pico-devkitm-2/esp32-pico-devkitm-2-overview.png
.. _ESP32-PICO-DevKitM-2: esp32/esp32-pico-devkitm-2/index.html

.. |ESP32-Ethernet-Kit| image:: ../_static/esp32-ethernet-kit/esp32-ethernet-kit-v1.2-overview.png
.. _ESP32-Ethernet-Kit: esp32/esp32-ethernet-kit/index.html

.. |ESP32-PICO-KIT-1| image:: ../_static/esp32-pico-kit-1/esp32-pico-kit-1-overview.png
.. _ESP32-PICO-KIT-1: esp32/esp32-pico-kit-1/index.html

.. |ESP32-PICO-KIT| image:: ../_static/esp32-pico-kit/esp32-pico-kit-overview.png
.. _ESP32-PICO-KIT: esp32/esp32-pico-kit/index.html

.. |ESP32-DevKitM-1| image:: ../_static/esp32-devkitm-1/esp32-DevKitM-1-isometric.png
.. _ESP32-DevKitM-1: esp32/esp32-devkitm-1/index.html


.. only:: esp32c3

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} Boards

      ESP32-C3-DevKitC-02 <esp32c3/esp32-c3-devkitc-02/index>
      ESP32-C3-DevKitM-1 <esp32c3/esp32-c3-devkitm-1/index>
      ESP32-C3-LCDkit <esp32c3/esp32-c3-lcdkit/index>

.. only:: esp32c6

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} Boards

      ESP32-C6-DevKitC-1 <esp32c6/esp32-c6-devkitc-1/index>
      ESP32-C6-DevKitM-1 <esp32c6/esp32-c6-devkitm-1/index>

.. only:: esp32c61

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} Boards

      ESP32-C61-DevKitC-1 <esp32c61/esp32-c61-devkitc-1/index>

.. only:: esp32h2

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} Boards

      ESP32-H2-DevKitM-1 <esp32h2/esp32-h2-devkitm-1/index>

.. only:: esp32p4

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} Boards

      ESP32-P4-Function-EV-Board <esp32p4/esp32-p4-function-ev-board/index>

.. only:: esp32c5

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} Boards

      ESP32-C5-DevKitC-1 <esp32c5/esp32-c5-devkitc-1/index>

.. only:: esp32c2

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} Boards

      ESP8684-DevKitM-1 <esp8684/esp8684-devkitm-1/index>
      ESP8684-DevKitC-02 <esp8684/esp8684-devkitc-02/index>

.. only:: esp32s3

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} Boards

      ESP32-S3-DevKitC-1 <esp32s3/esp32-s3-devkitc-1/index>
      ESP32-S3-DevKitM-1 <esp32s3/esp32-s3-devkitm-1/index>
      ESP32-S3-USB-OTG <esp32s3/esp32-s3-usb-otg/index>
      ESP32-S3-LCD-EV-Board <esp32s3/esp32-s3-lcd-ev-board/index>
      ESP32-S3-USB-Bridge <esp32s3/esp32-s3-usb-bridge/index>

.. only:: esp32s2

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} Boards

      ESP32-S2-DevKitC-1 <esp32s2/esp32-s2-devkitc-1/index>
      ESP32-S2-DevKitM-1 <esp32s2/esp32-s2-devkitm-1/index>

   .. toctree::
      :hidden:
      :caption: EOL (End of Life) Boards

      ESP32-S2-HMI-DevKit-1 <esp32s2/esp32-s2-hmi-devkit-1/index>
      ESP32-S2-Touch-Devkit-1 <esp32s2/esp32-s2-touch-devkit-1/index>
      ESP32-S2-Kaluga-1 <esp32s2/esp32-s2-kaluga-1/index>
      ESP32-S2-Saola-1 <esp32s2/esp32-s2-saola-1/index>

.. only:: esp32

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} Boards

      ESP32-DevKitC <esp32/esp32-devkitc/index>
      ESP32-DevKitM-1 <esp32/esp32-devkitm-1/index>
      ESP32-PICO-KIT-1 <esp32/esp32-pico-kit-1/index>
      ESP32-PICO-DevKitM-2 <esp32/esp32-pico-devkitm-2/index>
      ESP32-LCDKit <esp32/esp32-lcdkit/index>
      ESP32-Ethernet-Kit <esp32/esp32-ethernet-kit/index>
      ESP32-PICO-KIT <esp32/esp32-pico-kit/index>

   .. toctree::
      :hidden:
      :caption: EOL (End of Life) Boards

      ESP32-Sense-Kit <esp32/esp32-sense-kit/index>
      ESP32-MeshKit-Sense <esp32/esp32-meshkit-sensor/index>
      ESP-WROVER-KIT <esp32/esp-wrover-kit/index>

.. only:: other

   .. toctree::
      :hidden:
      :caption: Debugging and Flashing Boards

      ESP-Prog <other/esp-prog/index>
      ESP-Module-Prog-1(R) <other/esp-module-prog-1-r/index>
      ESP-Module-Prog-SUB-02 <other/esp-module-prog-sub-02/index>
      ESP-Module-Prog-SUB-01&04 <other/esp-module-prog-sub-01&04/index>

.. toctree::
   :hidden:
   :caption: Resources and Legal Notices

   Related Documentation and Resources <resources>
   Disclaimer and Copyright Notice <disclaimer-and-copyright>
