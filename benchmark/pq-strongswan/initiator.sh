#!/bin/bash

ARGS="$*"
OUTF="initiator.log"

sudo docker "exec" carol bash -c "$ARGS" > $OUTF


