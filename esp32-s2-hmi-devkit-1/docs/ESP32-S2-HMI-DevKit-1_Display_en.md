
[[中文]](./ESP32-S2-HMI-DevKit-1_Display_cn.md)

# Display and Touch Panel

The ESP32-S2-HMI-DevKit-1 has a 4.3-inch color TFT-LCD with 800×480 resolution, and a touch panel with an I2C interface. The interface type and data bit width of this display is controlled by programmable pins. This development board has been configured as 16-bit 8080 parallel communication via resistors.

This LCD uses RM68120 as its display IC and FT5436 as its touch IC.

## Communication

The display IC of the LCD used in ESP32-S2-HMI-DevKit-1 has been configured as 16-bit 8080 parallel communication, with totalled 18 GPIOs used, i.e., 16 data lines (LCD_D0...LCD_D15), a bit clock signal (LCD_WR) and a data/command distinguish signal (LCD_DC/LCD_RS).

The touch IC uses I2C interface to communicate with MCU and can share this interface with other I2C ICs, and thus does not need to occupy additional GPIOs interfaces. The touch IC supports interrupt signal output. The interrupt signal is first sent to the P2 pin of the IO expander, and the falling edge from this pin will generate a low level in the interrupt output pin of the IO expander, so that the MCU receives this interrupt signal. In this case, you can read the input level register of the IO expander to check whether this interrupt is from the touch IC. Once a read operation completed, the interrupt flag will be cleared.

## Backlight

This LCD has a built-in series LED, which needs to be drived by constant current using the Booster circuit. The rated current is 18 mA and the voltage is approximately 24 V (may not be accurate, only for reference). To prevent the feedback voltage of the Booster circuit always being 0 when the display is not connected, and thus causing high voltage loaded to both ends of the backlight filter capacitor, please make sure this capacitor has a withstand voltage of 38 V or more.

As PWM dimming may has flicker issue and some Booster IC does not support high-frequency PWM signal control, this development board uses DC dimming circuit to reach high performance. This DC dimming circuit inputs the VFB voltage to the operational amplifier, whose gain resistor is a digital potentiometer which can be modified via the I2C bus to change its gain level. This digital potentiometer is CAT5171, with 256 levels of resolution and a maximum resistance value of 50 k.

The EN pin of the Booster IC is controlled by the P7 pin of the IO expander in high level. If you want to keep the contents while turning off the display, please set this pin to low level so as to disable backlight.

## Touch

The capacitive touch panel on the development board uses a touch IC with a resolution of 800×480 and supports up to 5-point touch and hardware gesture recognition.

This display IC does not support conversion of the hardware coordinate directions. Therefore, for scenarios with such requirements, you may need to swap the data read by the touch IC via software or calculate its different value to the resolution. Multi-touch is supported by hardware. Therefore, we also provide some APIs for reading multiple touch points for you to use this function. However, since the LVGL used in the GUI library does not support multi-touch processing for now, you may need to handle the data of these touch points in the application layer yourself.
