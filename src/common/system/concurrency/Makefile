COMMON := ../../
THIRD_PARTY := ../../../thirdparty

project.includes := $(COMMON).. $(THIRD_PARTY)

# thread_pool
project.targets += thread_pool_test
project.tests   += thread_pool_test
thread_pool_test.sources := thread_pool_test.cpp
thread_pool_test.ldadd   := \
	$(if $(SOSO_BUILD),@libroot@/common/base/,../../base/)libbase.a \
	$(if $(SOSO_BUILD),@libroot@/common/system/,../)libsystem.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-lpthread

# fixed_thread_pool
project.targets += fixed_thread_pool_test
project.tests   += fixed_thread_pool_test
fixed_thread_pool_test.sources := fixed_thread_pool_test.cpp
fixed_thread_pool_test.ldadd   := \
	$(if $(SOSO_BUILD),@libroot@/common/base/,../../base/)libbase.a \
	$(if $(SOSO_BUILD),@libroot@/common/system/,../)libsystem.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-lpthread

# semaphore
project.targets += semaphore_test
project.tests   += semaphore_test
semaphore_test.sources := semaphore_test.cpp
semaphore_test.ldadd   := \
	$(if $(SOSO_BUILD),@libroot@/common/system/,../)libsystem.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-lpthread

project.targets += mutex_test
project.tests += mutex_test
mutex_test.sources := mutex_test.cpp
mutex_test.ldadd := \
	-pthread \
	$(if $(SOSO_BUILD),@libroot@/common/system/,../)libsystem.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a

ifdef BUILD_TEST
project.targets += spinlock_test
project.tests += spinlock_test
spinlock_test.sources := spinlock_test.cpp
spinlock_test.ldadd := \
	-pthread \
	$(if $(SOSO_BUILD),@libroot@/common/base/,../../base/)libbase@64@.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a
endif

project.targets += rwlock_test
project.tests += rwlock_test
rwlock_test.sources = rwlock_test.cpp
rwlock_test.ldadd = \
	-pthread \
	$(if $(SOSO_BUILD),@libroot@/common/system/,../)libsystem.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a

project.targets += lite_rwlock_test
project.tests += lite_rwlock_test
lite_rwlock_test.sources = lite_rwlock_test.cpp
lite_rwlock_test.ldadd = \
	-pthread \
	$(if $(SOSO_BUILD),@libroot@/common/system/,../)libsystem.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a

# sync_event
project.targets += sync_event_test
sync_event_test.sources = sync_event_test.cpp condition_variable.cpp sync_event.cpp

sync_event_test.ldadd := \
	-pthread \
	$(if $(SOSO_BUILD),@libroot@/common/system/,../)libsystem.a \
	$(if $(SOSO_BUILD),@libroot@/common/base/,../../base/)libbase@64@.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a

# bitmap
#project.targets += base_bitmap_test
#project.tests   += base_bitmap_test
base_bitmap_test.sources := base_bit_map_test.cc
base_bitmap_test.ldadd   := $(THIRD_PARTY)/gtest/lib@64@/libgtest.a

# circle queue
project.targets += circle_queue_test
project.tests   += circle_queue_test
circle_queue_test.sources := circle_queue_test.cc
circle_queue_test.ldadd   := \
	-lpthread \
	$(if $(SOSO_BUILD),@libroot@/common/system/,../)libsystem.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a

# share memory
project.targets += share_memory_test
project.tests   += share_memory_test
share_memory_test.sources := share_memory_test.cc share_memory.cc
share_memory_test.ldadd   := $(THIRD_PARTY)/gtest/lib@64@/libgtest.a -lpthread

# static shm queue
project.targets += static_shm_queue_test
project.tests   += static_shm_queue_test
static_shm_queue_test.sources := static_shm_queue_test.cc static_shm_queue.cc share_memory.cc
static_shm_queue_test.ldadd   := $(THIRD_PARTY)/gtest/lib@64@/libgtest.a -lpthread

# dynamic shm queue
project.targets += dynamic_shm_queue_test
project.tests   += dynamic_shm_queue_test
dynamic_shm_queue_test.sources := dynamic_shm_queue_test.cc dynamic_shm_queue.cc share_memory.cc
dynamic_shm_queue_test.ldadd   := $(THIRD_PARTY)/gtest/lib@64@/libgtest.a -lpthread

project.targets += blocking_queue_test
project.tests   += blocking_queue_test
blocking_queue_test.sources := blocking_queue_test.cpp condition_variable.cpp
blocking_queue_test.ldadd   := \
	-lpthread \
	$(if $(SOSO_BUILD),@libroot@/common/base/,../../base/)libbase.a \
	$(if $(SOSO_BUILD),@libroot@/common/system/,../)libsystem.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a

project.targets += named_pipe_test
# project.tests   += named_pipe_test
named_pipe_test.sources = named_pipe_unix.cpp named_pipe_test.cpp
named_pipe_test.ldadd   := \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-lpthread

include $(COMMON)builder/Generic.mak
