# PLAN: 添加一键发送邮件按钮

## 需求
在 esp_brookesia_phone 项目中添加一个 Email App，实现**一键发送邮件到指定QQ邮箱**的功能。

## 项目当前架构理解

```
esp_brookesia_phone/
├── main/main.cpp          # 入口：初始化并安装各个 App
├── main/CMakeLists.txt    # 主组件编译配置
├── components/apps/       # 各个 App 组件
│   ├── apps.h             # App 头文件汇总注册
│   ├── CMakeLists.txt     # 自动扫描所有 .cpp/.c 文件
│   ├── calculator/        # 计算器 App（继承 ESP_Brookesia_PhoneApp）
│   ├── music_player/      # 音乐播放器 App
│   ├── setting/           # 设置 App（含 WiFi 连接功能）
│   ├── game_2048/         # 2048游戏 App
│   ├── camera/            # 相机 App
│   └── video_player/      # 视频播放器 App
└── managed_components/    # ESP 官方组件（含 esp-brookesia 框架）
```

## App 开发模式

每个 App 继承 `ESP_Brookesia_PhoneApp`，需要实现：
- `init()` - 初始化（资源分配等）
- `run()` - 启动运行（创建 UI）
- `back()` - 返回操作
- `close()` - 关闭清理

构造时调用 `ESP_Brookesia_PhoneApp("App名称", &图标, true)`。

## WiFi 网络能力

Setting App 中已有完整的 WiFi 连接实现（`esp_wifi.h`, `esp_netif`），项目启动后可通过 Settings 连接 WiFi。Email App 可复用此 WiFi 连接发送邮件。

## 实施任务清单

### 1. 创建 Email App 目录结构
- 创建 `components/apps/email/` 目录
- 创建 `Email.hpp` 头文件
- 创建 `Email.cpp` 实现文件
- 创建 `assets/` 目录存放图标

### 2. 实现 SMTP 邮件发送功能
- QQ邮箱 SMTP 配置：服务器 `smtp.qq.com`，端口 `465` (SSL)
- 使用 mbedtls 实现 TLS 加密的 SMTP 客户端
- SMTP 流程：EHLO → AUTH LOGIN → MAIL FROM → RCPT TO → DATA → QUIT
- 需要 QQ 邮箱授权码（非登录密码），硬编码或通过配置设置
- 发送内容为预设文本（如 "Hello from ESP32-P4!"）

### 3. 实现 Email App UI
- 简单 UI：一个"发送邮件"按钮 + 状态提示标签
- 点击按钮后自动发送，显示发送状态（发送中/成功/失败）
- 预设收件人 QQ 邮箱、主题和正文

### 4. 创建 App 图标
- 生成 icon 资源文件（参考其他 app 的 assets 格式）
- 使用 LVGL 的 `LV_IMG_DECLARE` 宏声明

### 5. 注册并集成 App
- 在 `components/apps/apps.h` 中添加 `#include "email/Email.hpp"`
- 在 `main/main.cpp` 中创建并安装 Email App 实例

### 6. 验证编译
- 运行 `idf.py build` 确认编译通过

## 注意事项
- build/ 目录不读取（已编译产物）
- 参考 Calculator/MusicPlayer 的简洁模式实现
- SMTP 认证使用 QQ 邮箱授权码（需用户自行获取并填入代码）
- 发送邮件依赖 WiFi 已连接（Settings App 中处理）
- 使用 `esp_lv_adapter_lock/unlock` 保护 LVGL UI 操作
