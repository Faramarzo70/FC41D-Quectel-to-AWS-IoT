# FC41D-Quectel‑to‑AWS‑IoT

This project demonstrates how to connect a **Quectel FC41D Wi‑Fi/Bluetooth module** to **AWS IoT Core** and publish sensor data using **AT commands**.  It is based on the open‑source [ESP32‑AWS‑IoT template](https://github.com/survivingwithandroid/ESP32-AWS-IoT) but has been re‑implemented for the FC41D module.

Instead of using the ESP32’s built‑in Wi‑Fi stack, the FC41D is controlled over a UART using the module’s AT command interface.  An ESP32 development board reads a DHT11 temperature and humidity sensor and uses the FC41D to connect to a Wi‑Fi access point, establish an MQTT connection to AWS IoT Core and publish JSON messages.

## How it works

1. **Wi‑Fi connection** – The FC41D is set to station mode and instructed to join a Wi‑Fi access point with `AT+QSTAAPINFO=&lt;SSID&gt;,&lt;pwd&gt;`.  When the module reports `+QSTASTAT:WLAN_CONNECTED` and `+QSTASTAT:GOT_IP` the device has successfully acquired an IP address【693478592633391†L6369-L6378】.

2. **SSL and MQTT configuration** – After loading the CA certificate, device certificate and private key with `AT+QSSLCERT` the module is configured for MQTT over TLS.  Example commands from the official access guide include enabling all cipher suites (`AT+QSSLCFG="ciphersuite",1,0xFFFF`), enabling clean session (`AT+QMTCFG="session",1,1`) and turning on SSL/TLS verification【966357325308740†L505-L520】.

3. **Opening the MQTT connection** – The command `AT+QMTOPEN=1,"your‑endpoint‑ats.iot.region.amazonaws.com",8883` opens a TLS socket to AWS IoT Core【966357325308740†L521-L523】.  When the `+QMTOPEN: 1,0` URC is received the socket is ready.

4. **Connecting the MQTT client** – After the socket is open the client connects to AWS IoT Core with `AT+QMTCONN=1,"yourThingName"`【966357325308740†L524-L528】.  You may also subscribe to the device shadow topics with `AT+QMTSUB` if you need to receive updates.

5. **Publishing data** – The ESP32 reads temperature and humidity from the DHT11 sensor and builds a small JSON document.  To publish the message the code constructs an `AT+QMTPUB` command of the form `AT+QMTPUB=1,1,1,0,"&lt;topic&gt;",&lt;length&gt;,"&lt;json&gt;"`.  QoS 1 is used so that the module will retry until an acknowledgement is received.

The included `src/main.cpp` encapsulates these steps in Arduino code.  It uses `HardwareSerial` to talk to the FC41D on a second UART and prints all responses on the primary USB serial port for debugging.

## Getting started

1. **Hardware** – Connect the FC41D module to your ESP32 board using a free UART (for example, GPIO 16 as RX and GPIO 17 as TX).  Power the module as described in the datasheet.  Connect a DHT11 sensor to GPIO 4 (or modify `DHT_PIN` in the code).

2. **Certificates** – Download your AWS IoT device certificates and load them into the FC41D using the commands illustrated in the official access guide【966357325308740†L505-L520】.  The `certs.h` file in this repository contains placeholders where you can paste the PEM‑encoded certificates if you wish to send them to the module over the serial port.

3. **Configure `aws.h`** – Edit `src/aws.h` and set `AWS_END_POINT`, `DEVICE_NAME` and `MQTT_TOPIC` to match your AWS IoT settings.

4. **Build and upload** – This project uses [PlatformIO](https://platformio.org/).  Install PlatformIO in VS Code, open this folder and click **Build** → **Upload**.  Alternatively run `pio run --target upload` from the command line.

5. **Monitor** – Open the serial monitor at 115200 bps to see AT command output and sensor readings.

## A note about BLE

The FC41D also supports BLE connectivity.  The AT commands for BLE are documented in the same manual and include `AT+QBLEINIT`, `AT+QBLESCAN` and `AT+QBLECONN`【693478592633391†L6392-L6531】.  They are not used in this example but you can extend the project if needed.

## Reference

* **Quectel FC41D Wi‑Fi&Bluetooth Module AWS IoT Platform Access Guide**【966357325308740†L495-L529】.
* **Quectel FC41D AT Commands Manual**【693478592633391†L6369-L6378】.