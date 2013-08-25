#!/bin/bash
rm -rf ../build/* ../bin/* ../log/*
./clean_proxy.sh
./clean_zk.sh
exit 0

