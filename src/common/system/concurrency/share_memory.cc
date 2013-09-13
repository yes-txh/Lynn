// Copyright (c) 2011, Tencent Inc. All rights reserved.

#include "common/system/concurrency/share_memory.h"

#ifdef _WIN32

#else

strict_bool ShareMemory::ForceOpen(
    key_t key, size_t sz,
    const void *virtual_addr,
    int flags,
    bool *new_open)
{
    if (m_shm_id >= 0)
        return false;

    *new_open = true;

    // ���Դ��������ڴ�
    if (!Open(key, sz, kShmCreateExcl, kShmDefaultPerm)) {
        // ��EEXIST���󣬷���
        if (errno != EEXIST) {
            return false;
        }

        // ���Դ��Ѿ����ڵĹ����ڴ�
        if (!Open(key, sz, kShmOpen, kShmDefaultPerm)) {
            // sz��ͬ,ɾ���ɵ�,�ٴ���һ���µ�
            if (!Open(key, 0, kShmOpen, kShmDefaultPerm)) {
                return false;
            } else {
                if (!Remove())
                    return false;

                if (!Open(key, sz, kShmCreateExcl, kShmDefaultPerm)) {
                    return false;
                }
            }
        } else {
            *new_open = false;
        }
    }

    return Attach(virtual_addr, flags);
}

strict_bool ShareMemory::Open(key_t key, size_t size, const void *virtual_addr,
                              int flags) {
    if (m_shm_id >= 0)
        return false;

    if (!Open(key, size, kShmOpen, kShmDefaultPerm))
        return false;
    else
        return Attach(virtual_addr, flags);
}

strict_bool ShareMemory::Attach(const void *virtual_addr, int flags) {
    if (m_shm_id < 0 || m_attach_address)
        return false;

    m_attach_address = shmat(m_shm_id, virtual_addr, flags);

    if (m_attach_address == reinterpret_cast<void *>(-1))
        return false;

    return true;
}

strict_bool ShareMemory::Detach() {
    // û��open����attache
    if (m_shm_id < 0 || !m_attach_address)
        return false;

    int ret = shmdt(m_attach_address);

    if (0 == ret)
        m_attach_address = NULL;

    return ret == 0;
}

strict_bool ShareMemory::Remove() {
    // û��open
    if (m_shm_id < 0)
        return false;

    int ret = shmctl(m_shm_id, IPC_RMID, 0);

    if (0 == ret) {
        m_shm_id = -1;
        m_attach_address = NULL;
    }

    return ret == 0;
}

strict_bool ShareMemory::Control(int cmd, void *buf) {
    // û��open
    if (m_shm_id < 0)
        return false;

    // ������ɾ������
    if (IPC_RMID == cmd)
        return false;

    return shmctl(m_shm_id, cmd, static_cast<struct shmid_ds *>(buf)) == 0;
}

//////////////////////////////////////////////////////////////////////////

strict_bool ShareMemory::Open(key_t key, size_t size, int create, int perms) {
    if (m_shm_id >= 0)
        return false;

    m_attach_address = NULL;
    m_shm_size       = size;
    m_shm_id         = shmget(key, size, create | perms);

    return m_shm_id >= 0;
}

#endif // _WIN32
