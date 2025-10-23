#pragma once

#include <atomic>

#include "mb_client.h"
#include "mb_context.h"

namespace messagebus
{
    class MbPullClient : public MbClientBase
    {
    public:
        MbPullClient(std::shared_ptr<MbContext>& ctx, const std::string& url)
            : MbClientBase(ctx, url) {}
        ~MbPullClient();
        virtual bool InitializeConnection();
        bool Pull(std::string& context);
        std::string PullMessage();

    private:
        MbPullClient& operator=(const MbPullClient&);
        MbPullClient(const MbPullClient&);
    };
}