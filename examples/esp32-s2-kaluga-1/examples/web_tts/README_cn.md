# _web tts示例程序_

本示例采用的是本地的语音合成工具，您可以通过 web 发送中文句子，并将中文句子合成成语音进行播放。(注：语音是通过本地进行合成的)

## 1 快速入门

### 1.1 硬件准备

<table>
    <tr>
        <td ><img src="../../../docs/_static/esp32-s2-kaluga-1/ESP32-S2-Kaluga_V1.0_mainbody.png" width="300" ><p align=center>ESP32-S2-Kaluga-1 开发板</p></td>
        <td ><img src="../../../docs/_static/esp32-s2-kaluga-1/ESP-LyraT-8311A-V1.0.png" width="300"><p align=center>ESP-LyraT-8311A 音频板</p></td>
    </tr>
</table>

## 1.2 硬件连接

#### 1.2.1  硬件连接

1. 将 SW3 和 SW4 的拨码开关全部打开（ ON ），即主板左下角两组拨码开关。

2. 将音频板接入主板中。

### 1.3 配置工程

```
idf.py set-target esp32s2 menuconfig
```

* 在`Serial flasher config` 下配置串口相关信息。
* 设置 `Example Configuration`-> `audio pad type`: 配置板子版本。
* 设置 `Example Configuration`-> `WIFI Settings` : 配置 AP 信息。

### 1.4 烧录与使用

建立工程，并将其烧入到板子，然后运行监测工具，以查看串行输出:

```
idf.py -p PORT flash monitor
```

* 如果有两个端口号，最大的是 `PORT`。

(要退出串行监视器，请键入``Ctrl-]``)

有关配置和使用 ESP-IDF 构建项目的完整步骤，请参阅入门指南。

1. 连接 WIFI ：`esp32-s2-kaluga-tts` ( kaluga 上的 WIFI 名可在 menuconfig 中配置 ) 
2. 打开浏览器，访问 `192.168.4.1` （可在 `Example Configuration` 中修改），就会看到如下图的界面.
3. 此时输入中文句子，点击 `Save..` 即可播放对应的中文音频。


<div align="left"><img src="../../../docs/_static/esp32-s2-kaluga-1/kaluga_examples_web_tts_1.jpg" width = "650" alt="image 1-2" align=center /></div>  

您还可以在串行监视器上看到以下输出日志:

```
I (46) boot: ESP-IDF v4.2-dev-1304-g6d7897ae0 2nd stage bootloader
I (46) boot: compile time 17:39:20
I (46) boot: chip revision: 0
I (49) qio_mode: Enabling default flash chip QIO
I (54) boot.esp32s2: SPI Speed      : 80MHz
I (59) boot.esp32s2: SPI Mode       : QIO
I (64) boot.esp32s2: SPI Flash Size : 4MB
I (69) boot: Enabling RNG early entropy source...
I (74) boot: Partition Table:
I (78) boot: ## Label            Usage          Type ST Offset   Length
I (85) boot:  0 factory          factory app      00 00 00010000 002ee000
I (92) boot:  1 nvs              WiFi data        01 02 002fe000 00004000
I (100) boot:  2 wav              unknown          28 28 00302000 00020000
I (107) boot: End of partition table
I (112) esp_image: segment 0: paddr=0x00010020 vaddr=0x3f000020 size=0x218a00 (2198016) map
I (518) esp_image: segment 1: paddr=0x00228a28 vaddr=0x3ffc90a0 size=0x0496c ( 18796) load
I (523) esp_image: segment 2: paddr=0x0022d39c vaddr=0x40024000 size=0x00404 (  1028) load
0x40024000: _WindowOverflow4 at /home/chenzhengwei/esp/esp-idf/components/freertos/xtensa/xtensa_vectors.S:1730

I (525) esp_image: segment 3: paddr=0x0022d7a8 vaddr=0x40024404 size=0x02870 ( 10352) load
I (536) esp_image: segment 4: paddr=0x00230020 vaddr=0x40080020 size=0x806b8 (526008) map
0x40080020: _stext at ??:?

I (638) esp_image: segment 5: paddr=0x002b06e0 vaddr=0x40026c74 size=0x12428 ( 74792) load
0x40026c74: spi_flash_mmap_pages at /home/chenzhengwei/esp/esp-idf/components/spi_flash/flash_mmap.c:177

I (667) boot: Loaded app from partition at offset 0x10000
I (667) boot: Disabling RNG early entropy source...
I (668) cache: Instruction cache        : size 8KB, 4Ways, cache line size 32Byte
I (675) cpu_start: Pro cpu up.
I (679) cpu_start: Application information:
I (684) cpu_start: Project name:     web_tts
I (689) cpu_start: App version:      6ae1d8c
I (694) cpu_start: Compile time:     Jun 24 2020 17:39:14
I (700) cpu_start: ELF file SHA256:  fc4d30a6af43934a...
I (706) cpu_start: ESP-IDF:          v4.2-dev-1304-g6d7897ae0
I (712) cpu_start: Single core mode
I (717) heap_init: Initializing. RAM available for dynamic allocation:
I (724) heap_init: At 3FFD3160 len 00028EA0 (163 KiB): DRAM
I (730) heap_init: At 3FFFC000 len 00003A10 (14 KiB): DRAM
I (736) cpu_start: Pro cpu start user code
I (798) spi_flash: detected chip: gd
I (798) spi_flash: flash io: qio
I (798) cpu_start: Starting scheduler on PRO CPU.
I (900) wifi:wifi driver task: 3ffda938, prio:23, stack:3584, core=0
I (900) system_api: Base MAC address is not set
I (900) system_api: read default base MAC address from EFUSE
I (1080) wifi:wifi firmware version: cc57434
I (1080) wifi:wifi certification version: v7.0
I (1080) wifi:config NVS flash: enabled
I (1080) wifi:config nano formating: disabled
I (1080) wifi:Init dynamic tx buffer num: 32
I (1090) wifi:Init data frame dynamic rx buffer num: 32
I (1090) wifi:Init management frame dynamic rx buffer num: 32
I (1100) wifi:Init management short buffer num: 32
I (1100) wifi:Init static rx buffer size: 1600
I (1100) wifi:Init static rx buffer num: 10
I (1110) wifi:Init dynamic rx buffer num: 32
I (1140) wifi: wifi_init_softap finished.SSID:esp32-s2-kaluga-tts password:
I (1360) phy: phy_version: 303, c1e745d, Apr 11 2020, 14:22:04, 0, 2
I (1360) wifi:enable tsf
I (1360) wifi:mode : softAP (7c:df:a1:00:a5:3b)
I (1360) wifi:Total power save buffer number: 16
I (1370) wifi:Init max length of beacon: 752/752
I (1370) wifi:Init max length of beacon: 752/752
I (1500) DRV8311: ES8311 in Slave mode

I (1760) DRV8311: ES8311 in I2S Format

I (1930) gpio: GPIO[0]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (1930) gpio: GPIO[10]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (1930) DRV8311: SET: volume:178

I (1950) I2S: DMA Malloc info, datalen=blocksize=512, dma_buf_count=6
I (1950) I2S: DMA Malloc info, datalen=blocksize=512, dma_buf_count=6
I (1960) I2S: APLL: Req RATE: 16000, real rate: 15999.986, BITS: 16, CLKM: 1, BCK_M: 8, MCLK: 4095996.500, SCLK: 511999.562500, diva: 1, divb: 0
I (1960) ap_config: Starting web server on port: '80'
```


