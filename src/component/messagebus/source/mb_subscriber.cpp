#include "mb_subscriber.h"
#include "mb_util.h"

namespace messagebus
{
    MbSubscribeClient::~MbSubscribeClient()
    {
        if (_thread_recv != nullptr)
        {
            _stop_thread = true;
            _thread_recv->join();
            delete _thread_recv;
        }

        if (_sock != nullptr) zmq_close(_sock);
    }

    bool MbSubscribeClient::InitializeConnection()
    {
        _sock = zmq_socket(_mb_ctx->GetContext(), ZMQ_SUB);
        int rc = zmq_connect(_sock, _url.c_str());
        return (rc == 0);
    }

    bool MbSubscribeClient::Subscribe(const std::vector<std::string> topics, on_message_callback *msg_cb)
    {
        OptionalLock sync_lock(_thread_safe ? &_mtx : nullptr);

        int rc = 0;
        for (size_t index = 0; index < topics.size(); ++index)
        {
            rc = zmq_setsockopt(_sock, ZMQ_SUBSCRIBE, topics[index].c_str(), topics[index].size());
            _subscribed_topics.insert(topics[index]);
            if (rc != 0) return false;
        }

        if (_msg_cb != nullptr)
        {
            _msg_cb = msg_cb;

            if (_first_noblocking_recv)
            {
                int timeout = 500;
                zmq_setsockopt(_sock, ZMQ_RCVTIMEO, &timeout, sizeof(int));
                _first_noblocking_recv = !_first_noblocking_recv;
                _thread_recv = new std::thread(Receive, (void*)this);
            }
        }

        return true;
    }

    bool MbSubscribeClient::ReceiveMessage(MessageFrame *msg_frame)
    {
        int more;
        OptionalLock sync_lock(_thread_safe ? &_mtx : nullptr);
        std::vector<std::string> msg;
        size_t more_size = sizeof(more);

        do
        {
            zmq_msg_t part;
            zmq_msg_init(&part);
            int size = zmq_msg_recv(&part, _sock, 0);
            if (size <= 0) break;

            std::string msg_str = ObtainStringFromZmqMessage(&part);
            zmq_getsockopt(_sock, ZMQ_RCVMORE, &more, &more_size);
            zmq_msg_close(&part);

            if (msg_str.size() != 0) msg.push_back(msg_str);

        } while (more);
        
        if (msg.size() > 1) 
        {
            msg_frame->topic = msg[0];
            msg_frame->content = msg[1];
            return true;
        }
        else
        {
            return false;
        }
    }

    void MbSubscribeClient::Receive(void *pthis)
    {
        MbSubscribeClient *client = (MbSubscribeClient*)pthis;

        while (!client->GetStopThreadFlag())
        {   
            MessageFrame msgframe;
            if (client->ReceiveMessage(&msgframe))
            {
                client->_msg_cb(&msgframe);
            }
        }
        
    }

    const std::set<std::string>& MbSubscribeClient::GetTopics()
    {
        return _subscribed_topics;
    }

    bool MbSubscribeClient::NoblockingUsed()
    {
        return _first_noblocking_recv;
    }
}