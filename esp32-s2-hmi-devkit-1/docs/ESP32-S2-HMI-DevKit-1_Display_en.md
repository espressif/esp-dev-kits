# Display and touch panel

The ESP32-S2-HMI-DevKit-1 is equipped with a 4.3-inch color LCD display with a resolution of 800×480 and a touch panel with an I2C interface. The interface type and data bit width of the screen are controlled by programmable pins. The board has been communicated by means of a 16-bit 8080 side-port communication with a resistance configuration.

The LCD uses a display IC of RM68120 and a touch IC of FT5436.

## Communication

The display IC of the LCD used in ESP32-S2-HMI-DevKit-1 has been configured to communicate using a 16-bit 8080 interface using a total of 18 GPIO interfaces, including 16-bit data cables (LCD_D0... LCD_D15), bit clock signals (LCD_WR) and data commands distinguish signals (LCD_DC/LCD_RS).

Touch ICs use the I2C interface to communicate with the MCU and can be shared with other I2Cs and ICs without the need for additional GPIO interfaces. Touch IC supports the output signal after one in. First, the medium zone will be sent to the IO's P2 pin on the 20th, and the falling edge of the pin will cause the output foot of the IO extender to produce a low level, thus allowing the MCU to receive further. At this point, the source can be read by reading the input level register of the IO extender, so that the incision is from the touch IC.

## Backlight

The LCD has a built-in information LED that requires constant current drive using the Booster circuit. The rated current is 18 mA, at which point the current voltage is 24 V.

Because PWM mentring can cause flickering, and some Bosser ICs do not support high-frequency PWM signal control, the board uses a DC modulate circuit to address these issues. The DC switch power supply inputs the VFB voltage into the op amp, where the gain resistance is a digital proton, so its resistance can be modified by the I2C bus for the purpose of modifying the gain size.

The digital meter uses CAT5171 with a resolution of 256 and a maximum resistance of 50k.

At the same time, Booster IC's EN foot is controlled by the IO foot's extended P7 foot and is effective at high levels. To turn off the screen while preserving the video content, you can set the pin to low to turn off screen backlighting.

## Touch

The board's capacitive touch panel uses a touch IC resolution of 800×480 and supports up to 5 touch and hardware gesture recognition.

The display IC does not support hardware coordinate orientation conversion, so for displays that use a different rotation direction, it may be necessary for the software to exchange the data read by the touch IC or to make its resolution its own difference. Multi-touch is hardware support, and in the driver, we also provide APIs for reading multiple touch points for you to use, but the GUI library LVGL is not currently supporting multi-touch processing, and you may need to process the data for those touch points yourself at the app layer.
