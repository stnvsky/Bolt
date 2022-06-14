
#include "motors.h"

MotorsDriver motors;

void vTaskMotors( void * pvParameters )
{
    while (1) {
        motors.set_speed(1,2);
    }
}