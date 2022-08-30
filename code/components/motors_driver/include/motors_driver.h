//
// Created by Szymon Stanik on 13.06.22.
//

#ifndef BOLT_MOTORS_DRIVER_H
#define BOLT_MOTORS_DRIVER_H

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "driver/ledc.h"

#define MAX_DUTY       8150

#define DRV_A1    (gpio_num_t)17
#define DRV_A2    (gpio_num_t)16
#define DRV_B1    (gpio_num_t)18
#define DRV_B2    (gpio_num_t)19

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<DRV_A1) | (1ULL<<DRV_A2) | (1ULL<<DRV_B1) | (1ULL<<DRV_B2))

class MotorsDriver {
    public:
        MotorsDriver();
        void set_speed(int left, int right);
        void stop();
    private:
        ledc_channel_config_t pwm_channel_A;
        ledc_channel_config_t pwm_channel_B;
};

#endif //BOLT_MOTORS_DRIVER_H
