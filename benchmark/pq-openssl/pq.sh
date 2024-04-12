#!/bin/bash

IMAGE_NAME=pq-openssl
ARGS="$*"
VOLUME="./certs"


if ! docker images --format '{{.Repository}}' | grep -q "^$IMAGE_NAME$"; then
	echo "L'immagine $IMAGE_NAME non è presente"
	echo "Creo..."
	sudo docker build . -t $IMAGE_NAME
fi

sudo docker run --rm -v $VOLUME:/workspace -w /workspace $IMAGE_NAME $ARGS
