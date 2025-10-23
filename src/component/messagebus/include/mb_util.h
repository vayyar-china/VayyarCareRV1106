#pragma once

#include <vector>
#include <string>

#include <zmq/zmq.h>

namespace messagebus
{
    bool GetConnectionUrl(int timeout, std::vector<std::string> &urls);

    void StringSplit(const std::string &s, const std::string &delim, std::vector<std::string> &ret);

    std::string ObtainStringFromZmqMessage(zmq_msg_t *msg);
}