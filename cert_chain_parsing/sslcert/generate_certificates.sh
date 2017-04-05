#!/bin/sh

# Clean-up any old certificates
find . -name *.key -exec rm {} \;
find . -name *.crt -exec rm {} \;
rm *.old *.attr *.txt serial

touch index.txt
echo '01' > serial

# Generate the root certificate
echo "-----------------------------------------------"
echo "Generating root certificate ..."
echo "-----------------------------------------------"
openssl req -x509 -out ./certs/root.crt -newkey rsa:4096 -keyout ./private/root.key -days 365 -config root.cnf && \
    echo "Success!" || (echo "Failed!"; exit 1)

echo

# Generate CSR
echo "-----------------------------------------------"
echo "Generating CSR ..."
echo "-----------------------------------------------"
openssl req -out ./certs/interviewee.csr -newkey rsa:2048 -keyout ./private/interviewee.key -config interviewee.cnf && \
    echo "Success!" || (echo "Failed!"; exit 1)

echo

# Sign CSR with the root certificate
echo "-----------------------------------------------"
echo "Sign CSR with root certificate ..."
echo "-----------------------------------------------"
openssl ca -in ./certs/interviewee.csr -out ./certs/interviewee.crt -config root.cnf && \
    echo "Success!" || (echo "Failed!"; exit 1)

echo

# Export PKCS12 formatted crt file
echo "-----------------------------------------------"
echo "Exporting signed certificate as PKCS12 ..."
echo "-----------------------------------------------"
openssl pkcs12 -export -in ./certs/interviewee.crt -inkey ./private/interviewee.key -out ./certs/interviewee.p12 && \
    echo "Success!" || (echo "Failed!"; exit 1)
