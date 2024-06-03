#!/bin/bash

MAX_EVENTS_TO_PROCESS=100000
NUMTHROWS=1000

nuissyst -c throwcard.xml \
         -n ${MAX_EVENTS_TO_PROCESS} \
         -q error_throws=${NUMTHROWS} \
         -o errorbands.root