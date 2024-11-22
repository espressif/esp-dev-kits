
# ESP32-S3-USB-OTG All-in-One factory demo

Please use ESP-IDF [release/v4.4](https://github.com/espressif/esp-idf/tree/release/v4.4) branch

## Hardware Overview

* [ESP32-S3-USB-OTG online docs](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-usb-otg/index.html)

## How to use example

The example can be directly flashed to the board by [ESP Launchpad](https://espressif.github.io/esp-launchpad/?flashConfigURL=https://dl.espressif.com/AE/esp-dev-kits/config.toml).

<a href="https://espressif.github.io/esp-launchpad/?flashConfigURL=https://dl.espressif.com/AE/esp-dev-kits/config.toml&app=factory">
    <img alt="Try it with ESP Launchpad" src="https://espressif.github.io/esp-launchpad/assets/try_with_launchpad.png" width="200" height="56">
</a>

### Hardware Required

* An ESP32-S3-USB-OTG development board
* A USB cable for Power supply and programming

### Configure the Project

A default config is given as `sdkconfig.defaults`. Simply run `idf.py flash monitor` to run example.

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

```
W (447) app_main: ------------------Chip information------------
I (454) app_main: This is esp32s3 chip with 2 CPU core(s), WiFi/BLE,
I (461) app_main: silicon revision 0,
I (466) app_main: 8MB external flash
I (470) app_main: Minimum free heap size: 306020 bytes
W (476) app_main: -----------------MAC information-----------------
I (483) system_api: Base MAC address is not set
I (488) system_api: read default base MAC address from EFUSE
I (494) SoftAP MAC:: 84:f7:03:c0:14:f9
W (499) app_main: ---------------Application information--------------
I (506) app_main: Project name:     usb_otg_dev
I (511) app_main: App version:      0.1.0.0
...

```

## Troubleshooting

* Program upload failure

    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-dev-kits/issues)

We will get back to you as soon as possible.