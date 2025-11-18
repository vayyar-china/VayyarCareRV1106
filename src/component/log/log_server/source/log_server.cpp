#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include "json/json.h"
#include "mosquitto/mosquitto.h"

using namespace std;

#define FIFO_MODE 0666
#define DEBUG 1
#define PATH_MAX_RV 256

static volatile sig_atomic_t keep_running = 1;
static FILE *current_fp = NULL;

// 文件信息结构体
typedef struct {
    char name[PATH_MAX_RV];
    time_t mtime;  // 最后修改时间
} FileInfo;

// Log配置信息结构体
static struct {
    char fifo_name[PATH_MAX_RV]; //log管道文件
    char log_path[PATH_MAX_RV];   //Log保存目录   
    int time_RV_log_loop;   //log文件保存间隔s    
    int max_files;    //log文件保存的数量    
    int is_upload_load;   //是否上传至阿里云 
} LogConfig;

int load_config() {
    ifstream ifs;
    ifs.open("Log.json");
    
    if (!ifs.is_open()) {
        printf("can't open Log.json !\n");
        return -1;
    }
    
    Json::Reader reader;
    Json::Value root;    
    if (!reader.parse(ifs, root)) {
        printf("Parse Log.json failed! \n");
        ifs.close();
        return -1;
    }
    
    // 提取并输出数据
    string tmp_str;
    tmp_str = root["FIFO_NAME"].asString();    
    strcpy(LogConfig.fifo_name, tmp_str.c_str());
    
    tmp_str = root["LOG_PATH"].asString();   
    strcpy(LogConfig.log_path, tmp_str.c_str());

    LogConfig.time_RV_log_loop = root["TIME_RV_LOG_LOOP"].asInt();    
    LogConfig.max_files = root["MAX_FILES"].asInt();    
    LogConfig.is_upload_load = root["IS_UPLOAD_TOCLOUD"].asInt();  
    
#if DEBUG
    printf("fifo_name=%s, log_path=%s, time_RV_log_loop=%d, max_files=%d, is_upload_load=%d \n", LogConfig.fifo_name, LogConfig.log_path, LogConfig.time_RV_log_loop, LogConfig.max_files, LogConfig.is_upload_load);
#endif
    
    ifs.close();
    return 0;
}


// 比较函数：按修改时间升序排列（最老的在前）
int compare_files(const void *a, const void *b) {
    const FileInfo *fileA = (const FileInfo *)a;
    const FileInfo *fileB = (const FileInfo *)b;
    
    if (fileA->mtime < fileB->mtime) return -1;
    if (fileA->mtime > fileB->mtime) return 1;
    return 0;
}

// 获取目录中所有文件信息
int get_files(const char *dir_path, FileInfo **files, int *file_count) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char full_path[PATH_MAX_RV+1];
    int count = 0;
    int capacity = LogConfig.max_files+10;

    *files = (FileInfo *)malloc(capacity * sizeof(FileInfo));
    if (*files == NULL) {
#if DEBUG
        perror("malloc fail");
#endif
        return -1;
    }

    dir = opendir(dir_path);
    if (dir == NULL) {
#if DEBUG
        perror("can't open the dir");
#endif
        free(*files);
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        // 跳过 "." 和 ".." 目录
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // 构建完整路径
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        // 获取文件状态信息
        if (stat(full_path, &file_stat) == -1) {
            continue;
        }

        // 只处理普通文件
        if (S_ISREG(file_stat.st_mode)) {
            // 保存文件信息
            strncpy((*files)[count].name, full_path, PATH_MAX_RV - 1);
            (*files)[count].name[PATH_MAX_RV - 1] = '\0';
            (*files)[count].mtime = file_stat.st_mtime;
            count++;
        }
    }

    closedir(dir);
    *file_count = count;
    return 0;
}

// 删除最老的文件
int delete_oldest_files(FileInfo *files, int file_count, int max_files) {
    int deleted_count = 0;
    
    if (file_count <= max_files) {
        return 0;
    }

    // 按修改时间排序
    qsort(files, file_count, sizeof(FileInfo), compare_files);

    // 删除超出数量的最老文件
    for (int i = 0; i < file_count - max_files; i++) {
#if DEBUG
        printf("Is deleting the oldest file: %s\n", files[i].name);
#endif
        if (remove(files[i].name) == 0) {
            deleted_count++;
        } else {
#if DEBUG
            perror("Fail to delete the oldest file\n");
#endif
        }
    }

    return deleted_count;
}

int manage_files(const char *target_dir) {
    FileInfo *files = NULL;
    int file_count = 0;
    int deleted_count = 0;

    // 获取目录中所有文件
    if (get_files(target_dir, &files, &file_count) != 0) {
        // 清理内存
        if (files != NULL) {
            free(files);
        }        
        return -1;
    }
#if DEBUG
    printf("Current count of log files: %d\n", file_count);
#endif
    // 如果文件数量超过限制，删除最老的文件
    if (file_count > LogConfig.max_files) {
        deleted_count = delete_oldest_files(files, file_count, LogConfig.max_files);
#if DEBUG
        printf("Have deleted %d oldest files\n", deleted_count);
#endif
    } else {
#if DEBUG
        printf("File count not exceed, no need to delete files,\n");
#endif
    }

    // 显示剩余文件信息（按时间排序）
    if (file_count > 0) {
        qsort(files, file_count, sizeof(FileInfo), compare_files);
#if DEBUG
        printf("\nCurrent file list（modified timestamp）:\n");
#endif
        for (int i = 0; i < file_count && i < LogConfig.max_files; i++) {
            struct tm *timeinfo = localtime(&files[i].mtime);
            char time_str[64];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
#if DEBUG
            printf("%3d. %s (Modify time: %s)\n", i + 1, files[i].name, time_str);
#endif
        }
    }

    // 清理内存
    if (files != NULL) {
        free(files);
    }

    return 0;
}

//发送MQTT消息到本地MQTT broker, 供其它模块消费该消息
int Send_Msg_Local(const char *topic, const char *msg, int port) {
    struct mosquitto *mosq = NULL;
    int rc = 0;
    
    if (topic == NULL) {
        return -1;    
    }
    
    mosquitto_lib_init();

    mosq = mosquitto_new("sync_publisher", true, NULL);
    if (!mosq) {
#if DEBUG
        fprintf(stderr, "Error: Out of memory.\n");
#endif
        rc = -1;
        goto exit;
    }

    rc = mosquitto_connect(mosq, "localhost", port, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
#if DEBUG
        fprintf(stderr, "Unable to connect (%s).\n", mosquitto_strerror(rc));
#endif
        goto exit;
    }

    // 发布消息
    rc = mosquitto_publish(mosq, NULL, topic, strlen(msg), msg, 0, false);
    if (rc != MOSQ_ERR_SUCCESS) {
#if DEBUG
        fprintf(stderr, "Unable to publish (%s).\n", mosquitto_strerror(rc));
#endif
        goto exit;
    }
#if DEBUG
    printf("Message published to topic:%s, msg:%s !\n", topic, msg);
#endif
    // 等待消息发送完成
    mosquitto_loop(mosq, 1000, 1);

exit:
    if(mosq != NULL) {
        mosquitto_destroy(mosq);
    }
    mosquitto_lib_cleanup();
    return rc;
}


void signal_handler(int signum) {
    keep_running = 0;
    manage_files(LogConfig.log_path);
    if(LogConfig.is_upload_load == 1) {
        Send_Msg_Local("Log_upload", "Hello", 1883);
    }
}

int setup_timer() {
    struct sigaction sa;
    struct sigevent sev;
    struct itimerspec its;
    timer_t timerid;
    
    // 设置信号处理
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        return -1;
    }
    
    // 创建定时器
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;
    
    if (timer_create(CLOCK_MONOTONIC, &sev, &timerid) == -1) {
        perror("timer_create");
        return -1;
    }
    
    // 设置定时器为10分钟
    its.it_value.tv_sec = LogConfig.time_RV_log_loop;    // 首次触发时间：10分钟
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = LogConfig.time_RV_log_loop; // 后续间隔：10分钟
    its.it_interval.tv_nsec = 0;
    
    if (timer_settime(timerid, 0, &its, NULL) == -1) {
        perror("timer_settime");
        return -1;
    }
    return 0;
}

void get_current_time(char *timestring) {
    time_t current_time;
    struct tm *time_info;
    
    // 获取当前时间
    time(&current_time);
    
    // 转换为本地时间
    time_info = localtime(&current_time);
    
    // 格式化输出
    strftime(timestring, 100, "%Y_%m_%d_%H:%M:%S", time_info);
#if DEBUG
    printf("current time: %s\n", timestring);
#endif
}

int switch_file() {
    char filename[512];
    char timestring[100];
    
    if (current_fp != NULL) {
        fclose(current_fp);
        current_fp = NULL;
    }
    get_current_time(timestring);
    snprintf(filename, sizeof(filename), "%soutput_%s.log", LogConfig.log_path, timestring);
    
    current_fp = fopen(filename, "w");
    if (current_fp == NULL) {
        printf("fopen error: %s \n", filename);
        return -1;
    }
#if DEBUG
    printf("finish to switch new log file: %s\n", filename);
#endif
    keep_running = 1;
    return 0;
}

void write_data(const char *data) {
    if (current_fp != NULL) {
        fprintf(current_fp, "%s", data);
        fflush(current_fp); // 确保数据立即写入
    }
    return;
}


int main() {
    int fd = -1;
    char buffer[256];
    int n = 0;    
    int ret = 0;
    
    if(load_config() == -1) {
        perror("load config failed");
    }    
      
    // 创建FIFO（如果不存在）
    if (mkfifo(LogConfig.fifo_name, FIFO_MODE) == -1) {
        perror("mkfifo failed");
        // 如果管道已存在，继续执行
    }    
    
    // 创建log目录（如果不存在）
    if(mkdir(LogConfig.log_path, 0755) == -1) {        
        perror("mk log dir failed");
        // 如果log目录已存在，继续执行        
    }

#if DEBUG
    printf("Server waiting for connection...\n");
#endif
    fd = open(LogConfig.fifo_name, O_RDONLY);  // 阻塞直到客户端连接
#if DEBUG
    printf("Client connected!\n");
#endif
    
    // 创建第一个文件
    ret = switch_file();
    if(ret == -1) {
#if DEBUG
        printf("switch_file error! \n");  
#endif
        return -1;
    }
    ret = setup_timer();
    if(ret == -1) {
#if DEBUG
        printf("setup_timer error! \n");  
#endif
        return -1;
    }  
    
    while(1) {        
        if (keep_running) {
            // 读取数据
            n = read(fd, buffer, sizeof(buffer)-1);
            if (n > 0) {
                buffer[n] = '\0';
#if DEBUG
                printf("%s", buffer);  //往串口打印
#endif
                write_data(buffer);
            }
            usleep(10000);            
        } else {
            printf("time is up, prepare to switch new log file...\n");
            switch_file();
        }
    }
    
    if(fd != -1) {
        close(fd);
        fd = -1;
    }
    unlink(LogConfig.fifo_name);  // 删除管道文件
    if (current_fp != NULL) {
        fclose(current_fp);
        current_fp = NULL;
    }  
    
    return 0;
}
