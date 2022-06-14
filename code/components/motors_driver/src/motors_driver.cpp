//
// Created by szymon on 13.06.22.
//

#include "../include/motors_driver.h"

MotorsDriver::MotorsDriver() {
    this->io_conf.mode = GPIO_MODE_OUTPUT;
    this->io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    gpio_config(&this->io_conf);
    gpio_set_level(GPIO_OUTPUT_IO_0, 1);
    gpio_set_level(GPIO_OUTPUT_IO_1, 0);

    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer;

    ledc_timer.duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
    ledc_timer.freq_hz = 5000,                      // frequency of PWM signal
    ledc_timer.speed_mode = LEDC_HS_MODE;           // timer mode
    ledc_timer.timer_num = LEDC_HS_TIMER;            // timer index
    ledc_timer.clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock

            ledc_timer_config(&ledc_timer);
    /*
     * Prepare individual configuration
     * for each channel of LED Controller
     * by selecting:
     * - controller's channel number
     * - output duty cycle, set initially to 0
     * - GPIO number where LED is connected to
     * - speed mode, either high or low
     * - timer servicing selected channel
     *   Note: if different channels use one timer,
     *         then frequency and bit_num of these channels
     *         will be the same
     */

    this->ledc_channel[0].channel    = LEDC_HS_CH0_CHANNEL;
    this->ledc_channel[0].duty       = 0;
    this->ledc_channel[0].gpio_num   = LEDC_HS_CH0_GPIO;
    this->ledc_channel[0].speed_mode = LEDC_HS_MODE;
    this->ledc_channel[0].hpoint     = 0;
    this->ledc_channel[0].timer_sel  = LEDC_HS_TIMER;

    this->ledc_channel[1].channel    = LEDC_HS_CH1_CHANNEL;
    this->ledc_channel[1].duty       = 0;
    this->ledc_channel[1].gpio_num   = LEDC_HS_CH1_GPIO;
    this->ledc_channel[1].speed_mode = LEDC_HS_MODE;
    this->ledc_channel[1].hpoint     = 0;
    this->ledc_channel[1].timer_sel  = LEDC_HS_TIMER;

    this->ledc_channel[2].channel    = LEDC_LS_CH2_CHANNEL;
    this->ledc_channel[2].duty       = 0;
    this->ledc_channel[2].gpio_num   = LEDC_LS_CH2_GPIO;
    this->ledc_channel[2].speed_mode = LEDC_LS_MODE;
    this->ledc_channel[2].hpoint     = 0;
    this->ledc_channel[2].timer_sel  = LEDC_LS_TIMER;

    this->ledc_channel[3].channel    = LEDC_LS_CH3_CHANNEL;
    this->ledc_channel[3].duty       = 0;
    this->ledc_channel[3].gpio_num   = LEDC_LS_CH3_GPIO;
    this->ledc_channel[3].speed_mode = LEDC_LS_MODE;
    this->ledc_channel[3].hpoint     = 0;
    this->ledc_channel[3].timer_sel  = LEDC_LS_TIMER;

    // Set LED Controller with previously prepared configuration
    for (int ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        ledc_channel_config(&this->ledc_channel[ch]);
    }

    // Initialize fade service.
    ledc_fade_func_install(0);
};

int MotorsDriver::set_speed(int left, int right) {
        for (int ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_duty(this->ledc_channel[ch].speed_mode, this->ledc_channel[ch].channel, LEDC_TEST_DUTY);
            ledc_update_duty(this->ledc_channel[ch].speed_mode, this->ledc_channel[ch].channel);
        }
        vTaskDelay(2500 / portTICK_PERIOD_MS);

        for (int ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_duty(this->ledc_channel[ch].speed_mode, this->ledc_channel[ch].channel, 0);
            ledc_update_duty(this->ledc_channel[ch].speed_mode, this->ledc_channel[ch].channel);
        }
        vTaskDelay(2500 / portTICK_PERIOD_MS);

        gpio_set_level(GPIO_OUTPUT_IO_0, 0);
        gpio_set_level(GPIO_OUTPUT_IO_1, 1);

        for (int ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_duty(this->ledc_channel[ch].speed_mode, this->ledc_channel[ch].channel, LEDC_TEST_DUTY);
            ledc_update_duty(this->ledc_channel[ch].speed_mode, this->ledc_channel[ch].channel);
        }
        vTaskDelay(2500 / portTICK_PERIOD_MS);

        for (int ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_duty(this->ledc_channel[ch].speed_mode, this->ledc_channel[ch].channel, 0);
            ledc_update_duty(this->ledc_channel[ch].speed_mode, this->ledc_channel[ch].channel);
        }
        vTaskDelay(2500 / portTICK_PERIOD_MS);

        gpio_set_level(GPIO_OUTPUT_IO_0, 1);
        gpio_set_level(GPIO_OUTPUT_IO_1, 0);

    return 0;
};

int MotorsDriver::stop() {
    this->set_speed(0,0);
    return 0;
};
