#ifndef DISTANCE_H
#define DISTANCE_H

#include <driver/i2c.h>
#include "vl53l0x.hpp"

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"

#define I2C_SDA_PIN 26
#define I2C_SCL_PIN 25

#define XSHUT_PIN_1             GPIO_NUM_15
#define XSHUT_PIN_2             GPIO_NUM_22
#define XSHUT_PIN_3             GPIO_NUM_23

void vDistSensor( void * pvParameters );

#endif // DISTANCE_H