#include "Util.h"

namespace sysmodule
{
    size_t WriteFunction(void *ptr, size_t size, size_t nmemb, std::string *data)
    {
        data->append((char *)ptr, size * nmemb);
        return size * nmemb;
    }
}