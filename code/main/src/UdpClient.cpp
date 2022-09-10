
#include "UdpClient.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"

#include <string>

#include "IMU.h"

static const char *TAG = "example";

#define HOST_IP_ADDR "192.168.0.42"
#define PORT 12345

extern int change_kp1_alternative(const char* string);
extern int change_ki1_alternative(const char* string);
extern int change_kd1_alternative(const char* string);

void vTaskUdpClient(void *)
{
    int addr_family = 0;
    int ip_protocol = 0;

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;

    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        while(true);
    }
    ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);

    char payload[128] = "";

    while (true) {
        memset(payload, 0, sizeof(payload));
        sprintf(payload, "%.2f\n", get_theta());

        int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            break;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}