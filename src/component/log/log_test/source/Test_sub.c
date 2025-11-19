
#include <stdio.h>
#include <mosquitto/mosquitto.h>

// 消息接收回调函数
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    printf("Received message on topic '%s': %s\n", msg->topic, (char*)msg->payload);
}

int main() {
    struct mosquitto *mosq = NULL;
    int rc;

    mosquitto_lib_init();

    mosq = mosquitto_new("async_subscriber", true, NULL);
    if (!mosq) {
        fprintf(stderr, "Error: Out of memory.\n");
        return 1;
    }

    // 设置消息回调
    mosquitto_message_callback_set(mosq, on_message);

    rc = mosquitto_connect_async(mosq, "localhost", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Unable to connect (%s).\n", mosquitto_strerror(rc));
        return 1;
    }

    rc = mosquitto_subscribe(mosq, NULL, "Log_upload", 0);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Unable to subscribe (%s).\n", mosquitto_strerror(rc));
        return 1;
    }

    printf("Subscribed to topic 'Log_upload' (async mode)\n");

    // 启动异步网络循环
    rc = mosquitto_loop_start(mosq);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Unable to start loop (%s).\n", mosquitto_strerror(rc));
        return 1;
    }

    // 等待用户输入退出
    printf("Press Enter to exit...\n");
    getchar();

    mosquitto_loop_stop(mosq, false);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
