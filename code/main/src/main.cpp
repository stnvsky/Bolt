#include "motors.h"
#include "IMU.h"

extern "C" void app_main() {
    xTaskCreate(vTaskMotors, "task_motors", 1024*4, nullptr, 3, nullptr);
    xTaskCreate(vMPU, "task_mpu", 1024*4, nullptr, 3, nullptr);
}