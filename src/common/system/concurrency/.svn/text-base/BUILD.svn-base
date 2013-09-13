cc_library(
    name = 'concurrency',
    srcs = [
        'dynamic_shm_queue.cc',
        'once.cc',
        'share_memory.cc',
        'static_shm_queue.cc',
        'base_thread.cpp',
        'condition_variable.cpp',
        'fixed_thread_pool.cpp',
        'mutex.cpp',
        'named_pipe_unix.cpp',
        'named_pipe_windows.cpp',
        'semaphore.cpp',
        'sync_event.cpp',
        'system_error.cpp',
        'this_thread.cpp',
        'thread_pool.cpp',
        'thread_types.cpp',
        'thread_group.cpp'
    ],
    deps = [
        '//common/base/compatible:compatible',
        '//common/base:function',
        '//common/system/time:time',
        '//common/system:system',
        '#pthread'
    ]
)

cc_test(
    name = 'thread_test',
    srcs = 'thread_test.cpp',
    deps = [
        ':concurrency',
        '//thirdparty/perftools:tcmalloc'
    ]
)

cc_test(
    name = 'thread_pool_test',
    srcs = 'thread_pool_test.cpp',
    deps = ':concurrency'
)

cc_test(
    name = 'fixed_thread_pool_test',
    srcs = 'fixed_thread_pool_test.cpp',
    deps = ':concurrency',
)

cc_test(
    name = 'thread_group_test',
    srcs = 'thread_group_test.cpp',
    deps = ':concurrency'
)

cc_test(
    name = 'semaphore_test',
    srcs = 'semaphore_test.cpp',
    deps = ':concurrency'
)

cc_test(
    name = 'mutex_test',
    srcs = 'mutex_test.cpp',
    deps = ':concurrency'
)

cc_test(
    name = 'spinlock_test',
    srcs = 'spinlock_test.cpp',
    deps = ':concurrency'
)

cc_test(
    name = 'rwlock_test',
    srcs = 'rwlock_test.cpp',
    deps = ':concurrency'
)

cc_test(
    name = 'lite_rwlock_test',
    srcs = 'lite_rwlock_test.cpp',
    deps = ':concurrency'
)

cc_test(
    name = 'sync_event_test',
    srcs = 'sync_event_test.cpp',
    deps = ':concurrency'
)

cc_test(
    name = 'circle_queue_test',
    srcs = 'circle_queue_test.cc',
    deps = ':concurrency'
)

cc_test(
    name = 'share_memory_test',
    srcs = 'share_memory_test.cc',
    deps = ':concurrency'
)

cc_test(
    name = 'static_shm_queue_test',
    srcs = 'static_shm_queue_test.cc',
    deps = ':concurrency'
)

cc_test(
    name = 'dynamic_shm_queue_test',
    srcs = 'dynamic_shm_queue_test.cc',
    deps = ':concurrency'
)

cc_test(
    name = 'blocking_queue_test',
    srcs = 'blocking_queue_test.cpp',
    deps = ':concurrency'
)

cc_test(
    name = 'named_pipe_test',
    srcs = 'named_pipe_test.cpp',
    deps = ':concurrency'
)

