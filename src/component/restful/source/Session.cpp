#include <vector>
#include <string>
#include <iostream>

#include "Session.h"
#include "Util.h"

namespace sysmodule
{

    static int debug_callback(CURL* handle, curl_infotype type, char* data, size_t size, void* userptr)
    {
        // const char* text;
        (void)handle;
        (void)userptr;

        switch (type)
        {
            case CURLINFO_TEXT:
                std::cout << "[Info] " << data;
                break;
            case CURLINFO_HEADER_OUT:
                std::cout << "[HEADER][OUT] " << data;
                break;
            case CURLINFO_HEADER_IN:
                std::cout << "[HEADER][IN] " << data;
                break;
            case CURLINFO_DATA_IN:
            case CURLINFO_DATA_OUT:
            case CURLINFO_SSL_DATA_IN:
            case CURLINFO_SSL_DATA_OUT:
                return 0;
            default:
                return 0;
        }
        return 0;
    }

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

    bool Session::InitWithSsl(const std::string& cert, const std::string& cabundle, const std::string& key, bool debug_callback_on)
    {
        handler_ = curl_easy_init();
        if (!handler_) return false;

        if (debug_callback_on)
        {
            curl_easy_setopt(handler_, CURLOPT_VERBOSE, 1L);
            curl_easy_setopt(handler_, CURLOPT_DEBUGFUNCTION, debug_callback);
        }

        curl_easy_setopt(handler_, CURLOPT_SSLVERSION, CURL_SSLVERSION_MAX_TLSv1_2);
        if (!cert.empty()) curl_easy_setopt(handler_, CURLOPT_SSLCERT, cert.data());
        if (!cabundle.empty()) curl_easy_setopt(handler_, CURLOPT_CAINFO, cabundle.data());
        if (!key.empty()) curl_easy_setopt(handler_, CURLOPT_SSLKEY, key.data());
        curl_easy_setopt(handler_, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(handler_, CURLOPT_SSL_VERIFYHOST, 0L);
        
        curl_easy_setopt(handler_, CURLOPT_TIMEOUT_MS, 5000);

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