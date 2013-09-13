#ifndef COMMON_NETFRAME_EPOLL_EVENT_POLLER_HPP
#define COMMON_NETFRAME_EPOLL_EVENT_POLLER_HPP

#include <vector>
#include <sys/epoll.h>
#include "common/base/unique_ptr.hpp"
#include "common/netframe/event_poller.hpp"
#include "common/netframe/pipe_interrupter.hpp"
#include "common/netframe/eventfd_interrupter.hpp"

namespace netframe {

/// EventPoller using linux epoll
class EpollEventPoller : public EventPoller
{
public:
    EpollEventPoller(unsigned int max_fds = 0x10000);
    virtual ~EpollEventPoller();

    virtual bool RequestEvent(int fd, unsigned int event_mask);
    virtual bool RerequestEvent(int fd, unsigned int event_mask);
    virtual bool ClearEventRequest(int fd);
    virtual bool PollEvents(EventPoller::EventHandler* event_handler);
    virtual bool Interrupt();
private:
    int m_EpollFd;
    std::vector<epoll_event> m_EpollEvents;  ///< epoll event array
    //PipeInterrupter m_Interrupter;
    unique_ptr<Interrupter> m_Interrupter;
};

} // namespace netframe

#endif // COMMON_NETFRAME_EPOLL_EVENT_POLLER_HPP
