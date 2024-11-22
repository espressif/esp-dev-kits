# _adc button 示例程序_

此工程展示了 按下 audio 板上 adc 按键，来触发不同的事件。

## 1 快速入门

### 1.1 硬件准备

<table>
    <tr>
        <td ><img src="../../../docs/_static/esp32-s2-kaluga-1/ESP32-S2-Kaluga_V1.0_mainbody.png" width="300" ><p align=center>ESP32-S2-Kaluga-1 开发板</p></td>
        <td ><img src="../../../docs/_static/esp32-s2-kaluga-1/ESP-LyraT-8311A-V1.0.png" width="300"><p align=center>ESP-LyraT-8311A 音频板</p></td>
    </tr>
</table>

### 1.2 硬件连接

1. 将 SW3[7] ( kaluga v1.3: T6 )的拨码开关打开（ ON ）。
2. 将 GPIO45 上预留的两个管脚短接。
3. 将音频板插入主板中。

### 1.3 配置工程

```
idf.py set-target esp32s2 menuconfig
```

* 在 `Serial flasher config` 下配置串口相关信息。

### 1.4 构建和烧录

建立工程，并将其烧入到板子，然后运行监测工具，以查看串行输出:

```
idf.py -p PORT flash monitor
```

* 如果有两个端口号，最大的是 `PORT`。

(要退出串行监视器，请键入 ``Ctrl-]`` )

有关配置和使用 ESP-IDF 构建项目的完整步骤，请参阅入门指南。


### 1.5 示例输出


您依次按下audio板右侧的一排按键，就可以在串行监视器上看到以下输出日志:

```
I (49) boot: ESP-IDF v4.2-dev-1304-g6d7897ae0 2nd stage bootloader
I (49) boot: compile time 17:00:16
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
I (115) esp_image: segment 0: paddr=0x00010020 vaddr=0x3f000020 size=0x06d40 ( 27968) map
I (130) esp_image: segment 1: paddr=0x00016d68 vaddr=0x3ffbfd60 size=0x02220 (  8736) load
I (135) esp_image: segment 2: paddr=0x00018f90 vaddr=0x40024000 size=0x00404 (  1028) load
0x40024000: _WindowOverflow4 at /home/chenzhengwei/esp/esp-idf/components/freertos/xtensa/xtensa_vectors.S:1730

I (142) esp_image: segment 3: paddr=0x0001939c vaddr=0x40024404 size=0x06c7c ( 27772) load
I (158) esp_image: segment 4: paddr=0x00020020 vaddr=0x40080020 size=0x1c04c (114764) map
0x40080020: _stext at ??:?

I (185) esp_image: segment 5: paddr=0x0003c074 vaddr=0x4002b080 size=0x04ce0 ( 19680) load
0x4002b080: spi_flash_hal_set_write_protect at /home/chenzhengwei/esp/esp-idf/components/soc/src/hal/spi_flash_hal_iram.c:57

I (197) boot: Loaded app from partition at offset 0x10000
I (197) boot: Disabling RNG early entropy source...
I (198) cache: Instruction cache        : size 8KB, 4Ways, cache line size 32Byte
I (205) cpu_start: Pro cpu up.
I (209) cpu_start: Application information:
I (214) cpu_start: Project name:     esp32-s2-hmi
I (219) cpu_start: App version:      8fce901-dirty
I (225) cpu_start: Compile time:     Jun 22 2020 17:00:10
I (231) cpu_start: ELF file SHA256:  c817f72ca76da769...
I (237) cpu_start: ESP-IDF:          v4.2-dev-1304-g6d7897ae0
I (243) cpu_start: Single core mode
I (248) heap_init: Initializing. RAM available for dynamic allocation:
I (255) heap_init: At 3FFC2990 len 00039670 (229 KiB): DRAM
I (261) heap_init: At 3FFFC000 len 00003A10 (14 KiB): DRAM
I (267) cpu_start: Pro cpu start user code
I (326) spi_flash: detected chip: gd
I (326) spi_flash: flash io: qio
I (326) cpu_start: Starting scheduler on PRO CPU.
I (329) main: Initializing WS2812
I (9939) main: rec(K1) -> red
I (10939) main: mode(K2) -> green
I (11939) main: play(K3) -> blue
I (12739) main: set(K4) -> yellow
I (14339) main: vol(K5) -> purple
I (15539) main: vol+(K6) -> write
```


