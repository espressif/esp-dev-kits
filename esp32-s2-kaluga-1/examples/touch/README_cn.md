# _触摸板控制 LED 示例程序_

此功能主要展示触摸板控制 led 灯颜色及调节亮度，各按键功能如下：

PHOTO | PLAY/PAUSE | NETWORK | RECORD | VOL_UP | VOL_DOWN |
---|---|---|---|---|---|
绿色 | 红色 | 蓝色 | 关闭灯光 | 调亮灯光 | 调暗灯光 |

## 1 快速入门

### 1.1 硬件准备

<table>
    <tr>
        <td ><img src="../../../docs/_static/esp32-s2-kaluga-1/ESP32-S2-Kaluga_V1.0_mainbody.png" width="300" ><p align=center>ESP32-S2-Kaluga-1 开发板</p></td>
        <td ><img src="../../../docs/_static/esp32-s2-kaluga-1/ESP-LyraP-TOUCHA_V1.0.png" width="300"><p align=center>ESP-LyraP-TOUCHA 触摸板</p></td>
    </tr>
</table>

### 1.2 硬件连接

1. 将 SW3 和 SW4 的拨码开关全部关闭（ OFF ）,即主板左下角两组拨码开关，如下图.
  *  或者将 SW3[2 3 4 6 7] 和 SW4[5 8] (kaluga 1.3: T1, T2, T3, T5, T6，T11, T14)关闭（ OFF ）.
<div align="left"><img src="../../../docs/_static/esp32-s2-kaluga-1/kaluga_examples_touch_1.jpg" width = "650" alt="image 1-1" align=center /></div>

2. 将 GPIO45 上预留的两个管脚短接，如下图：
<div align="left"><img src="../../../docs/_static/esp32-s2-kaluga-1/kaluga_examples_touch_2.jpg" width = "650" alt="image 1-2" align=center /></div>

3. 将触摸板接入主板中。如下图：

<div align="left"><img src="../../../docs/_static/esp32-s2-kaluga-1/kaluga_examples_touch_3.jpg" width = "650" alt="image 1-3" align=center /></div> </br></br>

* ESP32_S2_Kaluga-1_v1.2 & ESP32_S2_Kaluga-1_v1.3 Touch 管脚分配

PHOTO | PLAY/PAUSE | NETWORK | RECORD | VOL_UP | VOL_DOWN | GUARD | SHIELD
 ---|---|---|---|---|---|---|---|
 GPIO2 | GPIO6 | GPIO11 | GPIO5 | GPIO1 | GPIO3 |GPIO4 | GPIO14

* ESP32_S2_kaluga-1_v1.1 Touch 管脚分配

PHOTO | PLAY/PAUSE | NETWORK | RECORD | VOL_UP | VOL_DOWN | GUARD | SHIELD
 ---|---|---|---|---|---|---|---|
 GPIO3 | GPIO8 | GPIO11 | GPIO13 | GPIO2 | GPIO9 |GPIO4 | GPIO14

### 1.3 配置工程

```
idf.py set-target esp32s2 menuconfig
```

* 在`Serial flasher config` 下配置串口相关信息。
* 设置 `Example Configuration`-> `WS2812 GPIO`: 配置 LED 灯的 GPIO 管脚。
* 设置 `Example Configuration`-> `Number of LEDS in a strip` :所接灯的数量。
* 设置 `Example Configuration`-> `touch pad type` :选择主板版本。

### 1.4 构建和烧录

建立工程，并将其烧入到板子，然后运行监测工具，以查看串行输出:

```
idf.py -p PORT flash monitor
```

* 如果有两个端口号，最大的是 `PORT`。

(要退出串行监视器，请键入``Ctrl-]``)

有关配置和使用 ESP-IDF 构建项目的完整步骤，请参阅入门指南。

### 1.5 示例输出

当程序烧录进去后，你会看到如下场景，则烧录成功。

您还可以在串行监视器上看到以下输出日志:

```
I (48) boot: chip revision: 0
I (51) boot.esp32s2: SPI Speed      : 40MHz
I (56) boot.esp32s2: SPI Mode       : DIO
I (61) boot.esp32s2: SPI Flash Size : 4MB
I (66) boot: Enabling RNG early entropy source...
I (71) boot: Partition Table:
I (75) boot: ## Label            Usage          Type ST Offset   Length
I (82) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (89) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (97) boot:  2 factory          factory app      00 00 00010000 00100000
I (104) boot: End of partition table
I (109) esp_image: segment 0: paddr=0x00010020 vaddr=0x3f000020 size=0x06370 ( 25456) map
I (125) esp_image: segment 1: paddr=0x00016398 vaddr=0x3ffbe240 size=0x01f68 (  8040) load
I (129) esp_image: segment 2: paddr=0x00018308 vaddr=0x40024000 size=0x00404 (  1028) load
0x40024000: _WindowOverflow4 at /home/chenzhengwei/esp/esp-idf/components/freertos/xtensa/xtensa_vectors.S:1730

I (136) esp_image: segment 3: paddr=0x00018714 vaddr=0x40024404 size=0x07904 ( 30980) load
I (154) esp_image: segment 4: paddr=0x00020020 vaddr=0x40080020 size=0x156c0 ( 87744) map
0x40080020: _stext at ??:?

I (179) esp_image: segment 5: paddr=0x000356e8 vaddr=0x4002bd08 size=0x02534 (  9524) load
0x4002bd08: xRingbufferReceive at /home/chenzhengwei/esp/esp-idf/components/esp_ringbuf/ringbuf.c:1104 (discriminator 2)

I (188) boot: Loaded app from partition at offset 0x10000
I (188) boot: Disabling RNG early entropy source...
I (191) cache: Instruction cache        : size 8KB, 4Ways, cache line size 32Byte
I (198) cpu_start: Pro cpu up.
I (202) cpu_start: Application information:
I (207) cpu_start: Project name:     touch
I (212) cpu_start: App version:      8344cda-dirty
I (217) cpu_start: Compile time:     Apr 22 2020 17:55:46
I (223) cpu_start: ELF file SHA256:  9edee7d2ad3b3f70...
I (229) cpu_start: ESP-IDF:          v4.2-dev-1126-gd85d3d969
I (236) cpu_start: Single core mode
I (240) heap_init: Initializing. RAM available for dynamic allocation:
I (247) heap_init: At 3FFBE23C len 00000004 (0 KiB): D/IRAM
I (253) heap_init: At 3FFC09E0 len 0003B620 (237 KiB): D/IRAM
I (260) heap_init: At 3FFFC000 len 00003A10 (14 KiB): DRAM
I (266) cpu_start: Pro cpu start user code
I (325) spi_flash: detected chip: generic
I (326) spi_flash: flash io: dio
I (326) cpu_start: Starting scheduler on PRO CPU.
I (329) Touch pad: Initializing WS2812
I (329) Touch pad: Initializing touch pad
I (329) Touch pad: Denoise function init
I (339) Touch pad: touch pad waterproof init
I (339) Touch pad: touch pad filter init
I (449) Touch pad: touch pad [8] base 17423, thresh 174
I (449) Touch pad: touch pad [3] base 17390, thresh 173
I (449) Touch pad: touch pad [11] base 16966, thresh 169
I (449) Touch pad: touch pad [13] base 15398, thresh 153
I (459) Touch pad: touch pad [2] base 15324, thresh 153
I (459) Touch pad: touch pad [9] base 17431, thresh 174
I (469) Touch pad: touch pad [4] base 15530, thresh 155
```

### 1.6 故障排除

* 烧录失败
  * 硬件连接不正确：运行 `idf.py monitor` ，然后重新启动电路板，以查看是否有任何输出日志。
  * 下载的波特率太高：降低 `menuconfig` 菜单中的波特率，然后重试。
* 触摸没有反应或触摸不良
  * 拨码开关没有正确关闭（ OFF ）。
  * 触摸板与主板连线接触不良。
  * GPIO45 没短接。

## 2 附录

* [ESP32-S2 技术规格书](https://www.espressif.com/sites/default/files/documentation/esp32-s2_datasheet_cn.pdf)
* [ESP32-S2-Kaluga-1 主板](../../../docs/_static/esp32-s2-kaluga-1/schematics/SCH_ESP32-S2-KALUGA-1_V1_2_20200325A.pdf)
* [ESP-LyraP-TOUCHA 触摸板](../../../docs/_static/esp32-s2-kaluga-1/schematics/SCH_ESP-LYRAP-TOUCHA_V1.1_20200325A.pdf)