#include "common/rpc/proto_rpc/rpc_controller.h"
#include "common/rpc/proto_rpc/rpc_service.h"

namespace rpc {

RpcController::~RpcController() {
    // Only pending request need to be canceled.
    if (m_status == Status_Pending) {
        if (m_handler) {
            m_handler->CancelRequest(m_sequence_id);
        }
    }
}

} // namespace rpc
