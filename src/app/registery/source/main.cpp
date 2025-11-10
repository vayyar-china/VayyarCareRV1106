#include "RestfulConnector.h"
#include "Auth.h"
#include "Device.h"
#include "MqttClientManager.h"
#include "MqttClient.h"

#include <json/json.h>

#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstring>
#include <string>
#include <chrono>

static bool ensure_certificates_exist()
{
    const std::string cert_dir = "./cert";
    const std::string private_key_path = "./cert/auth.pk.pem";
    const std::string x509_path = "./cert/auth.x509.pem";

    // create certs directory
    struct stat st;
    if (stat(cert_dir.c_str(), &st) == -1)
    {
        if (errno == ENOENT)
        {
            if (mkdir(cert_dir.c_str(), 0755) != 0)
            {
                std::cerr << "Failed to create directory: " << strerror(errno) << std::endl;
                return false;
            }
            std::cout << "Created certificates directory: " << cert_dir << std::endl;
        }
        else
        {
            std::cerr << "Eroor check directory: " << cert_dir << std::endl;
            return false;
        }
    }

    // check cert keys
    bool keys_exist = (access(private_key_path.c_str(), F_OK) == 0) &&
                        (access(x509_path.c_str(), F_OK) == 0);
    if (!keys_exist)
    {
        std::cout << "Generating new certificates keys ... " << std::endl;

        std::string pk_pem;
        std::string x509_pem;
        if (!generate_keys(pk_pem, x509_pem))
        {
            std::cerr << "Failed to generate certificates" << std::endl;
            return false;
        }

        std::ofstream pk_file(private_key_path);
        if (!pk_file)
        {
            std::cerr << "Failed to open pk file for writing" << std::endl;
            return false;
        }
        pk_file << pk_pem;
        pk_file.close();

        std::ofstream x509_file(x509_path);
        if (!x509_file)
        {
            std::cerr << "Failed to open x509 file for writing" << std::endl;
            return false;
        }
        x509_file << x509_pem;
        x509_file.close();

        std::cout << "Certificates generated and saved" << std::endl;
    }
    else
    {
        std::cout << "Using existed certificates" << std::endl;
    }

    return true;
}

static std::string mock_state_monitoring(std::string dev_id, uint64_t uptime)
{
    Json::Value root;

    root["agc_chip_id"] = 29185;
    root["agc_chip_revision"] = 176;
    root["configError"] = true;
    root["deviceId"] = dev_id;

    Json::Value device_state_root;
    device_state_root["type"] = "Monitoring";
    root["deviceState"] = device_state_root;

    root["dspVer"] = "WalabotHome_0.44.4.0.0x0ad88848";
    root["factoryBurnTime"] = "Fri Sep 19 11:03:50 2025";
    root["hardware"] = "2";
    root["hwRevRadar"] = "1";

    Json::Value logger_stats_root;
    logger_stats_root["bytesLogged"] = 442621;
    logger_stats_root["flashBadSectors"] = 0;
    logger_stats_root["flashEraseErrors"] = 0;
    logger_stats_root["flashSectorsErased"] = 108;
    logger_stats_root["flashWriteErrors"] = 0;
    logger_stats_root["msgsLogged"] = 2610;
    root["loggerStats"] = logger_stats_root;

    root["memoryUsage"] = 59;
    root["model"] = "VC1BBUS02";
    root["packageName"] = "com.walabot.home.esp.client";
    root["productType"] = "Falling";

    Json::Value rfprofile_root;
    rfprofile_root["rfBandWidth"] = "BW500";
    rfprofile_root["rfRegulationZone"] = "WW";
    root["rfProfile"] = rfprofile_root;

    root["sensitivityMap_md5"] = "";
    root["serialProduct"] = "VXTBB2214S00733";
    root["serialRadar"] = "BLUGBC0Q219S0185";
    root["status"] = "monitoring";
    root["temperature"] = 75.5625;
    root["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    root["upTime"] = uptime;
    root["versionName"] = "vayyar-care-china-v0.44.5.26";
    root["wifiMac"] = "7C:9E:BD:AD:05:2C";

    Json::Value wifistate_root;
    wifistate_root["bssid"] = "FE:2F:EF:9E:A9:A9";
    wifistate_root["rssi"] = -43;
    wifistate_root["ssid"] = "Goger-2.4G";
    root["wifiState"] = wifistate_root;
    
    std::string json_str = root.toStyledString();

    // std::cout << "[Device][State] payload=" << json_str << std::endl;

    return json_str;
}

static std::string mock_presence_event(std::string dev_id)
{
    Json::Value root;

    root["type"] = 4;
    root["deviceId"] = dev_id;

    Json::Value payload_root;
    payload_root["deviceId"] = dev_id;
    payload_root["presenceDetected"] = true; // false
    payload_root["presenceTargetType"] = 0;
    payload_root["roomPresenceIndication"] = 1; // 0
    payload_root["timestamp"]= std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    Json::Value targets_root(Json::arrayValue);
    for (size_t i = 0; i < 3; i++)
    {
        Json::Value target;
        target["id"] = i;
        target["xPosCm"] = rand() % (150 - (-150) + 1) + (-150);
        target["yPosCm"] = rand() % (350 - 0 + 1) + 0;
        target["zPosCm"] = rand() % (180 - 0 + 1) + 0;
        target["amplitude"] = 0;
        target["posture"] = 1;
        target["similarity"] = 0.9;

        targets_root.append(target);
    }
    payload_root["trackerTargets"] = targets_root;

    Json::Value presence_regionmap_root;
    for (size_t i = 0; i < 6; i++)
    {
        presence_regionmap_root[std::to_string(i)] = 0;
    }
    payload_root["presenceRegionMap"] = presence_regionmap_root;

    Json::Value rpm_regionmap_root;
    for (size_t i = 0; i < 6; i++)
    {
        rpm_regionmap_root[std::to_string(i)] = 0;
    }
    payload_root["rpmRegionMap"] = rpm_regionmap_root;

    root["payload"] = payload_root;

    std::string json_str = root.toStyledString();

    // std::cout << "[Device][Presence] payload=" << json_str << std::endl;

    return json_str;
}

int main()
{
    Device device;

    init_openssl();

    if (!ensure_certificates_exist())
    {
        std::cerr << "Failed to ensure cerficates exist." << std::endl;
        return 1;
    }

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
    std::string ali_events_report_pub_topic = ali_publish_topic_prefix + "/user/events";
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

        uint64_t uptime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        struct timeval timeout;
        bool is_state_reported = false;
        while (true)
        {
            // mock device state monitor
            if (!is_state_reported)
            {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                auto state_payload = mock_state_monitoring(device_id, uptime);
                std::cout << "[MQTTClient][Publish][" << ali_state_report_pub_topic << "] " << "state report " << state_payload << std::endl;
                MqttClientManager::instance().publish(ali_state_report_pub_topic, state_payload);
                is_state_reported = true;
            }


            // mock presence
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            if (select(0, nullptr, nullptr, nullptr, &timeout) == 0)
            {
                

                std::string presence_payload = mock_presence_event(device_id);
                std::cout << "[MQTTClient][Publish][" << ali_events_report_pub_topic << "] " << "state report " << presence_payload << std::endl;
                MqttClientManager::instance().publish(ali_events_report_pub_topic, presence_payload);
            }
            

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