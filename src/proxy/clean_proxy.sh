#!/bin/bash

#if [ -z $LYNN_HOME ]
#then
#    echo "LYNN_HOME is not set."
#    exit -2
#fi

#rm -rf $LYNN_HOME/src/proxy/worker/gen-cpp $LYNN_HOME/src/proxy/master/gen-cpp $LYNN_HOME/src/proxy/vm_worker/gen-cpp $LYNN_HOME/src/proxy/client/gen-cpp $LYNN_HOME/src/proxy/master/gen-py
rm -rf scheduler/gen-cpp collector/gen-cpp executor/gen-cpp
exit 0

