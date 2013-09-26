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

SET(VMWORKER_THRIFT_SOURCE
    ${THRIFT_SOURCE_PREFIX}/vm_worker/gen-cpp/vm_worker_constants.h
    ${THRIFT_SOURCE_PREFIX}/vm_worker/gen-cpp/vm_worker_constants.cpp
    ${THRIFT_SOURCE_PREFIX}/vm_worker/gen-cpp/vm_worker_types.h
    ${THRIFT_SOURCE_PREFIX}/vm_worker/gen-cpp/vm_worker_types.cpp
    ${THRIFT_SOURCE_PREFIX}/vm_worker/gen-cpp/VMWorker.h
    ${THRIFT_SOURCE_PREFIX}/vm_worker/gen-cpp/VMWorker.cpp)
ADD_CUSTOM_COMMAND (
    OUTPUT ${VMWORKER_THRIFT_SOURCE}
    COMMAND thrift --gen cpp -r -o ${CMAKE_SOURCE_DIR}/src/proxy/vm_worker
    ${CMAKE_SOURCE_DIR}/src/proxy/vm_worker/vm_worker.thrift
)

SET(CONF_FILE 
    ../conf_manager/dynamic_configuration.cpp
    ../conf_manager/zk_common.cpp
    ../conf_manager/watcher.cpp
    ../conf_manager/data_completion.cpp
    ../conf_manager/get_conf.cpp
   )

SET(SRC 
    ${SCHEDULER_THRIFT_SOURCE}
    ${COLLECTOR_THRIFT_SOURCE}
    ${EXECUTOR_THRIFT_SOURCE}
    ${VMWORKER_THRIFT_SOURCE}
    ${CONF_FILE}
    collector_conf.cpp
    machine.cpp
    machine_pool.cpp
    config_manager.cpp 
    collector_service.cpp
    collector_engine.cpp
    collector.cpp)

ADD_EXECUTABLE(collector ${SRC})

TARGET_LINK_LIBRARIES(collector thrift)
TARGET_LINK_LIBRARIES(collector pthread)
TARGET_LINK_LIBRARIES(collector log4cplus)
TARGET_LINK_LIBRARIES(collector classad)
TARGET_LINK_LIBRARIES(collector gflags)
TARGET_LINK_LIBRARIES(collector zookeeper_mt)

INSTALL(TARGETS collector RUNTIME DESTINATION bin)
