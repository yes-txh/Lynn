#!/bin/bash
echo "tickTime=2000
initLimit=5
syncLimit=2
server.1=127.0.0.1:2888:3888
server.2=127.0.0.1:2889:3889
server.3=127.0.0.1:2890:3890
" >> zoo.cfg

exit 0
