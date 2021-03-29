# Audio Record Example

This example shows how to acquire audio with ADC form the output of analog MIC.

## How to use example

After flashing the project to ESP32-S2-HMI-DevKit, a button and slider will show on screen.

By pressing the button, recording will start until you release the button or buffer is full. The buffer size is configured to 3s of sample rate.

When button is released, it will atomically play the audio you recorded.

By change the slider, you can change the audio output volume.
