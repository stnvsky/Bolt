//
// Created by szymon on 04.09.22.
//

#include "control.h"
#include "motors.h"
#include "IMU.h"

extern MotorsDriver motors;

extern int32_t velocity_left_1e6;  // [m/s] * 1e6
extern int32_t velocity_right_1e6; // [m/s] * 1e6

const auto theta_set = 0;
const auto velocity_set = 0;

const auto Kp1 = -50.0f;
const auto Ti1 = 0.24f;
const auto Td1 = 0.25f;

const auto Kp2 = 6.0f;
const auto Td2 = 1.5f;

const auto h = 0.05f; // [s]

static auto e1_sum = 0.0f;
static auto prev_e1 = 0.0f;
static auto prev_e2 = 0.0f;

[[noreturn]] void vTaskControl(void *)
{
    while (true) {
        auto theta = static_cast<float>(get_theta())/1000000.0;
        auto velocity = static_cast<float>((velocity_left_1e6+velocity_right_1e6))/2000000.0;

        auto e1 = theta_set - theta;
        auto e2 = velocity_set - velocity;

        e1_sum += e1;

        auto u1 = Kp1 * (e1 + (h/Ti1)*e1_sum + Td1*((e1 - prev_e1)/h));
        auto u2 = Kp2 * (e2 + Td2*((e2 - prev_e2)/h));
        auto u = u1 + u2;

        prev_e1 = e1;
        prev_e2 = e2;

        auto speed = static_cast<int32_t>(u);
        printf("theta = %.2f, velocity = %.2f, set_speed = %d\n", theta, velocity, speed);
        motors.set_speed(speed,speed);

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}