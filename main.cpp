//
// main.cpp
//
// Example application for connecting a Quectel FC41D Wi‑Fi/Bluetooth module
// to AWS IoT Core using AT commands.  The FC41D is controlled via a UART
// while an ESP32 reads temperature and humidity from a DHT11 sensor and
// publishes the values as JSON over MQTT.  This code was adapted from
// the ESP32‑AWS‑IoT template project to support the FC41D.

#include <Arduino.h>
#include <DHT.h>
#include <ArduinoJson.h>

#include "aws.h"
#include "certs.h"

// Pin definitions.  Adjust these to match your wiring.
#define DHT_PIN    4        // GPIO used for the DHT11 data line
#define DHT_TYPE   DHT11    // Sensor type
#define RXD2       16       // ESP32 RX pin connected to FC41D TX
#define TXD2       17       // ESP32 TX pin connected to FC41D RX

// Wi‑Fi credentials.  Provide the SSID and password of the access point
// that the FC41D should connect to.  Use only alphanumeric characters; if
// your credentials include commas or quotes you may need to escape them
// appropriately in the AT command.
static const char *WIFI_SSID     = "your_wifi_ssid";
static const char *WIFI_PASSWORD = "your_wifi_password";

// Create a second hardware serial port on UART2.  The first UART (Serial)
// is used for debugging over USB.  The second UART communicates with the FC41D.
HardwareSerial fc41dSerial(2);
// Instantiate the DHT sensor
DHT dht(DHT_PIN, DHT_TYPE);

// Send an AT command to the FC41D and print any response to the Serial monitor.
// A timeout can be specified to limit how long to wait for replies.
static void sendAT(const String &cmd, unsigned long timeoutMs = 3000) {
  // Echo the command to the host console for visibility
  Serial.print(F("⇒ "));
  Serial.println(cmd);
  // Send the command terminated by CR/LF
  fc41dSerial.print(cmd);
  fc41dSerial.print("\r\n");
  // Read responses until the timeout expires
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    while (fc41dSerial.available()) {
      char c = fc41dSerial.read();
      Serial.write(c);
    }
  }
  Serial.println();
}

// Connect the FC41D to the configured Wi‑Fi access point.  This function
// sends AT+QSTAAPINFO and then waits a few seconds for the module to report
// WLAN_CONNECTED and GOT_IP indications.  In a production system you would
// parse the URCs to verify connection status.
static void connectWifi() {
  String wifiCmd = String("AT+QSTAAPINFO=") + WIFI_SSID + "," + WIFI_PASSWORD;
  sendAT(wifiCmd, 8000); // allow extra time for association
  // Allow the module time to acquire an IP address.  Alternatively you
  // could poll the module with AT+QGETWIFISTATE or parse +QSTASTAT URCs.
  delay(5000);
}

// Configure MQTT parameters.  This example enables clean session and SSL
// verification.  Additional parameters such as keep‑alive can be added
// as necessary.
static void configureMQTT() {
  sendAT(F("AT+QMTCFG=\"session\",1,1"));
  sendAT(F("AT+QMTCFG=\"ssl\",1,1,1"));
  // Disable SNI (Server Name Indication).  If your endpoint requires SNI
  // support you can set the last parameter to 1.
  sendAT(F("AT+QSSLCFG=\"sni\",1,0"));
  // Enable certificate verification (2 = verify both server and client)
  sendAT(F("AT+QSSLCFG=\"verify\",1,2"));
}

// Open a TLS socket to AWS IoT Core.  The FC41D will report +QMTOPEN URCs
// indicating success or failure.  A 10 second timeout is used here as the
// connection requires DNS resolution and TLS handshake.
static void openMQTT() {
  String cmd = String("AT+QMTOPEN=1,\"") + AWS_END_POINT + "\",8883";
  sendAT(cmd, 10000);
}

// Connect the MQTT client over the previously opened socket.  The device
// name configured in aws.h is used as the client ID.
static void connectMQTT() {
  String cmd = String("AT+QMTCONN=1,\"") + DEVICE_NAME + "\"";
  sendAT(cmd, 5000);
}

// Publish a JSON document containing the current sensor values.  QoS1 is used
// to ensure the message is acknowledged.  The payload length must be
// specified in bytes.  The FC41D echoes a +QMTPUB URC when the publish
// completes.
static void publishSensor() {
  float temperature = dht.readTemperature();
  float humidity    = dht.readHumidity();
  float heatIndex   = dht.computeHeatIndex(temperature, humidity, false);

  // Build JSON payload
  StaticJsonDocument<128> doc;
  JsonObject eventObj = doc.createNestedObject("event");
  eventObj["temp"] = temperature;
  eventObj["hum"]  = humidity;
  eventObj["hi"]   = heatIndex;
  String payload;
  serializeJson(eventObj, payload);
  // Construct and send the publish command
  int len = payload.length();
  String cmd = String("AT+QMTPUB=1,1,1,0,\"") + MQTT_TOPIC + "\"," + String(len) + ",\"" + payload + "\"";
  sendAT(cmd, 5000);
}

// Keep track of the last time a publish was sent
static unsigned long lastPublish = 0;
static const unsigned long publishInterval = 10000; // milliseconds

void setup() {
  // Initialize serial ports
  Serial.begin(115200);
  // Give time for the host to open the serial monitor
  delay(1000);
  Serial.println(F("Initializing FC41D AWS IoT example..."));
  // Initialize UART for FC41D communication
  fc41dSerial.begin(115200, SERIAL_8N1, RXD2, TXD2);
  // Initialize sensor
  dht.begin();
  // Basic connectivity test
  sendAT(F("AT"));
  // Connect to Wi‑Fi
  connectWifi();
  // Configure MQTT settings
  configureMQTT();
  // Open TLS socket
  openMQTT();
  // Connect MQTT client
  connectMQTT();
}

void loop() {
  unsigned long now = millis();
  // Publish sensor data periodically
  if (now - lastPublish >= publishInterval) {
    lastPublish = now;
    publishSensor();
  }
  // Always drain the FC41D UART to display asynchronous URCs (e.g. +QMTRECV)
  while (fc41dSerial.available()) {
    char c = fc41dSerial.read();
    Serial.write(c);
  }
}