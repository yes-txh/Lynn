#ifndef COMMON_NETFRAME_WSA_EVENT_POLLER_HPP
#define COMMON_NETFRAME_WSA_EVENT_POLLER_HPP

#ifndef _WIN32
#error for Windows only
#endif

#define WIN32_LEAN_AND_MEAN

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <winsock2.h>
#include "common/base/common_windows.h"
#include "common/netframe/event_poller.hpp"

namespace netframe {

class WsaEventPoller : public EventPoller
{
public:
    WsaEventPoller();
    ~WsaEventPoller();

    /// @brief ��һ��Fd�������¼�
    /// @param fd Fd���ļ�������
    /// @param event_mask ������¼�����
    /// @return �ɹ�����ʧ��
    virtual bool RequestEvent(int fd, unsigned int event_mask);

    /// @brief ��һ��Socket�����������¼�
    /// @param fd Fd���ļ�������
    /// @param event_mask ������¼�����
    /// @return �ɹ�����ʧ��
    virtual bool RerequestEvent(int fd, unsigned int event_mask);

    /// @brief ���һ��Fd�ϵ������¼�����
    /// @param fd �������Fd
    virtual bool ClearEventRequest(int fd);

    /// @brief �ȴ��¼��Ĵ������ú�������
    /// @param events �Ѿ������˵��¼��б�
    /// @retval true ����
    /// @retval false ������������
    bool PollEvents(EventPoller::EventHandler* event_handler);

    bool Interrupt();
private:
    static HWND CreateHiddenWindow();
private:
    static ATOM s_WndClassAtom;
    HWND m_hWndMessage;  ///< ��Ϣ���ڵľ��
    HANDLE m_hEvent;
};

} // namespace netframe

#endif // COMMON_NETFRAME_WSA_EVENT_POLLER_HPP
