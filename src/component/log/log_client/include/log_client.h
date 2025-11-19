
#ifndef __LOG_CLIENT_H__
#define __LOG_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#include <string.h>

#define FIFO_NAME "/tmp/myfifo"
#define MODE 0666

#define DEBUG
    
__attribute__((visibility("default"))) int Log_c(char *message, int size);

#ifdef __cplusplus
}
#endif

#endif
