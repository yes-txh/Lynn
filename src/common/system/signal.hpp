#ifndef COMMON_SYSTEM_SIGNAL_HPP
#define COMMON_SYSTEM_SIGNAL_HPP

//////////////////////////////////////////////////////////////////////////
/// posix signal

#include <signal.h>

#ifdef __unix__
/// signal set
class SignalSet
{
public:
    SignalSet()
    {
        Clear();
    }

    bool Clear()
    {
        return sigemptyset(&m_set) == 0;
    }

    bool Fill()
    {
        return sigfillset(&m_set) == 0;
    }

    bool Add(int signum)
    {
        return sigaddset(&m_set, signum) == 0;
    }

    bool Delete(int signum)
    {
        return sigdelset(&m_set, signum) == 0;
    }

    bool IsMember(int signum) const
    {
        return sigismember(&m_set, signum) == 1;
    }

    bool IsEmpty() const
    {
        return sigisemptyset(&m_set) == 1;
    }

    const sigset_t* Address() const
    {
        return &m_set;
    }

    sigset_t* Address()
    {
        return &m_set;
    }
private:
    sigset_t m_set;
};

struct Signal
{
    static bool Send(pid_t pid, int signo);
    static bool Send(pid_t pid, int signo, const union sigval value);
    static bool SetMask(const SignalSet& set);
    static bool GetMask(SignalSet* set);
    static bool Block(const SignalSet& set);
    static bool Unblock(const SignalSet& set);
    static void Suspend(const SignalSet& set);
    static bool GetPending(SignalSet* set);
};
#endif // __unix__

#endif // COMMON_SYSTEM_SIGNAL_HPP
