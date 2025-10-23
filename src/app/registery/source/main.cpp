#include "RestfulConnector.h"
#include "Auth.h"
#include "Device.h"
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

    MqttMessageDispatcher dispatcher;
    AliyunMqttClient ali_client(device.get_device_registry().value().connection_details.product_key, // product_key, 
                                device.get_device_registry().value().connection_details.device_name, // device_id, 
                                device.get_device_registry().value().connection_details.device_secret, // "334f6508e309e328e6b326fee47965a5", 
                                device.get_device_registry().value().connection_details.mqtt_host + ":" + std::to_string(device.get_device_registry().value().connection_details.mqtt_port)); // "mqtts://glxyXxAokhY.iot-as-mqtt.cn-shanghai.aliyuncs.com:1883");

    ali_client.set_message_dispatcher(&dispatcher);

    // ali_client.set_message_callback([] (const std::string& topic, const std::string& payload) {
    //     std::cout << "[Aliyun][Received][" << topic << "] message = " << payload << std::endl;
    // });
    
    if (ali_client.connect())
    {
        // std::string ali_shadow_sub_topic = "/shadow/get/glxyXxAokhY/id_RTg6OUY6NkQ6RUY6RkQ6Qzg";
        // std::string ali_shadow_update_pub_topic = "/shadow/update/glxyXxAokhY/id_RTg6OUY6NkQ6RUY6RkQ6Qzg";
        // ali_client.subscribe(ali_shadow_topic);

        // std::cout << "Press enter to exit ..." << std::endl;
        // std::cin.get();

        MqttTask shadow_config_task("shadow_config", dispatcher);
        shadow_config_task.add_message_handler(ali_client.ali_shadow_sub_topic, 
            [](const MqttMessageDispatcher::MqttMessage& msg) { std::cout << "[Shadow task][Aliyun][Received][" << msg.topic << "] message = " << msg.payload << std::endl;});
        
        if (ali_client.subscribe(ali_client.ali_shadow_sub_topic))
        {
            ali_client.publish(ali_client.ali_shadow_update_pub_topic, "{method: \"get\"}");
        }

        while (true)
        {

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        shadow_config_task.stop();
        ali_client.disconnect();
    }

    return 0;
}