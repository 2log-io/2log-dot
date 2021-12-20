# The 2log dot

The 2log Dot is a smart, wifi enabled RFID card reader for the 2log system. It is based on freely available components, so it can be easily replicated. 

## Parts

To build a Dot you need:
- 1 Wemos D1 mini ESP32
- 1 PN532 RFID Reader
- 1 24xWS2812B LED circle (24 Nepoixels arranged in a circle)
- 1 Button
- 10kOhm resistor

## Wiring

| PN532  |  ESP32 |
|---|---|
|  PN532_SCK |  18 |
|  PN532_MOSI |  23 |
|  PN532_SS |  5 |
|  PN532_MISO | 19  |
|  PN532_IRQ | 27  |


| WS2812B  |  ESP32 |
|---|---|
| IN |  32 |


| Button  |  ESP32 |
|---|---|
|  OUT |  13 |

## Firmware (Linux)
Make sure that the drivers for the serial adapter of your ESP32 and the Python tool "esptool.py" is installed. 
Then download the latest build from our CI Toolchain here:
https://gitlab.com/Frime/2log-dot/-/jobs/artifacts/main/download?job=build
And flash the binaries with the following command to your ESP:
```
esptool.py  -p /dev/ttyUSB0 \
            -b 460800 \
            --before=default_reset \
            --after=hard_reset write_flash \
            --flash_mode dio \
            --flash_freq 40m \
            --flash_size 4MB \
            0x1000 bootloader/bootloader.bin \
            0x10000 2log-Dot.bin \
            0x8000 partition_table/partition-table.bin \
            0xe000 ota_data_initial.bin
```

You may need to replace /dev/ttyUSB0 with the path to the serial adapter found by your system.
