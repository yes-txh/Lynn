#ifndef COMMON_NETFRAME_COMMAND_QUEUE_HPP
#define COMMON_NETFRAME_COMMAND_QUEUE_HPP

#include <deque>
#include "common/base/uncopyable.hpp"
#include "common/netframe/command_event.hpp"

namespace netframe {

struct CommandEvent;

class CommandQueue
{
public:
    CommandQueue(size_t max_length);
    ~CommandQueue();
    bool IsEmpty() const;
    size_t Size() const;
    bool HasMore() const; ///< ���в���
    bool GetFront(CommandEvent* event);
    bool PopFront();

    /// @brief �������
    /// @param event ����¼�
    /// @param force �Ƿ�ǿ����ӣ���ǿ����ӣ�������������
    /// @retval true �ɹ�
    bool Enqueue(const CommandEvent& event, bool force = false);

    /// @brief �������Ȳ�ӣ�������������
    void EnqueueUrgent(const CommandEvent& event);

    DECLARE_UNCOPYABLE(CommandQueue);

private:
    std::deque<CommandEvent> m_Queue;
    size_t m_MaxLength;
};

inline bool CommandQueue::IsEmpty() const
{
    return m_Queue.empty();
}

inline size_t CommandQueue::Size() const
{
    return m_Queue.size();
}

inline bool CommandQueue::HasMore() const
{
    return m_Queue.size() > 1;
}

} // namespace netframe

#endif // COMMOM_NETFRAME_COMMAND_QUEUE_HPP
