#!/bin/bash

ARGS="$*"

sudo docker run --rm -v "./certs":"/certs" -w /certs strongx509/pq-strongswan $ARGS

pki --gen --type falcon1024 --outform pem > CA.falcon1024.pem
pki --self --type priv --in CA.dilithium5.pem --ca --lifetime 3652     --dn "CN=CA-Dilithium5"                      --outform pem > CA.dilithium5.cert.pem


 pki --issue --cacert CA.dilithium5.cert.pem --cakey CA.dilithium5.pem --type priv --in Moon.dilithium5.pem --lifetime 1461 --dn "CN=moon.dilithium5" --outform pem > Moon.dilithium5.cert.pem
