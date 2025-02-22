esp-dev-kits 文档
=================

:link_to_translation:`en:[English]`

.. only:: not other

   .. only:: html

      该文档详细介绍了 {IDF_TARGET_NAME} 系列开发板的用户指南和示例。如需查看其他芯片的开发板用户指南和示例，请在页面左上角的下拉列表中选择相应芯片进行切换。

   .. only:: latex

      该文档详细介绍了 {IDF_TARGET_NAME} 系列开发板的用户指南和示例。

.. only:: other

   .. only:: html

      该文档详细介绍了乐鑫调试和烧录开发板的用户指南，开发板适配不同的芯片。如需查看某一特定芯片的开发板用户指南和示例，请在页面左上角的下拉列表中选择相应芯片进行切换。

   .. only:: latex

      该文档详细介绍了乐鑫调试和烧录开发板的用户指南，开发板适配不同的芯片。

.. only:: esp32c2

   目前 {IDF_TARGET_NAME} 芯片仅包含 ESP8684 单个系列，因此本文档中提到的 {IDF_TARGET_NAME} 实际对应 ESP8684 系列芯片。

.. note::

   如需获取乐鑫全部系列开发板的有关信息，请访问 `乐鑫开发板概览 <https://www.espressif.com/zh-hans/products/devkits>`_。

.. only:: esp32p4

   =============================   ===========================
         **{IDF_TARGET_NAME} 系列开发板**
   -----------------------------------------------------------
   |ESP32-P4-Function-EV-Board|_
   -----------------------------   ---------------------------
   `ESP32-P4-Function-EV-Board`_
   =============================   ===========================

.. only:: esp32c5

   ============================   ===========================
         **{IDF_TARGET_NAME} 系列开发板**
   ----------------------------------------------------------
   |ESP32-C5-DevKitC-1|_
   ----------------------------   ---------------------------
   `ESP32-C5-DevKitC-1`_
   ============================   ===========================

.. only:: esp32h2

   ============================   ===========================
         **{IDF_TARGET_NAME} 系列开发板**
   ----------------------------------------------------------
   |ESP32-H2-DevKitM-1|_
   ----------------------------   ---------------------------
   `ESP32-H2-DevKitM-1`_
   ============================   ===========================

.. only:: esp32c3

   ============================   ===========================
         **{IDF_TARGET_NAME} 系列开发板**
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
         **{IDF_TARGET_NAME} 系列开发板**
   ----------------------------------------------------------
   |ESP32-C6-DevKitC-1|_          |ESP32-C6-DevKitM-1|_
   ----------------------------   ---------------------------
   `ESP32-C6-DevKitC-1`_          `ESP32-C6-DevKitM-1`_
   ============================   ===========================

.. only:: esp32c61

   ============================   ===========================
         **{IDF_TARGET_NAME} 系列开发板**
   ----------------------------------------------------------
   |ESP32-C61-DevKitC-1|_
   ----------------------------   ---------------------------
   `ESP32-C61-DevKitC-1`_
   ============================   ===========================

.. only:: esp32c2

   ============================   ===========================
         **{IDF_TARGET_NAME} 系列开发板**
   ----------------------------------------------------------
   |ESP8684-DevKitM-1|_            |ESP8684-DevKitC-02|_
   ----------------------------   ---------------------------
   `ESP8684-DevKitM-1`_            `ESP8684-DevKitC-02`_
   ============================   ===========================

.. only:: esp32s3

   ============================   ===========================
         **{IDF_TARGET_NAME} 系列开发板**
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
         **{IDF_TARGET_NAME} 系列开发板**
   ----------------------------------------------------------
   |ESP32-S2-DevKitC-1|_            |ESP32-S2-DevKitM-1|_
   ----------------------------   ---------------------------
   `ESP32-S2-DevKitC-1`_            `ESP32-S2-DevKitM-1`_
   ============================   ===========================

.. only:: esp32

   ============================   ===========================
         **{IDF_TARGET_NAME} 系列开发板**
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
         **调试和烧录开发板**
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
.. _ESP32-S2-DevKitC-1: esp32-s2-devkitc-1/index.html

.. |ESP32-S2-DevKitM-1| image:: ../_static/esp32-s2-devkitm-1/esp32-s2-devkitm-1-v1-isometric.png
.. _ESP32-S2-DevKitM-1: esp32-s2-devkitm-1/index.html

.. |ESP32-LCDKit| image:: ../_static/esp32-lcdkit/esp32-lcdkit-board.png
                  :height: 230px
                  :width: 400px
.. _ESP32-LCDKit: esp32-lcdkit/index.html

.. |ESP32-DevKitC| image:: ../_static/esp32-devkitc/esp32-devkitc-board.png
                  :height: 230px
                  :width: 400px
.. _ESP32-DevKitC: esp32-devkitc/index.html

.. |ESP32-S3-DevKitC-1| image:: ../_static/esp32-s3-devkitc-1/esp32-s3-devkitc-1-v1-isometric.png
.. _ESP32-S3-DevKitC-1: esp32-s3-devkitc-1/index.html

.. |ESP32-S3-DevKitM-1| image:: ../_static/esp32-s3-devkitm-1/esp32-s3-devkitm-1-v1-isometric.png
.. _ESP32-S3-DevKitM-1: esp32-s3-devkitm-1/index.html

.. |ESP32-S3-USB-OTG| image:: ../_static/esp32-s3-usb-otg/pic_product_esp32_s3_otg.png
                        :height: 230px
                        :width: 400px
.. _ESP32-S3-USB-OTG: esp32-s3-usb-otg/index.html

.. |ESP32-S3-LCD-EV-Board| image:: ../_static/esp32-s3-lcd-ev-board/ESP32-S3-LCD-EV-Board_480x480.png
.. _ESP32-S3-LCD-EV-Board: esp32-s3-lcd-ev-board/index.html

.. |ESP32-S3-USB-Bridge| image:: ../_static/esp32-s3-usb-bridge/esp32-s3-usb-bridge.png
.. _ESP32-S3-USB-Bridge: esp32-s3-usb-bridge/index.html

.. |ESP8684-DevKitM-1| image:: ../_static/esp8684-devkitm-1/esp8684-devkitm-1-v0.1-isometric.png
.. _ESP8684-DevKitM-1: esp8684-devkitm-1/index.html

.. |ESP8684-DevKitC-02| image:: ../_static/esp8684-devkitc-02/esp8684-devkitc-02-v1.0-isometric.png
.. _ESP8684-DevKitC-02: esp8684-devkitc-02/index.html

.. |ESP32-C3-DevKitC-02| image:: ../_static/esp32-c3-devkitc-02/esp32-c3-devkitc-02-v1-isometric.png
.. _ESP32-C3-DevKitC-02: esp32-c3-devkitc-02/index.html

.. |ESP32-C3-DevKitM-1| image:: ../_static/esp32-c3-devkitm-1/esp32-c3-devkitm-1-v1-isometric.png
.. _ESP32-C3-DevKitM-1: esp32-c3-devkitm-1/index.html

.. |ESP32-C3-LCDkit| image:: ../_static/esp32-c3-lcdkit/esp32-c3-lcdkit-isometric-raw.png
.. _ESP32-C3-LCDkit: esp32-c3-lcdkit/index.html

.. |ESP32-C6-DevKitC-1| image:: ../_static/esp32-c6-devkitc-1/esp32-c6-devkitc-1-isometric_v1.2.png
.. _ESP32-C6-DevKitC-1: esp32-c6-devkitc-1/index.html

.. |ESP32-C6-DevKitM-1| image:: ../_static/esp32-c6-devkitm-1/esp32-c6-devkitm-1-isometric.png
.. _ESP32-C6-DevKitM-1: esp32-c6-devkitm-1/index.html

.. |ESP32-C61-DevKitC-1| image:: ../_static/esp32-c61-devkitc-1/esp32-c61-devkitc-1-isometric.png
.. _ESP32-C61-DevKitC-1: esp32-c61-devkitc-1/index.html

.. |ESP-Prog| image:: ../_static/esp-prog/three_dimension.png
.. _ESP-Prog: esp-prog/index.html

.. |ESP32-H2-DevKitM-1| image:: ../_static/esp32-h2-devkitm-1/esp32-h2-devkitm-1-45.png
.. _ESP32-H2-DevKitM-1: esp32-h2-devkitm-1/index.html

.. |ESP32-P4-Function-EV-Board| image:: ../_static/esp32-p4-function-ev-board/esp32-p4-function-ev-board-isometric.png
.. _ESP32-P4-Function-EV-Board: esp32-p4-function-ev-board/index.html

.. |ESP32-C5-DevKitC-1| image:: ../_static/esp32-c5-devkitc-1/esp32-c5-devkitc-1-isometric_v1.1.png
.. _ESP32-C5-DevKitC-1: esp32-c5-devkitc-1/index.html

.. |ESP-Module-Prog-1| image:: ../_static/esp-module-prog-1-r/esp-module-prog-1.png
.. _ESP-Module-Prog-1: esp-module-prog-1-r/index.html

.. |ESP-Module-Prog-1R| image:: ../_static/esp-module-prog-1-r/esp-module-prog-1r.png
.. _ESP-Module-Prog-1R: esp-module-prog-1-r/index.html

.. |ESP-Module-Prog-SUB-01&04| image:: ../_static/esp-module-prog-sub-01&04/esp-module-prog-sub-0104.png
.. _ESP-Module-Prog-SUB-01&04: esp-module-prog-sub-01&04/index.html

.. |ESP-Module-Prog-SUB-02| image:: ../_static/esp-module-prog-sub-02/esp-module-prog-sub-02.png
.. _ESP-Module-Prog-SUB-02: esp-module-prog-sub-02/index.html

.. |ESP32-PICO-DevKitM-2| image:: ../_static/esp32-pico-devkitm-2/esp32-pico-devkitm-2-overview.png
.. _ESP32-PICO-DevKitM-2: esp32-pico-devkitm-2/index.html

.. |ESP32-Ethernet-Kit| image:: ../_static/esp32-ethernet-kit/esp32-ethernet-kit-v1.2-overview.png
.. _ESP32-Ethernet-Kit: esp32-ethernet-kit/index.html

.. |ESP32-PICO-KIT-1| image:: ../_static/esp32-pico-kit-1/esp32-pico-kit-1-overview.png
.. _ESP32-PICO-KIT-1: esp32-pico-kit-1/index.html

.. |ESP32-PICO-KIT| image:: ../_static/esp32-pico-kit/esp32-pico-kit-overview.png
.. _ESP32-PICO-KIT: esp32-pico-kit/index.html

.. |ESP32-DevKitM-1| image:: ../_static/esp32-devkitm-1/esp32-DevKitM-1-isometric.png
.. _ESP32-DevKitM-1: esp32-devkitm-1/index.html


.. only:: esp32c3

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} 系列开发板

      ESP32-C3-DevKitC-02 <esp32-c3-devkitc-02/index>
      ESP32-C3-DevKitM-1 <esp32-c3-devkitm-1/index>
      ESP32-C3-LCDkit <esp32-c3-lcdkit/index>

.. only:: esp32c6

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} 系列开发板

      ESP32-C6-DevKitC-1 <esp32-c6-devkitc-1/index>
      ESP32-C6-DevKitM-1 <esp32-c6-devkitm-1/index>

.. only:: esp32c61

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} 系列开发板

      ESP32-C61-DevKitC-1 <esp32-c61-devkitc-1/index>

.. only:: esp32h2

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} 系列开发板

      ESP32-H2-DevKitM-1 <esp32-h2-devkitm-1/index>

.. only:: esp32p4

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} 系列开发板

      ESP32-P4-Function-EV-Board <esp32-p4-function-ev-board/index>

.. only:: esp32c5

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} 系列开发板

      ESP32-C5-DevKitC-1 <esp32-c5-devkitc-1/index>

.. only:: esp32c2

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} 系列开发板

      ESP8684-DevKitM-1 <esp8684-devkitm-1/index>
      ESP8684-DevKitC-02 <esp8684-devkitc-02/index>

.. only:: esp32s3

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} 系列开发板

      ESP32-S3-DevKitC-1 <esp32-s3-devkitc-1/index>
      ESP32-S3-DevKitM-1 <esp32-s3-devkitm-1/index>
      ESP32-S3-USB-OTG <esp32-s3-usb-otg/index>
      ESP32-S3-LCD-EV-Board <esp32-s3-lcd-ev-board/index>
      ESP32-S3-USB-Bridge <esp32-s3-usb-bridge/index>

.. only:: esp32s2

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} 系列开发板

      ESP32-S2-DevKitC-1 <esp32-s2-devkitc-1/index>
      ESP32-S2-DevKitM-1 <esp32-s2-devkitm-1/index>
      寿命终止开发板 <eol/eol-boards>

.. only:: esp32

   .. toctree::
      :hidden:
      :caption: {IDF_TARGET_NAME} 系列开发板

      ESP32-DevKitC <esp32-devkitc/index>
      ESP32-DevKitM-1 <esp32-devkitm-1/index>
      ESP32-PICO-KIT-1 <esp32-pico-kit-1/index>
      ESP32-PICO-DevKitM-2 <esp32-pico-devkitm-2/index>
      ESP32-LCDKit <esp32-lcdkit/index>
      ESP32-Ethernet-Kit <esp32-ethernet-kit/index>
      ESP32-PICO-KIT <esp32-pico-kit/index>
      寿命终止开发板 <eol/eol-boards>

.. only:: other

   .. toctree::
      :hidden:
      :caption: 调试和烧录开发板

      ESP-Prog <esp-prog/index>
      ESP-Module-Prog-1(R) <esp-module-prog-1-r/index>
      ESP-Module-Prog-SUB-02 <esp-module-prog-sub-02/index>
      ESP-Module-Prog-SUB-01&04 <esp-module-prog-sub-01&04/index>

.. toctree::
   :hidden:
   :caption: 相关资源与法律声明

   相关文档和资源 <resources>
   免责声明和版权公告 <disclaimer-and-copyright>
