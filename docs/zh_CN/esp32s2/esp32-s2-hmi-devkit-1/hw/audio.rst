音频
==========

:link_to_translation:`en: [English]`

ESP32-S2-HMI-DevKit-1 支持音频播放与采集。您可以在 ``esp32-s2-hmi-devkit-1/examples/audio/`` 目录下找到音频播放和采集的示例。

音频播放
------------

ESP32-S2-HMI-DevKit-1 可以通过 DAC 或 PWM 进行音频输出。我们推荐使用 PWM 进行音频输出，因为 PWM 输出具有更低的噪声和更高的分辨率（DAC 输出分辨率为 8 位，PWM 输出则最高可以在 19.2 kHz 的采样率下，达到 12 位的分辨率）。

通过 IO 口输出的信号将先进入数字电位器 TPL0401 进行无损音量调节，然后通过一个 100 nF 的隔直电容 C33 与一个 200 kOhm 的电阻 R52。该 RC 电路将截止频率控制在 8 Hz 附近。除此之外，该信号还将被送至一个功率为 3 W 的 D 类音频功率放大器 NS4150，通过外部电阻将增益设定为 1.5 倍，从而将 3.3 V 的最大输出信号放大至略低于 PA 电源 (5 V) 的 4.95 V，在尽量增大输出音量的同时减少饱和失真。

.. figure:: ../../../../_static/esp32-s2-hmi-devkit-1/esp32-s2-hmi-devkit-1-audio-playback.png
   :align: center
   :alt: ESP32-S2-HMI-DevKit-1 音频播放原理图（点击放大）
   :scale: 45%
   :figclass: align-center

   ESP32-S2-HMI-DevKit-1 音频播放原理图（点击放大）

音频功放由 5 V 电源域供电。在使用音频播放功能前，请确保该电源域处于开启状态（参阅 :ref:`v-电源域-1` 小节）。

音频采集
------------

ESP32-S2-HMI-DevKit-1 可以通过芯片内置的 ADC 对模拟麦克风进行音频采集。

开发板搭载了一个灵敏度为 -38 dB 的模拟麦克风，并会将输出信号发送至固定增益的运算放大器 TLV6741 以放大信号。

.. figure:: ../../../../_static/esp32-s2-hmi-devkit-1/esp32-s2-hmi-devkit-1-microphone.png
   :align: center
   :alt: ESP32-S2-HMI-DevKit-1 模拟麦克风原理图（点击放大）
   :scale: 60%
   :figclass: align-center

   ESP32-S2-HMI-DevKit-1 模拟麦克风原理图（点击放大）

麦克风与运算放大器由可控 3.3 V 电源域供电。在使用音频采集功能前，请确保该电源域处于开启状态（参阅 :ref:`v-电源域-2` 小节）。

请使用 Timer Group 中断来进行音频采集，请勿在任务中使用诸如以下形式的代码进行音频采集：

.. code:: c

   size_t index = 0;
   uint16_t audio_data[configMAX_ACQUIRE_COUNT];
   do {
       audio_data[index] = adc1_get_raw(CONFIG_AUDIO_CHANNEL);
       ets_delay_us(1000000 / CONFIG_AUDIO_FREQ);
   } while (++index < CONFIG_MAX_ACQUIRE_COUNT)

若使用上述方式进行采集，将导致 CPU 被占用而无法释放，从而触发任务看门狗（如果未被禁用），并使得其他更低优先级的任务（包括 IDLE Task）无法运行。

在中断函数中进行 ADC 读数采集时，您需要重写 ADC 采集的函数，确保这些函数为 ``IRAM_ATTR`` 以降低中断的响应时间，并且将变量放置于 DRAM 中。同样的，在该函数中，不要使用任何信号量。您可以参考 ``examples`` 文件夹下的 ``audio/audio_record`` 的实现方式。

ADC 精度
------------

尽管缺少基准电压源以及采用 Buck 供电或将导致整体电源噪声较大，ESP32-S2 的 ADC 仍然具有较高的重复性。

ADC 使用 13 位分辨率，衰减为 11 dB，对应满量程电压为 2.6 V。经由 ADC1_CH8 轮询采集 AD584T 基准电压源的 2.5 V 电压输出 4096 次的未经校准原始值转换为电压后，我们得到了下图数据：

.. figure:: ../../../../_static/esp32-s2-hmi-devkit-1/ADC.png
   :align: center
   :alt: ESP32-S2-HMI-DevKit-1 ADC（点击放大）
   :scale: 35%
   :figclass: align-center

   ESP32-S2-HMI-DevKit-1 ADC（点击放大）

可以看到，在未经校准的数据下，绝大多数数据的误差在 ±0.005 V 内，其标准差 σ 仅为 3.339 LSB (0.00106 V)。这些误差主要来自于绝对精度，即偏置上。因此，对于 ADC 采样的音频，可将其失真性和噪声控制在较低的水平。

AD584T 的 10 V 电压在 0.1 ~ 10 Hz 的输出噪声峰值为 50 uV，2.5 V 电压的输出噪声峰值则由内部经过高精度激光调整的电阻分压得到，并由 10 V 输出测的晶体管提供最大 30 mA 的推挽能力。其在 2.5 V 下的输出噪声小于 16 位 ADC 的分辨能力，可以作为测试基准。
