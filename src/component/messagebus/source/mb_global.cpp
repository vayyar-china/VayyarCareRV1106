#include "mb_global.h"

namespace messagebus
{
    const char *kRequest = "cmsmb_url_request";

    const char *kServerUrl = "ipc:///tmp/req.ipc";

    const std::string kSplitSymbol = "#";

    const int kCmdPub = 0;
    const int kCmdSub = 1;
    const int kCmdPush = 2;
    const int kCmdPull = 3;


    std::shared_ptr<MbContext> g_context;
    std::shared_ptr<MbPublishClient> g_publisher;
    std::shared_ptr<MbSubscribeClient> g_subscriber;
    std::shared_ptr<MbPushClient> g_pusher;   // push log only
    std::shared_ptr<MbPullClient> g_puller;
}