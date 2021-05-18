# Power Example

Config power of ESP32-S2-HMI-DevKit-1.

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

Run the example, you will see LEDs shows input level of IO expander, switches to control output level of IO expander. Also, a "Sleep" button. By clicking it, the dev board will shutdown progammable power area and into deep sleep. You can wake it up by pressing wake up button (connected to IO0).

## Troubleshooting

For any technical queries, please open an [issue] (https://github.com/espressif/esp-dev-kits/issues) on GitHub. We will get back to you soon.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-idf/issues)

We will get back to you as soon as possible.
