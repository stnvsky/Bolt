#include "motors.h"
#include "IMU.h"
#include "control.h"

extern "C" void app_main() {
    xTaskCreate(vTaskMotors, "task_motors", 1024*4, nullptr, 3, nullptr);
    xTaskCreate(vtaskMPU, "task_mpu", 1024*4, nullptr, 3, nullptr);
    xTaskCreate(vTaskControl, "task_control", 1024*4, nullptr, 3, nullptr);
}