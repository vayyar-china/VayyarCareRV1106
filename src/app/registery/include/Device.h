#pragma once

#include <iostream>
#include <string>
#include <optional>

#include <json/json.h>

struct ProvisionConfig
{
    uint32_t version = 0;
    std::string wifi_ssid;
    std::string wifi_pass;
    std::string fallback_wifi_ssid;
    std::string fallback_wifi_pass;
    std::string http_url;
    std::string new_http_url;
    std::string project_id;
    std::string ntp_url;
    std::string package_name;
    std::string dps_key;
    std::string dps_id;
    uint32_t provision_version = 0;
};

struct ConnectionDetails
{
    std::string type;
    std::string mqtt_host;
    uint mqtt_port = 1883;
    std::string product_key;
    std::string device_name;
    std::string device_secret;
};

struct MqttHook
{
    std::string host;
    uint port = 1883;
    std::string username;
    std::string password;
};

struct MqttAuth
{
    std::string type;
    std::string device_secret;
    std::string product_key;
};

struct MqttDetails
{
    std::string mqtt_url;
    uint mqtt_port = 1883;
    std::string registry_id;
    std::string registry_region;
    std::string client_id;
    MqttAuth auth;
};

struct RFProfile
{
    std::string rf_regulation_zone;
    std::string rf_bandwidth;
};

struct DeviceRegistry
{
    ConnectionDetails connection_details;
    MqttDetails mqtt_details;
    MqttHook mqtt_hook;
    std::string auth_api_key;
    std::string registry_id;
    std::string product_type;
    RFProfile rf_profile;
};

class Device
{
public:
    Device();

    std::string get_device_id();

    std::string get_device_register_request();

    bool parse_device_registry(std::string json_data);
    std::optional<DeviceRegistry> get_device_registry();

private:
    std::optional<DeviceRegistry> _registry;
    std::string _device_id;
};

