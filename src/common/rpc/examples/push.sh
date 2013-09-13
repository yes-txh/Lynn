#!/bin/bash

if [ $# -ne 1 ]; then
	echo "usage: $0 <server-address>" > /dev/stderr
	exit 1
fi

for ((i=0; i<16; ++i)); do
	./echoclient --server_address=$1 &
done > push.log 2>&1
