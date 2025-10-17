# Factory Demo

[中文版本](./README_CN.md)

This example is based on ESP32-P4-EYE and demonstrates a comprehensive mini camera application that supports photo capture, timed photo capture, video recording, album preview, USB SD card mounting, and image parameter settings (with adjustable resolution, saturation, contrast, brightness, and hue). Building on this foundation, the application also integrates face detection, pedestrian detection, and YOLOv11 nano model-based object detection capabilities, enhancing intelligent visual recognition. The project comprehensively utilizes various peripheral resources of the development board, including MIPI-CSI camera interface, SPI LCD display interface, USB High-Speed interface, button input, rotary encoder, and SD card storage.

## Quick Start

### Prerequisites

* An ESP32-P4-EYE development board.
* A USB-C cable for power supply and programming.
* An SD card for storing photos and videos (optional)

### ESP-IDF Requirements

This example supports the following ESP-IDF version:

- ESP-IDF release/v5.5 and all bugfix releases

Please refer to the [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32p4/get-started/index.html) to set up the development environment. **Strongly recommended** to go through [Build Your First Project](https://docs.espressif.com/projects/esp-idf/en/latest/esp32p4/get-started/index.html#id8) to familiarize yourself with ESP-IDF and ensure the environment is set up correctly.

### Get the esp-dev-kits Repository

Before compiling examples from the esp-dev-kits repository, please run the following command in the terminal to clone the repository locally:

```
git clone --recursive https://github.com/espressif/esp-dev-kits.git
```

### Configuration

Run ``idf.py menuconfig`` and modify the ``Board Support Package`` configuration:

```
menuconfig > Component config > Board Support Package
```

## How to Use the Example

### Apply Patch

* When the pixel clock is set to 80MHz, the default SPI clock source may temporarily fail to meet timing requirements. To address this, please follow these steps to apply the patch file `0004-fix-spi-default-clock-source.patch`:

1. Change to the ESP-IDF root directory and check out the specific version (the patch is designed for IDF release/v5.5 commit:98cd765953dfe0e7bb1c5df8367e1b54bd966cce):

```bash
cd ~/esp-idf
git checkout release/v5.5
git checkout 98cd765953dfe0e7bb1c5df8367e1b54bd966cce
```

2. Copy the patch file to the ESP-IDF root directory, for example:

```bash
cp 0004-fix-spi-default-clock-source.patch ~/esp-idf/
```

3. Change to the ESP-IDF root directory:

```bash
cd ~/esp-idf
```

4. Apply the patch using the following command:

```bash
git apply 0004-fix-spi-default-clock-source.patch
```

* If you experience significant lag during photo capture or video recording, you can try applying the patch file `0004-fix-sdmmc-aligned-write-buffer.patch` using the same method as above.

### Compile and Flash the Example

Compile the project and flash it to the development board, then run the monitor tool to view the serial port output (replace `PORT` with your development board's serial port name):

```c
idf.py -p PORT flash monitor
```

Press ``Ctrl-]`` to exit the serial monitor.

For complete steps on configuring and using ESP-IDF to compile projects, please refer to the [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html).

### Feature Description

| Diagram | Feature | Description |
|---------|---------|-------------|
| ![Camera](https://dl.espressif.com/AE/esp-dev-kits/esp32-p4-eye-capture.png) | Photo Capture | Users can take photos by pressing the encoder button, with support for digital zoom through rotary encoder or button operations. <br> **Note**: Photo capture requires an SD card. Photos are saved by default in the `esp32_p4_pic_save` folder on the SD card and can be previewed through the album feature. |
| ![interval_cam](https://dl.espressif.com/AE/esp-dev-kits/esp32_p4_eye_timed_capture.png) | Timed Capture | Users can set the capture interval using buttons and start timed capture mode by pressing the encoder button. Digital zoom is also supported via the rotary encoder. Press any button to stop the timed capture. <br> **Note**: Timed capture requires an SD card. Photos are saved by default in the `esp32_p4_pic_save` folder on the SD card and can be previewed through the album feature. |
| ![video_mode](https://dl.espressif.com/AE/esp-dev-kits/esp32_p4_eye_video_record.png) | Video Recording | Users can start video recording by pressing the encoder button, with support for digital zoom through rotary encoder or button operations. <br> **Note**: Video recording requires an SD card. Videos are saved in MP4 format in the `esp32_p4_mp4_save` folder on the SD card. Currently, MP4 video preview is not supported in the album feature. |
| ![ai_detect](https://dl.espressif.com/AE/esp-dev-kits/esp32_p4_eye_video_ai.png) | AI Detection | AI detection supports face detection and pedestrian detection functions, based on esp-dl inference framework. Users can switch between function modes using the up and down buttons. <br> **Note**: Photo capture is not available in this mode. |
| ![album](https://dl.espressif.com/AE/esp-dev-kits/esp32_p4_eye_album_preview.png) | Album | Users can preview photos taken in photo capture or timed capture modes, and navigate through them using buttons. Press the encoder button to delete the current photo. If YOLO object detection is enabled in settings, object detection will be automatically performed on photos when browsing the album. |
| ![usb_disk](https://dl.espressif.com/AE/esp-dev-kits/esp32_p4_eye_usb_access.png) | USB Mounting | Connect the device to a PC via the USB 2.0 Device interface to directly access files on the SD card. |
| ![settings](https://dl.espressif.com/AE/esp-dev-kits/esp32_p4_eye_settings.png) | Settings | Users can configure whether to enable triaxial accelerometer rotation, whether to enable YOLO object detection, image resolution, whether to enable flash, and adjust image saturation, contrast, brightness, and hue. |

### Example Output

- The complete log is shown below:

    ```c
    I (27) boot: ESP-IDF v5.5-dev-2511-g2696c6bced8-dirt 2nd stage bootloader
    I (28) boot: compile time Apr  7 2025 20:58:06
    I (28) boot: Multicore bootloader
    I (31) boot: chip revision: v1.0
    I (33) boot: efuse block revision: v0.2
    I (36) boot.esp32p4: SPI Speed      : 80MHz
    I (40) boot.esp32p4: SPI Mode       : DIO
    I (44) boot.esp32p4: SPI Flash Size : 16MB
    I (48) boot: Enabling RNG early entropy source...
    I (52) boot: Partition Table:
    I (55) boot: ## Label            Usage          Type ST Offset   Length
    I (61) boot:  0 nvs              WiFi data        01 02 00009000 00006000
    I (68) boot:  1 phy_init         RF data          01 01 0000f000 00001000
    I (74) boot:  2 factory          factory app      00 00 00010000 00300000
    I (81) boot: End of partition table
    I (84) esp_image: segment 0: paddr=00010020 vaddr=480d0020 size=138a74h (1280628) map
    I (271) esp_image: segment 1: paddr=00148a9c vaddr=30100000 size=00424h (  1060) load
    I (273) esp_image: segment 2: paddr=00148ec8 vaddr=4ff00000 size=07150h ( 29008) load
    I (280) esp_image: segment 3: paddr=00150020 vaddr=48000020 size=c0638h (788024) map
    I (393) esp_image: segment 4: paddr=00210660 vaddr=4ff07150 size=13908h ( 80136) load
    I (407) esp_image: segment 5: paddr=00223f70 vaddr=4ff1aa80 size=03cf8h ( 15608) load
    I (412) esp_image: segment 6: paddr=00227c70 vaddr=50108080 size=0001ch (    28) load
    I (419) boot: Loaded app from partition at offset 0x10000
    I (419) boot: Disabling RNG early entropy source...
    I (433) hex_psram: vendor id    : 0x0d (AP)
    I (434) hex_psram: Latency      : 0x01 (Fixed)
    I (434) hex_psram: DriveStr.    : 0x00 (25 Ohm)
    I (435) hex_psram: dev id       : 0x03 (generation 4)
    I (439) hex_psram: density      : 0x07 (256 Mbit)
    I (443) hex_psram: good-die     : 0x06 (Pass)
    I (448) hex_psram: SRF          : 0x02 (Slow Refresh)
    I (452) hex_psram: BurstType    : 0x00 ( Wrap)
    I (456) hex_psram: BurstLen     : 0x03 (2048 Byte)
    I (461) hex_psram: BitMode      : 0x01 (X16 Mode)
    I (465) hex_psram: Readlatency  : 0x04 (14 cycles@Fixed)
    I (470) hex_psram: DriveStrength: 0x00 (1/1)
    I (475) MSPI DQS: tuning success, best phase id is 2
    I (657) MSPI DQS: tuning success, best delayline id is 10
    I esp_psram: Found 32MB PSRAM device
    I esp_psram: Speed: 200MHz
    I (752) mmu_psram: .rodata xip on psram
    I (813) mmu_psram: .text xip on psram
    I (814) hex_psram: psram CS IO is dedicated
    I (814) cpu_start: Multicore app
    I (825) cpu_start: Pro cpu start user code
    I (825) cpu_start: cpu freq: 400000000 Hz
    I (825) app_init: Application information:
    I (825) app_init: Project name:     p4_eye_factory_demo
    I (830) app_init: App version:      0.0.0
    I (833) app_init: Compile time:     Apr  7 2025 20:58:02
    I (838) app_init: ELF file SHA256:  2968383e9...
    I (843) app_init: ESP-IDF:          v5.5-dev-2511-g2696c6bced8-dirt
    I (849) efuse_init: Min chip rev:     v0.1
    I (853) efuse_init: Max chip rev:     v1.99 
    I (857) efuse_init: Chip rev:         v1.0
    I (860) heap_init: Initializing. RAM available for dynamic allocation:
    I (867) heap_init: At 4FF22310 len 00018CB0 (99 KiB): RAM
    I (872) heap_init: At 4FF3AFC0 len 00004BF0 (18 KiB): RAM
    I (877) heap_init: At 4FF40000 len 00040000 (256 KiB): RAM
    I (882) heap_init: At 5010809C len 00007F64 (31 KiB): RTCRAM
    I (887) heap_init: At 30100424 len 00001BDC (6 KiB): TCM
    I (893) esp_psram: Adding pool of 30656K of PSRAM memory to heap allocator
    I (899) spi_flash: detected chip: generic
    I (903) spi_flash: flash io: dio
    W (906) bsp_p4_eye: Auto-initializing ESP32-P4-EYE board using constructor attribute
    I (914) gpio: GPIO[46]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (922) gpio: GPIO[26]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (930) bsp_p4_eye: ESP32-P4-EYE board initialized successfully
    I (936) main_task: Started on CPU0
    I (939) esp_psram: Reserving pool of 32K of internal memory for DMA/internal allocations
    I (946) main_task: Calling app_main()
    I (954) gpio: GPIO[23]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (958) main: Initialize the display
    I (962) LVGL: Starting LVGL task
    I (965) gpio: GPIO[15]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (1275) app_storage: Settings loaded from NVS successfully
    I (1275) app_video_photo: Photo resolution set to 1920x1080
    I (1275) app_video_record: Photo resolution set to 640x480
    I (1280) app_storage: Settings saved to NVS successfully
    I (1285) app_storage: Settings saved to NVS successfully
    I (1290) app_storage: Settings saved to NVS successfully
    I (1295) app_storage: Camera settings loaded from NVS successfully
    W (1301) ui_extra: loaded_contrast: 53, loaded_saturation: 63, loaded_brightness: 59, loaded_hue: 5
    I (1327) main: Initialize the storage
    I (1327) app_storage: other wake up
    I (1327) app_storage: Interval state loaded: active=0, next_wake=0
    I (1329) app_storage: Interval state saved: active=0, next_wake=0
    I (1335) app_storage: Photo count saved: 0
    I (1354) app_video_stream: Interval photo stopped
    I (1354) app_storage: Photo count saved: 0
    I (1354) gpio: GPIO[45]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (1360) app_storage: SD card mounted
    W (1364) ldo: The voltage value 0 is out of the recommended range [500, 2700]
    I (1371) gpio: GPIO[42]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (1395) p4-eye: Setting LCD backlight: 100%
    I (1396) main: Initialize the application control module
    I (1396) gpio: GPIO[2]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (1402) gpio: GPIO[3]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (1424) gpio: GPIO[4]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (1424) gpio: GPIO[5]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    I (1432) button: IoT Button Version: 3.5.0
    I (1436) gpio: GPIO[3]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (1454) button: IoT Button Version: 3.5.0
    I (1454) gpio: GPIO[4]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (1461) app_album: SD card total space: 29970.00 MB
    I (1466) app_album: Calculated MAX_IMAGES: 20000 (based on 29970.00 MB total, 14985.00 MB allocated for storage)
    I (1556) app_album: Found 849 images in /sdcard/esp32_p4_pic_save (sorted by filename in descending order)
    I (1565) button: IoT Button Version: 3.5.0
    I (1565) gpio: GPIO[5]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (1567) button: IoT Button Version: 3.5.0
    I (1592) gpio: GPIO[2]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (1593) gpio: GPIO[48]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (1620) gpio: GPIO[47]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
    I (1620) Knob: Iot Knob Config Succeed, encoder A:48, encoder B:47, direction:0, Version: 1.0.0
    I (1627) main: Initialize the I2C
    W (1650) i2c.master: Please check pull-up resistances whether be connected properly. Otherwise unexpected behavior would happen. For more detailed information, please read docs
    I (1680) main: Initialize the video streaming application
    I (1682) ov2710: Detected Camera sensor PID=0x2710
    I (1798) app_video: version: 0.9.0
    I (1799) app_video: driver:  MIPI-CSI
    I (1799) app_video: card:    MIPI-CSI
    I (1799) app_video: bus:     esp32p4:MIPI-CSI
    I (1801) app_video: width=1920 height=1080
    I (1804) app_album: Loaded image: /sdcard/esp32_p4_pic_save/pic_0849.jpg (485062 bytes)
    I (1814) app_storage: Camera settings loaded from NVS successfully
    I (1862) app_storage: Photo count loaded: 0
    I (1862) app_storage: Photo count saved: 0
    I (1862) p4-eye: bsp_microphone_init: 0
    W (1862) i2s_common: dma frame num is adjusted to 256 to align the dma buffer with 64, bufsize = 512
    I (1871) I2S_IF: channel mode 1 bits:16/16 channel:2 mask:3
    I (1876) Adev_Codec: Open codec device OK
    I (1880) app_storage: Album initialized
    I (1884) app_video: Video Stream Start
    I (1884) app_storage: Directory /sdcard/esp32_p4_pic_save already exists
    I (1896) main_task: Returned from app_main()
    I (1934) app_storage: Next picture number will be: 850
    I (1934) app_storage: Directory check completed
    I (1934) app_storage: USB MSC initialization
    W (1936) usb_phy: Using UTMI PHY instead of requested internal PHY
    W (1957) tusb_desc: No Device descriptor provided, using default.
    W (1957) tusb_desc: No FullSpeed configuration descriptor provided, using default.
    W (1963) tusb_desc: No HighSpeed configuration descriptor provided, using default.
    W (1970) tusb_desc: No Qulifier descriptor provided, using default.
    W (1976) tusb_desc: No String descriptors provided, using default.
    I (1997) tusb_desc: 
    ┌─────────────────────────────────┐
    │  USB Device Descriptor Summary  │
    ├───────────────────┬─────────────┤
    │bDeviceClass       │ 0           │
    ├───────────────────┼─────────────┤
    │bDeviceSubClass    │ 0           │
    ├───────────────────┼─────────────┤
    │bDeviceProtocol    │ 0           │
    ├───────────────────┼─────────────┤
    │bMaxPacketSize0    │ 64          │
    ├───────────────────┼─────────────┤
    │idVendor           │ 0x303a      │
    ├───────────────────┼─────────────┤
    │idProduct          │ 0x4002      │
    ├───────────────────┼─────────────┤
    │bcdDevice          │ 0x100       │
    ├───────────────────┼─────────────┤
    │iManufacturer      │ 0x1         │
    ├───────────────────┼─────────────┤
    │iProduct           │ 0x2         │
    ├───────────────────┼─────────────┤
    │iSerialNumber      │ 0x3         │
    ├───────────────────┼─────────────┤
    │bNumConfigurations │ 0x1         │
    └───────────────────┴─────────────┘
    I (2197) TinyUSB: TinyUSB Driver installed
    I (2197) app_storage: USB MSC initialization DONE
    ...
    ```

## Technical Support and Feedback

Please provide feedback through the following channels:

- For technical questions, please visit the [esp32.com](https://esp32.com/viewforum.php?f=22) forum.
- For feature requests or bug reports, please create a new [GitHub issue](https://github.com/espressif/esp-dev-kits/issues).
- For ESP-LaunchPad related questions, please visit [ESP-LaunchPad](https://espressif.github.io/esp-launchpad/).

We will respond as soon as possible.
