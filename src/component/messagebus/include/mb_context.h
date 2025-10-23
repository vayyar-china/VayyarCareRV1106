#pragma once

#include <string>

#include <zmq/zmq.h>

namespace messagebus
{
    class MbContext
    {
    public:
        MbContext(bool thread_safe);
        ~MbContext();

        void *GetContext();
        bool IsThreadSafe();

    private:
        void *_context;
        bool _thread_safe;

        MbContext();
        MbContext(MbContext const &);
        void operator=(MbContext const &);
    };
}