# _Camera sample program_

This project shows that after the camera collects image data, it displays its image data on the display.

## 1 Quick Start

### 1.1 Hardware preparation

<table>
    <tr>
        <td ><img src="../../docs/_static/ESP32-S2-Kaluga_V1.0_mainbody.png" width="300" ><p align=center>ESP32-S2-Kaluga-1 development board</p></td>
        <td ><img src="../../docs/_static/ESP-LyraP-LCD32_V1.0.png" width="300"><p align=center>ESP-LyraP-LCD32 display</p></td>
        <td ><img src="../../docs/_static/ESP-LyraP-CAM_V1.0.png" width="300"><p align=center>ESP-LyraP-CAM camera</p></td>
    </tr>
</table>

## 1.2 Hardware connection

1. Turn on all the DIP switches of SW3 and SW4, that is, the two groups of DIP switches in the lower left corner of the motherboard, as shown below:
* Advanced: If you want to use some touchpad pins, just turn off the related DIP switch of the touchpad, please refer to the schematic diagram of the motherboard and touchpad for details.

<div align="left"><img src="../../docs/_static/kaluga_examples_touch_1.jpg" width = "450" ​​alt="image 1-2" align=center /></div>  

2. Insert the display into the inner row of the double-row pin on the motherboard (Note: Do not connect the audio board between the display and the motherboard), as shown below:

<div align="left"><img src="../../docs/_static/kaluga_examples_lcd_1.jpg" width = "450" ​​alt="image 1-2" align=center /></div>  

3. Insert the camera into the motherboard, as shown below:

<div align="left"><img src="../../docs/_static/kaluga_examples_camera_1.jpg" width = "450" ​​alt="image 1-2" align=center /></div>  

* ESP32_S2_Kaluga-1 LCD pin assignment

LCD PIN|MISO|MOSI|CLK|CS|D_C|RST|BCKL(BLCT)|
 ---|---| ---|---| ---|---| ---|---|
 **GPIO** |8|9|15|11|13|16|6

* ESP32_S2_Kaluga-1_v1.1 Camera pin assignment

CAMERA PIN|XCLK|PCLK|VSYNC|HSYNC|D2|D3|D4|D5|D6|D7|D8|D9|
---|---| ---|---| ---|---| ---|---|---|---|---|---|---|
**GPIO** |1|0|2|3|46|45|41|42|39|40|21|38 

* ESP32_S2_kaluga-1_v1.2 Camera pin assignment

CAMERA PIN|XCLK|PCLK|VSYNC|HSYNC|D2|D3|D4|D5|D6|D7|D8|D9|
---|---| ---|---| ---|---| ---|---|---|---|---|---|---|
**GPIO** |1|33|2|3|46|45|41|42|39|40|21|38

### 1.3 Configuration Engineering

```
idf.py set-target esp32s2 menuconfig
```

* Configure serial port related information under `Serial flasher config`.
* In `Example Configuration` -> `kaluga version`: select the kaluga motherboard version.
* In `Example Configuration` -> `jpeg mode`: select whether you need to enable jpeg decoding.
* In `Component config` -> `ESP32S2-specific` -> `Support for external, SPI-connected RAM`: This project needs to open SPIRAM to confirm whether it is open.

## 1.4 Build and burn

Build the project and burn it into the board, then run the monitoring tool to view the serial output:

```
idf.py -p PORT flash monitor
```

* If there are two port numbers, the largest one is `PORT`.

(To exit the serial monitor, type ``Ctrl-]``)

For the complete steps to configure and use ESP-IDF to build a project, please refer to the Getting Started Guide.

### 1.5 Sample output

When the program is burned in, you will see the following scene, then the burn is successful.

You can also see the following output log on the serial monitor:

```
 (60) boot.esp32s2: SPI Mode       : QIO
I (64) boot.esp32s2: SPI Flash Size : 4MB
I (69) boot: Enabling RNG early entropy source...
I (75) boot: Partition Table:
I (78) boot: ## Label            Usage          Type ST Offset   Length
I (85) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (93) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (100) boot:  2 factory          factory app      00 00 00010000 00100000
I (108) boot: End of partition table
I (112) esp_image: segment 0: paddr=0x00010020 vaddr=0x3f000020 size=0x067a8 ( 26536) map
I (126) esp_image: segment 1: paddr=0x000167d0 vaddr=0x3ffbe820 size=0x01f9c (  8092) load
I (132) esp_image: segment 2: paddr=0x00018774 vaddr=0x40024000 size=0x00404 (  1028) load
0x40024000: _WindowOverflow4 at /home/chenzhengwei/esp/esp-idf/components/freertos/xtensa/xtensa_vectors.S:1730

I (139) esp_image: segment 3: paddr=0x00018b80 vaddr=0x40024404 size=0x07498 ( 29848) load
I (155) esp_image: segment 4: paddr=0x00020020 vaddr=0x40080020 size=0x17570 ( 95600) map
0x40080020: _stext at ??:?

I (174) esp_image: segment 5: paddr=0x00037598 vaddr=0x4002b89c size=0x02f84 ( 12164) load
0x4002b89c: prvAcquireItemNoSplit at /home/chenzhengwei/esp/esp-idf/components/esp_ringbuf/ringbuf.c:325 (discriminator 1)

I (183) boot: Loaded app from partition at offset 0x10000
I (183) boot: Disabling RNG early entropy source...
I (183) cache: Instruction cache        : size 8KB, 4Ways, cache line size 32Byte
I (191) cache: Data cache               : size 8KB, 4Ways, cache line size 32Byte
I (198) spiram: Found 16MBit SPI RAM device
I (203) spiram: SPI RAM mode: sram 80m
I (207) spiram: PSRAM initialized, cache is in normal (1-core) mode.
I (214) cpu_start: Pro cpu up.
I (218) cpu_start: Application information:
I (223) cpu_start: Project name:     esp32-s2-hmi
I (228) cpu_start: App version:      535fb9e-dirty
I (234) cpu_start: Compile time:     Apr 27 2020 21:39:04
I (240) cpu_start: ELF file SHA256:  9b9cdfa6808cf056...
I (246) cpu_start: ESP-IDF:          v4.2-dev-1054-g7d0f1536e-dirty
I (253) cpu_start: Single core mode
I (476) spiram: SPI SRAM memory test OK
I (476) heap_init: Initializing. RAM available for dynamic allocation:
I (476) heap_init: At 3FFC1038 len 0003AFC8 (235 KiB): D/IRAM
I (483) heap_init: At 3FFFC000 len 00003A10 (14 KiB): DRAM
I (489) cpu_start: Pro cpu start user code
I (494) spiram: Adding pool of 2048K of external SPI memory to heap allocator
I (501) spiram: Reserving pool of 32K of internal memory for DMA/internal allocations
I (562) spi_flash: detected chip: generic
I (562) spi_flash: flash io: qio
I (562) cpu_start: Starting scheduler on PRO CPU.
I (565) gpio: GPIO[6]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (565) gpio: GPIO[11]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (575) gpio: GPIO[13]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (585) gpio: GPIO[16]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (595) lcd: lcd_buffer_size: 2048, lcd_dma_size: 1024, lcd_dma_node_cnt: 2

I (905) lcd: lcd init ok

I (905) gpio: GPIO[2]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 1| Intr:2 
I (905) cam: cam_xclk_pin setup

I (905) cam: cam_buffer_size: 7680, cam_dma_size: 3840, cam_dma_node_cnt: 2, cam_total_cnt: 40

I (915) cam: frame1_buffer_en

I (915) cam: frame2_buffer_en

I (1015) main: camera init done

```

### 1.6 Troubleshooting

* Burn failed
  * The hardware connection is incorrect: run `idf.py monitor` and then restart the board to see if there are any output logs.
  * The download baud rate is too high: lower the baud rate in the `menuconfig` menu and try again.
* Cannot display or display abnormally
  * The DIP switch is not properly turned on (ON).
  * Whether the camera or display is in poor contact.
  * Whether SPIRAM is turned on in `menuconfig`.
  * Do not connect the audio board between the display and the motherboard.

## 2 Appendix

* [ESP32-S2 data sheet] (../../docs/datasheet/esp32-s2_datasheet_en.pdf)
* [ESP32-S2-Kaluga-1 motherboard] (../../docs/schematic/SCH_ESP32-S2-KALUGA-1_V1_2_20200325A.pdf)
* [ESP-LyraP-CAM camera] (../../docs/schematic/SCH_ESP-LYRAP-CAM_V1_20200302.pdf)
* [ESP-LyraP-LCD32 display] (../../docs/schematic/SCH_ESP-LYRAP-LCD32_V1_1_20200324A.pdf)
