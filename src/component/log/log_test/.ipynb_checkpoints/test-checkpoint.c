
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "log_client.h"

int main() {
    char message[100];
    int ret=-1;
    int n=0;
    
    while(1) {
        n++;        
        sprintf(message, "1111111111 Message sent from %s L:%d\n", __FILE__, __LINE__);
        // 写入数据
        ret=Log_c(message, strlen(message));
        if(ret == -1) {
            printf("Return error! \n");      
        } else {
            printf("Message sent from %s", message);
        }
        usleep(100000);
    } 
    return 0;
}
