# Sensor Evaluation Example

An example to show how to use sensors on ESP32-S2-HMI-DevKit-1
## How to use example

### Hardware Required

* An ESP32-S2-HMI-DevKit-1 development board
* A USB-C cable for Power supply and programming

### Configure the Project

Open the project configuration menu (`idf.py menuconfig`). 

In the `HMI Board Config` menu:

* Set the board type to `ESP32-S2-HMI-DevKit-V2` in `HMI board`.
* Set LCD display IC to `RM68120` in `LCD Drivers`.

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

Run the example, you will see sensor evaluation interface.

You can select the sensor at the left of screen and see the value on the right.

## Troubleshooting

* Program upload failure

    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-idf/issues)

We will get back to you as soon as possible.
