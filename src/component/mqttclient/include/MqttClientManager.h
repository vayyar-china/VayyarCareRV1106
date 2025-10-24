#pragma once

#include "MqttClient.h"

#include <map>
#include <memory>
#include <vector>
#include <string>

class MqttClientManager
{
public:
    static MqttClientManager& instance();

    void init(const std::string& product_key,
            const std::string& device_name,
            const std::string& device_secret,
            const std::string& uri,
            const std::string& region = "cn-shanghai");
    bool connect();
    void disconnect();
    bool publish(const std::string& topic, const std::string& payload, int qos = 1);
    bool subscribe(const std::string& topic, int qos = 1);

    void register_task(const std::string& task_name, 
                    const std::vector<std::string>& topics,
                    MqttMessageDispatcher::MessageCallback callback);
    
    void unregister_task(const std::string& task_name);

    // std::string get_shadow_sub_topic() const;
    // std::string get_shadow_update_topic() const;
    // std::string get_state_report_topic() const;

private:
    MqttClientManager() = default;
    ~MqttClientManager();

    std::unique_ptr<AliyunMqttClient> _client;
    std::unique_ptr<MqttMessageDispatcher> _dispatcher;
    std::map<std::string, std::unique_ptr<MqttTask>> _tasks;
};