# _SPI LCD屏示例程序_

此工程展示了 jpeg 图片解码后缓存到文件系统里，并将图片输出到 LCD 屏幕上。

## 1 快速入门

### 1.1 硬件准备

<table>
    <tr>
        <td ><img src="../../../docs/_static/esp32-s2-kaluga-1/ESP32-S2-Kaluga_V1.0_mainbody.png" width="300" ><p align=center>ESP32-S2-Kaluga-1 开发板</p></td>
        <td ><img src="../../../docs/_static/esp32-s2-kaluga-1/ESP-LyraP-LCD32_V1.0.png" width="300"><p align=center>ESP-LyraP-LCD32 显示屏</p></td>
    </tr>
</table>

### 1.2 硬件连接

1. 将 SW3 和 SW4 的拨码开关全部打开（ ON ），即主板左下角两组拨码开关，如下图：
* 高级：如果想使用部分触摸板 GPIO 管脚，只需将触摸板相关拨码开关关闭OFF，详情请参阅主板和触摸板原理图。

<div align="left"><img src="../../../docs/_static/esp32-s2-kaluga-1/kaluga_examples_camera_1.jpg" width = "650" alt="image 1-2" align=center /></div>

2. 将显示屏插入主板双排排针的内侧一排，如下图:

<div align="left"><img src="../../../docs/_static/esp32-s2-kaluga-1/kaluga_examples_lcd_1.jpg" width = "650" alt="image 1-2" align=center /></div>

* ESP32_S2_kaluga-1_v1.1 LCD 管脚分配

LCD PIN|MISO|MOSI|CLK|CS|D_C|RST|BCKL(BLCT)|
 ---|---| ---|---| ---|---| ---|---|
 **GPIO** |GPIO8|GPIO9|GPIO15|GPIO11|GPIO13|GPIO16|GPIO6

### 1.3 配置工程

```
idf.py set-target esp32s2 menuconfig
```

* 在 `Serial flasher config` 下配置串口相关信息。
* 设置 `Example Configuration`->`LCD module type` : 配置显示屏类型。
* 设置 `Example Configuration`->`image type` : 设置显示动态 / 静态图。

### 1.4 构建和烧录

建立工程，并将其烧入到板子，然后运行监测工具，以查看串行输出:

```
idf.py -p PORT flash monitor
```

* 如果有两个端口号，最大的是 `PORT`。

(要退出串行监视器，请键入 ``Ctrl-]`` )

有关配置和使用 ESP-IDF 构建项目的完整步骤，请参阅入门指南。


### 1.5 示例输出

当程序烧录进去后，你会看到如下场景，则烧录成功。

![lcd展示图](../../../docs/_static/esp32-s2-kaluga-1/kaluga_examples_lcd_2.jpg)

您还可以在串行监视器上看到以下输出日志:

```
I (49) boot: ESP-IDF v4.2-dev-1097-g2e14149bf 2nd stage bootloader
I (49) boot: compile time 19:19:55
I (49) boot: chip revision: 0
I (53) qio_mode: Enabling default flash chip QIO
I (58) boot.esp32s2: SPI Speed      : 40MHz
I (63) boot.esp32s2: SPI Mode       : QIO
I (67) boot.esp32s2: SPI Flash Size : 4MB
I (72) boot: Enabling RNG early entropy source...
I (77) boot: Partition Table:
I (81) boot: ## Label            Usage          Type ST Offset   Length
I (88) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (96) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (103) boot:  2 factory          factory app      00 00 00010000 00100000
I (111) boot: End of partition table
I (115) esp_image: segment 0: paddr=0x00010020 vaddr=0x3f000020 size=0x16bc0 ( 93120) map
I (145) esp_image: segment 1: paddr=0x00026be8 vaddr=0x3ffbe1a0 size=0x02194 (  8596) load
I (147) esp_image: segment 2: paddr=0x00028d84 vaddr=0x40024000 size=0x00404 (  1028) load
0x40024000: _WindowOverflow4 at /home/chenzhengwei/esp/esp-idf/components/freertos/xtensa/xtensa_vectors.S:1730

I (152) esp_image: segment 3: paddr=0x00029190 vaddr=0x40024404 size=0x06e88 ( 28296) load
I (168) esp_image: segment 4: paddr=0x00030020 vaddr=0x40080020 size=0x16e44 ( 93764) map
0x40080020: _stext at ??:?

I (190) esp_image: segment 5: paddr=0x00046e6c vaddr=0x4002b28c size=0x02f0c ( 12044) load
0x4002b28c: split_if_necessary at /home/chenzhengwei/esp/esp-idf/components/heap/multi_heap.c:281 (discriminator 1)

I (199) boot: Loaded app from partition at offset 0x10000
I (199) boot: Disabling RNG early entropy source...
I (202) cache: Instruction cache        : size 8KB, 4Ways, cache line size 32Byte
I (209) cpu_start: Pro cpu up.
I (213) cpu_start: Application information:
I (218) cpu_start: Project name:     spi_lcd
I (223) cpu_start: App version:      f3c6b2f-dirty
I (228) cpu_start: Compile time:     Apr 20 2020 19:19:49
I (234) cpu_start: ELF file SHA256:  c76856ea7cd7e3ea...
I (240) cpu_start: ESP-IDF:          v4.2-dev-1097-g2e14149bf
I (247) cpu_start: Single core mode
I (251) heap_init: Initializing. RAM available for dynamic allocation:
I (258) heap_init: At 3FFBE198 len 00000008 (0 KiB): D/IRAM
I (264) heap_init: At 3FFC1088 len 0003AF78 (235 KiB): D/IRAM
I (271) heap_init: At 3FFFC000 len 00003A10 (14 KiB): DRAM
I (277) cpu_start: Pro cpu start user code
I (334) spi_flash: detected chip: generic
I (334) spi_flash: flash io: qio
I (335) cpu_start: Starting scheduler on PRO CPU.
I (737) LCD: LCD ID: 00290000
I (737) LCD: ST7789V detected.
I (737) LCD: kconfig: force CONFIG_LCD_TYPE_ST7789V.
I (737) LCD: ST7789V initialization.
```

### 1.6 故障排除

* 烧录失败
  * 硬件连接不正确：运行 `idf.py monitor`，然后重新启动电路板，以查看是否有任何输出日志。
  * 下载的波特率太高：降低 `menuconfig` 菜单中的波特率，然后重试。
* 没有显示图像
  * 拨码开关是否拨的正确：左下角两排拨码开关全部打开（ ON ）。
  * 程序中的 GPIO 配置是否正确：GPIO 管脚和显示屏引脚是否对应。

## 2 附录

* [ESP32-S2 技术规格书](https://www.espressif.com/sites/default/files/documentation/esp32-s2_datasheet_cn.pdf)
* [ESP32-S2-Kaluga-1 主板原理图](../../../docs/_static/esp32-s2-kaluga-1/schematics/SCH_ESP32-S2-KALUGA-1_V1_2_20200325A.pdf)
* [ESP-LyraP-LCD32 显示屏原理图](../../../docs/_static/esp32-s2-kaluga-1/schematics/SCH_ESP-LYRAP-LCD32_V1_1_20200324A.pdf)
