#pragma once

#include <curl/curl.h>
#include <string>
#include "Parameter.h"
#include "Response.h"

namespace sysmodule
{
    class Session
    {
    public:
        Session();
        virtual ~Session();
        virtual void SetHeaders(const std::vector<std::string>& headers);
        virtual void SetParameters(const Parameters& params);
        virtual void SetUrl(const std::string& url);
        virtual void SetBody(const std::string& body);
        virtual bool Init();
        virtual bool InitWithSsl(const std::string& cert, const std::string& cabundle, const std::string& key);
        virtual Response Get();
        virtual Response Post();
    private:
        CURL* handler_;
        std::string url_;

        virtual Response Request();

    };
}