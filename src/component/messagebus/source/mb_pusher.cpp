#include "mb_pusher.h"
#include "mb_optlock.h"

namespace messagebus
{
    MbPushClient::~MbPushClient()
    {
        if (_sock != nullptr) zmq_close(_sock);
    }

    bool MbPushClient::InitializeConnection()
    {
        _sock = zmq_socket(_mb_ctx->GetContext(), ZMQ_PUSH);

        int snd_hwm = 0;
        zmq_setsockopt(_sock, ZMQ_SNDHWM, &snd_hwm, sizeof(snd_hwm));
        int rc = zmq_connect(_sock, _url.c_str());

        return (rc == 0);
    }

    bool MbPushClient::Push(const std::string& content)
    {
        OptionalLock sync_lock(_thread_safe ? &_mtx : nullptr);

        if (content.empty()) return false;

        int length = content.size();
        int rc = zmq_send(_sock, content.c_str(), length, 0);

        if (length != rc) return false;
        return true;
    }
}