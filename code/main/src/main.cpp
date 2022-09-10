#include "motors.h"
#include "IMU.h"
#include "control.h"
#include "UdpClient.h"
#include "UdpServer.h"
#include "wifi.h"

#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"

extern "C" void app_main() {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    xTaskCreate(vTaskMotors, "task_motors", 1024*4, nullptr, 3, nullptr);
    xTaskCreate(vtaskMPU, "task_mpu", 1024*4, nullptr, 3, nullptr);
    xTaskCreate(vTaskControl, "task_control", 1024*4, nullptr, 3, nullptr);
    xTaskCreate(vTaskUdpClient, "task_udp_client", 1024*4, nullptr, 3, nullptr);
    xTaskCreate(vTaskUdpServer, "task_udp_server", 1024*4, nullptr, 3, nullptr);
}