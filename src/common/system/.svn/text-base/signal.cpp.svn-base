#include <stddef.h>
#include "common/system/signal.hpp"

#ifdef __unix__

bool Signal::Send(pid_t pid, int signo)
{
    return kill(pid, signo) == 0;
}

bool Signal::Send(pid_t pid, int signo, const union sigval value)
{
    return sigqueue(pid, signo, value);
}

bool Signal::SetMask(const SignalSet& set)
{
    return sigprocmask(SIG_SETMASK, set.Address(), NULL) == 0;
}

bool Signal::GetMask(SignalSet* set)
{
    return sigprocmask(0, NULL, set->Address()) == 0;
}

bool Signal::Block(const SignalSet& set)
{
    return sigprocmask(SIG_BLOCK, set.Address(), NULL) == 0;
}

bool Signal::Unblock(const SignalSet& set)
{
    return sigprocmask(SIG_UNBLOCK, set.Address(), NULL) == 0;
}

void Signal::Suspend(const SignalSet& set)
{
    sigsuspend(set.Address());
}

bool Signal::GetPending(SignalSet* set)
{
    return sigpending(set->Address()) == 0;
}

#endif // __unix__
