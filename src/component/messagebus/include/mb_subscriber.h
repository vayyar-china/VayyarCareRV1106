#pragma once

#include "mb.h"
#include "mb_client.h"
#include "mb_context.h"

#include <vector>
#include <thread>
#include <atomic>
#include <set>

namespace messagebus
{
    class MbSubscribeClient : public MbClientBase
    {
    public:
        MbSubscribeClient(std::shared_ptr<MbContext>& ctx, const std::string& url)
            : MbClientBase(ctx, url) 
        {
            _first_noblocking_recv = false;
            _thread_recv = nullptr;
            _stop_thread = false;
        }
        ~MbSubscribeClient();
        virtual bool InitializeConnection();
        bool Subscribe(const std::vector<std::string> topics, on_message_callback * msg_cb);
        static void Receive(void *pthis);
        const std::set<std::string>& GetTopics();
        bool ReceiveMessage(MessageFrame* msg_frame);
        bool NoblockingUsed();
        const std::atomic<bool>& GetStopThreadFlag() { return _stop_thread; }
    private:
        std::set<std::string> _subscribed_topics;
        std::atomic<bool> _first_noblocking_recv;
        std::atomic<bool> _stop_thread;
        on_message_callback *_msg_cb;
        std::thread *_thread_recv;

        MbSubscribeClient& operator=(const MbSubscribeClient&);
        MbSubscribeClient(const MbSubscribeClient&);
    };
}