#pragma once

#include <stdlib.h>
#include <string>

namespace sysmodule
{
    size_t WriteFunction(void *ptr, size_t size, size_t nmemb, std::string *data);
}