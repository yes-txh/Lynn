#!/bin/sh

# 自动更新svn revision number
# 编译前更新revision number以后，就会在各模块的/about.html页面中看到该revision num

# 默认获取xfs路径的svn info
# svn_src_path="https://tc-scm.tencent.com/setech/setech_infrastructure_rep/Infra_proj/trunk/src/xfs"
# svrpublib_path="https://tc-scm.tencent.com/setech/setech_infrastructure_rep/Infra_proj/trunk/src/common/baselib/svrpublib"
if [ $# -lt 2 ]; then                                                                               
    echo "Usage: ./get_revision_num.sh debug/release 32/64"                                                 
    exit                                                                                            
fi                                                                                                  

if [ "$1" != "debug" -a "$1" != "release" ]; then                                                      
    echo "Usage: ./get_revision_num.sh debug/release 32/64"                                        
    echo "       The first argument must be debug or release."                                         
    exit                                                                                               
fi                                                                                                     

if [ "$2" != "32" -a "$2" != "64" ]; then                                                              
    echo "Usage: ./get_revision_num.sh debug/release 32/64"                                                         
    echo "       The second argument must be 32 or 64."                                                
    exit                                                                                               
fi                                                                                                     

DEBUG_TAG=$1                                                                                           
BUILD_BITS=$2                                 

local_svn_src_path=`pwd | sed 's/common\/baselib\/svrpublib/xfs/'`


echo "
// auto generated  by svn info (/xfs/tools/script/get_revision_num.sh)
#include \"common/baselib/svrpublib/server_publib.h\"
#include \"common/baselib/svrpublib/xfs_auto_build_version.h\"


_START_XFS_BASE_NAMESPACE_

const char* GetXFSRevisionNum(){"

revision_num=$(svn info $local_svn_src_path | grep 'Revision')
echo "    return \"" ${revision_num}" \";
}"
echo ""

build_info=$(svn info $local_svn_src_path | sed s/$/'\\n<br>'/)

echo "const char* GetXFSBuildInfo(){"
echo "    return \" XfsVer: 1.2_20110623_build003 \\n<br> BuildType: "$DEBUG_TAG $BUILD_BITS" \\n<br> "${build_info}"\";
}"
echo ""

echo "const char* GetXFSVerAndUUID(){"
echo "    return \"xfs version: " ${revision_num} ", xfs-internal-uuid: D5FE64F8-6AC5-4254-8B5A-EED3C9FC72FE\";
}"

echo ""
echo "const char* GetXFSReleaseVer(){"
echo "    return \"xfs ver:1.2_20110623_build003\";
}"

echo "
_END_XFS_BASE_NAMESPACE_"
