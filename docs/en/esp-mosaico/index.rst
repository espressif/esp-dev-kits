ESP-Mosaico
===========

:link_to_translation:`zh_CN:[中文]`

ESP-Mosaico is an expandable smart interaction development kit based on ESP32-S31. It targets magnetic expansion, motion sensing, square touch display, and edge multimedia scenarios. The device integrates an ESP32-S31 SoC, a 480 × 480 QSPI square touch display, audio codec and amplifier, a 6-axis IMU, dual magnetometers, SPI NAND flash, and left/right 2 × 10P, 2.54 mm pitch module headers for camera and other expansions.

Reading Guide
-------------

Choose the documentation that matches your hardware and intended use:

- **CoreBoard hardware and development**: ESP-Mosaico has two CoreBoard versions, V1.2 and V1.0. Check the version on the CoreBoard silkscreen, then read the :doc:`User Guide (V1.2) <user_guide>` or the :doc:`User Guide (V1.0) <user_guide_v1.0>` for components, interfaces, pin assignments, and development preparation.
- **Expansion modules**: When using the camera module or multi-function interaction subboard, read the :doc:`Camera Module User Guide <user_guide_camera>` or :doc:`Multi-function Interaction Subboard User Guide <user_guide_interact>` alongside the corresponding mainboard guide. Both module guides apply to CoreBoard V1.2 and V1.0. Module PCB versions and mainboard versions must be identified separately.
- **Factory application operation and firmware updates**: For button operation, expansion features, and factory application usage, refer to the external `ESP-Mosaico Factory Application Guide <https://mosaico.espressif.com/guide/>`__. Use the `Online Firmware Update <https://mosaico.espressif.com/firmware-update/>`__ page to flash firmware in the browser. That website focuses on application operation, while the guides below provide hardware information and development references.

Documentation
-------------

.. toctree::
    :maxdepth: 2

    User Guide (V1.2) <user_guide>
    User Guide (V1.0) <user_guide_v1.0>
    Camera Module User Guide <user_guide_camera>
    Multi-function Interaction Subboard User Guide <user_guide_interact>
