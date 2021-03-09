# Hello World Example

Starts a FreeRTOS task to print "Hello World" and CPU / RAM information .

## How to use example

Follow detailed instructions provided specifically for this example. 

Select the instructions depending on Espressif chip installed on your development board:

- [ESP32-S2 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/get-started/index.html)

### Hardware Required

* An ESP32-S2-HMI-DevKit-1 development board
* A USB-C cable for Power supply and programming

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

```
--- idf_monitor on /dev/cu.SLAB_USBtoUART 2000000 ---
--- Quit: Ctrl+] | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H ---
I (84) cache: Instruction cache 	: size 8KB, 4Ways, cache line size 32Byte
I (84) cache: Data cache 		: size 8KB, 4Ways, cache line size 32Byte
I (84) spiram: Found 16MBit SPI RAM device
I (84) spiram: SPI RAM mode: sram 80m
I (84) spiram: PSRAM initialized, cache is in normal (1-core) mode.
I (85) cpu_start: Pro cpu up.
I (85) cpu_start: Application information:
I (85) cpu_start: Project name:     hello-world
I (86) cpu_start: App version:      6df62c7-dirty
I (86) cpu_start: Compile time:     Feb 28 2021 23:17:16
I (86) cpu_start: ELF file SHA256:  9e2aaaaffa6d66b2...
I (87) cpu_start: ESP-IDF:          v4.2-dev-2084-g98d5b5dfd
I (87) cpu_start: Single core mode
I (306) spiram: SPI SRAM memory test OK
I (306) heap_init: Initializing. RAM available for dynamic allocation:
I (306) heap_init: At 3FFBF850 len 0003C7B0 (241 KiB): DRAM
I (307) heap_init: At 3FFFC000 len 00003A10 (14 KiB): DRAM
I (307) cpu_start: Pro cpu start user code
I (307) spiram: Adding pool of 2048K of external SPI memory to heap allocator
I (308) spiram: Reserving pool of 32K of internal memory for DMA/internal allocations
I (362) spi_flash: detected chip: generic
I (362) spi_flash: flash io: dio
I (362) cpu_start: Starting scheduler on PRO CPU.
Hello world!
This is esp32s2 chip with 1 CPU core(s), WiFi, silicon revision 0, 4MB external flash
Internal RAM size :   246832 bytes free,   295359 bytes total.
External RAM size :  2097112 bytes free,  2097151 bytes total.
```

## Troubleshooting

For any technical queries, please open an [issue] (https://github.com/espressif/esp-dev-kits/issues) on GitHub. We will get back to you soon.