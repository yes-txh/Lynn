#!/bin/bash

cd ../../../../build64_release/common/crypto/ca/ca_client;

echo "Start testing using --identity and --role, press any key ..."
read
./certification_test --logtostderr=true --identity=fatliu --role=xfs

echo "Start testing using --ticket, press any key ..."
read
./certification_test --logtostderr=true --ticket=ZmF0bGl1CXhmcwk3MzkyNzE3NTQ=
