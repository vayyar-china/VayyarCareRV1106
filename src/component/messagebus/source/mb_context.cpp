#include "mb_context.h"

namespace messagebus
{
    MbContext::MbContext(bool thread_safe)
    {
        _context = zmq_ctx_new();
        _thread_safe = thread_safe;
    }

    MbContext::~MbContext()
    {
        zmq_ctx_term(_context);
    }

    void* MbContext::GetContext() { return _context; }

    bool MbContext::IsThreadSafe() { return _thread_safe; }
}