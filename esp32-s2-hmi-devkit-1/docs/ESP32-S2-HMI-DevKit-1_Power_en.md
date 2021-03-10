# Power

For low power consumption, high power efficiency, and battery power, the ESP32-S2-HMI-DevKit-1 is designed to be divided into 5 V power domains and 3.3 V power domains, some of which can be controlled from the power supply and the other part of which is configured to be always on when the hardware is designed.

The firmware burned at the factory on the board has been powered off for all controlled power domains and all ICs have been configured in low-power mode to reduce current consumption.
## The power domain of 3.3 V

The power domain is responsible for the majority of ICs and counting power supplies, but it is divided into two parts: an uncontrollable 3.3 V power domain and a controllable 3.3V power domain.

The uncontrollable 3.3 V power domain cannot be switched off by the software and is obtained by the Buck circuit. In the case of a USB power input, a 5 V power supply is entered from the USB cable and, in the case of a USB disconnect, a 3.6 to 4.2 V power supply is provided to the Buck IC by a lithium battery. This power domain is responsible for powering the ESP32-S2-WROVER module, as well as other devices that can be controlled by software into low-power mode.

The controllable 3.3 V power domain comes from the uncontrollable 3.3V power domain and is connected to the IO extender's P4 foot by the PMOS control switch, which is switched on at low level. This power supply is responsible for powering ICs with large static power consumption that do not enter low-power mode.
## 5 V power domain

The board's 5V power domain is responsible for powering the audio amplifier TWAI® and receiver. It comes from several sources:

- USB connector
- Externally input to the connector's 5V power port
- The lithium battery passes through the power supply behind the Booster circuit

The power supply via USB and external 5V inputs supplies power to all devices that require 5 V power (except CP2102N) and cannot be disconnected through the software. When powered by a battery, the BOSS IC's EN foot level can be controlled by extending the IO's P5 pin and 5 V is switched on at a high level.

The power supply entered through the USB connector at the bottom of the board will be divided into two paths, supplying power to the CP2102N along the way and becoming a power source after passing USB_5V. Since the CP2102N only needs to be in operation when connecting to the PC, the CP2102N will only be powered on when the USB port is connected. Any 5V power input shuts down the Booster IC and charges the on-board lithium-ion battery via the charging IC.
## Power dependency

The following features depend on the 5 V power domain:

- TWAI®
- Audio amplifier

The following features rely on a controlled 3.3 V power domain:

- Micro-SD card
- And the line
- Display and touch function
- WS2812 RGB LED
- Infrared diodes

## Power mode

The power mode depends on whether the USB connector is powered and the P4, P5 pins extended by the IO interface.

The P4 is a controllable 3.3 V power control pin that is switched on at a high level.

The P5 is a 5 V power control pin that is turned on at a high level and can be turned off when and only when battery time is used.
