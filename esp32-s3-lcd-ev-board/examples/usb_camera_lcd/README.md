# RGB LCD + USB Camera Demo

An example used to show video which captured be USB camera.

## How to use example

### Hardware Required

* An ESP32-S3-LCD-EV-Board development board with 800x480 LCD
* A CDC USB camera (Can be connected to USB port or Exanpsion Connector)
* An USB Type-C cable for Power supply and programming (Please connect to UART port instead of USB port)

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project. **Note that it must be powered with UART port instead of USB port.**

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Troubleshooting

* Program upload failure
    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.
    * Error message with `A fatal error occurred: Could not open /dev/ttyACM0, the port doesn't exist`: Please first make sure development board connected, then make board into "Download Boot" mode to upload by following steps:
        1. keep press "BOOT(SW2)" button
        2. short press "RST(SW1)" button
        3. release "BOOT(SW2)".
        4. upload program and reset

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-dev-kits/issues)

We will get back to you as soon as possible.
