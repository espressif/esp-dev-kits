ESP-Mosaico
===========

:link_to_translation:`en:[English]`

ESP-Mosaico 是乐鑫基于 ESP32-S31 打造的可扩展智能交互开发套件，面向磁吸扩展、运动感知、方形触摸屏与端侧多媒体等应用场景。设备搭载 480 × 480 QSPI 方形触摸屏、音频编解码与功放、六轴 IMU、双磁力计、SPI NAND flash，并提供左右两个 2 × 10P、2.54 mm 标准间距端子的模块接口，可用于摄像头等功能扩展。

阅读指引
--------

请根据所用硬件和使用需求选择相应文档：

- **CoreBoard 硬件与开发**：ESP-Mosaico 提供 CoreBoard V1.2 与 V1.0 两个硬件版本。请根据 CoreBoard 丝印确认版本，并查阅 :doc:`用户指南（V1.2） <user_guide>` 或 :doc:`用户指南（V1.0） <user_guide_v1.0>`，了解组件、接口、引脚分配与开发准备。
- **扩展模块**：使用摄像头或多功能交互子板时，请在对应主板指南的基础上，查阅 :doc:`摄像头模块用户指南 <user_guide_camera>` 或 :doc:`多功能交互子板用户指南 <user_guide_interact>`。这两篇指南均适用于 CoreBoard V1.2 和 V1.0；其中的模块 PCB 版本号与主板版本号应分别确认。
- **出厂应用操作与固件更新**：按键操作、扩展功能及出厂应用的使用方式请参阅外部网站的 `ESP-Mosaico 出厂应用使用说明 <https://mosaico.espressif.com/zh/guide/>`__；通过 `在线固件更新 <https://mosaico.espressif.com/zh/firmware-update/>`__ 页面可在浏览器中刷写固件。该网站侧重应用操作，本页下列指南侧重硬件说明与开发参考。

文档目录
--------

.. toctree::
    :maxdepth: 2

    用户指南（V1.2） <user_guide>
    用户指南（V1.0） <user_guide_v1.0>
    摄像头模块用户指南 <user_guide_camera>
    多功能交互子板用户指南 <user_guide_interact>
