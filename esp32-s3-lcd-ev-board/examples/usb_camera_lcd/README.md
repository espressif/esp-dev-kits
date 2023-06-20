# USB Cemera LCD Example

An example for using LCD display video which captured from USB camera.

* Transfer uvc frame to wifi http if `ENABLE_UVC_WIFI_XFER` is set to `1`, the real-time image can be fetched through Wi-Fi softAP (ssid: ESP32S3-UVC, http: 192.168.4.1). Due to this function will cause screen drift when PSRAM is 80M, it's only aviable with 120M PSRAM.
* Print log about SRAM and PSRAM memory if `LOG_MEM_INFO` is set to `1`, includes `Biggest/Free/Total` three types.

## How to use example

Please first read the [User Guide](https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html#esp32-s3-lcd-ev-board) of the ESP32-S3-LCD-EV-Board to learn about its software and hardware information.

### Hardware Required

* An ESP32-S3-LCD-EV-Board development board with subboard3 (800x480) or subboard2 (480x480)
* An USB camera that can output 800x480 or 480x320 resolution image
* An USB Type-C cable for Power supply and programming (Please connect to UART port instead of USB port)

### Configurations

Run `idf.py menuconfig` and go to `Board Support Package`:
* `BSP_LCD_SUB_BOARD`: Choose a LCD subboard according to hardware. Default use subboard3 (800x480).
* More configurations see BSP's [README](https://github.com/espressif/esp-bsp/tree/master/esp32_s3_lcd_ev_board#bsp-esp32-s3-lcd-ev-board).

### Hardware Connection

| USB_DP | USB_DM |
| ------ | ------ |
| GPIO20 | GPIO19 |

**Note:** If the camera is connected to the USB port, diode D1 needs to be short-circuited. About this, please check [Power Supply over USB](https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html#power-supply-over-usb) part in board's schematic.

### Build and Flash

1. The project configure PSRAM with 80M Octal by default. **For best performance**, please configure PSRAM with 120M DDR(Octal) by the following commands. see [here](../../README.md#psram-120m-ddr) for more details.
    ```
    rm -rf build sdkconfig sdkconfig.old
    idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.psram_octal_120m" reconfigure
    ```
2. Run `idf.py -p PORT flash monitor` to build, flash and monitor the project. **Note that it must be connected with UART port instead of USB port.**

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Troubleshooting

* Program build failure
    * Error message with `error: static assertion failed: "FLASH and PSRAM Mode configuration are not supported"`: Please modify the combination configuration of Flash (in `Serial flasher config`) and PSRAM (in `SPI RAM config`) like below.
        |   Flash   |    PSRAM    |
        | :-------: | :---------: |
        | QIO, 80M  | Octal, 80M  |
        | QIO, 120M | Octal, 120M |
    * Error message with `error: 'esp_lcd_rgb_panel_config_t' has no member named 'num_fbs'`: Please update the branch (release/v5.0 or master) of ESP-IDF.
* Program upload failure
    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.
    * Error message with `A fatal error occurred: Could not open /dev/ttyACM0, the port doesn't exist`: Please first make sure the development board connected, then make board into "Download Boot" by following steps:
        1. keep press "BOOT(SW2)" button
        2. short press "RST(SW1)" button
        3. release "BOOT(SW2)".
        4. upload program and reset

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-dev-kits/issues)

We will get back to you as soon as possible.
