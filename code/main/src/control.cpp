//
// Created by szymon on 04.09.22.
//

#include "freertos/FreeRTOS.h"
#include <freertos/semphr.h>
#include "control.h"
#include "motors.h"
#include "IMU.h"

extern SemaphoreHandle_t control_semaphore;

extern MotorsDriver motors;

extern int32_t velocity_left_1e6;  // [m/s] * 1e6
extern int32_t velocity_right_1e6; // [m/s] * 1e6

const auto theta_set = 0.0f;
const auto velocity_set = 0.0f;

auto Kp1 = -1.0f;
auto Ki1 = 0.0f;
auto Kd1 = 0.f;

auto Kp2 = 0.5f;
auto Kd2 = 0.1f;

const auto h = 0.05f; // [s]

static auto e1_sum = 0.0f;
static auto prev_e1 = 0.0f;
static auto prev_e2 = 0.0f;

int change_kp1_alternative(const char* string)
{
    Kp1 = strtof(string, nullptr);
    return 0;
}

int change_ki1_alternative(const char* string)
{
    Ki1 = strtof(string, nullptr);
    return 0;
}

int change_kd1_alternative(const char* string)
{
    Kd1 = strtof(string, nullptr);
    return 0;
}

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
    while (true)
    {
        if( xSemaphoreTake(control_semaphore, ( TickType_t )portMAX_DELAY ) == pdTRUE )
        {
            auto theta = get_theta();
            auto velocity = static_cast<float>((velocity_left_1e6+velocity_right_1e6))/2000000.0f;

            if (abs(theta) > 1)
            {
                motors.halt();
                while (abs(get_theta()) > 0.2)
                {
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
                motors.resume();
            }

//        auto e1 = theta_set - theta;
//        auto e2 = velocity_set - velocity;
//
//        e1_sum += e1;
//
//        auto u1 = Kp1*e1 + Ki1*h*e1_sum + Kd1*((e1 - prev_e1)/h);
//        auto u2 = Kp2*e2 + Kd2*((e2 - prev_e2)/h);
//        auto u = u1 + u2;
//
//        prev_e1 = e1;
//        prev_e2 = e2;

            auto e1 = theta_set - theta;
            e1_sum += e1;

            auto u = Kp1*e1 + Ki1*h*e1_sum + Kd1*((e1 - prev_e1)/h);
            prev_e1 = e1;

            auto speed = static_cast<int32_t>(u);
            //printf("%.2f\n", u);
            motors.set_speed(speed,static_cast<int>(speed*0.93));
        }
    }
}