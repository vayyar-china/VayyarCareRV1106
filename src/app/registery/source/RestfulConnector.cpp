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

    Response RestfulConnector::RegisterDevice(std::string device_id, std::string project_id, std::string body_payload)
    {
        auto session = CreateSessionWithSsl("", "./cert/auth.x509.pem", "");
        session->SetUrl("https://test-api.walabot-home.cn/device/");

        std::vector<std::string> headers;
        headers.push_back("keyver: 2");
        std::string bearer = "Bearer " + create_jwt((const uint8_t*)TOKEN, sizeof(TOKEN), "", project_id, device_id ,HTTP_REQ_JWT_EXPIRY);
        std::cout << "[[Bearer]] is /n" << bearer << std::endl;
        headers.push_back("Authorization: " + bearer);
        headers.push_back("Content-Type: application/json");
        session->SetHeaders(headers);
        std::cout << "[[Request]] is /n" << body_payload << std::endl;
        session->SetBody(body_payload);
        return session->Post();
    }
};