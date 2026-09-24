# Gesture Recognition 

This is an edge-side gesture recognition project based on TensorFlow Lite Micro. By collecting BMI270 data from the Sensairshuttle onboard sensor, it achieves local recognition of counterclockwise circles and V gestures.

| Counterclockwise circle | V gesture |
|:---:|:---:|
| ![counterclockwise](https://dl.espressif.com/AE/esp-iot-solution/gesture_recognition/counterclockwise.gif) | ![v](https://dl.espressif.com/AE/esp-iot-solution/gesture_recognition/v.gif) |

In this example, we will demonstrate how to collect data, build model training, and deploy.

## Data Collection Flow

Long press and release the Boot button to enter data collection mode. In data collection mode, each press of the Boot button will trigger data collection when the sum of absolute values of three-axis angular velocity exceeds the trigger threshold. Therefore, please complete the gesture immediately after pressing the button.

![Data Collection](https://dl.espressif.com/AE/esp-iot-solution/gesture_recognition/data_collect.png)

You can organize the data for each gesture type into separate txt files and use a Python script to convert them into CSV files. You can download the dataset used in this example [here](https://dl.espressif.com/AE/esp-iot-solution/gesture_recognition/shuttle_dataset.zip).

Next, you can refer to the following script to split the dataset for subsequent training:

```python
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split


def extract_data():
    # Read data files
    o_data = pd.read_csv('./o.csv', sep=',', header=None)
    v_data = pd.read_csv('./v.csv', sep=',', header=None)
    unknown_data = pd.read_csv('./unknown.csv', sep=',', header=None)

    # Create labels
    o_label = np.zeros(o_data.shape[0], dtype=int)
    v_label = np.ones(v_data.shape[0], dtype=int)
    unknown_label = np.full(unknown_data.shape[0], 2, dtype=int)

    # Combine feature data and labels (use .values for all to ensure numpy arrays)
    X_raw = np.vstack([o_data.values, v_data.values, unknown_data.values])
    y = np.concatenate([o_label, v_label, unknown_label])

    # Display the number of samples for each class
    print(f"o samples: {o_label.shape[0]}")
    print(f"v samples: {v_label.shape[0]}")
    print(f"unknown samples: {unknown_label.shape[0]}")

    print(f"Total samples: {len(y)}")

    num_samples = X_raw.shape[0]
    num_timesteps = 200  # 200 timesteps
    num_axes = 3  # x, y, z three axes

    # Reshape data: reshape 600 data points in each row into 200x3 matrix
    X = X_raw.reshape(num_samples, num_timesteps, num_axes)

    return X, y


if __name__ == "__main__":
    X, y = extract_data()

    X = np.array(X, dtype=np.float32)
    y = np.array(y, dtype=np.int32)

    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3, random_state=42)
```

## AI Processing Flow

#### Model Training

Based on the TensorFlow framework, we build the following neural network model:

```python
from tensorflow import keras

model = keras.Sequential([
    keras.layers.Conv1D(filters=8, kernel_size=5, padding='same', activation='relu', input_shape=(200, 3)),
    keras.layers.MaxPooling1D(pool_size=4),

    keras.layers.Conv1D(filters=16, kernel_size=5, padding='same', activation='relu'),
    keras.layers.MaxPooling1D(pool_size=4),

    keras.layers.GlobalAveragePooling1D(),

    keras.layers.Dense(32, activation='relu'),
    keras.layers.Dropout(0.2),

    # Output layer
    keras.layers.Dense(3, activation='softmax')
])

model.compile(
    optimizer=keras.optimizers.Adam(learning_rate=0.001),
    loss='sparse_categorical_crossentropy',
    metrics=['accuracy']
)

history = model.fit(
    X_train, y_train,
    validation_data=(X_test, y_test),
    epochs=300,
    batch_size=64,
    verbose=1,
    shuffle=True
)
```

![Training Curves](https://dl.espressif.com/AE/esp-iot-solution/gesture_recognition/training_curves.png)

#### Model Quantization and Deployment

```
import tensorflow as tf
from tensorflow import keras

model = keras.models.load_model(model_path)
converter = tf.lite.TFLiteConverter.from_keras_model(model)

converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.target_spec.supported_types = [tf.float32]
converter.inference_input_type = tf.float32
converter.inference_output_type = tf.float32

tflite_model = converter.convert()

with open(output_path, "wb") as f:
    f.write(tflite_model)
```

To run on the edge side, the tflite file needs to be converted to C++:

```
xxd -i model.tflite > model.cpp
```

## Example Output

The firmware uses the Boot button (GPIO28) to control the current operating mode by default, including data collection mode and automatic inference mode. The default mode is automatic inference mode. You can long-press and release the Boot button to switch modes:

* **Collection Mode**: Press the Boot button to start collection. When the sum of absolute values of three-axis angular velocity exceeds the set threshold, all sampled values will be printed.
* **Automatic Inference Mode**: No button press required. When the sum of absolute values of three-axis angular velocity exceeds the set threshold, inference will be performed automatically.

An abbreviated application log is shown below. Bootloader output and timestamps are omitted because they vary by ESP-IDF revision and build.

```text
I (1385) button: IoT Button Version: 4.1.5
I (1397) i2c_bus: i2c0 bus inited
I (1399) i2c_bus: I2C Bus V2 Config Succeed, Version: 1.5.2
I (1405) BMI2_ESP32: Creating I2C device with address 0x68
I (1410) BMI2_ESP32: I2C device created successfully
I (1434) BMI2_ESP32: Detected FreeRTOS tick rate: 1050 Hz
I (2212) bmi270_api: BMI270 sensor created successfully
I (2223) main_task: Returned from app_main()
I (3941) Model Pipeline: Motion detected: Unknown (confidence: 100.0%)
I (5022) Model Pipeline: Motion detected: Unknown (confidence: 99.9%)
I (8263) Model Pipeline: Motion detected: Unknown (confidence: 98.0%)
I (9992) Model Pipeline: Motion detected: Counterclockwise circle (confidence: 100.0%)
I (11719) Model Pipeline: Motion detected: Unknown (confidence: 95.0%)
I (13232) Model Pipeline: Motion detected: V (confidence: 99.9%)
I (14525) Model Pipeline: Motion detected: Unknown (confidence: 100.0%)
I (15605) Model Pipeline: Motion detected: Unknown (confidence: 99.2%)
```
