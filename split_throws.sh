#!/bin/bash

MAX_EVENTS_TO_PROCESS=10
NUMTHROWS=2

for i in 0 1 2 3 4 5 6 7 8 9; do

  sleep 1

  mkdir gen_${i}; cd gen_${i}

  cp ../DUNEMecSandbox.so .

echo  nuissyst -c ../throwcard.xml \
           -f ThrowErrors \
           -n ${MAX_EVENTS_TO_PROCESS} \
           -q error_throws=${NUMTHROWS} \
           -s $(( NUMTHROWS * i )) \
           -o errorbands_${i}

  rm DUNEMecSandbox.so

  cd -

done
