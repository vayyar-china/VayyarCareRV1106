#pragma once

#include <string>

#include "mb_client.h"
#include "mb_context.h"

namespace messagebus
{
    class MbPushClient : public MbClientBase
    {
    public:
        MbPushClient(std::shared_ptr<MbContext>& ctx, const std::string& url)
            : MbClientBase(ctx, url) {}
        ~MbPushClient();
        virtual bool InitializeConnection();
        bool Push(const std::string& context);

    private:
        MbPushClient& operator=(const MbPushClient&);
        MbPushClient(const MbPushClient&);
    };
}