# 86box_demo Example

A GUI demo designed for smart control panel which is usually used fo rwall-mounted 86 type box. **Note that this example is only applicable to 480 x 480 LCD.**

## How to use example

### Hardware Required

* An ESP32-S3-LCD-EV-Board development board
* An USB Type-C cable for Power supply and programming

### Configure

Run `idf.py menuconfig` and go to `Board Configuration`:

1. Based on hardware to `Select LCD Sub Board` and related configuration.
2. Set `Frequency of lcd pclk`
3. Choose whether to `Avoid tearing effect`
4. Chosse to `Select lvgl mode for avoiding tearing` (available only when step `3` was chosen to true)
5. Set `Priority of lcd refresh task` (available only when step `3` was chosen to true)
6. Set `Screen refresh period(ms)` (available only when step `3` was chosen to true)

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

Run the example, you can achieve a smooth graphic operation experience, for example horizontally swiping between weather report, hot water heater and warm air heater page. Trigger the control centre by swiping down from the bottom of the screen. Open music player to test the animation performance of the board.

## Troubleshooting

* Program upload failure
    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.
    * Error message with `A fatal error occurred: Could not open /dev/ttyACM0, the port doesn't exist`: Please first make sure development board connected, then make board into "Download Boot" mode to upload by following steps:
        1. keep press "BOOT(SW2)" button
        2. short press "RST(SW1)" button
        3. release "BOOT(SW2)".
        4. upload program and reset
* LCD screen drift
  * Slow down the PCLK frequency
  * Adjust other timing parameters like PCLK clock edge (by `pclk_active_neg`), sync porches like VBP (by `vsync_back_porch`) according to your LCD spec
  * Enable `CONFIG_SPIRAM_FETCH_INSTRUCTIONS` and `CONFIG_SPIRAM_RODATA`, which can saves some bandwidth of SPI0 from being consumed by ICache.
* LCD screen tear effect
    * Using avoid tearing effect.
* LCD screen flickering after enable avoid tearing effect
    * Set lower screen refresh period.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-idf/issues)

We will get back to you as soon as possible.
