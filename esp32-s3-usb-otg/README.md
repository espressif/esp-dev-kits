# ESP32-S3-USB-OTG Development Board

ESP32-S3-USB-OTG is a development board that focuses on USB-OTG function verification and application development. It is based on ESP32-S3 SoC, supports Wi-Fi and BLE 5.0 wireless functions, and supports USB host and USB device functions. It can be used to develop applications such as wireless storage devices, Wi-Fi network cards, LTE MiFi, multimedia devices, virtual keyboards and mice. The development board has the following features:

* Onboard ESP32-S3-MINI-1-N8 module, with built-in 8MB flash
* Onboard USB Type-A host and device interface, built-in USB interface switching circuit
* Onboard USB to serial debugging chip (Micro USB interface)
* Onboard 1.3-inch LCD color screen, support GUI
* Onboard SD card interface, compatible with SDIO and SPI interfaces
* Onboard charging IC, can be connected to lithium battery

<div align="center">
    <img src="./_static/pic_product_esp32_s3_otg.png" width="400px"/>
</div>
<div align="center"> ESP32-S3-USB-OTG (click to enlarge) </div>

**The document consists of the following major sections:**

* [Getting Started](https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32s3/esp32-s3-usb-otg/user_guide.html#getting-started): Provides a brief overview of ESP32-S3-USB-OTG and necessary hardware and software information.
* [Hardware Reference](https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32s3/esp32-s3-usb-otg/user_guide.html#hardware-reference): Provide detailed hardware information of ESP32-S3-USB-OTG.

## Examples

* [All-in-One factory demo](./examples/factory/)
* [ESP-IoT-Solution USB Device demos](https://github.com/espressif/esp-iot-solution/tree/usb/add_usb_solutions/examples/usb/device)
* [ESP-IoT-Solution USB Host demos](https://github.com/espressif/esp-iot-solution/tree/usb/add_usb_solutions/examples/usb/host)

## Released Bin

* [Factory bin](./examples/factory/factory_bin)

## Related Documents

* [ESP32-S3 Technical Specification](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_cn.pdf) (PDF)
* [ESP32-S3-MINI-1 & ESP32-S3-MINI-1U Technical Specifications](https://www.espressif.com/sites/default/files/documentation/esp32-s3-mini-1_mini-1u_datasheet_cn.pdf) (PDF)
* [Espressif Product Selection Tool](https://products.espressif.com/#/product-selector?names=)
* [ESP32-S3-USB-OTG schematic diagram](../docs/_static/schematics/esp32-s3-usb-otg/SCH_ESP32-S3_USB_OTG.pdf) (PDF)
* [ESP32-S3-USB-OTG PCB layout drawing](../docs/_static/schematics/esp32-s3-usb-otg/PCB_ESP32-S3_USB_OTG.pdf) (PDF)
  
