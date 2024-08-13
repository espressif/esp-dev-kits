_请注意，本文档为 esp-dev-kits 示例 README.md 文件的模板。在使用此模板时，请将所有斜体占位符文本替换为具体示例相关的内容。_

# _示例标题_

_英文版本链接_

[英文版本](./README_EN.md)

_介绍该示例的功能。介绍部分应尽量回答以下几个问题：_
- _该示例是什么？有什么用？_
- _该示例用到了开发板的哪些功能（例如 USB 主机、传感器、智能面板等）？_
- _基于此示例可以创建什么？即应用程序/使用案例等。_


## 快速入门


### 准备工作

_该示例在哪块开发板上运行？_

_如果需要用到其他设备，如服务器、蓝牙设备、应用程序、辅助芯片等，请在此处列出。若有相关链接，请附上。解释如何设置这些设备。_


### ESP-IDF 要求

_指定该示例推荐或要求使用的 ESP-IDF 分支。可参考以下表述。_

- 此示例支持 ESP-IDF release/v[x.y] 及以上版本。默认情况下，在 ESP-IDF release/v[x.y] 上运行。
- 请参照 [ESP-IDF 编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html) 设置开发环境。**强烈推荐** 通过 [编译第一个工程](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html#id8) 来熟悉 ESP-IDF，并确保环境设置正确。

### 获取 esp-dev-kits 仓库

_描述克隆仓库的方法。_

在编译 esp-dev-kits 仓库中的示例之前，请先在终端中运行以下命令，将该仓库克隆到本地：

```
git clone --recursive https://github.com/espressif/esp-dev-kits.git
```


### 配置

- _描述在 menuconfig 中需要配置的重要项目，例如 FatFs 的长文件名支持、兼容音频板的选择、芯片类型、PSRAM 时钟和 Wi-Fi/LwIP 参数。请参考以下示例。_

    运行 ``idf.py menuconfig`` 并修改 QWeather 和 Wi-Fi 配置：

    ```
    menuconfig > QWeather > QWEATHER_REQUEST_KEY
    ```

- _若有必要，请描述其他软件的配置，例如指定补丁。_


## 如何使用示例


### 编译和烧录示例

_编译和烧录示例需要用到的命令。请参考以下示例。_

编译项目并将其烧录到开发板上，运行监视工具可查看串行端口输出（将 `PORT` 替换为所用开发板的串行端口名）：

```c
idf.py -p PORT flash monitor
```

输入``Ctrl-]`` 可退出串口监视。

有关配置和使用 ESP-IDF 来编译项目的完整步骤，请参阅 [ESP-IDF 快速入门指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html) 。


### 示例输出

_添加该示例的预期输出效果。请参考以下示例。_

- 完整日志如下所示：

    ```c
    I (64) boot: Chip Revision: 3
    I (35) boot: ESP-IDF v3.3.1-203-g0c1859a5a 2nd stage bootloader
    I (35) boot: compile time 21:43:15
    I (35) boot: Enabling RNG early entropy source...
    I (41) qio_mode: Enabling default flash chip QIO
    I (46) boot: SPI Speed      : 80MHz
    I (50) boot: SPI Mode       : QIO
    I (54) boot: SPI Flash Size : 8MB
    ```
- 以下动画展示了示例在不同开发板上运行的效果。


## 故障排除

_此部分为可选内容。如果有用户可能会遇到的问题或错误，请在此提及。对于非常简单的示例，可以删除此部分。_

## 技术支持与反馈

请通过以下渠道进行反馈：

- 有关技术问题，请访问 [esp32.com](https://esp32.com/viewforum.php?f=22) 论坛。
- 有关功能请求或错误报告，请创建新的 [GitHub 问题](https://github.com/espressif/esp-dev-kits/issues)。


我们会尽快回复。
