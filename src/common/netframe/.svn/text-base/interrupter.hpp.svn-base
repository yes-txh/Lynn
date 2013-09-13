#ifndef COMMON_NETFRAME_INTERRUPTER_HPP
#define COMMON_NETFRAME_INTERRUPTER_HPP

#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>

namespace netframe {

class Interrupter
{
public:
    /// Destructor.
    virtual ~Interrupter() { }

    /// Interrupt the select call.
    virtual bool Interrupt() = 0;

    /// Reset the select interrupt. Returns true if the call was interrupted.
    virtual bool Reset() = 0;

    /// Get the read descriptor to be passed to select.
    virtual int GetReadFd() const = 0;
};

} // namespace netframe

#endif // COMMON_NETFRAME_INTERRUPTER_HPP

