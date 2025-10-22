#include "RestfulConnector.h"
#include "Auth.h"

#include <iostream>

int main()
{
    init_openssl();

    auto res = sysmodule::RestfulConnector::GetInstance().RegisterDevice();

    std::cout << "[[RegisterDevice response]] error code " << res.error << ", with ret code " << res.ret_code << ", and response " << res.res_string << std::endl;

    return 0;
}