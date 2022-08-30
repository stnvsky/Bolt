
#include "motors.h"

MotorsDriver motors;

void vTaskMotors( void * pvParameters )
{
    while (1) {
        motors.set_speed(-20, 50);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        motors.set_speed(-100, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        motors.set_speed(-30, -30);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        motors.set_speed(-30, 40);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        motors.set_speed(-50, 60);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        motors.set_speed(100, 100);
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
}