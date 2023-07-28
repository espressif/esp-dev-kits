# _touch 音乐播放器程序_

此功能主要展示触摸板控制 audio 音频播放及调节音量，各按键功能如下：

PHOTO | PLAY/PAUSE | NETWORK | RECORD | VOL_UP | VOL_DOWN |
---|---|---|---|---|---|
上一首 | 播放/暂停 | 无 | 下一首 | 调大音量 | 调小音量 |

## 1 快速入门

### 1.1 硬件准备

<table>
    <tr>
        <td ><img src="../../../docs/_static/esp32-s2-kaluga-1/ESP32-S2-Kaluga_V1.0_mainbody.png" width="300" ><p align=center>ESP32-S2-Kaluga-1 开发板</p></td>
        <td ><img src="../../../docs/_static/esp32-s2-kaluga-1/ESP-LyraP-TOUCHA_V1.0.png" width="300"><p align=center>ESP-LyraP-TOUCHA 触摸板</p></td>
        <td ><img src="../../../docs/_static/esp32-s2-kaluga-1/ESP-LyraT-8311A-V1.0.png" width="300"><p align=center>ESP-LyraT-8311A 音频板</p></td>
    </tr>
</table>

### 1.2 硬件连接

#### 1.2.1 ESP32\_S2\_Kaluga-1\_v1.2  硬件连接

1. 将 SW3[ 8 ] 和 SW4[ 2、4、6、8 ] 的拨码开关打开（ ON ），即主板左下角两组拨码开关，如下图：

 **ESP32\_S2\_Kaluga-1\_v1.3**: 将拨码开关的 T7、T8、T10、T12、T14打开（ ON ）。
<div align="left"><img src="../../../docs/_static/esp32-s2-kaluga-1/kaluga_examples_touch_audio_1.jpg" width = "650" alt="image 1-1" align=center /></div>

2. 将 GPIO45 上预留的两个管脚短接，如下图：
<div align="left"><img src="../../../docs/_static/esp32-s2-kaluga-1/kaluga_examples_touch_2.jpg" width = "650" alt="image 1-2" align=center /></div>

3. 将触摸板和音频板接入主板中。如下图：

<div align="left"><img src="../../../docs/_static/esp32-s2-kaluga-1/kaluga_examples_touch_audio_2.jpg" width = "650" alt="image 1-3" align=center /></div> </br></br>

* ESP32_S2_Kaluga-1_v1.2 Touch 管脚分配

PIN| PHOTO | PLAY/PAUSE | NETWORK | RECORD | VOL_UP | VOL_DOWN | GUARD | SHIELD
 ---|---|---|---|---|---|---|---|---|
 GPIO| 6 | 2 | 11 | 5 | 1 | 3 | 4 | 14

* ESP32_S2_Kaluga-1_v1.2 Audio 管脚分配

PIN | I2S_MCLK | I2S_SCLK | I2S_LCLK | I2S_DOUT | I2S_DSIN | PA_EN | I2C_SCL | I2C_SDA
 ---|---|---|---|---|---|---|---|---|
 GPIO | 35 | 18 | 17 | 12 | 46 | 10 | 7 | 8


#### 1.2.2 ESP32\_S2\_Kaluga-1\_v1.1 硬件连接

1. 将 SW3[ 8 ] 和 SW4[ 4、6 ] 的拨码开关打开（ ON ），即主板左下角两组拨码开关.

2. 将 GPIO45 上预留的两个管脚短接，如下图：

<div align="left"><img src="../../../docs/_static/esp32-s2-kaluga-1/kaluga_examples_touch_2.jpg" width = "650" alt="image 1-2" align=center /></div>

3. 将触摸板和音频板接入主板中。并将 audio 板子的 1SDO 接至 GPIO36, SDA 接至 GPIO37。

* ESP32\_S2\_kaluga-1\_v1.1 Touch 管脚分配

PIN |PHOTO | PLAY/PAUSE | NETWORK | RECORD | VOL_UP | VOL_DOWN | GUARD | SHIELD
 ---|---|---|---|---|---|---|---|---|
 GPIO| 3 | 8 | 11 | 13 | 2 | 9 |4 | 14

* ESP32_S2_Kaluga-1_v1.2 Audio 管脚分配

PIN | I2S_MCLK | I2S_SCLK | I2S_LCLK | I2S_DOUT | I2S_DSIN | PA_EN | I2C_SCL | I2C_SDA
---|---|---|---|---|---|---|---|---|
GPIO | 35 | 17 | 18 | 14(36) | 12 | 10 | 7 | 8(37)

### 1.3 配置工程

```
idf.py set-target esp32s2 menuconfig
```

* 在`Serial flasher config` 下配置串口相关信息。
* 设置 `Example Configuration`-> `WS2812 GPIO`: 配置 LED 灯的 GPIO 管脚。
* 设置 `Example Configuration`-> `touch pad type` & `audio pad type` :选择子板版本。
* 设置 `Example Configuration`-> `WIFI config` :打开 WIFI 配置。

### 1.4 构建和烧录

#### 1.4.1 烧入音频文件

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
I (46) boot: ESP-IDF v4.2-dev-1303-gcd4fa46f5 2nd stage bootloader
I (46) boot: compile time 15:15:43
I (46) boot: chip revision: 0
I (49) qio_mode: Enabling default flash chip QIO
I (55) boot.esp32s2: SPI Speed      : 80MHz
I (59) boot.esp32s2: SPI Mode       : QIO
I (64) boot.esp32s2: SPI Flash Size : 4MB
I (69) boot: Enabling RNG early entropy source...
I (74) boot: Partition Table:
I (78) boot: ## Label            Usage          Type ST Offset   Length
I (85) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (93) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (100) boot:  2 factory          factory app      00 00 00010000 00100000
I (108) boot:  3 storage          Unknown data     01 82 00110000 002f0000
I (115) boot: End of partition table
I (119) esp_image: segment 0: paddr=0x00010020 vaddr=0x3f000020 size=0x0d010 ( 53264) map
I (138) esp_image: segment 1: paddr=0x0001d038 vaddr=0x3ffbf450 size=0x01ee8 (  7912) load
I (140) esp_image: segment 2: paddr=0x0001ef28 vaddr=0x40024000 size=0x00404 (  1028) load
0x40024000: _WindowOverflow4 at /home/chenzhengwei/esp/esp-idf/components/freertos/xtensa/xtensa_vectors.S:1730

I (146) esp_image: segment 3: paddr=0x0001f334 vaddr=0x40024404 size=0x00ce4 (  3300) load
I (156) esp_image: segment 4: paddr=0x00020020 vaddr=0x40080020 size=0x2a398 (172952) map
0x40080020: _stext at ??:?

I (195) esp_image: segment 5: paddr=0x0004a3c0 vaddr=0x400250e8 size=0x0a360 ( 41824) load
0x400250e8: esp_reset_reason_get_hint at /home/chenzhengwei/esp/esp-idf/components/esp_system/port/esp32s2/reset_reason.c:107

I (211) boot: Loaded app from partition at offset 0x10000
I (211) boot: Disabling RNG early entropy source...
I (212) cache: Instruction cache        : size 8KB, 4Ways, cache line size 32Byte
I (219) cpu_start: Pro cpu up.
I (223) cpu_start: Application information:
I (228) cpu_start: Project name:     esp32-s2-hmi
I (233) cpu_start: App version:      fc9c586-dirty
I (239) cpu_start: Compile time:     May 19 2020 15:15:35
I (245) cpu_start: ELF file SHA256:  336b7f63612e5487...
I (251) cpu_start: ESP-IDF:          v4.2-dev-1303-gcd4fa46f5
I (257) cpu_start: Single core mode
I (262) heap_init: Initializing. RAM available for dynamic allocation:
I (269) heap_init: At 3FFC1BD0 len 0003A430 (233 KiB): DRAM
I (275) heap_init: At 3FFFC000 len 00003A10 (14 KiB): DRAM
I (281) cpu_start: Pro cpu start user code
I (340) spi_flash: detected chip: generic
I (340) spi_flash: flash io: qio
I (340) cpu_start: Starting scheduler on PRO CPU.
I (343) main: [APP] Startup..
I (343) main: [APP] Free memory: 238380 bytes
I (343) main: [APP] IDF version: v4.2-dev-1303-gcd4fa46f5
I (353) main: Initializing SPIFFS
I (413) main: Partition size: total: 2824001, used: 2596344
I (413) main: Reading file
I (443) main: Read from file: 'esp32s2-kaluga-1'
I (443) main: Initializing WS2812
I (443) Touch pad: Initializing touch pad
I (453) Touch pad: Denoise function init
I (453) Touch pad: touch pad waterproof init
I (463) Touch pad: touch pad filter init 1
I (473) DRV8311: ES8311 in Slave mode

I (483) DRV8311: ES8311 in I2S Format

I (483) gpio: GPIO[0]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0
I (483) gpio: GPIO[10]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0
I (493) DRV8311: SET: volume:127

I (503) I2S: DMA Malloc info, datalen=blocksize=1024, dma_buf_count=6
I (503) I2S: DMA Malloc info, datalen=blocksize=1024, dma_buf_count=6
I (523) I2S: APLL: Req RATE: 44100, real rate: 44099.988, BITS: 16, CLKM: 1, BCK_M: 8, MCLK: 11289597.000, SCLK: 1411199.625000, diva: 1, divb: 0
I (523) AUDIO: start to decode /spiffs/To_meet_the_prime_time_44k.mp3
I (563) Touch pad: test init: touch pad [6] base 15347, thresh 1534
I (563) Touch pad: test init: touch pad [2] base 14503, thresh 1450
I (563) Touch pad: test init: touch pad [11] base 16087, thresh 1608
I (583) Touch pad: test init: touch pad [5] base 14373, thresh 1437
I (583) Touch pad: test init: touch pad [1] base 15543, thresh 1554
I (583) Touch pad: test init: touch pad [3] base 14856, thresh 1485
I (603) Touch pad: test init: touch pad [4] base 32953, thresh 3295
I (603) I2S: APLL: Req RATE: 44100, real rate: 44099.988, BITS: 16, CLKM: 1, BCK_M: 8, MCLK: 11289597.000, SCLK: 1411199.625000, diva: 1, divb: 0
I (613) AUDIO: mp3file info---bitrate=128000,layer=3,nChans=2,samprate=44100,outputSamps=2304
I (633) AUDIO: 0x3ffc68b8

I (633) DRV8311: SET: volume:127
```

### 1.6 故障排除

* 烧录失败
  * 硬件连接不正确：运行 `idf.py monitor` ，然后重新启动电路板，以查看是否有任何输出日志。
  * 下载的波特率太高：降低 `menuconfig` 菜单中的波特率，然后重试。
  * 其他原因：先拔下外设，如 `audio` 和 `camera`设备。再进行烧录。
* 触摸没有反应或触摸不良
  * 拨码开关没有正确关闭（ OFF ）。
* 音频没有声音：
  * 未插入 `audio` 板子和喇叭。
  * 分区表未设置或设置错误。
  * 音频相关的拨码开关没有正确打开（ ON ）。

## 2 附录

* [ESP32-S2 技术规格书](https://www.espressif.com/sites/default/files/documentation/esp32-s2_datasheet_cn.pdf)
* [ESP32-S2-Kaluga-1 主板](../../../docs/_static/esp32-s2-kaluga-1/schematics/SCH_ESP32-S2-KALUGA-1_V1_2_20200325A.pdf)
* [ESP-LyraP-TOUCHA 触摸板](../../../docs/_static/esp32-s2-kaluga-1/schematics/SCH_ESP-LYRAP-TOUCHA_V1.1_20200325A.pdf)