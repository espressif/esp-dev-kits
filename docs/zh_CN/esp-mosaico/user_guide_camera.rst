======================
ESP-Mosaico 摄像头模块
======================

:link_to_translation:`en:[English]`

本指南介绍 ESP-Mosaico **摄像头模块** （Camera Subboard）的硬件版本、硬件接口、安装方式与软件使用要点。摄像头模块提供 **V1.2** 和 **V1.4** 两个版本，均通过 CoreBoard / BaseBoard 左侧 ``2 × 10P`` 模块插槽接入，提供 DVP 图像采集能力。

.. note::

  摄像头模块 **仅支持左侧插槽** （``H2``，EEPROM 地址 0x50）。请勿插入右侧插槽。主板版本请参阅 :doc:`user_guide` 或 :doc:`user_guide_v1.0`。

两个版本均由主机扩展接口供电，通过 8-bit DVP 输出图像数据，SCCB / I2C 用于图像传感器与模块 EEPROM 通信。两个版本的引脚分配相同，图像传感器型号、晶振频率、默认输出分辨率以及 EEPROM 型号与封装有所不同。

本指南包括如下内容：

- `硬件版本`_：依次介绍 V1.2 和 V1.4 版本的 PCB 外观、组件与摄像头参数。
- `入门指南`_：模块整体外观、必备硬件、安装说明与配套软件示例。
- `硬件参考`_：DVP 管脚、电源与使用限制。
- `相关文档`_：相关文档链接。
- `免责声明和版权公告`_：免责声明和版权公告。

硬件版本
========

请根据摄像头模块 PCB 正面的版本号丝印确认硬件版本，并通过以下链接查阅对应版本的详细说明：

- `V1.2 版本`_：搭载 OV3640 摄像头。
- `V1.4 版本`_：搭载 SC101IOT 摄像头。

以上版本号均指摄像头模块的 PCB 版本，应与 CoreBoard 主板版本区分。每个版本按“版本概述、PCB 正面标识与接口、PCB 反面组件、摄像头参数”的顺序介绍。各版本的安装方式与引脚分配相同，共用下文的 `入门指南`_ 和 `硬件参考`_。

V1.2 版本
---------

V1.2 版本概述
^^^^^^^^^^^^^

V1.2 版本搭载 OV3640 DVP 图像传感器，使用传感器外部的 24 MHz 晶振提供 ``XCLK``，不占用主机 GPIO，默认采集格式为 **1024 × 768 UYVY**。

模块 EEPROM 采用 **AT24C02BS(GMIC)**，封装为 **SOP-8**。

V1.2 PCB 正面标识与接口
^^^^^^^^^^^^^^^^^^^^^^^

PCB 正面无器件，主要标有模块名称、PCB 版本号 V1.2 和日期。

.. figure:: ../../_static/esp-mosaico/esp-mosaico-camera-v1.2-front.png
   :alt: V1.2 摄像头模块 PCB 正面（点击放大）
   :width: 80%
   :figclass: align-center

   V1.2 摄像头模块 PCB 正面（点击放大）

正面主要为丝印与测试焊盘，说明如下。

.. list-table:: V1.2 版本 PCB 正面标识与接口
   :widths: 8 28 64
   :header-rows: 1

   * - 序号
     - 主要组件
     - 描述
   * - 1
     - 模块名称丝印
     - 标有 ``Module-Camera`` 和 ``ESP Mosaico``，用于标识摄像头模块。
   * - 2
     - 版本与日期丝印
     - 标有 ``260804 V1.2``，用于标识 PCB 日期与版本。
   * - 3
     - EEPROM 测试焊盘
     - 标有 ``SDA``、``SCL``、``WP``、``GND`` 和 ``3V3``，分别对应数据、时钟、写保护、地和 3.3 V 电源信号。

V1.2 PCB 反面组件
^^^^^^^^^^^^^^^^^

所有器件均布局在 PCB 反面，包括摄像头 FPC 连接器、24M 晶振、两颗闪光灯、摄像头状态指示灯（红色 LED）、AT24C02BS(GMIC) EEPROM（SOP-8 封装）及供电电路。

与 ESP-Mosaico 插接的排针采用夹板焊接工艺，焊点位于 PCB 正反两面。

.. figure:: ../../_static/esp-mosaico/esp-mosaico-camera-v1.2-back.png
   :alt: V1.2 摄像头模块 PCB 反面（点击放大）
   :width: 80%
   :figclass: align-center

   V1.2 摄像头模块 PCB 反面（点击放大）

反面主要组件说明如下，位置以图中方向为准。

.. list-table:: V1.2 版本 PCB 反面组件
   :widths: 8 28 64
   :header-rows: 1

   * - 序号
     - 主要组件
     - 描述
   * - 1
     - 摄像头 FPC 连接器
     - 位于中央，用于连接 OV3640 摄像头，传输图像数据、控制信号与电源。
   * - 2
     - AT24C02BS(GMIC) EEPROM
     - 位于 FPC 连接器右侧，采用 SOP-8 封装；用于存储模块识别信息，左侧插槽的 7-bit 地址为 0x50。
   * - 3
     - 1.5 V LDO
     - 位于右上方，为摄像头提供 1.5 V 电源。
   * - 4
     - Camera Flash-1（闪光灯 1）
     - 位于右侧，用于补光。由 GPIO34 控制，拉低亮起、拉高熄灭；应配置为开漏输出，防止主机关机掉电时短暂闪烁。
   * - 5
     - 2 × 10-Pin Header（模块排针）
     - 位于下方，采用夹板焊接工艺，焊点位于 PCB 正反两面；用于接入 BaseBoard 左侧 ``H2`` 插槽。
   * - 6
     - 摄像头状态指示灯（红色 LED）
     - 位于左下方，摄像头启动并工作时亮起，进入休眠（sleep）或关闭时熄灭。
   * - 7
     - Camera Flash-2（闪光灯 2）
     - 位于左侧，用于补光，与闪光灯 1 共用 GPIO34 控制，电平与开漏输出要求相同。
   * - 8
     - 2.8 V LDO
     - 位于左上方，为摄像头提供 2.8 V 电源。
   * - 9
     - 24 MHz 晶振
     - 位于 FPC 连接器左上方，为 OV3640 提供 ``XCLK``，不占用主机 GPIO。

V1.2 摄像头参数
^^^^^^^^^^^^^^^

.. list-table:: V1.2 版本摄像头参数
   :widths: 30 70
   :header-rows: 1

   * - 参数
     - 规格
   * - 模块 PCB 版本号
     - V1.2
   * - 图像传感器
     - OV3640
   * - 传感器类型与像素数
     - 320 万像素 CMOS
   * - 传感器尺寸
     - 1/4 英寸
   * - 像素尺寸
     - 1.75 μm
   * - 镜头焦距
     - 4.15 mm，定焦
   * - 光圈
     - F2.4
   * - 视场角
     - 68°
   * - 适用场景
     - 常规视觉采集
   * - 镜头畸变
     - <1%
   * - 默认输出格式
     - 1024 × 768 UYVY
   * - 输出分辨率与帧率
     - 1024 × 768 @ 30 FPS
   * - 支持的图像格式
     - RAW / YUV / RGB
   * - 自动图像控制
     - AEC（自动曝光控制）/ AGC（自动增益控制）/ AWB（自动白平衡）

V1.4 版本
---------

V1.4 版本概述
^^^^^^^^^^^^^

V1.4 版本搭载 SC101IOT DVP 图像传感器，使用板载 20 MHz 晶振提供 ``XCLK``，不占用主机 GPIO，默认输出格式为 **1280 × 720 UYVY**。

模块 EEPROM 采用 **24C02S(XBLW)**，封装为 **SOT23-5**。

V1.4 PCB 正面标识与接口
^^^^^^^^^^^^^^^^^^^^^^^

PCB 正面无器件，主要标有模块名称、PCB 版本号 V1.4 和日期。

.. figure:: ../../_static/esp-mosaico/esp-mosaico-camera-v1.4-front.png
   :alt: V1.4 摄像头模块 PCB 正面（点击放大）
   :width: 80%
   :figclass: align-center

   V1.4 摄像头模块 PCB 正面（点击放大）

正面主要为丝印与测试焊盘，说明如下。

.. list-table:: V1.4 版本 PCB 正面标识与接口
   :widths: 8 28 64
   :header-rows: 1

   * - 序号
     - 主要组件
     - 描述
   * - 1
     - 模块名称丝印
     - 标有 ``Module-Camera`` 和 ``ESP Mosaico``，用于标识摄像头模块。
   * - 2
     - 版本与日期丝印
     - 标有 ``20260901 V1.4``，用于标识 PCB 日期与版本。
   * - 3
     - EEPROM 测试焊盘
     - 标有 ``SDA``、``SCL``、``WP``、``GND`` 和 ``3V3``，分别对应数据、时钟、写保护、地和 3.3 V 电源信号。

V1.4 PCB 反面组件
^^^^^^^^^^^^^^^^^

所有器件均布局在 PCB 反面，包括摄像头 FPC 连接器、20M 晶振、两颗闪光灯、摄像头状态指示灯（红色 LED）、24C02S(XBLW) EEPROM（SOT23-5 封装）及供电电路。

与 ESP-Mosaico 插接的排针采用夹板焊接工艺，焊点位于 PCB 正反两面。

.. figure:: ../../_static/esp-mosaico/esp-mosaico-camera-v1.4-back.png
   :alt: V1.4 摄像头模块 PCB 反面（点击放大）
   :width: 80%
   :figclass: align-center

   V1.4 摄像头模块 PCB 反面（点击放大）

反面主要组件说明如下，位置以图中方向为准。

.. list-table:: V1.4 版本 PCB 反面组件
   :widths: 8 28 64
   :header-rows: 1

   * - 序号
     - 主要组件
     - 描述
   * - 1
     - 摄像头 FPC 连接器
     - 位于中央，用于连接 SC101IOT 摄像头，传输图像数据、控制信号与电源。
   * - 2
     - 24C02S(XBLW) EEPROM
     - 位于 FPC 连接器右侧，采用 SOT23-5 封装；用于存储模块识别信息，左侧插槽的 7-bit 地址为 0x50。
   * - 3
     - Camera Flash-1（闪光灯 1）
     - 位于右侧，用于补光。由 GPIO34 控制，拉低亮起、拉高熄灭；应配置为开漏输出，防止主机关机掉电时短暂闪烁。
   * - 4
     - 2 × 10-Pin Header（模块排针）
     - 位于下方，采用夹板焊接工艺，焊点位于 PCB 正反两面；用于接入 BaseBoard 左侧 ``H2`` 插槽。
   * - 5
     - 摄像头状态指示灯（红色 LED）
     - 位于左下方，摄像头启动并工作时亮起，进入休眠（sleep）或关闭时熄灭。
   * - 6
     - Camera Flash-2（闪光灯 2）
     - 位于左侧，用于补光，与闪光灯 1 共用 GPIO34 控制，电平与开漏输出要求相同。
   * - 7
     - 2.8 V LDO
     - 位于左上方，为摄像头提供 2.8 V 电源。
   * - 8
     - 20 MHz 晶振
     - 位于 FPC 连接器左上方，为 SC101IOT 提供 ``XCLK``，不占用主机 GPIO。

V1.4 摄像头参数
^^^^^^^^^^^^^^^

V1.4 版本的 SC101IOT 摄像头模组提供矮镜头和高镜头两种规格，均支持手动对焦。两种镜头的光学参数分别列于下表，详细信息请参阅 `相关文档`_ 中对应的模组规格书。

.. list-table:: V1.4 版本摄像头参数
   :widths: 30 70
   :header-rows: 1

   * - 参数
     - 规格
   * - 模块 PCB 版本号
     - V1.4
   * - 图像传感器
     - SC101IOT
   * - 传感器类型与像素数
     - 100 万像素 CMOS
   * - 传感器尺寸
     - 1/4.2 英寸
   * - 像素尺寸
     - 2.9 μm
   * - 镜头焦距
     - | 矮镜头：3.1 mm，支持手动对焦
       | 高镜头：4.3 mm，支持手动对焦
   * - 光圈
     - | 矮镜头：F1.8（±5%）
       | 高镜头：F2.4（±5%）
   * - 视场角
     - | 矮镜头：对角（D）79°，水平（H）73°，垂直（V）42°
       | 高镜头：对角（D）52°，水平（H）46°，垂直（V）26°
   * - 适用场景
     - 常规视觉采集
   * - 镜头畸变
     - | 矮镜头：<1.0%
       | 高镜头：<0.5%
   * - 默认输出格式
     - 1280 × 720 UYVY
   * - 输出分辨率与帧率
     - 1280 × 720 @ 30 FPS（8-bit，最大传输规格）
   * - 支持的图像格式
     - RAW / YUV422 / RGB
   * - 自动图像控制
     - AEC（自动曝光控制）/ AGC（自动增益控制）/ AWB（自动白平衡）

.. _Getting-started_esp-mosaico-camera:

入门指南
========

本节适用于 V1.2 和 V1.4 版本。各版本的 PCB 外观、组件与参数请参阅 `硬件版本`_。

模块整体外观
------------

摄像头模块采用翻转式设计，通过合拢或展开镜头所在的翻转部分，可在前摄与后摄状态之间切换。

下图左列为前摄状态，右列为后摄状态，每列上下两张分别展示该状态下模块的两侧外观。

- **前摄状态（合拢，左列）**：镜头所在的翻转部分与模块主体合拢，整体外观紧凑。左上图为镜头侧，左下图为闪光灯侧，镜头与两颗闪光灯朝向相反。
- **后摄状态（翻转展开，右列）**：镜头所在的翻转部分展开，镜头与两颗闪光灯朝向一致，可利用闪光灯为拍摄补光。右上图展示镜头与闪光灯，右下图展示另一侧的翻转连接结构。

.. figure:: ../../_static/esp-mosaico/esp-mosaico-camera-module-overview.png
   :alt: 摄像头模块前摄与后摄状态的两侧外观（左列：合拢前摄；右列：展开后摄；点击放大）
   :width: 60%
   :figclass: align-center

   摄像头模块整体外观：左列为前摄状态（合拢），右列为后摄状态（翻转展开）（点击放大）

必备硬件
--------

- ESP-Mosaico（CoreBoard V1.0 或 V1.2）
- ESP-Mosaico 摄像头模块
- USB 数据线（支持数据传输）
- 电脑（Windows、Linux 或 macOS）

安装说明
--------

1. 确认主机已关机或已关闭对外 3.3 V / 5 V 输出。
2. 将摄像头模块按照“橘色镜头朝上，闪光灯朝下”方向插入主机 **左侧** 模块插槽（``H2``），详情见 `ESP-Mosaico 使用指南 <https://mosaico.espressif.com/zh/guide/>`__。
3. 通过 Type-C 为 ESP-Mosaico 供电并开机。扩展 3.3 V / 5 V 仅在 GPIO60 置低时输出。

下图展示摄像头模块插接在 ESP-Mosaico 主体左侧模块插槽（``H2``）后的状态。安装时请对齐模块排针与插槽，确保连接牢固。

.. figure:: ../../_static/esp-mosaico/esp-mosaico-camera-module-installed.png
   :alt: 摄像头模块与 ESP-Mosaico 主体插接示意图（点击放大）
   :width: 60%
   :figclass: align-center

   摄像头模块与 ESP-Mosaico 主体插接示意图（点击放大）

配套软件示例
------------

摄像头模块提供以下配套示例，使用方法请参阅各示例的说明文档：

- `camera_lcd_preview：屏幕预览示例 <https://github.com/esp-mosaico/esp-mosaico-bsp/tree/master/examples/camera_lcd_preview>`__
- `camera_photo_app：拍照与图库示例 <https://github.com/esp-mosaico/esp-mosaico-bsp/tree/master/examples/camera_photo_app>`__

用户也可以使用 `mosaico_module_camera 摄像头组件 <https://github.com/esp-mosaico/esp-mosaico-bsp/tree/master/components/mosaico_module_camera>`__ 开发自己的应用程序。

.. _Hardware-reference_esp-mosaico-camera:

硬件参考
========

接口位置
--------

摄像头模块通过 ``2 × 10P``、2.54 mm 间距的模块排针接入 BaseBoard 左侧 ``H2`` 插槽。主机侧连接器定义见 :doc:`user_guide` 的模块接口章节。

.. figure:: ../../_static/esp-mosaico/esp-mosaico-expansion-interface-left.png
   :alt: 左侧模块接口电路图（点击放大）
   :scale: 45%
   :figclass: align-center

   左侧模块接口电路图（点击放大）

DVP 与控制管脚
--------------

.. figure:: ../../_static/esp-mosaico/esp-mosaico-camera-module-interface-pinout.png
   :alt: 摄像头模块接口引脚分配图（点击放大）
   :width: 80%
   :figclass: align-center

   摄像头模块接口引脚分配图（点击放大）

下表适用于 V1.2 和 V1.4 版本，所有引脚分配均相同。``XCLK`` 由模块晶振提供，不通过左侧插槽的 GPIO 输出，频率见对应的 `V1.2 版本`_ 或 `V1.4 版本`_ 说明。

.. list-table::
   :widths: 18 16 16 50
   :header-rows: 1

   * - 信号
     - GPIO
     - 左侧插槽引脚
     - 说明
   * - DVP_D0
     - GPIO16
     - 9
     - 数据位 0
   * - DVP_D1
     - GPIO15
     - 11
     - 数据位 1
   * - DVP_D2
     - GPIO33
     - 13
     - 数据位 2；与 USB Serial/JTAG D- 复用
   * - DVP_D3
     - GPIO4
     - 12
     - 数据位 3
   * - DVP_D4
     - GPIO14
     - 10
     - 数据位 4；占用左侧 EEPROM 地址选择脚
   * - DVP_D5
     - GPIO12
     - 8
     - 数据位 5
   * - DVP_D6
     - GPIO18
     - 5
     - 数据位 6
   * - DVP_D7
     - GPIO13
     - 6
     - 数据位 7
   * - VSYNC
     - GPIO55
     - 1
     - 场同步
   * - DE
     - GPIO19
     - 3
     - 数据有效
   * - PCLK
     - GPIO17
     - 7
     - 像素时钟
   * - RESET
     - GPIO53
     - 2
     - Sensor 复位
   * - PWDN
     - GPIO48
     - 4
     - Sensor 掉电控制
   * - FLASH
     - GPIO34
     - 15
     - 闪光灯；拉低亮起，拉高熄灭，应配置为开漏输出；与 USB Serial/JTAG D+ 复用，默认关闭
   * - SCCB_SDA
     - GPIO0
     - 16
     - Sensor / EEPROM 数据线
   * - SCCB_SCL
     - GPIO1
     - 14
     - Sensor / EEPROM 时钟线
   * - VCC_3V3
     -
     - 19
     - 3.3 V 供电（由 GPIO60 控制）
   * - 5V_OUT / 5V_IN
     -
     - 18 / 17
     - 5 V 输出 / 输入
   * - GND
     -
     - 20
     - 地

I2C 与模块发现
--------------

- 模块 EEPROM：型号与封装见各版本说明，左侧地址 **0x50**，板型 **0x07** （``MOSAICO_BOARD_TYPE_CAMERA``）。
- Sensor SCCB 与 EEPROM 共用扩展 I2C：SDA = GPIO0，SCL = GPIO1。CoreBoard **V1.2** 上该总线为对外 I2C1；**V1.0** 上与板载器件共享同一路 I2C。
- 打开摄像头后，GPIO14 改为 DVP D4，左侧插槽 EEPROM 访问暂停，直到 ``mosaico_camera_del()`` / ``bsp_subboard_camera_release()`` 释放资源。

使用限制
--------

.. important::

  - **仅左侧插槽** 。插入右侧时，BSP 返回 ``ESP_ERR_NOT_SUPPORTED``，不会配置 DVP。
  - GPIO33 / GPIO34 与板载 USB Serial/JTAG PHY 复用。占用摄像头后，左侧插槽的 USB Serial/JTAG 不可用；板载 Type-C USB-OTG 控制台不受影响。
  - 默认输出格式见对应版本的摄像头参数表。软件默认使用 4 个帧缓冲。取到的帧必须通过 ``mosaico_camera_return_frame()`` 归还，且在 ``mosaico_camera_restart()`` 前归还全部帧。
  - 模块上电后需等待板载 ``XCLK`` 稳定（BSP 默认约 20 ms）再访问 SCCB。

.. _Related-documents_esp-mosaico-camera:

相关文档
========

-  `V1.2 摄像头模块（OV3640）PCB 原理图 <https://dl.espressif.com/ae/mosaico/hardware/SCH_SCH_ESP_Mosaico_OV3640_CameraBoard_V1_2_2026-09-21.pdf>`__ (PDF)
-  `V1.4 摄像头模块（SC101IOT）PCB 原理图 <https://dl.espressif.com/ae/mosaico/hardware/SCH_SCH_ESP_Mosaico_SC101IOT_CameraBoard_V1_4A_2026-09-21.pdf>`__ (PDF)
- `OV3640 数据手册 <https://dl.espressif.com/ae/mosaico/hardware/OV3640_CSP_1.1.pdf>`__ (PDF)
- `OV3640 摄像头模组规格书 <https://dl.espressif.com/ae/mosaico/hardware/Renheng_OV3640_Camera_Module_21_mm_68_20260724.pdf>`__ (PDF)
- `SC101IOT 数据手册 <https://dl.espressif.com/ae/mosaico/hardware/SC101IoT_Datasheet_V0.5.pdf>`__ (PDF)
- `SC101IOT 摄像头模组（矮镜头）规格书 <https://dl.espressif.com/ae/mosaico/hardware/AS-AGTO08AI101D4-21-V2.0.pdf>`__ (PDF)
- `SC101IOT 摄像头模组（高镜头）规格书 <https://dl.espressif.com/ae/mosaico/hardware/AS-AGT08AI101D4-21%EF%BC%88%E5%85%89%E5%9C%882.4%EF%BC%89.pdf>`__ (PDF)


免责声明和版权公告
==================

请参阅 :doc:`免责声明和版权公告 <../disclaimer-and-copyright>`。
