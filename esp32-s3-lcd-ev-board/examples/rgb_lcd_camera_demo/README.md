# LVGL Demos Example

An example used to show video which captured be USB camera.

## How to use example

### Hardware Required

* An ESP32-S3-LCD-EV-Board development board with 800x480 LCD
* A CDC USB camera
* An USB Type-C cable for Power supply and programming

### Software Required

* IDF with Octal PSRAM 120M patch (See [here](../factory/README.md#idf-patch))

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project. **Note that it must be powered with UART port instead of USB port.**

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## IDF Patch

The patch is intended to achieve best perfermance of RGB LCD by using PSRAM 120MHz feature. Therefore, the examples can work normally under the default LCD PCLK of 18 MHz.
The patch can be applied in IDF master **(commit id:f3159)** by following commands:
```
cd <root directory of IDF>
git checkout --recurse-submodules master
git checkout --recurse-submodules f3159
git apply <path of the patch>/idf_psram_120m.patch
git status      # Check whether the operation is successful, the output should look like below:

HEAD detached at f315986401
Changes not staged for commit:
  (use "git add <file>..." to update what will be committed)
  (use "git restore <file>..." to discard changes in working directory)
        modified:   components/esp_hw_support/port/esp32s3/rtc_clk.c
        modified:   components/esp_psram/esp32s3/Kconfig.spiram
        modified:   components/hal/esp32s3/include/hal/spimem_flash_ll.h
        modified:   components/spi_flash/esp32s3/mspi_timing_tuning_configs.h
        modified:   components/spi_flash/esp32s3/spi_timing_config.c
        modified:   components/spi_flash/esp32s3/spi_timing_config.h
        modified:   components/spi_flash/spi_flash_timing_tuning.c

Untracked files:
  (use "git add <file>..." to include in what will be committed)
        tools/test_apps/system/flash_psram/sdkconfig.ci.f8r8_120ddr
        tools/test_apps/system/flash_psram/sdkconfig.ci.f8r8_120ddr_120ddr
```

## Troubleshooting

* LCD screen drift
  * Slow down the PCLK frequency
  * Adjust other timing parameters like PCLK clock edge (by `pclk_active_neg`), sync porches like VBP (by `vsync_back_porch`) according to your LCD spec
  * Enable `CONFIG_SPIRAM_FETCH_INSTRUCTIONS` and `CONFIG_SPIRAM_RODATA`, which can saves some bandwidth of SPI0 from being consumed by ICache.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-idf/issues)

We will get back to you as soon as possible.
