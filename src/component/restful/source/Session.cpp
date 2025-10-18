#include <vector>
#include <string>

#include "Session.h"
#include "Util.h"

namespace sysmodule
{
    Session::Session()
    {
        // the global init should put in restful client
        url_ = "";
    }

    bool Session::Init()
    {
        handler_ = curl_easy_init();
        return (handler_ != nullptr);
    }

    bool Session::InitWithSsl(const std::string& cert, const std::string& cabundle, const std::string& key)
    {
        handler_ = curl_easy_init();
        if (!handler_) return false;

        curl_easy_setopt(handler_, CURLOPT_SSLENGINE_DEFAULT, 1L);
        curl_easy_setopt(handler_, CURLOPT_SSLCERT, cert.data());
        curl_easy_setopt(handler_, CURLOPT_CAINFO, cabundle.data());
        curl_easy_setopt(handler_, CURLOPT_SSLKEY, key.data());
        curl_easy_setopt(handler_, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(handler_, CURLOPT_SSL_VERIFYHOST, 2L);

        return (CURLE_OK == curl_easy_perform(handler_));
    }

    Session::~Session()
    {
        if (handler_)
            curl_easy_cleanup(handler_);
    }

    void Session::SetHeaders(const std::vector<std::string> &headers)
    {
        struct curl_slist *chunk = NULL;
        for (auto &header : headers)
        {
            chunk = curl_slist_append(chunk, header.data());
            curl_easy_setopt(handler_, CURLOPT_HTTPHEADER, chunk);
        }
    }

    void Session::SetParameters(const Parameters &param)
    {
        if (param.Empty()) return;

        url_ += "?";
        url_ += param.GetParamUrl();
    }

    void Session::SetUrl(const std::string& url)
    {
        url_ = url;
    }

    void Session::SetBody(const std::string& body)
    {
        if (handler_) 
        {
            curl_easy_setopt(handler_, CURLOPT_POSTFIELDSIZE, body.length());
            curl_easy_setopt(handler_, CURLOPT_POSTFIELDS, body.data());
        }
    }

    Response Session::Get()
    {
        if (!handler_) return Response();

        curl_easy_setopt(handler_, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(handler_, CURLOPT_POST, 0L);

        return Request();
    }

    Response Session::Post()
    {
        if (!handler_) return Response();

        curl_easy_setopt(handler_, CURLOPT_HTTPGET, 0L);
        curl_easy_setopt(handler_, CURLOPT_POST, 1L);

        return Request();
    }

    Response Session::Request()
    {
        curl_easy_setopt(handler_, CURLOPT_URL, url_.data());

        std::string response_string = "";
        std::string header_string = "";
        curl_easy_setopt(handler_, CURLOPT_WRITEFUNCTION, WriteFunction);
        curl_easy_setopt(handler_, CURLOPT_WRITEDATA, &response_string);
        curl_easy_setopt(handler_, CURLOPT_HEADERDATA, &header_string);

        auto ret = curl_easy_perform(handler_);
        auto error = std::string(curl_easy_strerror(ret));
        char* raw_url;
        long response_code;
        double elapsed;
        curl_easy_getinfo(handler_, CURLINFO_RESPONSE_CODE, &response_code);
        curl_easy_getinfo(handler_, CURLINFO_TOTAL_TIME, &elapsed);
        curl_easy_getinfo(handler_, CURLINFO_EFFECTIVE_URL, &raw_url);

        return Response(response_code, elapsed, ret, error, raw_url, response_string, header_string);

    }
}