#ifndef IMU_H
#define IMU_H

#include "MPU.hpp"
#include "SPIbus.hpp"
#include "mpu/math.hpp"
#include "mpu/types.hpp"

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_log.h"

static constexpr int MOSI = 13;
static constexpr int MISO = 12;
static constexpr int SCLK = 9;
static constexpr int CS = 10;
static constexpr uint32_t CLOCK_SPEED = 100000;  // up to 1MHz for all registers, and 20MHz for sensor data registers only


void vMPU( void * pvParameters );

#endif // IMU_H