# Factory Bin

This is the first version of bin for factory firmware which is programmed by [86box_demo](../86box_demo/).

# IDF Patch

The patch is intended to achieve best perfermance of RGB LCD by using PSRAM 120MHz feature. Therefore, the examples can work normally under the default LCD PCLK of 18 MHz.
The patch can be applied in IDF master(commit id:f3159) by following command:
```
git apply idf_psram_120m.patch
```