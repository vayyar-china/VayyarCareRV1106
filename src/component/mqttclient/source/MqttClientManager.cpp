#include "MqttClientManager.h"

MqttClientManager& MqttClientManager::instance()
{
    static MqttClientManager instance;
    return instance;
}

void MqttClientManager::init(const std::string& product_key,
                            const std::string& device_name,
                            const std::string& device_secret,
                            const std::string& uri,
                            const std::string& region)
{
    _client = std::make_unique<AliyunMqttClient>(product_key, device_name, device_secret, uri, region);
    _dispatcher = std::make_unique<MqttMessageDispatcher>();
    _client->set_message_dispatcher(_dispatcher.get());
}

bool MqttClientManager::connect()
{
    if (_client && _client->connect())
    {
        // _client->subscribe(_client->ali_batch_update_pub_topic);
        return true;
    }

    return false;
}

void MqttClientManager::disconnect()
{
    if (_client && _client->is_connected())
    {
        _client->disconnect();
    }
}
    
bool MqttClientManager::publish(const std::string& topic, const std::string& payload, int qos)
{
    if (_client && _client->is_connected())
    {
        return _client->publish(topic, payload, qos);
    }

    return false;
}
    
bool MqttClientManager::subscribe(const std::string& topic, int qos)
{
    if (_client && _client->is_connected())
    {
        return _client->subscribe(topic, qos);
    }
    return false;
}

void MqttClientManager::register_task(const std::string& task_name, 
                                    const std::vector<std::string>& topics,
                                    MqttMessageDispatcher::MessageCallback callback)
{
    if (!_dispatcher) 
    {
        std::cerr << "[MQTTClient][Manager] dispatcher not initialized" << std::endl;
        return;
    }

    std::cout << "[MQTTClient][Manager] register task: " << task_name << std::endl;
    std::cout << "[MQTTClient][Manager] topics: ";
    for (const auto& topic : topics) std::cout << topic << " ";
    std::cout << std::endl;

    auto task = std::make_unique<MqttTask>(task_name, *_dispatcher, topics);
    for (const auto& topic : topics)
    {
        task->add_message_handler(topic, callback);
    }
    _tasks[task_name] = std::move(task);
}
    

void MqttClientManager::unregister_task(const std::string& task_name)
{
    if (_tasks.find(task_name) != _tasks.end())
    {
        _tasks.erase(task_name);
    }
}

// std::string MqttClientManager::get_shadow_sub_topic() const
// {

// }
// std::string MqttClientManager::get_shadow_update_topic() const
// {

// }

// std::string MqttClientManager::get_state_report_topic() const
// {

// }

MqttClientManager::~MqttClientManager()
{
    disconnect();
    _tasks.clear();
    if(_dispatcher) _dispatcher->stop();
}