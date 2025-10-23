#pragma once

#include <memory>
#include <mutex>

#include "mb_context.h"
#include "mb_optlock.h"

namespace messagebus
{
    class MbClientBase
    {
    public:
        MbClientBase(std::shared_ptr<MbContext>& ctx, const std::string& url)
            : _mb_ctx(ctx), _url(url), _thread_safe(ctx->IsThreadSafe()), _sock(nullptr) {}

        virtual ~MbClientBase() {}
        virtual bool InitializeConnection() = 0;
        virtual void* GetSock() { return _sock; }

    protected:
        std::shared_ptr<MbContext> _mb_ctx;
        std::mutex _mtx;
        std::string _url;
        bool _thread_safe;
        void *_sock;

    private:
        MbClientBase();
        const MbClientBase &operator=(const MbClientBase&);
    };
}