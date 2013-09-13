#include "common/net/http/http_client.h"
#include "glog/logging.h"

namespace poppy {

bool HttpClient::ConnectServer(const std::string& server_address,
        HttpHandler* handler) {
    SocketAddressInet address(server_address);
    HttpClientConnection* connection = new HttpClientConnection(this, handler);
    int64_t socket_id = m_net_frame->AsyncConnect(
            address,
            connection,
            kMaxMessageSize);
    if (socket_id < 0) {
        LOG(ERROR) << "Failed to connect to server: " << server_address;
        delete connection;
        return false;
    }
    m_connection_manager.AddConnection(socket_id);
    return true;
}

} // namespace poppy
