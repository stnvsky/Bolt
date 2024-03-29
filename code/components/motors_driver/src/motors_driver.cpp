//
// Created by Szymon Stanik on 13.06.22.
//

#include "../include/motors_driver.h"

MotorsDriver::MotorsDriver() {
    gpio_config_t io_conf;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    gpio_config(&io_conf);
    gpio_set_level(DRV_A1, 0);
    gpio_set_level(DRV_A2, 0);
    gpio_set_level(DRV_B1, 0);
    gpio_set_level(DRV_B2, 0);

    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer;
    ledc_timer.duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
    ledc_timer.freq_hz = 5000,                      // frequency of PWM signal
    ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;           // timer mode
    ledc_timer.timer_num = LEDC_TIMER_0;            // timer index
    ledc_timer.clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    ledc_timer_config(&ledc_timer);

    this->pwm_channel_A.channel    = LEDC_CHANNEL_0;
    this->pwm_channel_A.duty       = 0;
    this->pwm_channel_A.gpio_num   = (gpio_num_t)(4);
    this->pwm_channel_A.speed_mode = LEDC_HIGH_SPEED_MODE;
    this->pwm_channel_A.hpoint     = 0;
    this->pwm_channel_A.timer_sel  = LEDC_TIMER_0;
    ledc_channel_config(&this->pwm_channel_A);

    this->pwm_channel_B.channel    = LEDC_CHANNEL_1;
    this->pwm_channel_B.duty       = 0;
    this->pwm_channel_B.gpio_num   = (gpio_num_t)(21);
    this->pwm_channel_B.speed_mode = LEDC_HIGH_SPEED_MODE;
    this->pwm_channel_B.hpoint     = 0;
    this->pwm_channel_B.timer_sel  = LEDC_TIMER_0;
    ledc_channel_config(&this->pwm_channel_B);
};

const auto MOTORS_SAMPLING_PERIOD = 50.0f;
auto InertionTime = 100.0f;

int change_inertion(int argc, char **argv)
{
    long arg = strtol(argv[1], nullptr, 10);
    InertionTime = static_cast<float>(arg);
    return 0;
}

void MotorsDriver::set_speed(int left, int right) {

    const int MAX_SPEED = 100;
    if(!is_halt)
    {
        int left_speed;
        int right_speed;

        if (left < -MAX_SPEED) left = -MAX_SPEED;
        if (left > MAX_SPEED) left = MAX_SPEED;
        if (right < -MAX_SPEED) right = -MAX_SPEED;
        if (right > MAX_SPEED) right = MAX_SPEED;

        static float L_speed = 0;
        static float R_speed = 0;
        static float Tp = MOTORS_SAMPLING_PERIOD; 	/* Okres próbkowania */
        auto T = (float)InertionTime;		/* Stała czasowa inercji nastawy silnika [ms] */

//    L_speed = (Tp/T) * (static_cast<float>(left) - L_speed) + L_speed;
//    R_speed = (Tp/T) * (static_cast<float>(right) - R_speed) + R_speed;

        L_speed = static_cast<float>(left);
        R_speed = static_cast<float>(right);

        if (L_speed < 0)
        {
            gpio_set_level(DRV_A1, 1);
            gpio_set_level(DRV_A2, 0);
            left_speed = (-static_cast<int32_t>(L_speed) * MAX_DUTY)/ 100;
        }
        else
        {
            gpio_set_level(DRV_A1, 0);
            gpio_set_level(DRV_A2, 1);
            left_speed = static_cast<int32_t>((L_speed) * MAX_DUTY)/ 100;
        }

        ledc_set_duty(this->pwm_channel_A.speed_mode, this->pwm_channel_A.channel, left_speed);
        ledc_update_duty(this->pwm_channel_A.speed_mode, this->pwm_channel_A.channel);

        if (R_speed < 0)
        {
            gpio_set_level(DRV_B1, 0);
            gpio_set_level(DRV_B2, 1);
            right_speed = (-static_cast<int32_t>(R_speed) * MAX_DUTY)/ 100;
        }
        else
        {
            gpio_set_level(DRV_B1, 1);
            gpio_set_level(DRV_B2, 0);
            right_speed = (static_cast<int32_t>(R_speed) * MAX_DUTY)/ 100;
        }

        ledc_set_duty(this->pwm_channel_B.speed_mode, this->pwm_channel_B.channel, right_speed);
        ledc_update_duty(this->pwm_channel_B.speed_mode, this->pwm_channel_B.channel);
    }
};

void MotorsDriver::halt() {
    this->set_speed(0,0);
    is_halt = true;
};

void MotorsDriver::resume() {
    is_halt = false;
};
