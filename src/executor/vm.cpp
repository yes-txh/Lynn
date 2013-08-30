/********************************
 FileName: executor/vm.cpp
 Author:   WangMin
 Date:     2013-08-27
 Version:  0.1
 Description: virtual machine, include kvm and lxc
*********************************/

#include "executor/vm.h"

using lynn::WriteLocker;
using lynn::ReadLocker;

/*VM::VM(const TaskInfo& info) {
    // TODO why?
    // can't define VM::VM in vm.cpp, but it works to define VM::VM in vm.h
}*/

void VM::Hello() {
    printf("Hello world\n");
}

