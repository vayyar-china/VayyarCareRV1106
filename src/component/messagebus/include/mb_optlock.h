#pragma once

#include <mutex>

namespace messagebus
{
    class OptionalLock
    {
    public:
        OptionalLock (std::mutex* mutex): _mutex(mutex)
        {
            if(_mutex != nullptr)
                _mutex->lock ();
        }

        ~OptionalLock ()
        {
            if(_mutex != nullptr)
                _mutex->unlock ();
        }

    private:

        std::mutex* _mutex;
        OptionalLock (const OptionalLock&);
        const OptionalLock &operator = (const OptionalLock&);
    };
}