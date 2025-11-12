#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

#include <messagebus.h>

static std::atomic<bool> g_initialized{false};
static std::atomic<int> g_instance_count{0};

MessageBus::Subscription::~Subscription() { unsubscribe(); }

void MessageBus::Subscription::unsubscribe()
{
    if (_active.exchange(false)) _bus->internal_unsubscribe(_topic, _subscription_id);
}

MessageBus::MessageBus(const Config& config) : _config(config)
{
    if (_config.client_id.empty())
    {
        _config.client_id = _config.app_id + "_" + std::to_string(time(nullptr));
    }

    global_init();
    g_instance_count++;

    _mosq = mosquitto_new(_config.client_id.c_str(), true, this);
    if (!_mosq) throw std::runtime_error("[MessageBusMosq][Error] Failed to create Mosquitto client");

    mosquitto_connect_callback_set(_mosq, on_connect_wrapper);
    mosquitto_disconnect_callback_set(_mosq, on_disconnect_wrapper);
    mosquitto_message_callback_set(_mosq, on_message_wrapper);
    mosquitto_subscribe_callback_set(_mosq, on_subscribe_wrapper);
    mosquitto_unsubscribe_callback_set(_mosq, on_unsubscribe_wrapper);
}

MessageBus::~MessageBus()
{
    _destroying = true;

    if (_mosq)
    {
        disconnect();
        mosquitto_destroy(_mosq);
    }

    if (--g_instance_count == 0)
    {
        global_cleanup();
    }
}

void MessageBus::global_init()
{
    if (!g_initialized.exchange(true))
    {
        mosquitto_lib_init();
    }
}

void MessageBus::global_cleanup()
{
    if (g_initialized.exchange(false))
    {
        mosquitto_lib_cleanup();
    }
}

bool MessageBus::connect()
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (!_config.username.empty())
    {
        int rc = mosquitto_username_pw_set(_mosq, _config.username.c_str(), _config.password.c_str());
        if (rc != MOSQ_ERR_SUCCESS)
        {
            if (_error_handler) _error_handler("[MessageBusMosq][Error] Failed to set credentials: " + std::string(mosquitto_strerror(rc)));
            return false;
        }
    }

    int rc = mosquitto_connect(_mosq, _config.host.c_str(), _config.port, _config.keepalive);
    if (rc !=  MOSQ_ERR_SUCCESS)
    {
        if (_error_handler) _error_handler("[MessageBusMosq][Error] Failed to connect: " + std::string(mosquitto_strerror(rc)));
        return false;
    }

    mosquitto_loop_start(_mosq);
    return true;
}

bool MessageBus::disconnect()
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_mosq && _connected)
    {
        mosquitto_loop_stop(_mosq, true);
        int rc = mosquitto_disconnect(_mosq);
        _connected = false;
        return rc == MOSQ_ERR_SUCCESS;
    }
    return true;
}

void MessageBus::set_connection_handler(ConnectionHandler handler)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _connection_handler = handler;
}

void MessageBus::set_error_handler(std::function<void(const std::string& error)> handler)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _error_handler = handler;
}

bool MessageBus::publish(const std::string& topic, const std::string& payload, QoS qos, bool retain)
{
    return publish(topic, payload.data(), payload.size(), qos, retain);
}
    
bool MessageBus::publish(const std::string& topic, const void* data, size_t len, QoS qos, bool retain)
{
    if (!ensure_connected()) return false;

    int rc = mosquitto_publish(_mosq, nullptr, topic.c_str(), len, data, static_cast<int>(qos), retain);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        if (_error_handler) _error_handler("[MessageBusMosq][Error] Failed to publish: " + std::string(mosquitto_strerror(rc)));
        return false;
    }
    return true;
}

bool MessageBus::send_to_app(const std::string& target_app, const std::string& message_type, const std::string&payload, QoS qos)
{
    std::string topic = "app/" + target_app + "/in/" + message_type;
    return publish(topic, payload, qos);
}
    
bool MessageBus::send_to_adapter(const std::string& message_type, const std::string&payload, QoS qos)
{
    std::string topic = "app/mqttadapter/in/" + message_type;
    return publish(topic, payload, qos);
}
    
bool MessageBus::broadcast(const std::string& message_type, const std::string&payload, QoS qos)
{
    std::string topic = "broadcast/" + message_type;
    return publish(topic, payload, qos);
}

MessageBus::SubscriptionPtr MessageBus::subscribe(const std::string& topic, MessageHandler handler, QoS qos)
{
    if (!ensure_connected()) return nullptr;

    int mid;
    int rc = mosquitto_subscribe(_mosq, &mid, topic.c_str(), static_cast<int>(qos));
    if (rc != MOSQ_ERR_SUCCESS)
    {
        if (_error_handler) _error_handler("[MessageBusMosq][Error] Failed to subscribe: " + std::string(mosquitto_strerror(rc)));
        return nullptr;
    }

    int subscription_id = _next_subscription_id++;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _subscriptions[topic].push_back({ handler, qos, subscription_id });
    }

    return std::make_shared<Subscription>(this, topic, subscription_id);
}
    
MessageBus::SubscriptionPtr MessageBus::subscribe_to_app_messages(const std::string& message_type, MessageHandler handler, QoS qos)
{
    std::string topic = generate_app_topic("in", message_type);
    return subscribe(topic, handler, qos);
}
    
MessageBus::SubscriptionPtr MessageBus::subscribe_to_adapter(MessageHandler handler, QoS qos)
{
    std::string topic = "app/mqttadapter/out/+";
    return subscribe(topic, handler, qos);
}
    
MessageBus::SubscriptionPtr MessageBus::subscribe_to_broadcast(MessageHandler handler, QoS qos)
{
    std::string topic = "broadcast/+";
    return subscribe(topic, handler, qos);
}
    
MessageBus::SubscriptionPtr MessageBus::subscribe_to_all_app_messages(MessageHandler handler, QoS qos)
{
    std::string topic = "app/+/out/+";
    return subscribe(topic, handler, qos);
}

void MessageBus::on_connect_wrapper(struct mosquitto* mosq, void* obj, int reason_code)
{
    MessageBus* bus = static_cast<MessageBus*>(obj);
    if (!bus->_destroying) bus->on_connect(reason_code);
}
    
void MessageBus::on_disconnect_wrapper(struct mosquitto* mosq, void* obj, int reason_code)
{
    MessageBus* bus = static_cast<MessageBus*>(obj);
    if (!bus->_destroying) bus->on_disconnect(reason_code);
}
     
void MessageBus::on_message_wrapper(struct mosquitto* mosq, void* obj, const struct mosquitto_message* msg)
{
    MessageBus* bus = static_cast<MessageBus*>(obj);
    if (!bus->_destroying) bus->on_message(msg);
}

void MessageBus::on_subscribe_wrapper(struct mosquitto* mosq, void* obj, int mid, int qos_count, const int* granted_qos)
{
    MessageBus* bus = static_cast<MessageBus*>(obj);
    if (!bus->_destroying) bus->on_subscribe(mid, qos_count, granted_qos);
}
    
void MessageBus::on_unsubscribe_wrapper(struct mosquitto* mosq, void* obj, int mid)
{
    MessageBus* bus = static_cast<MessageBus*>(obj);
    if (!bus->_destroying) bus->on_unsubscribe(mid);
}

void MessageBus::on_connect(int reason_code)
{
    _connected = (reason_code == 0);

    if (_connection_handler) _connection_handler(_connected);

    if (_connected)
    {
        std::cout << "[MessageBusMosq][Info] messagebus connected to " << _config.app_id << std::endl;

        std::lock_guard<std::mutex> lock(_mutex);
        for (const auto& topic_subs : _subscriptions)
        {
            for (const auto& sub_info : topic_subs.second)
            {
                mosquitto_subscribe(_mosq, nullptr, topic_subs.first.c_str(), static_cast<int>(sub_info.qos));
            }
        }
    }
    else 
    {
        std::cerr << "[MessageBusMosq][Error] messagebus failed to connect " << reason_code << std::endl;
        if (_config.auto_reconnect) schedule_reconnect();
    }
}
    
void MessageBus::on_disconnect(int reason_code)
{
    bool was_connected = _connected.exchange(false);

    if (was_connected)
    {
        if (_connection_handler) _connection_handler(false);
    }

    std::cout << "[MessageBusMosq][Info] messagebus disconnected to " << _config.app_id << std::endl;

    if (_config.auto_reconnect)
    {
        if (reason_code != 0) schedule_reconnect();
    }
}
    
void MessageBus::on_message(const struct mosquitto_message* msg)
{
    std::string topic(msg->topic);
    std::string payload(static_cast<const char*>(msg->payload), msg->payloadlen);

    std::vector<SubscriptionInfo> matching_handlers;
    {
        std::lock_guard<std::mutex> lock(_mutex);

        for (const auto& topic_subs : _subscriptions)
        {
            if (topic == topic_subs.first || 
                (topic_subs.first.find('+') != std::string::npos && 
                 topic.substr(0, topic_subs.first.find('+')) == topic_subs.first.substr(0, topic_subs.first.find('+')))) 
            {
                
                for (const auto& sub_info : topic_subs.second) 
                {
                    matching_handlers.push_back(sub_info);
                }
            }
        }
    }

    for (const auto& sub_info : matching_handlers)
    {
        try
        {
            sub_info.handler(topic, payload);
        }
        catch(const std::exception& e)
        {
            if (_error_handler) _error_handler("[MessageBus][Error] message handler error " + std::string(e.what()));
        }
        
    }
}


void MessageBus::on_subscribe(int mid, int qos_count, const int* granted_qos)
{
    if (qos_count > 0)
    {
        std::cout << "[MessageBus][Info] subscribe successfully mid: " << mid << ", granted QoS: " << granted_qos[0] << std::endl;
    }
    else
    {
        std::cerr << "[MessageBus][Error] failed to subscribe" << std::endl;
    }
}
    
void MessageBus::on_unsubscribe(int mid)
{
    std::cout << "[MessageBus][Info] unsubscribe successfully mid: " << mid << std::endl;
}

bool MessageBus::ensure_connected()
{
    if (_connected)
    {
        if (_config.auto_reconnect) connect();
        return _connected;
    }
    return true;
}

void MessageBus::schedule_reconnect()
{
    if (!_reconnect_scheduled.exchange(true))
    {
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(_config.reconnect_delay_ms));
            _reconnect_scheduled = false;
            if (!_connected && !_destroying) do_reconnect();
        }).detach();
    }
}

void MessageBus::do_reconnect()
{
    std::cout << "[MessageBus][Info] attempting to reconnect to " << _config.app_id << std::endl;
    connect();
}

void MessageBus::internal_unsubscribe(const std::string& topic, int subscription_id)
{
    std::lock_guard<std::mutex> lock(_mutex);

    auto topic_it = _subscriptions.find(topic);
    if (topic_it != _subscriptions.end())
    {
        auto& subs = topic_it->second;
        subs.erase(std::remove_if(subs.begin(), subs.end(), [subscription_id](const SubscriptionInfo& info) {
            return info.subscription_id == subscription_id;
        }), subs.end());

        if (subs.empty())
        {
            _subscriptions.erase(topic_it);
            mosquitto_unsubscribe(_mosq, nullptr, topic.c_str());
        }
    }
}

std::string MessageBus::generate_app_topic(const std::string& direction, const std::string& message_type) const
{
    return "app/" + _config.app_id + "/" + direction + "/" + message_type;
}