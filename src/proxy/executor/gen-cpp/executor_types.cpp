/**
 * Autogenerated by Thrift Compiler (0.9.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "executor_types.h"

#include <algorithm>



int _kTaskEntityStateValues[] = {
  TaskEntityState::TASKENTITY_NOTFOUND,
  TaskEntityState::TASKENTITY_WAITING,
  TaskEntityState::TASKENTITY_STARTING,
  TaskEntityState::TASKENTITY_RUNNING,
  TaskEntityState::TASKENTITY_FINISHED,
  TaskEntityState::TASKENTITY_FAILED
};
const char* _kTaskEntityStateNames[] = {
  "TASKENTITY_NOTFOUND",
  "TASKENTITY_WAITING",
  "TASKENTITY_STARTING",
  "TASKENTITY_RUNNING",
  "TASKENTITY_FINISHED",
  "TASKENTITY_FAILED"
};
const std::map<int, const char*> _TaskEntityState_VALUES_TO_NAMES(::apache::thrift::TEnumIterator(6, _kTaskEntityStateValues, _kTaskEntityStateNames), ::apache::thrift::TEnumIterator(-1, NULL, NULL));

int _kVMTypeValues[] = {
  VMType::VM_UNKNOWN,
  VMType::VM_KVM,
  VMType::VM_LXC
};
const char* _kVMTypeNames[] = {
  "VM_UNKNOWN",
  "VM_KVM",
  "VM_LXC"
};
const std::map<int, const char*> _VMType_VALUES_TO_NAMES(::apache::thrift::TEnumIterator(3, _kVMTypeValues, _kVMTypeNames), ::apache::thrift::TEnumIterator(-1, NULL, NULL));

int _kVMStateValues[] = {
  VMState::VM_NOTFOUND,
  VMState::VM_OFFLINE,
  VMState::VM_ONLINE,
  VMState::VM_SERVICE_ONLINE
};
const char* _kVMStateNames[] = {
  "VM_NOTFOUND",
  "VM_OFFLINE",
  "VM_ONLINE",
  "VM_SERVICE_ONLINE"
};
const std::map<int, const char*> _VMState_VALUES_TO_NAMES(::apache::thrift::TEnumIterator(4, _kVMStateValues, _kVMStateNames), ::apache::thrift::TEnumIterator(-1, NULL, NULL));

