# ESP-SensairShuttle Factory Demo

[English Version](./README.md)

## 概述

`esp-dev-kits` 中正式维护的 ESP-SensairShuttle Factory Demo，包含指南针、环境、
手势检测、2048 和出厂指南。更新以问题修复、硬件适配和 ESP-IDF 兼容为主。
体验其他固件后，可用本工程恢复出厂镜像。

## 硬件要求

### 支持的开发板
- **ESP32-SensairShuttle**

### 所需传感器
- **BMI270** - 6 轴 IMU（加速度计 + 陀螺仪）
- **BMM350** - 3 轴磁力计
- **BME690** - 环境传感器（温度、湿度、气压、气体）

### I2C 配置
默认配置（可在开发板 YAML 中修改）：
- SDA: GPIO 2
- SCL: GPIO 3
- 频率: 100 kHz

## 软件要求

### ESP-IDF 版本
- 支持 ESP-IDF release/v5.5 与 release/v6.0。已在 esp32c5 上用 **v5.5.4** 和 **v6.0.1** 完成编译验证。

### 依赖组件
- 本地 `brookesia_*` 组件：树内 Brookesia 0.7 快照，与 Registry `espressif/brookesia_system_core` 0.8 不可互换。
- `esp_board_manager` - 硬件抽象层
- `esp_lvgl_port` - LVGL 集成
- `espressif/bme690` - 环境传感器驱动
- `espressif/bmi270_sensor` - IMU 驱动
- `espressif/bmm350` - 磁力计驱动
- Boost 线程库 - 多线程支持

## 快速开始

先按 [开发板用户指南](https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/esp32c5/esp-sensairshuttle/index.html)
确认主板版本和所需传感器子板。

### 1. 进入示例目录

```bash
cd examples/esp-sensairshuttle/examples/factory_demo
```

### 2. 设置 ESP-IDF 环境

按照 [ESP-IDF 入门指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32c5/get-started/index.html) 设置开发环境。
请使用上文列出的版本。Linux 或 macOS：

```bash
. $HOME/esp/esp-idf/export.sh
```

Windows 请在对应版本的 ESP-IDF 终端中执行后续命令。

### 3. 生成开发板配置（重要）

示例使用 ESP Board Manager `~0.7.3~1`、`esp_boards` 板包 `0.6.2` 和上游
`esp_boards/esp_sensairshuttle` 板定义。
应用 amend 保留已验收的横屏布局和触摸映射，传感器应用自行初始化驱动。

先运行 `set-target esp32c5` 选择芯片并下载 Registry 依赖，再生成板配置。
切换板包或修改 amend 后请重新生成。ESP-IDF 5.5 通过下面的路径加载命令扩展，
ESP-IDF 6.0 也支持自动发现组件扩展。

```bash
# Linux / macOS
idf.py set-target esp32c5
export IDF_EXTRA_ACTIONS_PATH="$PWD/managed_components/espressif__esp_board_manager"
idf.py bmgr -c ./boards -b esp_sensairshuttle
```

```bat
REM Windows
idf.py set-target esp32c5
set IDF_EXTRA_ACTIONS_PATH=%CD%\managed_components\espressif__esp_board_manager
idf.py bmgr -c .\boards -b esp_sensairshuttle
```

### 4. 编译和烧录

```bash
# 编译项目
idf.py build

# 烧录到设备（将 PORT 替换为您的串口）
idf.py -p PORT flash monitor
```

退出串口监视器，请输入 `Ctrl-]`。

## 出厂固件说明

### 操作系统功能说明

- **打开应用**：点击应用程序图标

- **切换显示（可以找到不同应用）**：左右滑动页面

- **最小化打开的程序**：向上滑动虚拟主页条

- **打开程序切换器**：向上滑动虚拟条并持续维持长按

    - 在程序切换器页面，可以将缩略图向上滑动关闭选中应用的后台。或者按缩略图下方的垃圾箱标志关闭当前打开的所有应用。

    - 在程序切换器页面，屏幕最上方会显示已使用的 SRAM 和 PSRAM 的比例。

## 应用程序指南

### Environment（需要在插入 BME690 传感器子板后使用）

显示当前环境的温度、相对湿度、气压、空气质量。

- **相对湿度界面**：显示相对湿度以及当前湿度的舒适程度表情（表情颜色会根据舒适程度变化）
  - 30-60%：^_^（舒适，绿色显示）
  - 20-30% 或 60-70%：0_0（一般，黄色显示）
  - <20% 或 >70%：T_T（哭脸，不舒适，橙色显示）

- **气压界面**：显示当前气压以及推测出的天气（天气文字颜色会根据气压变化）
  - >1018.25 hPa（标准气压 1013.25 hPa + 5.0 hPa）：Maybe Sunny（可能晴天，绿色显示）
  - <1008.25 hPa（标准气压 1013.25 hPa - 5.0 hPa）：Maybe Rainy（可能雨天，红色显示）
  - 1008.25-1018.25 hPa：Good（正常，灰色显示）

- **空气质量界面**：显示室内空气质量指数 (IAQ) 以及 CO2 等效浓度（IAQ 数值和等级文字颜色会根据空气质量等级变化）。
  - **IAQ 显示**：0-500 范围映射为 0-100% 显示，数值越低空气质量越好。界面会显示英文等级文字（颜色会根据等级变化）
    - 0-50：Excellent（优秀，绿色显示）
    - 51-100：Good（良好，浅绿色显示）
    - 101-150：Fair（一般，黄色显示）
    - 151-200：Poor（较差，橙色显示）
    - 201-300：Bad（差，红色显示）
    - 301-500：Very Bad（很差，深红色显示）
  - **CO2 等效浓度**：显示估算的 CO2 浓度（单位：ppm），数值颜色会根据当前 IAQ 等级变化（与等级文字颜色一致）。
  - **使用说明**：
    - 空气质量通过 BME690 传感器的 BSEC 算法实现，首次使用需要 5-30 分钟收集初始数据，15-30 分钟后 IAQ 数值精度提升
    - 运行 4-24 小时后，IAQ 精度达到最高，数值最可信；保持应用持续运行以维持校准状态；远离热源、强烈气流和化学物质可获得最佳性能

### Gesture Detect（需要在插入 BMI270 & BMM350 传感器子板后使用）

检测开发板的当前运动状态并显示以下类别。

- **Roll**：检测设备滚动动作（角度 > 90°）。当检测到滚动时，界面会显示当前设备的绝对朝向信息，例如 "X Down [vvv]"、"Y Up [<<<]"、"Z Down [X]" 等，其中箭头符号（如 [^^^]、[vvv]、[>>>]、[<<<]、[X]、[O]）表示翻滚方向。支持的朝向包括：X Up/Down、Y Up/Down、Z Up/Down。
- **Shake**：检测设备摇晃动作。当检测到摇晃时，界面会显示检测到晃动动作的坐标轴信息，例如 "Shake heavy on X"、"Shake slight on Y/Z" 等，可以显示单个坐标轴（X、Y、Z）或多个坐标轴的组合（如 X/Y、Y/Z、X/Y/Z），并区分重度摇晃（heavy）和轻度摇晃（slight）。

### Compass（需要在插入 BMI270 & BMM350 传感器子板后使用）

实现指南针功能。需要保持水平、远离磁干扰源。

- **首次校准**：
  1. 从主屏幕启动指南针应用
  2. 按照屏幕指示进行校准
  3. 缓慢地在 **所有方向** 旋转设备（8 字形图案）
  4. 确保覆盖所有空间方位，校准至少需要采集 500 个样本

- **重新校准**：
  - **长按屏幕**：在指南针界面长按可重新进行校准
  - **自动保存**：校准完成后硬铁/软铁参数与状态写入 NVS，重启后自动恢复

- **使用指南针**：
  - 指针将自动指向磁北
  - 倾斜补偿确保设备不水平时也能准确显示
  - 实时航向更新频率为 20 Hz
  - **数据持久化**：校准数据在设备重启后仍然有效

- **故障排除**：
  - **校准失败**：确保远离磁干扰源（电子设备、金属结构、缠绕的供电线）
  - **读数不稳定**：在空旷区域重新校准
  - **数据丢失**：如果校准数据丢失，系统会自动提示重新校准

### 2048

2048 游戏。

### Factory Guide

显示当前 `用户指南` 网页链接的二维码。


## 项目结构

```
factory_demo/
├── main/                          # 主应用程序
│   ├── main.cpp                  # 主程序入口
│   ├── display.cpp               # 显示初始化
│   ├── display.hpp               # 显示头文件
│   ├── idf_component.yml         # 依赖项配置
│   └── CMakeLists.txt            # 构建配置
├── common_components/            # 公共组件
│   ├── brookesia_app_compass/           # 指南针应用
│   ├── brookesia_app_gesture_detect/    # 手势检测应用
│   ├── brookesia_app_temperature/       # 温度与空气质量应用
│   ├── brookesia_app_game_2048/         # 2048 游戏应用
│   ├── brookesia_app_factory_guide/     # 工厂指南应用
│   ├── brookesia_system_core/          # 树内 Brookesia 0.7 核心组件
│   └── brookesia_system_phone/         # 树内 Brookesia 0.7 Phone 系统
├── boards/                       # 应用专用 BMGR amend 配置
│   └── esp_sensairshuttle/       # SensairShuttle 开发板配置
└── CMakeLists.txt                # 项目构建配置
```

## 已知限制

### 指南针相关
1. **磁干扰**：强磁场附近性能会下降
2. **动态磁偏角**：尚未实现手动磁偏角校正

### 空气质量相关
3. **IAQ 校准时间**：首次使用需要 5-30 分钟初始校准
4. **BSEC 状态持久化**：当前未实现状态保存，重启后需重新校准
5. **环境适应性**：BSEC 算法在稳定环境中表现最佳

## 故障排除

### 传感器初始化失败
```
检查 I2C 连接
验证传感器地址（BMI270: 0x68, BMM350: 0x14/0x15, BME690: 0x76）
确保 I2C 线路上有上拉电阻
检查传感器子板是否正确插入
```

### IAQ 一直显示初始值或不更新
```
问题：IAQ 一直是 50，精度一直是 0
原因：
  1. 传感器刚启动，正在稳定中（需要等待一段时间）
  2. 气体传感器数据无效（检查加热器是否工作）
  3. BSEC 没有收到连续的有效数据
  
解决方法：
  1. 耐心等待至少 10 分钟，观察精度是否从 0 变化
  2. 检查串口日志中是否有 "Gas measurement not valid" 警告
  3. 确认传感器正确供电和连接
  4. 增加线程栈大小（至少 16KB）
  5. 查看 BSEC 日志输出，确认算法正常运行
```

### IAQ 精度无法提升
```
问题：IAQ 精度长时间停留在 0 或 1
原因：
  1. 环境过于稳定，缺少空气质量变化
  2. 传感器需要暴露在不同空气质量环境中学习
  
解决方法：
  1. 将设备暴露在不同环境中（室外、厨房、通风良好的房间）
  2. 持续运行 24 小时以上
  3. 确保传感器能够检测到空气质量的变化
```


### 手势检测应用初始化失败
```
问题：应用显示 "Initialize BMI270 failed. Please insert the BMI270 sub-board."
原因：
  1. BMI270 传感器子板未插入或连接有问题
  2. I2C 总线初始化失败
  3. BMI270 传感器配置失败
  
解决方法：
  1. 检查 BMI270 & BMM350 传感器子板是否正确插入
  2. 检查 I2C 连接（SDA: GPIO 2, SCL: GPIO 3）
  3. 检查串口日志中的具体错误信息
  4. 验证传感器地址（BMI270: 0x68）
  5. 确保 I2C 线路上有上拉电阻
```

### 程序崩溃或栈溢出
```
问题：Guru Meditation Error: Stack protection fault
原因：
  1. 线程栈空间不足（BSEC 需要更大的栈）
  2. 单次日志格式化参数过多
  
解决方法：
  1. 将 boost::thread 栈大小增至 16KB 或更大
  2. 减少单条日志输出的参数数量
  3. 避免在日志中格式化过多浮点数
```

### 线程相关问题
```
建议：
  1. 打开调试日志（例如在 close() 中增加 ESP_LOGI）
  2. 检查串口输出中的线程停止相关信息
  3. 确认 bmm_running_ 标志位的状态切换是否符合预期
```


## 贡献

报告问题时，请提供主板版本、固件提交、ESP-IDF 版本、已连接传感器和串口日志。

## 许可证

Apache License 2.0，见 [LICENSE](../../LICENSE)。

## 技术支持

- **论坛**: [esp32.com](https://esp32.com/viewforum.php?f=35)
- **GitHub Issues**: [esp-dev-kits/issues](https://github.com/espressif/esp-dev-kits/issues)
- **文档**: [ESP-SensairShuttle 用户指南](https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/esp32c5/esp-sensairshuttle/index.html)
