#include "mb_puller.h"
#include "mb_optlock.h"
#include "mb_util.h"

namespace messagebus
{
    MbPullClient::~MbPullClient()
    {
        if (_sock != nullptr) zmq_close(_sock);
    }

    bool MbPullClient::InitializeConnection()
    {
        _sock = zmq_socket(_mb_ctx->GetContext(), ZMQ_PULL);

        int recv_hwm = 0;
        zmq_setsockopt(_sock, ZMQ_RCVHWM, &recv_hwm, sizeof(recv_hwm));
        int rc = zmq_connect(_sock, _url.c_str());

        return (rc == 0);
    }

    bool MbPullClient::Pull(std::string& content)
    {
        content = PullMessage();
        return (content != "");
    }

    std::string MbPullClient::PullMessage()
    {
        OptionalLock sync_lock(_thread_safe ? &_mtx : nullptr);
        std::string ret = "";

        zmq_msg_t part;
        zmq_msg_init(&part);
        int size = zmq_msg_recv(&part, _sock, 0);
        if (size <= 0) return ret;

        ret = ObtainStringFromZmqMessage(&part);
        zmq_msg_close(&part);

        return ret;
    }
}