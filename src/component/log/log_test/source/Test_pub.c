
#include <stdio.h>
#include <mosquitto.h>

int main() {
    struct mosquitto *mosq = NULL;
    int rc = 0;

    mosquitto_lib_init();

    mosq = mosquitto_new("sync_publisher", true, NULL);
    if (!mosq) {
        fprintf(stderr, "Error: Out of memory.\n");
        rc = -1;
        goto exit;
    }

    rc = mosquitto_connect(mosq, "localhost", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Unable to connect (%s).\n", mosquitto_strerror(rc));
        goto exit;
    }

    // 发布消息
    rc = mosquitto_publish(mosq, NULL, "test/topic", 6, "Hello", 0, false);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Unable to publish (%s).\n", mosquitto_strerror(rc));
        goto exit;
    }

    printf("Message published to topic 'test/topic'\n");

    // 等待消息发送完成
    mosquitto_loop(mosq, 1000, 1);

exit:
    if(mosq != NULL) {
        mosquitto_destroy(mosq);
    }
    mosquitto_lib_cleanup();
    return rc;
}
