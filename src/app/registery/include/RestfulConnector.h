#pragma once

#include <memory>
#include <vector>

#include "Session.h"

namespace sysmodule
{
    class RestfulConnector
    {

    public:
        static RestfulConnector &GetInstance()
        {
            static RestfulConnector instance;
            return instance;
        }

        std::shared_ptr<Session> CreateSession();
        std::shared_ptr<Session> CreateSessionWithSsl(const std::string& cert, const std::string& cabundle, const std::string& key);

        Response RegisterDevice();

    private:
        RestfulConnector();
        ~RestfulConnector();
    };
    

    
}