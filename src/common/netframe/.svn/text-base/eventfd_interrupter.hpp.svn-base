#ifndef COMMON_NETFRAME_EVENTFD_INTERRUPTER_HPP
#define COMMON_NETFRAME_EVENTFD_INTERRUPTER_HPP

#include "common/netframe/eventfd.h"
#include <stdexcept>

#include "common/netframe/interrupter.hpp"

namespace netframe {

class EventFdInterrupter : public Interrupter
{
public:
    // Constructor.
    EventFdInterrupter()
    {
        m_Fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        if (m_Fd < 0)
            throw std::runtime_error("EventFdInterrupter");
    }

    // Destructor.
    ~EventFdInterrupter()
    {
        ::close(m_Fd);
    }

    // Interrupt the select call.
    bool Interrupt()
    {
        return eventfd_write(m_Fd, 1) == 0;
    }

    // Reset the select interrupt. Returns true if the call was interrupted.
    bool Reset()
    {
        eventfd_t data;
        return eventfd_read(m_Fd, &data) == 0;
    }

    // Get the read descriptor to be passed to select.
    int GetReadFd() const
    {
        return m_Fd;
    }

private:
    int m_Fd;
};

} // namespace netframe

#endif // COMMON_NETFRAME_EVENTFD_INTERRUPTER_HPP

