//
// Created by szymon on 04.09.22.
//

#include "control.h"
#include "motors.h"
#include "IMU.h"

extern MotorsDriver motors;

extern int32_t velocity_left_1e6;  // [m/s] * 1e6
extern int32_t velocity_right_1e6; // [m/s] * 1e6

const auto theta_set = 0.0f;
const auto velocity_set = 0.0f;

auto Kp1 = -80.0f;
auto Ki1 = -10.69f;
auto Kd1 = -0.2f;

auto Kp2 = 0.5f;
auto Kd2 = 0.1f;

const auto h = 0.05f; // [s]

static auto e1_sum = 0.0f;
static auto prev_e1 = 0.0f;
static auto prev_e2 = 0.0f;

int change_kp1(int argc, char **argv)
{
    Kp1 = strtof(argv[1], nullptr);
    return 0;
}

int change_ki1(int argc, char **argv)
{
    Ki1 = strtof(argv[1], nullptr);
    return 0;
}

int change_kd1(int argc, char **argv)
{
    Kd1 = strtof(argv[1], nullptr);
    return 0;
}

int change_kp2(int argc, char **argv)
{
    Kp2 = strtof(argv[1], nullptr);
    return 0;
}

int change_kd2(int argc, char **argv)
{
    Kd2 = strtof(argv[1], nullptr);
    return 0;
}

int print_settings(int argc, char **argv)
{
    printf("\n\nSettings:\n");
    printf("Kp1: %.2f\n", Kp1);
    printf("Ki1: %.2f\n", Ki1);
    printf("Kd1: %.2f\n", Kd1);
    printf("Kp2: %.2f\n", Kp2);
    printf("Kd2: %.2f\n", Kd2);
    return 0;
}

[[noreturn]] void vTaskControl(void *)
{
    while (true) {
        auto theta = get_theta();
        auto velocity = static_cast<float>((velocity_left_1e6+velocity_right_1e6))/2000000.0f;

        auto e1 = theta_set - theta;
        auto e2 = velocity_set - velocity;

        e1_sum += e1;

        auto u1 = Kp1*e1 + Ki1*h*e1_sum + Kd1*((e1 - prev_e1)/h);
        auto u2 = Kp2*e2 + Kd2*((e2 - prev_e2)/h);
        auto u = u1 + u2;

        prev_e1 = e1;
        prev_e2 = e2;

        auto speed = static_cast<int32_t>(u);
        //printf("theta = %.2f, velocity = %.2f, set_speed = %ld\n", theta, velocity, speed);
        motors.set_speed(speed,speed);

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}