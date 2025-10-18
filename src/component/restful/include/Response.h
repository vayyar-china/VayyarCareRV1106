#pragma once

#include <string>

namespace sysmodule
{
    struct Response
    {

        Response():res_code(-1), elapsed(0), ret_code(-1), error(""), raw_url(""){}
        Response(int res_code_, double elapsed_, int ret_code_, std::string error_, std::string raw_url_, std::string res_string_, std::string header_string_):
                    res_code(res_code_), 
                    elapsed(elapsed_), 
                    ret_code(ret_code_), 
                    error(error_), 
                    raw_url(raw_url_), 
                    res_string(res_string_), 
                    header_string(header_string_){}

        Response& operator=(const Response& other)
        {
            if (this != &other)
            {
                this->ret_code = other.ret_code;
                this->elapsed = other.elapsed;
                this->ret_code = other.ret_code;
                this->error = other.error;
                this->raw_url = other.raw_url;
                this->res_string = other.res_string;
                this->header_string = other.header_string;
            }
            return *this;
        }

        bool Invalid()
        {
            return ((ret_code == -1) && (error.empty()));
        }

        int res_code;       // url response code
        double elapsed;
        int ret_code;       // curl api perform return code
        std::string error;
        std::string raw_url;
        std::string res_string;
        std::string header_string;
    };
}