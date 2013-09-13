#!/bin/sh

# 自动更新svn revision number
# 判断生成的auto_build_version.h.tmp中的Revision num和auto_build_version.h中是否一致，如果一致就不更新auto_build_version.h文件了

# 编译前更新revision number以后，就会在各模块的/about.html页面中看到该revision num

# 需要被加到BUILD环境里面执行
# default get svn info of xfs
if [ $# -lt 2 ]; then
    echo "Usage: ./auto_update_svn_info.sh debug/release 32/64"
    exit
fi

if [ "$1" != "debug" -a "$1" != "release" ]; then
    echo "Usage: ./auto_update_svn_info.sh debug/release 32/64"
    echo "       The first argument must be debug or release."
    exit
fi

if [ "$2" != "32" -a "$2" != "64" ]; then
    echo "Usage: ./auto_update_svn_info.sh debug/release 32/64"
    echo "       The second argument must be 32 or 64."
    exit
fi

DEBUG_TAG=$1
BUILD_BITS=$2


current_path="."
blade_path="../common/baselib/svrpublib"
# 默认是blade的path
path=$blade_path
#if [ $# -ne 0 ] 
#then
#    path=$current_path
#fi

source ./tools/script/test_xfs_cluster/output_text_color.sh 

echo ""
$path/get_revision_num.sh $DEBUG_TAG $BUILD_BITS > $path/source/auto_build_version.cc.tmp
if [ -e $path/source/auto_build_version.cc.tmp ]; then
    if [ -e $path/source/auto_build_version.cc ]; then
        Revision_old=`grep "Revision: " $path/source/auto_build_version.cc | head -n1 | cut -d" " -f8`
        Revision_new=`grep "Revision: " $path/source/auto_build_version.cc.tmp | head -n1 | cut -d" " -f8`
        if [ $Revision_old != $Revision_new ]; then
            mv $path/source/auto_build_version.cc.tmp $path/source/auto_build_version.cc
            output_color_text ${OK_COLOR} "svn info of : xfs path -- auto updated successful"
        else
            #判断文件cksum值是否一致
            cksum_old=`cksum $path/source/auto_build_version.cc | cut -d" " -f1`
            cksum_new=`cksum $path/source/auto_build_version.cc.tmp | cut -d" " -f1`
            if [ $cksum_old -ne $cksum_new ]; then
                mv $path/source/auto_build_version.cc.tmp $path/source/auto_build_version.cc
                output_color_text ${OK_COLOR} "svn info of : xfs path -- auto updated successful"
           else
                output_color_text ${OK_COLOR} "svn info of : xfs path -- already up to date"
           fi
        fi
    else
        mv $path/source/auto_build_version.cc.tmp $path/source/auto_build_version.cc
        output_color_text ${OK_COLOR} "svn info of : xfs path -- auto updated successful"
    fi
fi

rm -f $path/source/auto_build_version.cc.tmp
