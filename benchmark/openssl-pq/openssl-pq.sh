#!/bin/bash

IMAGE_NAME=openssl-oqs
CONTAINER_NAME=openssl-oqs
args="$*"
# introdurre il check per vedere se l'immagine già esiste
# se esiste allora skippo ed eseguo il container 
# alrimenti faccio la build
# sudo docker build . -t $IMAGE_NAME
sudo docker run --rm -v ./theseus:/theseus -w /theseus $IMAGE_NAME $args
