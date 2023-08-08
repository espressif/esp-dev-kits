# USB File System

This example demonstrates how to use USB HOST on ESP32-S2/S3 to read and write files on a USB flash drive. You can use the development board `ESP32-S3-LCD-EV-Board-2` to connect to a USB flash drive and display its contents on the screen.

Currently supported file formats for preview are:
* JPG
* PNG
* BMP
* GIF
* MP3 (requires an external speaker)

## How to Use the Example

Please first read the [User Guide](https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html#esp32-s3-lcd-ev-board) of the ESP32-S3-LCD-EV-Board to learn about its software and hardware information.

* idf_version: >= release/v5.1

### Hardware Requirements

* An `ESP32-S3-LCD-EV-Board` development board with sub board3 (800x480).
* A USB flash drive with a capacity less than 32GB.
* A USB Type-C cable for power supply and programming (please connect to the UART port instead of the USB port).

### Configurations

Run `idf.py menuconfig` and go to `Board Support Package`:
* `BSP_LCD_SUB_BOARD`: Choose an LCD subboard according to hardware. Default use sub board3 (800x480).
* More configurations see the BSP's [README](https://github.com/espressif/esp-bsp/tree/master/esp32_s3_lcd_ev_board#bsp-esp32-s3-lcd-ev-board).

### Hardware Connection

| USB_DP | USB_DM |
| ------ | ------ |
| GPIO20 | GPIO19 |

**Note:** If the msc is connected to the USB port, diode D1 needs to be short-circuited. Please check the [Power Supply over USB](https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32s3/esp32-s3-lcd-ev-board/user_guide.html#power-supply-over-usb) section in the board's schematic for more details.

### Build and Flash

1. Run `idf.py -p PORT flash monitor` to build, flash, and monitor the project. **Note that it must be connected to the UART port instead of the USB port.**

(To exit the serial monitor, type `Ctrl-]`.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

The following animation is a demonstration of the development board running the example.

![usb_msc_file_sys](https://dl.espressif.com/AE/esp-dev-kits/s3-lcd-ev-board_examples_usb_msc_file_sys_5.gif)

## Troubleshooting

* Program build failure
    * Error message with `error: static assertion failed: "FLASH and PSRAM Mode configuration are not supported"`: Please modify the combination configuration of Flash (in `Serial flasher config`) and PSRAM (in `SPI RAM config`) like below.
        |   Flash   |    PSRAM    |  IDF Version   |
        | :-------: | :---------: | -------------- |
        | QIO, 120M | Octal, 120M | >=release/v5,1 |
    * Error message with `error: 'esp_lcd_rgb_panel_config_t' has no member named 'num_fbs'`: Please update the branch (release/v5.0 or master) of ESP-IDF.
* Program upload failure
    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.
    * Error message with `A fatal error occurred: Could not open /dev/ttyACM0, the port doesn't exist`: Please first make sure the development board is connected, then make the board into "Download Boot" by following steps:
        1. Keep pressing the "BOOT(SW2)" button.
        2. Short press the "RST(SW1)" button.
        3. Release the "BOOT(SW2)" button.
        4. Upload the program and reset.

## Technical Support and Feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum.
* For feature requests or bug reports, create a [GitHub issue](https://github.com/espressif/esp-dev-kits/issues).

We will get back to you as soon as possible.