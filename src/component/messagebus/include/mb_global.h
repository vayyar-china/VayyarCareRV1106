#pragma once

#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>

#include "mb_context.h"
#include "mb_publisher.h"
#include "mb_pusher.h"
#include "mb_puller.h"
#include "mb_subscriber.h"

namespace messagebus
{
    extern const char* kRequest;
    extern const char* kServerUrl;
    extern const std::string kSplitSymbol;
        // kCmdPub, kCmdSub, kLogPub, kLogSub are url sequence index in 
    // response message from message bus server
    extern const int kCmdPub;   
    extern const int kCmdSub;
    extern const int kCmdPull;
    extern const int kCmdPush;
    
    extern std::shared_ptr<MbContext> g_context;
    extern std::shared_ptr<MbPublishClient> g_publisher;
    extern std::shared_ptr<MbSubscribeClient> g_subscriber;
    extern std::shared_ptr<MbPushClient> g_pusher;   // push log only
    extern std::shared_ptr<MbPullClient> g_puller; 

    class CmsRC
    {
        public:
            static const int CMSMB_OK;
            static const int CMSMB_FETCH_URL;
            static const int CMSMB_PUB_NOT_INIT;
            static const int CMSMB_SUB_NOT_INIT;
            static const int CMSMB_SEND;
            static const int CMSMB_NOBLOCKING_USED;
            static const int CMSMB_NULLPTR_INPUT;
            static const int CMSMB_RECEIVE_FAIL; 
    };
}