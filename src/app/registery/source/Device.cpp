#include "Device.h"


Device::Device()
    : _registry(std::make_optional<DeviceRegistry>())
{
    // dev_id = std::string("id_") + base64::encode(wifi_mac_str);
    _device_id = std::string("id_");
}

std::string Device::get_device_id() { return _device_id; }

bool Device::parse_device_registry(std::string json_data)
{
    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;

    std::istringstream json_stream(json_data);
    bool parsing_success = Json::parseFromStream(builder, json_stream, &root, &errs);

    if (!parsing_success)
    {
        std::cerr << "[Device][Registry] failed to parse device registery response: " << errs << std::endl;
    }

    Json::Value connectiondetails = root["connectionDetails"];
    _registry.value().connection_details.type = connectiondetails["type"].asString();
    _registry.value().connection_details.device_name = connectiondetails["deviceName"].asString();
    _registry.value().connection_details.device_secret = connectiondetails["deviceSecret"].asString();
    _registry.value().connection_details.product_key = connectiondetails["productKey"].asString();
    _registry.value().connection_details.mqtt_host = connectiondetails["mqttHost"].asString();
    _registry.value().connection_details.mqtt_port = connectiondetails["mqttPort"].asUInt();

    Json::Value mqttdetails = root["mqttDetails"];
    _registry.value().mqtt_details.mqtt_url = mqttdetails["mqttUrl"].asString();
    _registry.value().mqtt_details.mqtt_port = mqttdetails["mqttPort"].asUInt();
    _registry.value().mqtt_details.registry_id = mqttdetails["registryId"].asString();
    _registry.value().mqtt_details.registry_region = mqttdetails["registryRegion"].asString();
    _registry.value().mqtt_details.client_id = mqttdetails["clientId"].asString();
    
    Json::Value auth = mqttdetails["auth"];
    _registry.value().mqtt_details.auth.type = auth["type"].asString();
    _registry.value().mqtt_details.auth.device_secret = auth["deviceSecret"].asString();
    _registry.value().mqtt_details.auth.product_key = auth["productKey"].asString();

    Json::Value mqtthook = root["mqttHook"];
    _registry.value().mqtt_hook.host = mqtthook["host"].asString();
    _registry.value().mqtt_hook.port = mqtthook["port"].asUInt();
    _registry.value().mqtt_hook.username = mqtthook["username"].asString();
    _registry.value().mqtt_hook.password = mqtthook["password"].asString();

    _registry.value().auth_api_key = root["authApiKey"].asString();
    _registry.value().registry_id = root["registryId"].asString();
    _registry.value().product_type = root["productType"].asString();

    Json::Value rfprofile = root["rfProfile"];
    _registry.value().rf_profile.rf_regulation_zone = rfprofile["rfRegulationZone"].asString();
    _registry.value().rf_profile.rf_bandwidth = rfprofile["rfBandWidth"].asString();

    std::cout << "[Device][Registry] deviceName=" << _registry.value().connection_details.device_name 
                << " deviceSecret=" << _registry.value().connection_details.device_secret
                << " productKey=" << _registry.value().connection_details.product_key 
                << " mqttHost=" << _registry.value().connection_details.mqtt_host 
                << " mqttPort=" << _registry.value().connection_details.mqtt_port << std::endl;

    return true;
}

std::optional<DeviceRegistry> Device::get_device_registry()
{
    return _registry;
}

std::string Device::get_device_register_request()
{
    Json::Value root;
    root["appId"] = "com.walabot.home.client";

    Json::Value device_info;
    device_info["buildId"] = "NA";
    device_info["buildTime"] = 1754897120000;
    Json::Value current_apk_meta_data;
    current_apk_meta_data["buildType"] = "release";
    current_apk_meta_data["flavor"] = "esp";
    current_apk_meta_data["packageName"] = "com.walabot.home.esp.client";
    current_apk_meta_data["versionName"] = "vayyar-care-china-v0.44.5.21";
    device_info["currentApkMetadata"] = current_apk_meta_data;
    device_info["deviceId"] = "id_RTg6OUY6NkQ6RUY6RkQ6Qzg";
    device_info["factoryBurnTime"] = "Tue Jun 14 06:08:57 2022";
    device_info["factoryVersionName"] = "vayyar-care-china-v0.43.8.16-wx_dev";
    device_info["hardware"] = "0000000000000001";
    device_info["hwRevRadar"] = "0000000000000002";
    device_info["lat"] = 0;
    device_info["lng"] = 0;
    device_info["model"] = "VC1BBUS02";
    device_info["roomType"] = 1;
    device_info["sdkVersion"] = 1;
    device_info["serialProduct"] = "VXTBB2224S01486";
    device_info["serialRadar"] = "BLUGBC0Q222S0B94";

    root["deviceInfo"] = device_info;
    root["keyVer"] = 2;
    root["packageName"] = "com.walabot.home.esp.client";
    root["publicKey"] = "-----BEGIN CERTIFICATE-----\nMIIC5zCCAc+gAwIBAgIQAwIAHq+2zVoAAgAAAAAAADANBgkqhkiG9w0BAQsFADAA\nMB4XDTM5MTAxMjA1MDYxNFoXDTE5MTAxMjA1MDYxNFowGzEZMBcGA1UEAwwQaG9t\nZS53YWxhYm90LmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMB1\nKRfS3P722MJBCOX6tE5QJSAPvzqcNeZ8mDtMamuxvsu9xKw7jviYquPRj0T9xo5V\nVk3N5QK989uvkKkhElnuzAB5c0VPM5J7AgLCsddSk1pPzeEKztWJ32Mpoz7rOaOa\nlb/sRMC8dIv5amhF1C0d5eDcnRFPjK/L5XHkAL7+I44Ew+LNe8tmN2pdKEdheTu0\nFiBU2FIXFYYdGz5Or2tPo2bF/HiAEIeVTa6NrnGLO59fwURAvqJfpEHiP9LU9qfE\njS605FjsOpax7u4UuFhzvenE48LShXVD+zG4sb5d0X7JZi91f975MzeE/SDr75Pg\n476IkWvHiy7wLgGeD6cCAwEAAaNCMEAwHQYDVR0OBBYEFCqDb1dc1fEJj48/uR6q\nJKVr10w4MB8GA1UdIwQYMBaAFCqDb1dc1fEJj48/uR6qJKVr10w4MA0GCSqGSIb3\nDQEBCwUAA4IBAQCPA9Qon61HW2ar6WErM4kv/mjgkNLfFcmcBQ9z3XlD5bKZbsFE\neo8RFnKkHKTjFcq2ANFnUpIvVnw+pWwZ3s7VCDJu8UaRNPeV3BIl+artnhp1LMGb\nK7RlNctjeXUQybYj6Blm6vj3YV+seBwTO7emjMH9jIcr9wLWb/dynAguRA0CIk3w\n28gEhA2XfnBjUgqymvjZjCcuujPBooekDRkpEKLXRTw7IXKAPKBpI2SqGcZK2ZQl\nvtG6eKEvpTXhGUtIYEKCVSObRazIB/bX2mcG6tM7ReLRG6Dc4BK3V/CJDhT+O14W\nUSCNCb03fkgSGiU2qKrTWj9SGmifhgqLtyMH\n-----END CERTIFICATE-----\n";

    // return "{\"appId\":\"com.walabot.home.client\",\"deviceInfo\":{\"buildId\":\"NA\",\"buildTime\":1754897120000,\"currentApkMetadata\":{\"buildType\":\"release\",\"flavor\":\"esp\",\"packageName\":\"com.walabot.home.esp.client\",\"versionName\":\"vayyar-care-china-v0.44.5.21\"},\"deviceId\":\"id_RTg6OUY6NkQ6RUY6RkQ6Qzg\",\"factoryBurnTime\":\"Tue Jun 14 06:08:57 2022\",\"factoryVersionName\":\"vayyar-care-china-v0.43.8.16-wx_dev\",\"hardware\":\"0000000000000001\",\"hwRevRadar\":\"0000000000000002\",\"lat\":0,\"lng\":0,\"model\":\"VC1BBUS02\",\"roomType\":1,\"sdkVersion\":1,\"serialProduct\":\"VXTBB2224S01486\",\"serialRadar\":\"BLUGBC0Q222S0B94\"},\"keyVer\":2,\"packageName\":\"com.walabot.home.esp.client\",\"publicKey\":\"-----BEGIN CERTIFICATE-----\\nMIIC5zCCAc+gAwIBAgIQAwIAHq+2zVoAAgAAAAAAADANBgkqhkiG9w0BAQsFADAA\\nMB4XDTM5MTAxMjA1MDYxNFoXDTE5MTAxMjA1MDYxNFowGzEZMBcGA1UEAwwQaG9t\\nZS53YWxhYm90LmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMB1\\nKRfS3P722MJBCOX6tE5QJSAPvzqcNeZ8mDtMamuxvsu9xKw7jviYquPRj0T9xo5V\\nVk3N5QK989uvkKkhElnuzAB5c0VPM5J7AgLCsddSk1pPzeEKztWJ32Mpoz7rOaOa\\nlb/sRMC8dIv5amhF1C0d5eDcnRFPjK/L5XHkAL7+I44Ew+LNe8tmN2pdKEdheTu0\\nFiBU2FIXFYYdGz5Or2tPo2bF/HiAEIeVTa6NrnGLO59fwURAvqJfpEHiP9LU9qfE\\njS605FjsOpax7u4UuFhzvenE48LShXVD+zG4sb5d0X7JZi91f975MzeE/SDr75Pg\\n476IkWvHiy7wLgGeD6cCAwEAAaNCMEAwHQYDVR0OBBYEFCqDb1dc1fEJj48/uR6q\\nJKVr10w4MB8GA1UdIwQYMBaAFCqDb1dc1fEJj48/uR6qJKVr10w4MA0GCSqGSIb3\\nDQEBCwUAA4IBAQCPA9Qon61HW2ar6WErM4kv/mjgkNLfFcmcBQ9z3XlD5bKZbsFE\\neo8RFnKkHKTjFcq2ANFnUpIvVnw+pWwZ3s7VCDJu8UaRNPeV3BIl+artnhp1LMGb\\nK7RlNctjeXUQybYj6Blm6vj3YV+seBwTO7emjMH9jIcr9wLWb/dynAguRA0CIk3w\\n28gEhA2XfnBjUgqymvjZjCcuujPBooekDRkpEKLXRTw7IXKAPKBpI2SqGcZK2ZQl\\nvtG6eKEvpTXhGUtIYEKCVSObRazIB/bX2mcG6tM7ReLRG6Dc4BK3V/CJDhT+O14W\\nUSCNCb03fkgSGiU2qKrTWj9SGmifhgqLtyMH\\n-----END CERTIFICATE-----\\n\"}";

    std::string json_str = root.toStyledString();

    std::cout << "[Device][Register] request payload=" << json_str << std::endl;

    return json_str;
}