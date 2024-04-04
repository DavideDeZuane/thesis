#!/bin/bash

GREEN='\033[0;32m' 
DEFAULT='\033[0m' 
BOLD='\033[1m' 

CERT=$1



alg_sig=$(openssl x509 -in dilithium2.crt -noout -text -certopt no_header -certopt no_version -certopt no_validity -certopt no_issuer -certopt no_subject -certopt no_serial -certopt no_extensions -certopt no_signame | grep 'Signature Algorithm' | awk -F ': ' '{print $2}')
dim_sig=$(openssl x509 -in $CERT -text -noout -certopt ca_default -certopt no_validity -certopt no_serial -certopt no_subject -certopt no_extensions -certopt no_signame | sed 's/://g' | tr -d '\t' | grep -v 'Signature Algorithm' | grep -v 'Signature Value' | wc -c)
dim_pub=$(openssl x509 -in $CERT -pubkey -noout | grep -v 'BEGIN' | grep -v 'END' | wc -c)




echo -e "${BOLD}PubKey Dimension${DEFAULT}: ${GREEN}$dim_pub byte${DEFAULT}"
echo -e "${BOLD}Signature Dimension${DEFAULT}: ${GREEN}$dim_sig byte${DEFAULT}"
echo -e "${BOLD}Signature Algotithm${DEFAULT}: ${GREEN}$alg_sig${DEFAULT}"
