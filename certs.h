//
// certs.h
//
// Placeholder for AWS IoT certificates and private key.  These strings are
// not used directly by the FC41D example but can be transmitted to the
// module using AT commands such as AT+QSSLCERT.  Replace the contents
// between the BEGIN/END markers with your actual PEM‑encoded certificates
// if you wish to send them over the serial port.
//
#pragma once

const char AWS_PUBLIC_CERT[] =
"-----BEGIN CERTIFICATE-----\n"
"YOUR_CA_CERTIFICATE_HERE\n"
"-----END CERTIFICATE-----\n";

const char AWS_PRIVATE_KEY[] =
"-----BEGIN RSA PRIVATE KEY-----\n"
"YOUR_PRIVATE_KEY_HERE\n"
"-----END RSA PRIVATE KEY-----\n";

const char AWS_DEVICE_CERT[] =
"-----BEGIN CERTIFICATE-----\n"
"YOUR_DEVICE_CERT_HERE\n"
"-----END CERTIFICATE-----\n";