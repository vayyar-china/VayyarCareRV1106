#include <iostream>
#include <chrono>
#include <thread>

#include <messagebus.h>

int main()
{
    MessageBus::Config config;
    config.app_id = "test_app";
    config.host = "localhost";
    config.port = 1883;
    config.auto_reconnect = true;

    MessageBus bus(config);

    bus.set_connection_handler([](bool connected) {
        if (connected) std::cout << "[MqttAdapter][Info] connected to MQTT broker" << std::endl;
        else std::cout << "[MqttAdapter][Info] disconnected from MQTT broker" << std::endl;
    });

    bus.set_error_handler([](const std::string& error) {
        std::cerr << "[MqttAdapter][Error] messagebus error" << std::endl; 
    });

    if (bus.connect())
    {
        auto sub = bus.subscribe_to_app_messages("data_test", [](const std::string& topic, const std::string& payload) {
            std::cout << "[MqttAdapter][Message] received payload size " << payload.size() 
                << "@" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << std::endl;
        });
    }


    return 0;
}

