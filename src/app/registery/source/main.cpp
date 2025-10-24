#include "RestfulConnector.h"
#include "Auth.h"
#include "Device.h"
#include "MqttClientManager.h"
#include "MqttClient.h"

#include <iostream>

int main()
{
    Device device;

    init_openssl();

    std::string project_id = "walabot-home";
    std::string device_id = "id_RTg6OUY6NkQ6RUY6RkQ6Qzg";
    std::string body_payload = device.get_device_register_request();
    
    auto res = sysmodule::RestfulConnector::GetInstance().RegisterDevice(device_id, project_id, body_payload);

    std::cout << "[[RegisterDevice response]] response code " << res.res_code << std::endl;
    std::cout << "[[RegisterDevice response]] response data " << res.res_string << std::endl;

    device.parse_device_registry(res.res_string);

    if (!device.get_device_registry().has_value())
    {
        std::cerr << "[Registry][Main] fail to get empty device registry" << std::endl;
        return 0;
    }

    std::string ali_shadow_sub_topic = "/shadow/get/" + device.get_device_registry().value().connection_details.product_key + "/" + device.get_device_registry().value().connection_details.device_name; //glxyXxAokhY/id_RTg6OUY6NkQ6RUY6RkQ6Qzg";
    std::string ali_shadow_update_pub_topic = "/shadow/update/" + device.get_device_registry().value().connection_details.product_key + "/" + device.get_device_registry().value().connection_details.device_name; //glxyXxAokhY/id_RTg6OUY6NkQ6RUY6RkQ6Qzg";
    std::string ali_publish_topic_prefix = "/" + device.get_device_registry().value().connection_details.product_key + "/" + device.get_device_registry().value().connection_details.device_name;
    std::string ali_state_report_pub_topic = ali_publish_topic_prefix + "/user/state";
    std::string ali_batch_update_pub_topic = ali_publish_topic_prefix + "/user/batch_update";

    std::vector<std::string> sub_topics;
    sub_topics.push_back(ali_shadow_sub_topic);

    MqttClientManager::instance().init(
        device.get_device_registry().value().connection_details.product_key, // product_key, 
        device.get_device_registry().value().connection_details.device_name, // device_id, 
        device.get_device_registry().value().connection_details.device_secret, // "334f6508e309e328e6b326fee47965a5", 
        device.get_device_registry().value().connection_details.mqtt_host + ":" + std::to_string(device.get_device_registry().value().connection_details.mqtt_port)
    );

    if (MqttClientManager::instance().connect())
    {
        MqttClientManager::instance().register_task("shadow_config", sub_topics,
            [](const MqttMessageDispatcher::MqttMessage& msg) { 
                std::cout << "[Shadow task][Aliyun][Received][" << msg.topic << "] message = " << msg.payload << std::endl;
            }
        );

        MqttClientManager::instance().subscribe(ali_shadow_sub_topic);

        MqttClientManager::instance().publish(ali_shadow_update_pub_topic, "{method: \"get\"}");

        while (true)
        {

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        MqttClientManager::instance().disconnect();
    }


    // if (ali_client.connect())
    // {

    //     MqttTask shadow_config_task("shadow_config", dispatcher);
        // shadow_config_task.add_message_handler(ali_client.ali_shadow_sub_topic, 
    //         [](const MqttMessageDispatcher::MqttMessage& msg) { std::cout << "[Shadow task][Aliyun][Received][" << msg.topic << "] message = " << msg.payload << std::endl;});
        
    //     if (ali_client.subscribe(ali_client.ali_shadow_sub_topic))
    //     {
    //         ali_client.publish(ali_client.ali_shadow_update_pub_topic, "{method: \"get\"}");
    //     }

    //     while (true)
    //     {

    //         std::this_thread::sleep_for(std::chrono::milliseconds(10));
    //     }

    //     shadow_config_task.stop();
    //     ali_client.disconnect();
    // }

    return 0;
}