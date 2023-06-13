# Factory Bin

## For ESP32-S3-LCD-EV-Board (Subboard2 480x480)

Programmed by [86box_smart_panel](../examples/86box_smart_panel/).

* [ESP32-S3-LCD-EV-Board_fac_v0_2_0.bin](./bin/ESP32-S3-LCD-EV-Board_fac_v0_2_0.bin)

## For ESP32-S3-LCD-EV-Board-2 (Subboard3 800x480)

Programmed by [smart_panel](../examples/smart_panel/).

* [ESP32-S3-LCD-EV-Board-2_fac_v0_5_0.bin](./bin/ESP32-S3-LCD-EV-Board-2_fac_v0_5_0.bin)

# IDF Patch

The [patch](./patch/release5.0_psram_octal_120m.patch) is intended to achieve the best performance of RGB LCD by using the **PSRAM Octal 120 MHz** feature. The patch is only used for the **release/v5.0** branch of ESP-IDF. For the **master** branch, the PSRAM Octal 120 MHz feature can be directly used by enabling the `IDF_EXPERIMENTAL_FEATURES` option.

**Note: The PSRAM Octal 120 MHz feature is an experimental feature and it has temperature risks as below.**
  * Cannot guarantee normal functioning with a temperature higher than 65 degrees Celsius.
  * Temperature changes can also cause the crash of accessing to PSRAM/Flash, see [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/flash_psram_config.html#all-supported-modes-and-speeds) for more details.

## How to use patch

Please make sure your IDF project is clean (use `git status` to check), then the patch can be applied by following commands:

```
cd <root directory of IDF>
git apply --whitespace=fix <path of the patch>/release5.0_psram_octal_120m.patch # Nothing return if success
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

These uncommitted modifications can be cleared by the following commands and **the other uncommitted modifications will be cleared too**.

```
git reset --hard
git clean -xdf
```
