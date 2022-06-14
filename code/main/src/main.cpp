/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdlib.h>

#include "freertos/FreeRTOS.h"

#include "motors.h"
#include "IMU.h"
#include "distance.h"

extern "C" void app_main() {
    xTaskCreate(vTaskMotors, "task_motors", 1024*2, NULL, 3, NULL);
    xTaskCreate(vMPU, "task_mpu", 1024*2, NULL, 3, NULL);
    xTaskCreate(vDistSensor, "task_dist", 1024*2, NULL, 3, NULL);
}