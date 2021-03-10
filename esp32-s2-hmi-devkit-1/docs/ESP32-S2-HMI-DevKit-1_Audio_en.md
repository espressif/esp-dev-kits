# Audio

ESP32-S2-HMI-DevKit-1 supports audio playback and capture. You can find examples of audio playback and acquisition in the `examples/audio/`

## Audio playback

ESP32-S2-HMI-DevKit-1 can be output by DAC or PWM. We recommend using PWM for audio output because the PWM output has lower noise and higher resolution (DAC output resolution is 8 bits, and PWM output can reach a resolution of up to 12 bits at a sample rate of 19.2kHz).

Signals output through the IO port will first enter the digital meter for non-destructive adjustment, and then pass a 100 nF straight capacitor with a 200k resistance, with a cutoff frequency around 8 Hz. In addition, the signal is sent to a Class D audio power amplifier with a power of 3W, which sets the gain to 1.5 times through an external resistor, thereby amplifying the 3.3V output signal to 4.95 V slightly less than the PA power supply (5 V), reducing saturation distortion while minimizing the incoming sound to the sound output.

The audio amplifier is powered by a 5V power domain. Before you use the playback feature, make sure that the power domain is turned on.

## Audio acquisition

Esp32-S2-HMI-DevKit-1 performs audio acquisition on the analog outer end through the chip's built-in ADC.

The board is equipped with an analog pad with a sensitivity of -38 dB and sends the output signal to a fixed gain op amp to amplify the signal.

The external amplifier is powered by a controllable 3.3 V power domain. Make sure that the power domain is turned on before using the acquisition function.

>    Use timer Group to process audio acquisition, and do not use code such as the following for acquisition in tasks:
> ```c
>    size_t index = 0;
>    uint16_t audio_data[configMAX_ACQUIRE_COUNT];
>    do {
>        audio_data[index] = adc1_get_raw(CONFIG_AUDIO_CHANNEL);
>        ets_delay_us(1000000 / CONFIG_AUDIO_FREQ);
>    } while (++index < CONFIG_MAX_ACQUIRE_COUNT)
> ```
>    Collecting using the above methods will result in the CPU being consumed and cannot be released, thereby engaging in task gate dogs (if not disabled) and making other lower-priority tasks, including IDLE Task, unable to function.
>    When entering an ADC read acquisition into a system, you need to rewrite the functions collected by IRAM_ATTR reduce the response time of the rate and place variables in DRAM. Similarly, do not use any senum in this function.

## ADC accuracy

The ADC of esp32-S2 is highly repetitive despite the lack of a reference and the fact that it is powered by Buck or will result in high overall power supply noise.

We use USB to power the ESP32-S2-HMI-DevKit-1, using 13-bit resolution, attenuation of 11 dB, and range voltage of 2.6 V. After ADC1_CH8 4096 uncalibrated original values of the AD584T reference were converted to voltage via a ADC1_CH8-wheel acquisition of the 2.5 V voltage output of the AD584T reference, we obtained the following figure data:

![ADC](_static/ADC.png)
![image](https://user-images.githubusercontent.com/16070445/110633932-ab665a80-81a9-11eb-97d3-2d640c2045f3.png)


As you can see, under uncalibrated data, the error of the data is within ±0.005 V, and its standard deviation σ is only 3.339 LSB (0.00106 V). These errors come mainly from absolute accuracy, i.e. bias. Therefore, for ADC-sampled audio, its distortion and noise can be controlled at a high level.

>    The 10 V voltage of the AD584T has an output noise peak of 50 uV from 0.1 to 10 Hz, 
>    while the output noise peak of 2.5 V is obtained by an internal high-precision laser-adjusted voltage resistance, 
>    and the transistor measured by the 10 V output provides a maximum push-pull capacity of 30 mA. 
>    Its output noise at 2.5 V is less than the resolution of the 16-bit ADC and can be used as a test benchmark.
