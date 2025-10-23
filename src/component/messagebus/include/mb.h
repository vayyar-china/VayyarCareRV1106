#pragma once

#include <string>
#include <vector>

#define EC_MB_OK 				0
#define EC_MB_FETCH_URL 		1
#define EC_MB_PUB_NOT_INIT 		2
#define EC_MB_SUB_NOT_INIT 		3
#define EC_MB_PULL_NOT_INIT		4
#define EC_MB_PUSH_NOT_INIT		5
#define EC_MB_SEND 				6
#define EC_MB_NOBLOCKING_USED 	7
#define EC_MB_NULLPTR_INPUT 	8
#define EC_MB_RECEIVE_FAIL 		9

#define CLIENT_CMD_PUB			0x0001
#define CLIENT_CMD_SUB			0x0002
#define CLIENT_CMD_PULL			0x0004
#define CLIENT_CMD_PUSH			0x0008
#define CLIENT_ALL				0xFFFF

typedef std::vector<std::string> MessageBusTopics;

struct MessageFrame
{
    std::string topic;
    std::string content;
};

typedef void (on_message_callback) (MessageFrame*);

namespace messagebus
{
    int mb_ipc_init(bool thread_safe, int flag = CLIENT_ALL);
    int mb_pub(const std::string& topic, void* data, size_t data_len);
    int mb_push(const std::string& msg);
    int mb_pull(std::string& msg);
    int mb_sub(MessageBusTopics& topic, on_message_callback *msg_cb);
    int mb_recv(MessageFrame* msg_frame);
}