#include "mb_publisher.h"
#include "mb_optlock.h"

#include <string>
#include <iostream>

namespace messagebus
{
    MbPublishClient::~MbPublishClient()
    {
        if (_sock != nullptr) zmq_close(_sock);
    }

    bool MbPublishClient::InitializeConnection()
    {
        _sock = zmq_socket(_mb_ctx->GetContext(), ZMQ_PUB);
        int rc = zmq_connect(_sock, _url.c_str());
        return (rc == 0);
    }

    bool MbPublishClient::Publish(const std::string& topic, const void* payload, size_t data_len)
    {
        OptionalLock sync_lock(_thread_safe ? &_mtx : nullptr);

        int topic_length = topic.size();
        int rc = zmq_send(_sock, topic.c_str(), topic_length, ZMQ_SNDMORE);

        if (topic_length != rc) return false;
        rc = zmq_send(_sock, payload, data_len, 0);

        if ((int)data_len != rc) return false;

        return true;
    }
}
