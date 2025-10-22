#include <curl/curl.h>

#include "Auth.h"
#include "RestfulConnector.h"

namespace sysmodule
{
    RestfulConnector::RestfulConnector()
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    RestfulConnector::~RestfulConnector()
    {
        curl_global_cleanup();
    }

    std::shared_ptr<Session> RestfulConnector::CreateSession()
    {
        auto session = std::make_shared<Session>();
        session->Init();
        // return std::move(session);
        return session;
    }

    std::shared_ptr<Session> RestfulConnector::CreateSessionWithSsl(const std::string& cert, const std::string& cabundle, const std::string& key)
    {
        auto session = std::make_shared<Session>();
        session->InitWithSsl(cert, cabundle, key, true);
        // return std::move(session);
        return session;
    }

    Response RestfulConnector::RegisterDevice()
    {
        auto session = CreateSessionWithSsl("", "./cert/auth.x509.pem", "");
        session->SetUrl("https://test-api.walabot-home.cn/device/");

        std::vector<std::string> headers;
        headers.push_back("keyver: 2");
        std::string project_id = "walabot-home";
        std::string device_id = "id_RTg6OUY6NkQ6RUY6RkQ6Qzg";
        std::string bearer = "Bearer " + create_jwt((const uint8_t*)TOKEN, sizeof(TOKEN), "", project_id, device_id ,HTTP_REQ_JWT_EXPIRY);
        std::cout << "[[Bearer]] is /n" << bearer << std::endl;
        headers.push_back("Authorization: " + bearer);
        headers.push_back("Content-Type: application/json");
        session->SetHeaders(headers);
        std::string body = "{\"appId\":\"com.walabot.home.client\",\"deviceInfo\":{\"buildId\":\"NA\",\"buildTime\":1754897120000,\"currentApkMetadata\":{\"buildType\":\"release\",\"flavor\":\"esp\",\"packageName\":\"com.walabot.home.esp.client\",\"versionName\":\"vayyar-care-china-v0.44.5.21\"},\"deviceId\":\"id_RTg6OUY6NkQ6RUY6RkQ6Qzg\",\"factoryBurnTime\":\"Tue Jun 14 06:08:57 2022\",\"factoryVersionName\":\"vayyar-care-china-v0.43.8.16-wx_dev\",\"hardware\":\"0000000000000001\",\"hwRevRadar\":\"0000000000000002\",\"lat\":0,\"lng\":0,\"model\":\"VC1BBUS02\",\"roomType\":1,\"sdkVersion\":1,\"serialProduct\":\"VXTBB2224S01486\",\"serialRadar\":\"BLUGBC0Q222S0B94\"},\"keyVer\":2,\"packageName\":\"com.walabot.home.esp.client\",\"publicKey\":\"-----BEGIN CERTIFICATE-----\\nMIIC5zCCAc+gAwIBAgIQAwIAHq+2zVoAAgAAAAAAADANBgkqhkiG9w0BAQsFADAA\\nMB4XDTM5MTAxMjA1MDYxNFoXDTE5MTAxMjA1MDYxNFowGzEZMBcGA1UEAwwQaG9t\\nZS53YWxhYm90LmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMB1\\nKRfS3P722MJBCOX6tE5QJSAPvzqcNeZ8mDtMamuxvsu9xKw7jviYquPRj0T9xo5V\\nVk3N5QK989uvkKkhElnuzAB5c0VPM5J7AgLCsddSk1pPzeEKztWJ32Mpoz7rOaOa\\nlb/sRMC8dIv5amhF1C0d5eDcnRFPjK/L5XHkAL7+I44Ew+LNe8tmN2pdKEdheTu0\\nFiBU2FIXFYYdGz5Or2tPo2bF/HiAEIeVTa6NrnGLO59fwURAvqJfpEHiP9LU9qfE\\njS605FjsOpax7u4UuFhzvenE48LShXVD+zG4sb5d0X7JZi91f975MzeE/SDr75Pg\\n476IkWvHiy7wLgGeD6cCAwEAAaNCMEAwHQYDVR0OBBYEFCqDb1dc1fEJj48/uR6q\\nJKVr10w4MB8GA1UdIwQYMBaAFCqDb1dc1fEJj48/uR6qJKVr10w4MA0GCSqGSIb3\\nDQEBCwUAA4IBAQCPA9Qon61HW2ar6WErM4kv/mjgkNLfFcmcBQ9z3XlD5bKZbsFE\\neo8RFnKkHKTjFcq2ANFnUpIvVnw+pWwZ3s7VCDJu8UaRNPeV3BIl+artnhp1LMGb\\nK7RlNctjeXUQybYj6Blm6vj3YV+seBwTO7emjMH9jIcr9wLWb/dynAguRA0CIk3w\\n28gEhA2XfnBjUgqymvjZjCcuujPBooekDRkpEKLXRTw7IXKAPKBpI2SqGcZK2ZQl\\nvtG6eKEvpTXhGUtIYEKCVSObRazIB/bX2mcG6tM7ReLRG6Dc4BK3V/CJDhT+O14W\\nUSCNCb03fkgSGiU2qKrTWj9SGmifhgqLtyMH\\n-----END CERTIFICATE-----\\n\"}";
        std::cout << "[[Request]] is /n" << body << std::endl;
        session->SetBody(body);
        return session->Post();
    }
};