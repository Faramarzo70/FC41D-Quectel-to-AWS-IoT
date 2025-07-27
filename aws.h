//
// aws.h
//
// Configuration for the AWS IoT endpoint and MQTT topic used by the FC41D
// example.  Edit these definitions to match your AWS IoT setup.  The endpoint
// should be the ATS (Amazon Trust Services) hostname assigned to your device.
//
#pragma once

// AWS IoT endpoint (without protocol). For example:
//   "a1srr3yo833bl6-ats.iot.us-east-1.amazonaws.com"
#define AWS_END_POINT "your-endpoint-ats.iot.region.amazonaws.com"

// MQTT client ID / thing name.  This should match the AWS IoT thing name
// associated with the device certificate.
#define DEVICE_NAME "my_fc41d_device"

// MQTT topic to publish to.  A common pattern is
// "$aws/things/<thingName>/shadow/update" which updates the device shadow.
#define MQTT_TOPIC "$aws/things/my_fc41d_device/shadow/update"