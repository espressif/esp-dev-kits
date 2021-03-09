# LED Blink Example

Blink WS2812 LED and show the color on screen.

[WS2812](http://www.world-semi.com/Certifications/WS2812B.html) is a digital RGB LED which integrates a driver circuit and a single control wire. The protocol data format defined in WS2812 is compatible to that in RMT peripheral. This example will illustrate how to drive an WS2812 LED strip based on the RMT driver.

## How to Use Example

### Hardware Required

* An ESP32-S2-HMI-DevKit-1 development board
* A USB-C cable for Power supply and programming
* A WS2812 LED(Already on development board)

### Configure the Project

Open the project configuration menu (`idf.py menuconfig`). 

In the `Example Connection Configuration` menu:

* Set the GPIO number used for transmitting the IR signal under `RMT TX GPIO` optin.
* Set the number of LEDs in a strip under `Number of LEDS in a strip` option.

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

Run the example, you will see LED blink with different color. 

## Troubleshooting

For any technical queries, please open an [issue] (https://github.com/espressif/esp-dev-kits/issues) on GitHub. We will get back to you soon.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-idf/issues)

We will get back to you as soon as possible.