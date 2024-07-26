| Supported Targets | ESP32-P4 |
| ----------------- | -------- |

# ESP-UI Demo

This example, based on ESP_UI, demonstrates an Android-like interface that includes many different applications.

[ESP-UI](https://github.com/espressif/esp-ui) is a UI runtime framework based on LVGL, designed to provide a consistent UI development experience for screens of various sizes and shapes.

## Getting Started

### Prerequisites

* An ESP32-P4-Function-EV-Board.
* A 7-inch 1024 x 600 LCD screen powered by the [EK79007](../../docs/_static/esp32-p4-function-ev-board/camera_display_datasheet/display_driver_chip_EK79007AD_datasheet.pdf) IC, accompanied by a 32-pin FPC connection [adapter board](../../docs/_static/esp32-p4-function-ev-board/schematics/esp32-p4-function-ev-board-lcd-subboard-schematics.pdf) ([LCD Specifications](../../docs/_static/esp32-p4-function-ev-board/camera_display_datasheet/display_datasheet.pdf)).
* A USB-C cable for power supply and programming.
* Please refer to the following steps for the connection:
    * **Step 1**. According to the table below, connect the pins on the back of the screen adapter board to the corresponding pins on the development board.

        | Screen Adapter Board | ESP32-P4-Function-EV-Board |
        | -------------------- | -------------------------- |
        | 5V (any one)         | 5V (any one)               |
        | GND (any one)        | GND (any one)              |
        | PWM                  | GPIO26                     |
        | LCD_RST              | GPIO27                     |

    * **Step 2**. Connect the FPC of LCD through the `MIPI_DSI` interface.
    * **Step 3**. Use a USB-C cable to connect the `USB-UART` port to a PC (Used for power supply and viewing serial output).
    * **Step 4**. Turn on the power switch of the board.

### ESP-IDF Required

* This example supports IDF release/v5.3 and later branches. By default, it runs on IDF release/v5.3.
* Please follow the ESP-IDF Programming Guide to set up the development environment. We highly recommend you Build Your First Project to get familiar with ESP-IDF and make sure the environment is set up correctly.

### Configuration

Run `idf.py menuconfig` and go to `Board Support Package`.

## How to Use the Example

### Build and Flash the Example

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.