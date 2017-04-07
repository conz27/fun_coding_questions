# Problem Summary:

Using an open-source tool, generate two X.509 certificates. The second certificate should be signed by the first,
forming a certificate chain in PKCS12 (.p12) format. Then, write code with the help of an open-source library,
that will print out whether or not a certificate is a self signed certificate, and the fingerprint of each
certificate. Please submit your certificates and build instructions along with your code (your submission should
contain enough information for us to reproduce your work).

## Enviroment:
Tested on Ubuntu 16.04

## Prerequisites:
* CMake >= 3.0.2
* OpenSSL
* libssl
* libcrypto

## Generate Certificates
```
cd sslcert
./generate_certificates.sh
```

## Run 'certinfo' program
```
cd sslcert/src
# Build Application
./build.sh
# Run Application
./certinfo ../certs/root.crt
./certinfo ../certs/interviewee.crt
```
