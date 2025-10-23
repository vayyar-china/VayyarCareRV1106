#pragma once

#include "mb_client.h"

namespace messagebus
{
    class MbPublishClient : public MbClientBase
    {
    public:
        MbPublishClient(std::shared_ptr<MbContext>& ctx, const std::string& url) 
            : MbClientBase(ctx, url) {}
        ~MbPublishClient();
        virtual bool InitializeConnection();
        bool Publish(const std::string& topic, const void* payload, size_t data_len);

    private:
        MbPublishClient operator=(const MbPublishClient&);
        MbPublishClient(const MbPublishClient&);
    };
}