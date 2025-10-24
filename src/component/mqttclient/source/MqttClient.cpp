#include "MqttClient.h"
#include "Auth.h"

static void traceCallback(enum MQTTCLIENT_TRACE_LEVELS level, char* message)
{
    switch (level)
    {
    case MQTTCLIENT_TRACE_MAXIMUM:
    case MQTTCLIENT_TRACE_PROTOCOL:
        std::cout << "[MQTTCLIENT][TRACE] " << message << std::endl;
        break;
    case MQTTCLIENT_TRACE_MINIMUM:
        std::cout << "[MQTTCLIENT][INFO] " << message << std::endl;
        break;
    case MQTTCLIENT_TRACE_ERROR:
        std::cout << "[MQTTCLIENT][ERROR] " << message << std::endl;
        break;
    case MQTTCLIENT_TRACE_SEVERE:
        std::cout << "[MQTTCLIENT][SEVERE] " << message << std::endl;
        break;
    default:
        std::cout << "[MQTTCLIENT][LOG] " << message << std::endl;
        break;
    }
}

AliyunMqttClient::AliyunMqttClient(const std::string& product_key,
                                    const std::string& device_name,
                                    const std::string& device_secret,
                                    const std::string& uri,
                                    const std::string& region)
    : _product_key(product_key), _device_name(device_name), _device_secret(device_secret), _server_uri(uri),
    _region(region), _connected(false)
{
    MQTTClient_setTraceLevel(MQTTCLIENT_TRACE_ERROR);
    MQTTClient_setTraceCallback(traceCallback);

    ali_shadow_sub_topic = "/shadow/get/" + _product_key + "/" + device_name; //glxyXxAokhY/id_RTg6OUY6NkQ6RUY6RkQ6Qzg";
    ali_shadow_update_pub_topic = "/shadow/update/" + _product_key + "/" + device_name; //glxyXxAokhY/id_RTg6OUY6NkQ6RUY6RkQ6Qzg";
    ali_publish_topic_prefix = "/" + _product_key + "/" + device_name;
    ali_state_report_pub_topic = ali_publish_topic_prefix + "/user/state";
    ali_batch_update_pub_topic = ali_publish_topic_prefix + "/user/batch_update";
    

    _timestamp = std::to_string((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())).count());

    _client_id = _product_key + "." + _device_name + "|timestamp=" + _timestamp + ",_v=paho-java-1.0.0,securemode=2,signmethod=hmacsha256|";//"|securemode=3,signmethod=hmacsha1|";
    std::cout << "[Aliyun][ClientId]=" << _client_id << std::endl;
}

AliyunMqttClient::~AliyunMqttClient()
{
    disconnect();
    if (_client) MQTTClient_destroy(&_client);
}

bool AliyunMqttClient::connect()
{
    if (_connected) return true;

    // create client
    int rc = MQTTClient_create(&_client, _server_uri.c_str(), _client_id.c_str(),
                            MQTTCLIENT_PERSISTENCE_DEFAULT, nullptr);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        std::cerr << "Failed to create MQTT client " << error_string(rc) << std::endl;
        return false;
    }

    // config connection
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    opts.keepAliveInterval = 60;
    opts.cleansession = 1;
    opts.reliable = 1;

    // config username
    std::string username = _device_name + "&" + _product_key;
    opts.username = username.c_str();

    // config password
    std::string password = generate_password();
    opts.password = password.c_str();

    // config SSL
    MQTTClient_SSLOptions sslopts = MQTTClient_SSLOptions_initializer;
    sslopts.trustStore = "/etc/ssl/certs/ca-certificates.crt";
    sslopts.verify = 0;
    opts.ssl = &sslopts;

    // config callback
    MQTTClient_setCallbacks(_client, this, nullptr, message_arrived, nullptr);

    // Connect to server
    std::cout << "Connecting Aliyun MQTT ..." << std::endl;
    rc = MQTTClient_connect(_client, &opts);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        std::cerr << "Failed to connect: " << error_string(rc) << std::endl;
        return false;
    }

    _connected = true;
    return true;
}

void AliyunMqttClient::disconnect()
{
    if (_connected)
    {
        MQTTClient_disconnect(_client, 10000);
        _connected = false;
    }
}

bool AliyunMqttClient::publish(const std::string& topic, const std::string& payload, int qos)
{
    if (!_connected) return false;

    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = const_cast<char*>(payload.data());
    pubmsg.payloadlen = payload.size();
    pubmsg.qos = qos;
    pubmsg.retained = 0;

    MQTTClient_deliveryToken token;
    int rc = MQTTClient_publishMessage(_client, topic.c_str(), &pubmsg, &token);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        std::cerr << "Failed to publish message: " << error_string(rc) << std::endl;
        return false;
    }

    rc = MQTTClient_waitForCompletion(_client, token, 10000);
    return rc == MQTTCLIENT_SUCCESS;
}

bool AliyunMqttClient::subscribe(const std::string& topic, int qos)
{
    if (!_connected) return false;

    std::cout << "[Aliyun][Subscribe] topic = " << topic << std::endl;
    int rc = MQTTClient_subscribe(_client, topic.c_str(), qos);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        std::cerr << "Failed to subscribe: " << error_string(rc) << std::endl;
        return false;
    }
    return true;
}

void AliyunMqttClient::set_message_callback(MessageCallback callback) { _message_callback = callback; }

void AliyunMqttClient::get_device_info(std::string& product_key, std::string& device_name, std::string& device_secret) const
{
    product_key = _product_key;
    device_name = _device_name;
    device_secret = _device_secret;
}

bool AliyunMqttClient::is_connected() const { return _connected; }

void AliyunMqttClient::set_message_dispatcher(MqttMessageDispatcher* dispatcher)
{
    _dispatcher = dispatcher;
}

std::string AliyunMqttClient::generate_password()
{
    // std::string timestamp = std::to_string((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())).count());

    std::string password = "clientId" + _product_key + "." + _device_name +
                        "deviceName" + _device_name +
                        "productKey" + _product_key +
                        "timestamp" + _timestamp;
    
    std::string signature = generate_hmac(_device_secret, password);

    std::cout << "[Aliyun][hmac]=" << signature << std::endl;

    return signature;
}

std::string AliyunMqttClient::error_string(int rc) { return MQTTClient_strerror(rc); }

MqttMessageDispatcher::MqttMessageDispatcher() : _running(true)
{
    _dispatcher_thread = std::thread(&MqttMessageDispatcher::dispatch_loop, this);
}

MqttMessageDispatcher::~MqttMessageDispatcher()
{
    stop();
}

void MqttMessageDispatcher::stop()
{
    _running = false;
    _cv.notify_all();
    if (_dispatcher_thread.joinable()) _dispatcher_thread.join();
}

void MqttMessageDispatcher::register_task_queue(const std::string& task_name, MessageQueue* queue, const std::vector<std::string>& topics)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _task_queues[task_name] = { queue, topics };
}

void MqttMessageDispatcher::unregister_task_queue(const std::string& task_name)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _task_queues.erase(task_name);
}

void MqttMessageDispatcher::register_topic_callback(const std::string& topic, MessageCallback callback)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _topic_callbacks[topic] = callback;
}

void MqttMessageDispatcher::unregister_topic_callback(const std::string& topic)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _topic_callbacks.erase(topic);
}

void MqttMessageDispatcher::register_notifier(const std::string& task_name, std::function<void()> notifier)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _notifiers[task_name] = notifier;
}
    
void MqttMessageDispatcher::unregister_notifier(const std::string& task_name)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _notifiers.erase(task_name);
}

void MqttMessageDispatcher::add_message(const MqttMessage& message)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _message_queue.push(message);
    }
    _cv.notify_one();
}

void MqttMessageDispatcher::dispatch_loop()
{
    while (_running)
    {
        MqttMessage message;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _cv.wait(lock, [this] { return !_message_queue.empty() || !_running; });

            if (!_running) break;

            if (_message_queue.empty()) continue;

            message = _message_queue.front();
            _message_queue.pop();
        }

        std::cout << "[MqttDispatcher] dispatcher topic " << message.topic << " callback to task queue" << std::endl;
        // dispatch_to_topic_callbacks(message);
        dispatch_to_task_queues(message);
    }
    
}

void MqttMessageDispatcher::dispatch_to_topic_callbacks(const MqttMessage& message)
{
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto& [topic, callback] : _topic_callbacks)
    {
        if (is_topic_match(topic, message.topic))
        {
            callback(message);
        }
    }
}

void MqttMessageDispatcher::dispatch_to_task_queues(const MqttMessage& message)
{
    std::lock_guard<std::mutex> lock(_mutex);

    std::cout << "[MQTTClient][Dispatcher] Dispatching message: " << message.topic << std::endl;

    for (auto& [task_name, task_info] : _task_queues)
    {
        for (const auto& topic : task_info.topics)
        {
            if (is_topic_match(topic, message.topic))
            {
                std::cout << "[MQTTClient][Dispatcher] topic match: " << topic << " -> " << message.topic << std::endl;
                std::cout << "[MQTTClient][Dispatcher] adding to queue for task : " << task_name << std::endl;

                task_info.queue->push(message);
                if (auto it = _notifiers.find(task_name); it != _notifiers.end())
                {
                    it->second();
                }
                break;
            }
        }
    }
}

bool MqttMessageDispatcher::is_topic_match(const std::string& pattern, const std::string& topic)
{
    if (pattern == topic) return true;
    if (pattern == "#") return true;

    std::vector<std::string> pattern_parts = split(pattern, '/');
    std::vector<std::string> topic_parts = split(topic, '/');

    size_t i = 0;
    for (; i < pattern_parts.size() && i < topic_parts.size(); i++)
    {
        if (pattern_parts[i] == "+") continue;
        if (pattern_parts[i] == "#") return true;
        if (pattern_parts[i] != topic_parts[i]) return false;
    }

    return i == pattern_parts.size() && i == topic_parts.size();
}

std::vector<std::string> MqttMessageDispatcher::split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(str);
    while (std::getline(token_stream, token, delimiter))
    {
        tokens.push_back(token);
    }

    return tokens;
}

MqttTask::MqttTask(const std::string& name, MqttMessageDispatcher& dispatcher, const std::vector<std::string>& topics)
    : _name(name), _dispatcher(dispatcher), _running(true)
{
    _dispatcher.register_task_queue(_name, &_task_queue, topics);

    _dispatcher.register_notifier(_name, [this] { _cv.notify_one(); });

    _task_thread = std::thread(&MqttTask::run, this);
}

MqttTask::~MqttTask()
{
    stop();
}

void MqttTask::stop()
{
    _running = false;
    _cv.notify_one();
    if (_task_thread.joinable()) _task_thread.join();
    _dispatcher.unregister_task_queue(_name);
    _dispatcher.unregister_notifier(_name);
}

void MqttTask::add_message_handler(const std::string& topic, MqttMessageDispatcher::MessageCallback callback)
{
    _handlers[topic] = callback;
}

void MqttTask::run()
{
    while (_running)
    {
        MqttMessageDispatcher::MqttMessage message;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _cv.wait(lock, [this] { return !_task_queue.empty() || !_running; } );

            if (!_running) break;

            if (_task_queue.empty()) continue;

            message = _task_queue.front();
            _task_queue.pop();
        }

        process_message(message);
    }
    std::cout << "[Aliyun] task is about to leave" << std::endl;
}

void MqttTask::process_message(const MqttMessageDispatcher::MqttMessage& message)
{
    for (auto& [topic, handler] : _handlers)
    {
        // if (_dispatcher.is_topic_match(topic, message.topic))
        {
            handler(message);
        }
    }
}