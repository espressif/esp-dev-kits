# Factory Bin

## For Subboard2 (480x480)

Programmed by [86box_demo](../86box_demo/).

* [subboard2_480_480_v1_0.bin](./bin/subboard2_480_480_v1_0.bin)

## For Subboard3 (800x480)

Programmed by [smart_panel](../smart_panel/).

* [subboard3_800_480_v1_0.bin](./bin/subboard3_800_480_v1_0.bin)

# IDF Patch

The [patch](./patch/release5.0_psram_octal_120m.patch) is intended to achieve best perfermance of RGB LCD by using **PSRAM Octal 120MHz** feature. The patch is only used for the branch **release/v5.0** of ESP-IDF.

**Note: The feature of PSRAM Octal 120MHz has temperature risk and cannot guarantee it works normally above 65 degrees celsius.**

Please make sure your IDF project is clean (use `git status` to check), then the patch can be applied by following commands:

```
cd <root directory of IDF>
git apply <path of the patch>/release5.0_psram_octal_120m.patch # Nothing return if success
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