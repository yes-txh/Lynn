//  auto_baselib.h
//  wookin@tencent.com
//  2010-10-21
//
//  0:这是一个初始化svrpublib的独立头文件
//  1:所有的App需要显示初始化baselib, 以确
//    定一些对象的构造顺序
//
//  2:在不需要svrpublib其他功能只是调用
//    初始化的场合直接包含这个头文件
//
//
// ///////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_AUTO_BASELIB_H_
#define COMMON_BASELIB_SVRPUBLIB_AUTO_BASELIB_H_

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

// 初始化svrpublib
void InitBaseLib();

// 卸载svrpublib
void ShutdownBaseLib();

//
// class:       AutoBaseLib
// description:
//              初始化及自动析构svrpublib
//
class AutoBaseLib {
public:
    AutoBaseLib() {
        InitBaseLib();
    }

    ~AutoBaseLib() {
        ShutdownBaseLib();
    }
};

//
// class:       CXSocketLibAutoManage
// description:
//              用于win32自动调用WSAStartup
//              linux下面不需要初始化socket协议栈
//
class AutoSocketStartup {
public:
    AutoSocketStartup();
    ~AutoSocketStartup();
};
#define CXSocketLibAutoManage AutoSocketStartup

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_AUTO_BASELIB_H_
