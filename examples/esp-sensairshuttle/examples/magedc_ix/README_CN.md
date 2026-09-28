# MagEDC IX Demo — EDC 解压玩具 互动 Demo

[English](README.md) | 中文

基于 **ESP32-C5 SensairShuttle** 的磁珠 EDC（解压/把玩）互动 Demo。用 **BMM350** 磁力计感知一颗磁珠在 3×3（9 个点位）的磁铁阵列面板上的位置，配合 LED 灯带的实时视觉反馈和喇叭音效，并通过网页（`upperlevelmachine/game_demo.html`）实时联机做小游戏（跑酷 / 音游 / 钓鱼）。

![ESP-SensairShuttle EDC-IX 外壳实物](assets/edc_ix.webp)

## 开源外壳

实现图中 EDC-IX 形态所需的外壳模型已经开源，可从
[MakerWorld：ESP-SensairShuttle EDC-IX](https://makerworld.com/zh/models/3305518-esp-sensairshuttle-edc-ix)
下载并制作。

## 工作原理

- **BMM350 磁力计**：100Hz 采样，EMA 平滑后用相对于中心点位的极坐标 + 高斯置信度判定磁珠落在哪个点位。
- **BMI270 IMU**：用于运动唤醒，配合深睡省电。
- **LED 灯带（WS2812）**：实时高亮当前点位；颜色随操作速度由蓝→橙→红变化。
- **喇叭（I2S PDM）**：每个点位播放对应音效，可手势开关、网页调参。
- **Wi-Fi（STA）**：联网后通过 HTTP POST（控制）+ SSE（实时遥测）与网页通信。
- **网页上位机**：`upperlevelmachine/game_demo.html`，浏览器本地打开即可，提供实时可视化与小游戏。


## 硬件需求

- **开发板**：ESP-SensairShuttle（ESP32-C5）
- **传感器**：BMM350（磁力计）、BMI270（IMU，运动唤醒）（已全部集成在shuttle board上）
- **外设**：WS2812 LED 灯带、PDM 喇叭
- **磁铁阵列**：九颗用于点位约束（磁吸手感）的磁铁
- **磁珠**：一颗用于滑动识别位置的磁珠

## 软件需求

### ESP-IDF 版本
- `idf: ">=5.5"`，目标芯片 `esp32c5`。已在 **v5.5.4**、**v6.0.1**、**v6.0.3** 上编译验证。

### 依赖
- `espressif/bmm350`（^1.1.0，ESP Component Registry）
- `espressif/bmi270_sensor`（^0.4.0，ESP Component Registry）
- `espressif/esp_board_manager`、`espressif/led_strip`、`espressif/mdns`

## 编译与烧录

### 1. 进入示例目录

```bash
cd examples/esp-sensairshuttle/examples/magedc_ix
```

### 2. 设置 ESP-IDF 环境

按 [ESP-IDF 入门指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32c5/get-started/index.html) 配置环境（目标芯片 `esp32c5`）。

```bash
. $HOME/esp/esp-idf/export.sh   # Windows: . $env:IDF_PATH\export.ps1
```

### 3. 生成开发板配置（重要）

示例使用 ESP Board Manager `~0.7.3~1`、`esp_boards` 板包 `0.6.2` 和上游
`esp_boards/esp_sensairshuttle` 板定义。
自动 amend 裁掉 LCD 和应用自行管理的编解码器；BMGR 管理共享 I2C、LED 灯带和喇叭使能脚。
MagEDC 自行初始化 BMI270 / BMM350，以及自定义 24 kHz PDM 音频通路。

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
idf.py build
idf.py -p PORT flash monitor   # 将 PORT 替换为你的串口
```

退出串口监视器：`Ctrl-]`。

## 怎么玩（首次完整流程）

### 第 1 步：首次必做 — 9 点磁校准

首次烧录（NVS 无校准数据）后，设备会**自动进入 9 点校准向导**，灯带先红色频闪提示。

1. 校准从**中心位**开始；灯带会用**琥珀色**高亮当前待校准的点位（中心位则灯带**全亮**）。
2. 把磁珠放到高亮点位上**保持静止**。移动会打断采样，当前点重新采集；完成稳定采样后该点亮**绿色**确认，并**自动切换到下一个点位**。
3. 依次完成全部 9 个点位，校准结果写入 NVS（断电不丢）。

> **校准数据保留**：完整有效的校准数据会在复位、深睡唤醒及保留 NVS 和兼容校准数据版本的正常固件升级后恢复。数据缺失、损坏或版本不兼容时，设备会自动重新进入 9 点向导。
>
> 需要重新校准时（EDC 外壳无暴露按键）：网页「串口连接」后点**「🎯 重新校准」按钮**下发 `RECALIB`（也可用任意串口监视器手动发送 `RECALIB`），或 `idf.py erase-flash` 擦除后重新烧录。

### 第 2 步：打开网页并配网

用 **Chrome / Edge** 打开本地网页 `upperlevelmachine/game_demo.html`（需 Web Serial）：点 **「串口连接」** 选设备串口（115200）→ 填 Wi-Fi SSID/密码（开放网络留空）→ 点 **「配网并连接」**。设备联网后串口打印 `WIFI_READY:<ip>`，网页自动读取 IP 并经 HTTP/SSE 连接。

### 第 3 步：开玩与声音

连接成功后，移动磁珠在 9 个点位间切换，网页会实时显示磁珠位置，并可进入「跑酷 / 音游 / 钓鱼」等模式。

- **手势开关声音**：磁珠依次停留 **1 号位 → 9 号位**（每处稍作停留）即可切换扬声器**开 / 关**。
- **开启后**：磁珠落在不同点位会演奏**不同音调的钢琴声**，可像弹琴一样演奏（中心位为静音中转位，不发声）。
- **钓鱼模式**：在网页「钓鱼」标签页中，从下往上快速滑动磁珠**抛竿**；上钩后**左右滑动**与鱼对抗、**顺时针环滑**收线，配合动态水面与阶段 HUD 把鱼收上来。

### 第 4 步：放下休眠 / 拿起唤醒

不想玩时把设备放下静置即可：

- 运行态连续 **60 秒**无有效点位操作后，可进入**深度睡眠**。校准完成后重新开始计时；Wi-Fi 活动或 IMU 唤醒条件未就绪时暂缓休眠。
- 再次**拿起或摇动**（IMU 检测到震动）即自动**唤醒**并恢复运行，无需任何按键。

## 网页自动连接

网页默认使用缓存的设备地址自动连接，连接成功后掉线会自动重试。首次连接未找到设备时，可点「搜索设备」恢复。受浏览器安全策略限制，**串口授权**仍需手动点一次「串口连接」。

## 手势配置标签页（九宫格自定义映射）

网页新增 **手势配置** 标签页：

- 在九宫格按数字顺序录制 **2~8 步**手势（例如 `1 -> 2 -> 3`）
- 给手势命名，并映射到开放功能
- 当前开放功能：
  - `切换喇叭发声`（通过设备 `/api/audio` 控制）

网页在设备明确返回成功后才确认保存。请求被拒绝或 NVS 写入失败时，原有手势继续生效。

> 说明：`切换喇叭发声` 手势会下发到固件并写入 NVS（设备侧持久化），因此即使关闭网页，喇叭手势仍可在设备上生效。

## 再次使用

- 设备会**自动重连**已保存的 Wi-Fi，并恢复完整有效的校准数据。
- 想再次拿到设备 IP：重新点「串口连接」，网页会自动探测 `WIFI_STATUS` 并填入 IP；网页也会缓存上次的 IP，可直接点「IP 连接」。
- 清除已保存的 Wi-Fi：串口发送 `WIFI_CLEAR`。

## 串口命令（供调试 / 上位机）

| 命令 | 说明 |
| --- | --- |
| `WIFI_CFG\t<ssid>\t<pass>` | 配网（保存凭据并连接），字段用 Tab 分隔 |
| `WIFI_STATUS` | 查询状态，已联网时回 `WIFI_READY:<ip>` |
| `WIFI_STOP` | 断开 Wi-Fi |
| `WIFI_CLEAR` | 清除已保存凭据并断开 |
| `RECALIB` | 重新进入 9 点磁校准 |

## 低功耗设计

固件采用 **三档功耗策略**，在"随手就能玩"和"放下就省电"之间取得平衡：

1. **运行态（联网/交互）— 自动 Light Sleep + DTIM10 Modem Sleep**
   - 通过 `esp_pm_configure`（`light_sleep_enable = true`）开启 ESP-IDF 自动 Light Sleep：空闲间隙 CPU 与射频自动下电，由中断/定时器秒级无感唤醒。需配套 `CONFIG_PM_ENABLE=y` 与 `CONFIG_FREERTOS_USE_TICKLESS_IDLE=y`（已在 `sdkconfig` 开启）。
   - Wi-Fi 采用 **Modem Sleep（`WIFI_PS_MAX_MODEM`）+ DTIM10**：设置 `listen_interval = WIFI_STA_LISTEN_INTERVAL`（默认 **10**），station 每约 10 个 beacon（约 1s）才唤醒射频收一次包，beacon 间隙射频下电，联网待机功耗相比每 beacon 唤醒大幅降低。
   - **延迟权衡**：DTIM10 只影响**下行**——网页发的 HTTP POST 命令最高约 1s 才被设备接收；SSE 为设备**上行**推流可随时发送，**不受影响**，实时数据流畅度基本不变。要更低延迟可调小 `WIFI_STA_LISTEN_INTERVAL`（如 3），要更省电可调大。
2. **"禁睡"窗口 — PM Lock 保护关键时段**
   - 在 Wi-Fi 连接/重试、网页配对宽限（`WIFI_WEB_PAIRING_GRACE_MS`，默认 60s）、控制活跃窗口（`WIFI_ACTIVE_WINDOW_MS`，默认 4s）内，用 `ESP_PM_NO_LIGHT_SLEEP` 锁临时禁止 Light Sleep，保证配网与实时 SSE 推流不被打断；窗口结束自动释放回到自动 Light Sleep。
3. **深度睡眠（放下静置）— μA 级待机 + 运动唤醒**
   - 运行态按实际经过时间计算，连续 `IDLE_SLEEP_MS`（默认 **60s**）无有效点位操作后可进入 **Deep Sleep**。校准期间不计入空闲时长，校准完成后重新计时。
   - 睡前依序关断：静音喇叭 → 清空并刷新 LED → Wi-Fi 下电（`wifi_stream_prepare_sleep`）→ BMM350 进 `SUSPEND` → 清除所有唤醒源。
   - 仅保留 **BMI270 运动中断** 作为唤醒源（`esp_sleep_enable_ext1_wakeup`，`IMU_INT_PIN` 高电平触发），并用 `gpio_hold_en` 锁存引脚电平，避免深睡期间漂移误唤醒。
   - **拿起/摇动 → IMU 触发 → 唤醒冷启动**，检测到 `ESP_RST_DEEPSLEEP` 后 `gpio_hold_dis` 释放引脚、重建外设，并从 NVS 恢复完整有效的校准数据。

> Wi-Fi 正在连接或网页配对窗口仍活跃、BMI270 唤醒配置失败、唤醒引脚配置失败或持续为高时，设备都会**暂缓深睡**，保持运行，等待运动唤醒条件就绪。
>
> 相关阈值见下方「电源 / 睡眠」参数表（`IDLE_SLEEP_MS` / `WIFI_ACTIVE_WINDOW_MS` / `WIFI_WEB_PAIRING_GRACE_MS` / `WIFI_IDLE_STOP_MS`）。若不需要 Wi-Fi，可将 `ENABLE_WIFI` 置 0 获得最省电的纯本地交互。

## 开发者指南

### 源码文件结构（`main/`）

| 文件 | 作用 |
| --- | --- |
| `main.cpp` | 主程序：传感器采样、EMA 滤波、状态机（校准/运行）、判区调度、LED 渲染、深睡/唤醒、`app_main` 入口。暴露 `app_request_recalibration()` 供串口触发重校准。 |
| `zone_polar_detect.c/.h` | 极坐标判区核心：`r/θ` + Z 门控，输出点位 id 与置信度。 |
| `zone_wizard.c/.h` | 9 点校准向导：状态机（提示/采样/确认）、逐点推进、写入 NVS。 |
| `zone_storage.c/.h` | 校准数据 NVS 读写、schema 版本管理、深睡恢复判定。 |
| `calib_nvs.c/.h` | 固件指纹记录、升级时保留已有校准、磁中心数据校验与清除。 |
| `mag_zones_config.h` | `polar_zone_t` 结构、判定模式开关、出厂默认 9 点 zone（冷启动兜底）。 |
| `wifi_stream.c/.h` | Wi-Fi STA 连接、低功耗（Auto Light Sleep/Modem Sleep）、HTTP 服务 + SSE 推流、串口命令解析（`WIFI_CFG/WIFI_STATUS/WIFI_STOP/WIFI_CLEAR/RECALIB`）。 |
| `web_portal.c/.h` | HTTP 路由（`/events` SSE、`/api/audio`）、遥测 JSON 打包、音频调参控制消息处理。 |
| `zone_tone.c/.h` | PDM 扬声器：各点位嵌入 WAV 播放、运行时调音（音量/时长/包络）、开关。 |
| `audio_tuning.h` | 音频触发门控参数结构（稳定保持时长 / 置信度阈值 / 冷却）。 |

> 上位机网页：`upperlevelmachine/game_demo.html`（浏览器本地打开，串口配网 + HTTP/SSE 连接 + 小游戏）。

### 主机回归测试

在示例目录中运行以下命令，验证校准、运行时、网络和网页逻辑。环境需安装 Python 3、Node.js 和主机 C/C++ 编译器（`cc`、`c++）。网络测试还使用已下载的 cJSON 组件，或已激活 ESP-IDF 5.5 环境中的 cJSON：

```bash
python3 tests/test_calibration.py
python3 tests/test_runtime.py
python3 tests/test_network.py
node --test tests/test_frontend_connection.mjs
```

### 可配置参数（编译期宏）

**功能总开关**

| 宏 | 文件 | 默认 | 说明 |
| --- | --- | --- | --- |
| `ENABLE_WIFI` | `wifi_stream.h` | 1 | 0=关闭 Wi-Fi/网页（仅本地交互、最省电）。 |
| `ENABLE_ZONE_TONE` | `zone_tone.h` | 1 | 0=关闭扬声器提示音。 |
| `TEMP_SKIP_REFLASH_WIZARD` | `main.cpp` | 0 | 开发期临时跳过冷启动校准（1=用现有/默认 zone 直接进运行）。 |

**判区 / 校准**

| 宏 | 文件 | 默认 | 说明 |
| --- | --- | --- | --- |
| `ZONE_DETECT_USE_XYZ` | `mag_zones_config.h` | 0 | 1=XYZ 三轴盒判定；0=极坐标。 |
| `ZONE_POLAR_Z_GATE` | `mag_zones_config.h` | 1 | 极坐标模式用 z 范围做置信度门控。 |
| `CALIB_SCHEMA_VER` | `mag_zones_config.h` | 5 | NVS 校准结构版本，升级后旧数据需重校。 |

**运动 / 动作判定（`main.cpp`）**

| 宏 | 默认 | 说明 |
| --- | --- | --- |
| `ACTION_MIN_HOLD_MS` | 70 | 动作点位最短保持时间（去抖）。 |
| `ACTION_RELEASE_MS` | 110 | 动作释放时间。 |
| `VISUAL_RATE_LEAK_MS` | 300 | LED 速度漏积分衰减时间常数（颜色蓝→红）。 |
| `MOTION_SLIDE_ENTER/EXIT` | 72 / 45 | 滑动状态进入/退出阈值（自适应去抖）。 |
| `SPEAKER_TOGGLE_STEP_*` | 2200 / 140 | 手势开关扬声器的步进超时/保持。 |

**电源 / 睡眠**

| 宏 | 文件 | 默认 | 说明 |
| --- | --- | --- | --- |
| `IDLE_SLEEP_MS` | `main.cpp` | 60000 | 无操作进入深睡的空闲时长。 |
| `WIFI_STA_LISTEN_INTERVAL` | `wifi_stream.c` | 10 | DTIM 监听间隔（beacon 数）；配合 `WIFI_PS_MAX_MODEM` 省电。调小=低延迟，调大=更省电。 |
| `WIFI_ACTIVE_WINDOW_MS` | `wifi_stream.c` | 4000 | 控制活跃窗口内禁止 Light Sleep。 |
| `WIFI_WEB_PAIRING_GRACE_MS` | `wifi_stream.c` | 60000 | 联网后给网页连接的配对宽限（窗口内禁深睡）。调大=空联网更耗电；调小=可能没打开网页就睡了。 |
| `WIFI_IDLE_STOP_MS` | `wifi_stream.c` | 0 | >0 时无控断链停 Wi-Fi；0=禁用。 |

**Wi-Fi / 引脚**

| 宏 | 文件 | 默认 | 说明 |
| --- | --- | --- | --- |
| `WIFI_DEFAULT_SSID/PASS` | `wifi_stream.c` | 空 | 留空=开源默认（必须串口配网）；填入=固件内置默认账号。 |
| `WIFI_STA_HOSTNAME` | `wifi_stream.c` | `magedc` | 路由器租约里识别设备的 DHCP 主机名。 |
| `WIFI_CONNECT_TIMEOUT_MS` / `WIFI_MAX_RETRY` | `wifi_stream.c` | 30000 / 8 | 连接超时与重试次数。 |
| Board Manager `led_strip` | 板级 YAML | GPIO27 / 28 灯 | MagEDC amend 将上游 1 灯改为 28 灯环。 |
| `IMU_INT_PIN` | `main.cpp` | GPIO0 | BMI270 运动唤醒中断引脚。 |
| Board Manager `i2c_master` | 板级 YAML | 共享 | BMI270 与 BMM350 使用的原生 I2C 总线。 |

> 音频音量/时长/包络、判定门控（置信度/冷却）等可在**运行时**通过网页或串口 JSON 调整，无需重新编译（见 `zone_tone.h` / `audio_tuning.h`）。

## 故障排除

- **网页提示不支持串口**：请使用 Chrome / Edge 打开 `game_demo.html`。
- **配网后连不上**：确认电脑与设备在同一局域网、IP 填写正确、设备已打印 `WIFI_READY`。
- **磁珠点位识别不准（需要重新校准）**：完整有效的校准数据在复位、唤醒和兼容固件升级后继续保留。数据缺失、损坏或版本不兼容时会自动重启向导；也可主动重校准：
  1. **串口 `RECALIB`（推荐，免烧录）**：运行中网页「串口连接」后点**「🎯 重新校准」按钮**（或任意串口监视器手动发送 `RECALIB`），设备会清除旧标定并立即重启 9 点向导；**不影响已保存的 Wi-Fi**。
  2. **擦除 NVS（恢复全新设备）**：清掉校准标记，下次启动重走首次校准。本机 ESP32-C5 用 `python -m esptool --chip esp32c5 -p COM7 erase_region 0x9000 0x6000`（或 `idf.py erase-flash`）。⚠️ 会**同时清掉 Wi-Fi 凭据**，之后需重新串口配网。
- **校准停留在当前点**：让磁珠在高亮点保持静止。移动会丢弃被打断的这批采样，并重新采集当前点。
- **后台页面功耗**：页面失焦（切后台）时会自动降频，恢复前台后自动恢复实时刷新。

如有技术问题，请在 [GitHub](https://github.com/espressif/esp-dev-kits/issues) 提交 issue。
