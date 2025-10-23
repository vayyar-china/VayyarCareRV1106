#include "mb_util.h"
#include "mb_global.h"

#include <chrono>
#include <thread>
#include <cstring>

namespace messagebus
{
    bool GetConnectionUrl(int timeout, std::vector<std::string> &urls)
    {
        void *ctx = zmq_ctx_new();
        void *s_req = zmq_socket(ctx, ZMQ_REQ);

        int rc = zmq_connect(s_req, kServerUrl);
        if (rc != 0) return false;

        std::this_thread::sleep_for(std::chrono::microseconds(10000));

        int req_length = strlen(kRequest);
        rc = zmq_send(s_req, kRequest, req_length, 0);
        if (req_length != rc) return false;

        rc = zmq_setsockopt(s_req, ZMQ_RCVTIMEO, &timeout, sizeof(int));
        if (rc != 0) return false;

        zmq_msg_t part;
        zmq_msg_init(&part);
        int size = zmq_msg_recv(&part, s_req, 0);
        if (size == -1) return false;

        std::string part_str = ObtainStringFromZmqMessage(&part);
        zmq_msg_close(&part);

        StringSplit(part_str, kSplitSymbol, urls);

        zmq_close(s_req);
        zmq_ctx_term(ctx);

        return true;
    }

    void StringSplit(const std::string &s, const std::string &delim, std::vector<std::string> &ret)
    {
        size_t start_pos = 0;
        size_t index = s.find_first_of(delim, start_pos);

        while (index != std::string::npos)
        {
            ret.push_back(s.substr(start_pos, index - start_pos));
            start_pos = index + 1;
            index = s.find_first_of(delim, start_pos);
        }

        if (index - start_pos > 0)
        {
            ret.push_back(s.substr(start_pos, index - start_pos));
        }
    }

    std::string ObtainStringFromZmqMessage(zmq_msg_t *msg)
    {
        if (msg == nullptr)
        {
            return "";
        }

        const char* data = static_cast<const char*>(zmq_msg_data(msg));
        size_t size = zmq_msg_size(msg);

        std::string str(data, size);
        return str;
    } 
}