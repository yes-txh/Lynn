//
//  fake_epoll.cpp
//  wookin@tencent.com
// ///////////////////////////////////////////////////////

#include <string.h>
#include <stdio.h>

#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/fake_epoll.h"

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

#ifdef _FAKE_EPOLL

#ifndef WIN32
#pragma message("***WARNING***,fake Epoll building on non WIN32 OS,please check _FAKE_EPOLL\r\n");
#endif //

#ifdef WIN32
#pragma   warning(disable:4127)
#endif // WIN32

//
//  -1:fail
//
EPOLLHANDLE fake_epoll_create(int32_t size) {
    CXThreadAutoLock autoLock(&__GetFakeepollMutex());
    EPOLLHANDLE epoll_handle = (EPOLLHANDLE)-1;
    if (size > 0) {
        FakeEPOLL_HANDLE* handle = new FakeEPOLL_HANDLE;
        if (handle) {
            memset((unsigned char*)handle, 0, sizeof(FakeEPOLL_HANDLE));
            handle->m_buffer = new unsigned char[sizeof(FakeEpollNode)*size];
            if (handle->m_buffer) {
                memset(handle->m_buffer, 0, sizeof(FakeEpollNode)*size);
                for (int32_t i = 0; i < size; i++) {
                    FakeEpollNode* node =
                        reinterpret_cast<FakeEpollNode*>(
                            handle->m_buffer+sizeof(FakeEpollNode) * i
                        );

                    node->node.sockfd = INVALID_SOCKET;
                    node->next = handle->m_empty_head;
                    handle->m_empty_head = node;
                }
                epoll_handle = (EPOLLHANDLE)handle;
            } else {
                //  Maybe out of memory
                delete handle;
                handle = NULL;
            }
        }
    }
    return epoll_handle;
}

int32_t    fake_epoll_close(EPOLLHANDLE epfd) {
    CXThreadAutoLock autoLock(&__GetFakeepollMutex());
    int32_t err = -1;
    if (epfd && epfd != (EPOLLHANDLE)INVALID_HANDLE) {
        FakeEPOLL_HANDLE* handle = reinterpret_cast<FakeEPOLL_HANDLE*>(epfd);

        delete []handle->m_buffer;
        delete handle;
        err = 0;
    }
    return err;
}

//
//  EPOLL_CTL_ADD:这种情况下如果fd已经在epoll_set集合内则ADD失败
//  EPOLL_CTL_MOD:这种情况下如果和已经存在user data不相等, 则失败,
//                防止重复加入导致节点丢失情况
//
int32_t fake_epoll_ctl(EPOLLHANDLE epfd, int32_t op, SOCKET fd,
                       const struct epoll_event* ev) {
    int32_t err = -1;
    if (epfd &&
            epfd != (EPOLLHANDLE)INVALID_HANDLE &&
            fd != INVALID_SOCKET && ev) {
        FakeEPOLL_HANDLE* handle = reinterpret_cast<FakeEPOLL_HANDLE*>(epfd);
        if (op == EPOLL_CTL_ADD &&
                handle->m_empty_head) {
            if (!epoll_fd_exist(epfd, fd)) {
                //  Get an empty node
                FakeEpollNode* node = handle->m_empty_head;
                handle->m_empty_head = handle->m_empty_head->next;
                node->next = 0;

                //  Save data
                node->node.sockfd = fd;
                node->node.epollevent = *ev;

                //  Add to valid queue
                node->next = handle->m_valid_head;
                handle->m_valid_head = node;

                //  Seek to head
                handle->m_valid_cursor = handle->m_valid_head;
                err = 0;
            } else {
                LOG(ERROR) << "fd:" << fd <<", already in epoll set.";
            }
        } else if (op == EPOLL_CTL_DEL || op == EPOLL_CTL_MOD) {
            FakeEpollNode* pre_node = 0;
            FakeEpollNode* ptr = handle->m_valid_head;
            while (ptr) {
                if (ptr->node.sockfd == fd) {
                    err = 0;
                    if (op == EPOLL_CTL_DEL) {
                        //  Remove node
                        if (pre_node)
                            pre_node->next = ptr->next;
                        else
                            handle->m_valid_head = ptr->next;

                        //  Seek cursor pointer
                        if (ptr == handle->m_valid_cursor)
                            handle->m_valid_cursor = ptr->next;

                        //  Save ptr. to empty nodes queue
                        ptr->next = 0;
                        ptr->next = handle->m_empty_head;
                        handle->m_empty_head = ptr;
                    } else if (op == EPOLL_CTL_MOD) {
                        //  找到对应的节点但是两次指向的user data不同,
                        //  修改失败。
                        //
                        //  避免造成节点丢失
                        if (ptr->node.epollevent.data.ptr != ev->data.ptr) {
                            LOG(ERROR) << "fake_epoll_ctl fail, "
                                       "event->data.ptr[0x" << (void*)ev->data.ptr <<
                                       "] != exist "
                                       "ptr->node.epollevent.data.ptr:0x"
                                       << (void*)ptr->node.epollevent.data.ptr;
                            err = -1;
                            break;
                        }
                        ptr->node.epollevent = *ev;
                    }
                    break; // Found item
                }
                pre_node = ptr;
                ptr = ptr->next;
            }
        } else
            err = -1;
    }
    return err;
}

//
//  -1:fail
//  0:time out
//
//  timeout:milliseconds
//
int32_t fake_epoll_wait(EPOLLHANDLE epfd,
                        struct epoll_event* events,
                        int32_t max_events,
                        int32_t timeout) {
    int32_t err = 0;
    FakeEPOLL_HANDLE* handle = reinterpret_cast<FakeEPOLL_HANDLE*>(epfd);
    if (events && max_events &&
            handle &&
            handle != INVALID_HANDLE &&
            handle->m_valid_head) { // ? have valid nodes
        int32_t max_limit = MIN(max_events, FD_SETSIZE);
        int32_t num_err_set_count = 0;

        uint32_t max_fd_val = 0;
        epoll_fd_set rfd;
        epoll_fd_set wfd;
        epoll_fd_set efd;

        //  Seek to head
        handle->m_valid_cursor = handle->m_valid_cursor  ?
                                 handle->m_valid_cursor :
                                 handle->m_valid_head;

        FakeEpollNode* old_cursor = handle->m_valid_cursor;
        while (num_err_set_count < max_limit &&
                (num_err_set_count > 0 ?
                 (handle->m_valid_cursor != old_cursor) :
                 1) // Break cursor loop
              ) {
            if ((handle->m_valid_cursor->node.epollevent.events & EPOLLIN) ==
                    EPOLLIN)
                epoll_FD_SET(handle->m_valid_cursor->node.sockfd,
                             &rfd,
                             handle->m_valid_cursor->node.epollevent.data);

            if ((handle->m_valid_cursor->node.epollevent.events & EPOLLOUT) ==
                    EPOLLOUT)
                epoll_FD_SET(handle->m_valid_cursor->node.sockfd,
                             &wfd,
                             handle->m_valid_cursor->node.epollevent.data);

            epoll_FD_SET(handle->m_valid_cursor->node.sockfd,
                         &efd,
                         handle->m_valid_cursor->node.epollevent.data);
            num_err_set_count++;

            //  Get max fd
            max_fd_val = MAX(handle->m_valid_cursor->node.sockfd, max_fd_val);
            //  Seek to next
            handle->m_valid_cursor = handle->m_valid_cursor->next;
            //  Seek to head
            handle->m_valid_cursor = handle->m_valid_cursor ?
                                     handle->m_valid_cursor :
                                     handle->m_valid_head;
        }

        struct timeval tm = {0};
        tm.tv_sec = timeout/1000;
        tm.tv_usec = (timeout%1000)*1000;
        int32_t result_count = 0;
        uint32_t i = 0;
        int32_t num_fds = select(max_fd_val + 1,
                                 &rfd.set,
                                 &wfd.set,
                                 &efd.set,
                                 &tm);
        if (num_fds == 0)
            err = 0;
        else if (num_fds < 0) {
            memset(events,
                   0,
                   sizeof(struct epoll_event) * max_events);

            err = -1;
#ifdef WIN32
            if (WSAGetLastError() == WSAENOTSOCK)
#else // linux
            if (errno == EBADF)
#endif
            { //
                for (i = 0;
                        i < efd.num_fd_count && result_count < max_events;
                        i++) {
                    struct sockaddr raddr;
                    socklen_t len_addr = sizeof(raddr);
                    if (getpeername(efd.fd_array[i], &raddr, &len_addr) != 0) {
#ifdef WIN32
                        int32_t temp_err = WSAGetLastError();
                        if (temp_err == WSAENOTSOCK)
#else // linux
                        int32_t temp_err = errno;
                        if (temp_err == EBADF)
#endif
                        { // linux
                            events[result_count].data = efd.data_array[i];
                        events[result_count].events = EPOLLERR;
                        result_count++;
                    }
                }
        }
        err = result_count;
    }
} else if (num_fds > 0) {
    memset(events,
           0,
           sizeof(struct epoll_event) * max_events);

    //  Error
    for (i = 0; i < efd.num_fd_count && result_count < max_events; i++) {
        if (SAFE_FD_ISSET(efd.fd_array[i], &efd.set))
            combin_events(events,
                          max_events,
                          &efd.data_array[i],
                          EPOLLERR);
    }

    //  Read fd
    for (i = 0; i < rfd.num_fd_count && result_count < max_events; i++) {
        if (SAFE_FD_ISSET(rfd.fd_array[i], &rfd.set))
            combin_events(events,
                          max_events,
                          &rfd.data_array[i],
                          EPOLLIN);
    }

    //  Write fd
    for (i = 0; i < wfd.num_fd_count && result_count < max_events; i++) {
        if (SAFE_FD_ISSET(wfd.fd_array[i], &wfd.set))
            combin_events(events,
                          max_events,
                          &wfd.data_array[i],
                          EPOLLOUT);
    }
    err = get_valid_count(events, max_events);
}
}
return err;
       }

       void combin_events(struct epoll_event* events, int32_t max_events,
epoll_data_t* data, int32_t num_event) {
    if (events && max_events && data) {
        int32_t i = 0;
        int32_t valid_count = 0;
        char found_it = 0;
        for (i = 0; i < max_events; i++) {
            if (events[i].events == 0)
                break;
            else {
                valid_count++;
                if (memcmp(&events[i].data,
                           data,
                sizeof(epoll_data_t)) == 0) {
                    found_it = 1;
                    break;
                }
            }
        }

        if (!found_it) {
            if (valid_count < max_events) {
                valid_count++;
                found_it = 1;
            }
        }

        if (found_it) {
            memcpy(&(events[valid_count-1].data),
                   data,
                   sizeof(epoll_data_t));

            events[valid_count-1].events |= num_event;
        }
    }
}

int32_t  get_valid_count(struct epoll_event* events, int32_t max_events) {
    int32_t num_count = 0;
    if (events && max_events) {
        for (int32_t i = 0; i < max_events; i++) {
            if (events[i].events != 0)
                num_count++;
            else
                break;
        }
    }
    return num_count;
}

void epoll_FD_SET(uint32_t fd, epoll_fd_set* pset, epoll_data_t user_data) {
    uint32_t i = 0;
    for (i = 0; i < pset->num_fd_count; i++) {
        if (pset->fd_array[i] == fd)
            break;
    }

    if (i == pset->num_fd_count &&
    pset->num_fd_count < FD_SETSIZE) {
        //  Save user data and fd handle
        pset->fd_array[i] = fd;
        pset->data_array[i] = user_data;
        pset->num_fd_count++;

        //  Real FD_SET
        FD_SET(fd, &pset->set);
    }
}

bool epoll_fd_exist(EPOLLHANDLE epfd, SOCKET fd) {
    bool b = false;
    FakeEPOLL_HANDLE* handle = reinterpret_cast<FakeEPOLL_HANDLE*>(epfd);
    if (handle->m_valid_head) {
        FakeEpollNode* ptr = handle->m_valid_head;
        while (ptr) {
            if (ptr->node.sockfd == fd) {
                b = true;
                break;
            }
            ptr = ptr->next;
        }
    }
    return b;
}
#endif // _FAKE_EPOLL

_END_XFS_BASE_NAMESPACE_
