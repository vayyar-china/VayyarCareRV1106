#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "log_client.h"

int Log_c(char *message, int size) {
    int fd;
    if((message == NULL) || (size <=0)) {
        printf("Input error!\n");
        return -1;
    }
    
#ifdef DEBUG
    //printf("Client connecting to server...\n");
#endif
    
    fd = open(FIFO_NAME, O_WRONLY);  // 阻塞直到服务器端准备好
    
#ifdef DEBUG
    //printf("Connected to server!\n");
#endif
    
    // 写入数据
    write(fd, message, size);
    
#ifdef DEBUG
    //printf("%s", message);
#endif

    close(fd);
    return 0;
}
