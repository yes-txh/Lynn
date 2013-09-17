SET(THRIFT_SOURCE_PREFIX ${CMAKE_SOURCE_DIR}/src/proxy)

SET(SCHEDULER_THRIFT_SOURCE
    ${THRIFT_SOURCE_PREFIX}/scheduler/gen-cpp/scheduler_constants.h 
    ${THRIFT_SOURCE_PREFIX}/scheduler/gen-cpp/scheduler_constants.cpp
    ${THRIFT_SOURCE_PREFIX}/scheduler/gen-cpp/scheduler_types.h
    ${THRIFT_SOURCE_PREFIX}/scheduler/gen-cpp/scheduler_types.cpp
    ${THRIFT_SOURCE_PREFIX}/scheduler/gen-cpp/Scheduler.h
    ${THRIFT_SOURCE_PREFIX}/scheduler/gen-cpp/Scheduler.cpp)
ADD_CUSTOM_COMMAND (
    OUTPUT ${SCHEDULER_THRIFT_SOURCE}
    COMMAND thrift --gen cpp -r -o ${CMAKE_SOURCE_DIR}/src/proxy/scheduler
    ${CMAKE_SOURCE_DIR}/src/proxy/scheduler/scheduler.thrift
)

SET(COLLECTOR_THRIFT_SOURCE
    ${THRIFT_SOURCE_PREFIX}/collector/gen-cpp/collector_constants.h
    ${THRIFT_SOURCE_PREFIX}/collector/gen-cpp/collector_constants.cpp
    ${THRIFT_SOURCE_PREFIX}/collector/gen-cpp/collector_types.h
    ${THRIFT_SOURCE_PREFIX}/collector/gen-cpp/collector_types.cpp
    ${THRIFT_SOURCE_PREFIX}/collector/gen-cpp/Collector.h
    ${THRIFT_SOURCE_PREFIX}/collector/gen-cpp/Collector.cpp)
ADD_CUSTOM_COMMAND (
    OUTPUT ${COLLECTOR_THRIFT_SOURCE}
    COMMAND thrift --gen cpp -r -o ${CMAKE_SOURCE_DIR}/src/proxy/collector
    ${CMAKE_SOURCE_DIR}/src/proxy/collector/collector.thrift
)

SET(EXECUTOR_THRIFT_SOURCE
    ${THRIFT_SOURCE_PREFIX}/executor/gen-cpp/executor_constants.h
    ${THRIFT_SOURCE_PREFIX}/executor/gen-cpp/executor_constants.cpp
    ${THRIFT_SOURCE_PREFIX}/executor/gen-cpp/executor_types.h
    ${THRIFT_SOURCE_PREFIX}/executor/gen-cpp/executor_types.cpp
    ${THRIFT_SOURCE_PREFIX}/executor/gen-cpp/Executor.h
    ${THRIFT_SOURCE_PREFIX}/executor/gen-cpp/Executor.cpp)
ADD_CUSTOM_COMMAND (
    OUTPUT ${EXECUTOR_THRIFT_SOURCE}
    COMMAND thrift --gen cpp -r -o ${CMAKE_SOURCE_DIR}/src/proxy/executor
    ${CMAKE_SOURCE_DIR}/src/proxy/executor/executor.thrift
)

#SET(COMMON_SOURCE
#    ../proxy/collector_wrapper.cpp)

SET(SRC 
    # ${COMMON_SOURCE}
    ${SCHEDULER_THRIFT_SOURCE}
    ${COLLECTOR_THRIFT_SOURCE}
    ${EXECUTOR_THRIFT_SOURCE}
    config.cpp
    system.cpp
    task_entity.cpp
    task_entity_pool.cpp
    vm.cpp
    kvm.cpp
    # lxc.cpp
    vm_pool.cpp
    resource_manager.cpp
    work_thread.cpp
    service.cpp
    executor.cpp)

ADD_EXECUTABLE(executor ${SRC})
# ADD_EXECUTABLE(clean_environment clean_environment.cpp)

TARGET_LINK_LIBRARIES(executor pthread)
TARGET_LINK_LIBRARIES(executor thrift)
TARGET_LINK_LIBRARIES(executor log4cplus)
TARGET_LINK_LIBRARIES(executor gflags)
TARGET_LINK_LIBRARIES(executor classad)
TARGET_LINK_LIBRARIES(executor virt)
TARGET_LINK_LIBRARIES(executor lxc)

# TARGET_LINK_LIBRARIES(clean_environment virt)

INSTALL(TARGETS executor RUNTIME DESTINATION bin)
# INSTALL(TARGETS clean_environment RUNTIME DESTINATION bin)
