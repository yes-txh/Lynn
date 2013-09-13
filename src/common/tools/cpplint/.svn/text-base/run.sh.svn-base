#!/bin/bash
m_path=$1
allfile=`find $m_path -iregex '.*\(\.h\|\.cpp\|\.hpp\)'`
#allfile=`find $m_path -iregex '.*\.cpp'`
for file in $allfile
do
    #echo $file && sh path.sh --output=vs7 $file
    echo $file && ~/python_example/cpplint.py --output=vs7 $file
done
