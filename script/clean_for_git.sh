#!/bin/bash
rm -rf ../build/* ../bin/* ../log/* ../test/*
./clean_proxy.sh
./clean_zk.sh
exit 0

