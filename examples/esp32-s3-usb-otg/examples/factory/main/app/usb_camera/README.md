## USB Camera LCD Display Demo 说明

该示例程序代码仅用于 ESP32-S2 USB Host UVC 功能测试与评估，不建议基于此开发量产产品，原因和完整示例请查看 [esp-iot-solution/usb/uvc_stream](https://github.com/espressif/esp-iot-solution/tree/usb/add_usb_solutions/components/usb/uvc_stream)

* 支持 USB Camera 数据流获取和解析
* 支持 JPEG 本地软件解码
* 支持 SPI LCD 屏幕显示

### 硬件准备

* ESP32-S2 USB 摄像头选型需要满足的参数（2021.03）：

    1. 摄像头兼容 USB1.1 全速模式
    2. 摄像头自带 MJPEG 压缩
    3. 摄像头支持设置端点`wMaxPacketSize`为`512`
    4. MJPEG 支持 **320*240** 分辨率
    5. MJPEG 支持设置帧率到 15 帧/s

* ESP32-S2 USB 摄像头硬件接线：
  1. USB 摄像头 VBUS 请使用 5V 电源独立供电，亦可使用 IO 控制 VBUS 通断
  2. UBS 摄像头 D+ D- 数据线请按常规差分信号标准走线
  3. USB 摄像头 D+ (绿线) 接 ESP32-S2 GPIO20
  4. USB 摄像头 D- (白线) 接 ESP32-S2 GPIO19

### 编译示例代码

示例代码基于 `esp32-s3-usb-otg` 开发板编写，可按以下过程直接编译烧写：

1. 确认 ESP-IDF 环境成功搭建（Demo 测试推荐使用 master 分支）
2. 安装 `ESP-IDF Component Manager`
   1. `. ./export.sh` to add ESP-IDF environment values
   2. `pip install idf-component-manager --upgrade`
3. 设置编译目标为 `esp32-s2`, `idf.py set-target esp32s2`
4. 编译、下载、并查看输出, `idf.py build flash monitor`
5. 如果 `ESP-IDF Component Manager` 安装成功，编译过程中项目所需的组件将自动下载

### 错误处理

1. 如果编译时报 `fatal error: usb.h: No such file or directory`，请将 `esp-idf/components/usb/CMakeLists.txt` 按照以下修改:

    ```
    idf_component_register(SRCS "hcd.c"
                        INCLUDE_DIRS "private_include"
                        PRIV_INCLUDE_DIRS "")
    ```

2. 在 SPI 刷屏应用中，如果出现屏幕偶尔小范围花屏的情况（ESP32-S2 PRE-ECO Chip Bug）, 请在 `esp-idf/components/driver/spi_master.c` ->> 函数 `spi_new_trans` ->> `spi_hal_user_start(hal)` 之前添加一行 `ets_delay_us(10)`:

    ```
    ets_delay_us(10);
    //Kick off transfer
    spi_hal_user_start(hal);
    ```

### 使用说明

1. 注意 LCD 屏幕和摄像头接口接线即可
2. 屏幕默认直接输出摄像头图像
3. 用户可通过使能 `boot animation` 开启开机动画，验证 LCD 屏幕是否能够成功点亮

### 性能参数

**ESP32-S2 240Mhz**：

| 典型分辨率  | USB 典型传输帧率 | JPEG 最大解码+显示帧率* |
| :-----: | :--------------: | :----------------------: |
| 320*240 |        15        |           ~12            |
| 160*120 |        30        |           ~28            |
|         |                  |                          |

1. *JPEG 解码+ 显示帧率随 CPU 负载波动
2. 其它分辨率以实际测试为准，或按照解码时间和分辨率正相关估算
