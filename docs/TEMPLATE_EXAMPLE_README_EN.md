_Note that this is a template for an esp-dev-kits example README.md file. When using this template, replace all these emphasized placeholder texts with example-specific content._

# _Example Title_

_Link to Chinese Version_

[中文版本](./README_CN.md)

_Introduce the functionality demonstrated by the example. The introduction should preferably answer the following questions:_
- _What is this example? What does it do?_
- _What features of the board (e.g., USB HOST, sensors, smart panel) does it use?_
- _What could someone create based on this example? i.e., applications/use cases/etc._


## Getting Started


### Prerequisites

_Provide the name of the specific board(s) this example is intended to run on._

_If any other items are needed, such as server, Bluetooth device, app, second chip, list them here. Include links if applicable. Explain how to set them up._


### ESP-IDF Required

_Specify the ESP-IDF branch recommended or required by the example. Take the following statement as a reference._

- This example supports ESP-IDF release/v[x.y] and later branches. By default, it runs on ESP-IDF release/v[x.y].
- Please follow the [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) to set up the development environment. **We highly recommend** you [Build Your First Project](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#build-your-first-project) to get familiar with ESP-IDF and make sure the environment is set up correctly.

### Get the esp-dev-kits Repository

_Describe the way to clone the repository._

To start from the examples in esp-dev-kits, clone the repository to the local PC by running the following commands in the terminal:

```
git clone --recursive https://github.com/espressif/esp-dev-kits.git
```


### Configuration

- _Describe important items to configure in menuconfig, such as long file name support for FatFs, selection of compatible audio board, chip type, PSRAM clock, and Wi-Fi/LwIP parameters. Below is an example._

    Run ``idf.py menuconfig`` and modify the QWeather and Wi-Fi configuration:

    ```
    menuconfig > QWeather > QWEATHER_REQUEST_KEY
    ```

- _Describe the configuration of other software if required, such as specifying a patch._


## How to Use the Example


### Build and Flash the Example

_Command to build and flash the example. Below is an example._

Build the project and flash it to the board, then run monitor tool to view serial output (replace `PORT` with your board's serial port name):

```c
idf.py -p PORT flash monitor
```

To exit the serial monitor, type ``Ctrl-]``.

See the [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.


### Example Output

_Add the expected output effect of the example. Below are two references._

- The complete log is as follows:

    ```c
    I (64) boot: Chip Revision: 3
    I (35) boot: ESP-IDF v3.3.1-203-g0c1859a5a 2nd stage bootloader
    I (35) boot: compile time 21:43:15
    I (35) boot: Enabling RNG early entropy source...
    I (41) qio_mode: Enabling default flash chip QIO
    I (46) boot: SPI Speed      : 80MHz
    I (50) boot: SPI Mode       : QIO
    I (54) boot: SPI Flash Size : 8MB
    ```
- The following animations show the example running on different development boards.


## Troubleshooting

_This section is optional. If there are any problems or errors which many users might encounter, mention them here. Remove this section for very simple examples where nothing is likely to go wrong._


## Technical Support and Feedback

Please use the following feedback channels:

- For technical queries, go to the [esp32.com](https://esp32.com/viewforum.php?f=22) forum.
- For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-dev-kits/issues).

We will get back to you as soon as possible.
