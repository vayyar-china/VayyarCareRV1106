#include "mb.h"
#include "mb_util.h"
#include "mb_global.h"

#include <memory>

namespace messagebus
{
    int mb_ipc_init(bool thread_safe, int flag)
    {
        g_publisher.reset();
        g_subscriber.reset();
        g_puller.reset();
        g_pusher.reset();
        
        g_context.reset();
        g_context = std::make_shared<MbContext>(thread_safe);

        std::vector<std::string> urls;

        if (!GetConnectionUrl(3000, urls)) return EC_MB_FETCH_URL;

        if (CLIENT_CMD_PUB & flag)
        {
            g_publisher = std::make_shared<MbPublishClient>(g_context, urls[kCmdPub]);
            if (!g_publisher->InitializeConnection()) return EC_MB_PUB_NOT_INIT;
        }

        if (CLIENT_CMD_SUB & flag)
        {
            g_subscriber = std::make_shared<MbSubscribeClient>(g_context, urls[kCmdSub]);
            if (!g_subscriber->InitializeConnection()) return EC_MB_SUB_NOT_INIT;
        }

        if (CLIENT_CMD_PULL & flag)
        {
            g_puller = std::make_shared<MbPullClient>(g_context, urls[kCmdPull]);
            if (!g_puller->InitializeConnection()) return EC_MB_PULL_NOT_INIT;
        }

        if (CLIENT_CMD_PUSH & flag)
        {
            g_pusher = std::make_shared<MbPushClient>(g_context, urls[kCmdPush]);
            if (!g_pusher->InitializeConnection()) return EC_MB_PUSH_NOT_INIT;
        }

        std::this_thread::sleep_for(std::chrono::microseconds(10000));

        return EC_MB_OK;
    }

    int mb_pub(const std::string& topic, void* data, size_t data_len)
    {
        if (!g_publisher) return EC_MB_PUB_NOT_INIT;
        return (g_publisher->Publish(topic, data, data_len) ? 0 : -1);
    }

    int mb_push(const std::string& msg)
    {
        if (!g_pusher) return EC_MB_PUSH_NOT_INIT;
        return (g_pusher->Push(msg) ? 0 : -1);
    }

    int mb_pull(std::string& msg)
    {
        if (!g_puller) return EC_MB_PULL_NOT_INIT;
        return (g_puller->Pull(msg) ? 0 : -1);
    }

    int mb_sub(MessageBusTopics& topic, on_message_callback *msg_cb)
    {
        if (!g_subscriber) return EC_MB_SUB_NOT_INIT;
        return (g_subscriber->Subscribe(topic, msg_cb) ? 0 : -1);
    }

    int mb_recv(MessageFrame* msg_frame)
    {
        if (msg_frame == nullptr) return EC_MB_NULLPTR_INPUT;
        if (!g_subscriber) return EC_MB_SUB_NOT_INIT;

        msg_frame->topic = "";
        msg_frame->content = "";

        auto psub = g_subscriber;
        if (psub->NoblockingUsed()) return EC_MB_NOBLOCKING_USED;
        if (!(psub->ReceiveMessage(msg_frame))) return EC_MB_RECEIVE_FAIL;

        return EC_MB_OK;
    }
}