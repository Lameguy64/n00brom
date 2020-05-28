#!/bin/bash

build_date=$(date +%Y%m%d%H%M)

armips -strequ build_date $build_date \
	-sym n00brom.sym -temp n00brom.lst \
	n00brom.asm

# cannot determine if armips was successful or not
# because armips apears to return a random exit status
cat romhead.bin >n00brom.rom
cat ramprog.bin >>n00brom.rom

