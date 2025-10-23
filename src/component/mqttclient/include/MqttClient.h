#pragma once

#include <string>
#include <functional>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <ctime>
#include <chrono>
#include <cstring>
#include <queue>
#include <mutex>
#include <map>
#include <condition_variable>
#include <thread>
#include <atomic>

#include <paho-mqtt/MQTTClient.h>
#include <openssl/ssl.h>

class MqttMessageDispatcher
{
public:
    struct MqttMessage
    {
        std::string topic;
        std::string payload;
    };

    using MessageCallback = std::function<void(const MqttMessage&)>;
    using MessageQueue = std::queue<MqttMessage>;

    MqttMessageDispatcher();
    ~MqttMessageDispatcher();

    void stop();
    void register_task_queue(const std::string& task_name, MessageQueue* queue);
    void unregister_task_queue(const std::string& task_name);
    void register_topic_callback(const std::string& topic, MessageCallback callback);
    void unregister_topic_callback(const std::string& topic);
    void register_notifier(const std::string& task_name, std::function<void()> notifier);
    void unregister_notifier(const std::string& task_name);
    void add_message(const MqttMessage& message);
    bool is_topic_match(const std::string& pattern, const std::string& topic);

private:
    void dispatch_loop();
    void dispatch_to_topic_callbacks(const MqttMessage& message);
    void dispatch_to_task_queues(const MqttMessage& message);

    std::vector<std::string> split(const std::string& str, char delimiter);

    std::mutex _mutex;
    std::condition_variable _cv;
    std::queue<MqttMessage> _message_queue;
    std::map<std::string, MessageQueue*> _task_queues;
    std::map<std::string, MessageCallback> _topic_callbacks;
    std::map<std::string, std::function<void()>> _notifiers;
    std::thread _dispatcher_thread;
    std::atomic<bool> _running;
};

class MqttTask
{
public:
    MqttTask(const std::string& name, MqttMessageDispatcher& dispatcher);
    ~MqttTask();

    void stop();
    void add_message_handler(const std::string& topic, MqttMessageDispatcher::MessageCallback callback);

private:
    void run();
    void process_message(const MqttMessageDispatcher::MqttMessage& message);

    std::string _name;
    MqttMessageDispatcher& _dispatcher;
    std::mutex _mutex;
    std::condition_variable _cv;
    MqttMessageDispatcher::MessageQueue _task_queue;
    std::map<std::string, MqttMessageDispatcher::MessageCallback> _handlers;
    std::thread _task_thread;
    std::atomic<bool> _running;
};

class AliyunMqttClient
{
public:
    using ConnectCallback = std::function<void(bool success, const std::string& message)>;
    using MessageCallback = std::function<void(const std::string& topic, const std::string& payload)>;

    AliyunMqttClient(const std::string& product_key,
                    const std::string& device_name,
                    const std::string& device_secret,
                    const std::string& uri,
                    const std::string& region = "cn-shanghai");
    ~AliyunMqttClient();

    bool connect();
    void disconnect();

    bool publish(const std::string& topic, const std::string& payload, int qos = 1);
    bool subscribe(const std::string& topic, int qos = 1);

    void set_message_callback(MessageCallback callback);

    void get_device_info(std::string& product_key, std::string& device_name, std::string& device_secret) const;

    bool is_connected() const;

    void set_message_dispatcher(MqttMessageDispatcher* dispatcher);

    static int message_arrived(void* context, char* topic_name, int topic_len, MQTTClient_message* message)
    {
        AliyunMqttClient* client = static_cast<AliyunMqttClient*>(context);
        if (client)
        {
            std::cout << "[Aliyun][MQTTClient][" << topic_name << "] message arrived" << std::endl;
            std::string topic(topic_name, topic_len);
            std::string payload(static_cast<char*>(message->payload), message->payloadlen);

            MqttMessageDispatcher::MqttMessage msg{topic, payload};

            if (client->_dispatcher)
            {
                client->_dispatcher->add_message(msg);
            }

            // if (client->_message_callback)
            // {
            //     client->_message_callback(topic, payload);
            // }
        }

        MQTTClient_freeMessage(&message);
        MQTTClient_free(topic_name);

        return 1;
    }

    std::string ali_shadow_sub_topic;

    std::string ali_shadow_update_pub_topic;
    std::string ali_state_report_pub_topic;
    std::string ali_batch_update_pub_topic;
    std::string ali_publish_topic_prefix;

private:
    std::string generate_password();

    std::string error_string(int rc);

    // static int message_arrived(void* content, char* topic_name, int topic_len, MQTTClient_message* message)
    // {
    //     AliyunMqttClient* client = static_cast<AliyunMqttClient*>(content);
    //     if (client && client->_message_callback)
    //     {
    //         std::string topic(topic_name);
    //         std::string payload(static_cast<char*>(message->payload), message->payloadlen);
    //         client->_message_callback(topic, payload);
    //     }

    //     MQTTClient_freeMessage(&message);
    //     MQTTClient_free(topic_name);

    //     return 1;
    // }

    std::string _product_key;
    std::string _device_name;
    std::string _device_secret;
    std::string _server_uri;
    std::string _region;
    std::string _client_id;
    std::string _timestamp;
    
    MQTTClient _client;
    bool _connected;
    
    MessageCallback _message_callback;
    
    MqttMessageDispatcher* _dispatcher = nullptr;
};
