#!/bin/bash

ARGS="$*"

sudo docker run --rm -v "./certs":"/certs" -w /certs strongx509/pq-strongswan $ARGS
