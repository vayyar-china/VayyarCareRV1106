#pragma once

#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>

#include <mosquitto/mosquitto.h>

class MessageBus
{
public:
    using MessageHandler = std::function<void(const std::string& topic, const std::string& payload)>;
    using ConnectionHandler = std::function<void(bool connected)>;

    enum class QoS
    {
        AT_MOST_ONCE = 0,
        AT_LEAST_ONCE = 1,
        EXACTLY_ONCE = 2
    };

    struct Config
    {
        std::string app_id;
        std::string client_id;
        std::string host = "localhost";
        int port = 1883;
        int keepalive = 60;
        std::string username;
        std::string password;
        bool auto_reconnect = true;
        int reconnect_delay_ms = 5000;
    };

    MessageBus(const Config& config);
    ~MessageBus();

    bool connect();
    bool disconnect();
    bool is_connected() const { return _connected; }

    void set_connection_handler(ConnectionHandler handler);
    void set_error_handler(std::function<void(const std::string& error)> handler);

    bool publish(const std::string& topic, const std::string& payload, QoS qos=QoS::AT_MOST_ONCE, bool retain=false);
    bool publish(const std::string& topic, const void* data, size_t len, QoS qos=QoS::AT_MOST_ONCE, bool retain=false);

    bool send_to_app(const std::string& target_app, const std::string& message_type, const std::string&payload, QoS qos=QoS::AT_MOST_ONCE);
    bool send_to_adapter(const std::string& message_type, const std::string&payload, QoS qos=QoS::AT_MOST_ONCE);
    bool broadcast(const std::string& message_type, const std::string&payload, QoS qos=QoS::AT_MOST_ONCE);

    class Subscription
    {
    public:
        Subscription(MessageBus* bus, const std::string& topic, int subscription_id)
            : _bus(bus), _topic(topic), _subscription_id(subscription_id) {}
        ~Subscription();
        void unsubscribe();
        bool is_active() const { return _active; }
        std::string topic() const { return _topic; }

    private:
        MessageBus* _bus;
        std::string _topic;
        int _subscription_id;
        std::atomic<bool> _active{ true };
    };

    using SubscriptionPtr = std::shared_ptr<Subscription>;
    SubscriptionPtr subscribe(const std::string& topic, MessageHandler handler, QoS qos=QoS::AT_MOST_ONCE);
    SubscriptionPtr subscribe_to_app_messages(const std::string& message_type, MessageHandler handler, QoS qos=QoS::AT_MOST_ONCE);
    SubscriptionPtr subscribe_to_adapter(MessageHandler handler, QoS qos=QoS::AT_MOST_ONCE);
    SubscriptionPtr subscribe_to_broadcast(MessageHandler handler, QoS qos=QoS::AT_MOST_ONCE);
    SubscriptionPtr subscribe_to_all_app_messages(MessageHandler handler, QoS qos=QoS::AT_MOST_ONCE);

    std::string get_app_id() const { return _config.app_id; }
    std::string get_client_id() const { return _config.client_id; }

    static void global_init();
    static void global_cleanup();

private:
    MessageBus(const MessageBus&) = delete;
    MessageBus& operator=(const MessageBus&) = delete;

    struct SubscriptionInfo
    {
        MessageHandler handler;
        QoS qos;
        int subscription_id;
    };

    Config _config;
    struct mosquitto* _mosq;
    std::atomic<bool> _connected{ false };
    std::atomic<bool> _destroying{ false };

    mutable std::mutex _mutex;
    std::unordered_map<std::string, std::vector<SubscriptionInfo>> _subscriptions;
    std::atomic<int> _next_subscription_id{ 1 };

    ConnectionHandler _connection_handler;
    std::function<void(const std::string& error)> _error_handler;

    std::atomic<bool> _reconnect_scheduled{ false };

    static void on_connect_wrapper(struct mosquitto* mosq, void* obj, int reason_code);
    static void on_disconnect_wrapper(struct mosquitto* mosq, void* obj, int reason_code);
    static void on_message_wrapper(struct mosquitto* mosq, void* obj, const struct mosquitto_message* msg);
    static void on_subscribe_wrapper(struct mosquitto* mosq, void* obj, int mid, int qos_count, const int* granted_qos);
    static void on_unsubscribe_wrapper(struct mosquitto* mosq, void* obj, int mid);

    void on_connect(int reason_code);
    void on_disconnect(int reason_code);
    void on_message(const struct mosquitto_message* msg);
    void on_subscribe(int mid, int qos_count, const int* granted_qos);
    void on_unsubscribe(int mid);

    bool ensure_connected();
    void schedule_reconnect();
    void do_reconnect();
    void internal_unsubscribe(const std::string& topic, int subscription_id);
    std::string generate_app_topic(const std::string& direction, const std::string& message_type) const;
};