# esp-dev-kits

## Quick start

- install ESP-IDF - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/#get-started-get-prerequisites
- clone the repository:
```
git clone git@github.com:espressif/esp-dev-kits.git
cd esp-dev-kits
git submodule update --init --recursive
```
- change the directory to an example e.g.: `cd esp32-s2-hmi-devkit-1/examples/smart-panel`
- build: `idf.py build`
- flash and monitor: `idf.py flash monitor --port PORT`
