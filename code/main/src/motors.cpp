#include "motors.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include <cmath>
#include <vector>

#include "esp_console.h"
#include "esp_vfs_dev.h"

MotorsDriver motors;

const auto GPIO_HALL_SENSOR_1_A =    static_cast<gpio_num_t>(36);
const auto GPIO_HALL_SENSOR_1_B =    static_cast<gpio_num_t>(35);
const auto GPIO_HALL_SENSOR_2_A =    static_cast<gpio_num_t>(25);
const auto GPIO_HALL_SENSOR_2_B =    static_cast<gpio_num_t>(26);
const auto GPIO_INPUT_PIN_SEL   =    ((1ULL<<GPIO_HALL_SENSOR_1_A) | (1ULL<<GPIO_HALL_SENSOR_1_B) \
                                     | (1ULL<<GPIO_HALL_SENSOR_2_A) | (1ULL<<GPIO_HALL_SENSOR_2_B));
const auto ESP_INTR_FLAG_DEFAULT = 0;

const auto VELOCITY_COUNT_PERIOD = 100; // [ms]
const auto VELOCITY_COUNT_FREQUENCY = 1000 / VELOCITY_COUNT_PERIOD; // [hZ]

const auto MOTOR_GEAR = 10; // motor gear n:1
const auto WHEEL_DELIMETER = 9; // [cm]

const auto PI_TIMES_1E6 = static_cast<uint32_t>(M_PI * 1000000);
const auto WHEEL_CIRCUMFERENCE_1E6 = WHEEL_DELIMETER * PI_TIMES_1E6 / 100; // [m]
const auto ONE_TICK_DISTANCE_1E6 = WHEEL_CIRCUMFERENCE_1E6 / (12 * 2 * MOTOR_GEAR); // (12 * 2 * MOTOR_GEAR) ticks sum - 1 rotation
const auto TICKS_TO_METERS_PER_SECOND_1E6 = ONE_TICK_DISTANCE_1E6 * VELOCITY_COUNT_FREQUENCY;

int32_t hall_1_a_count = 0;
int32_t hall_1_b_count = 0;
int32_t hall_2_a_count = 0;
int32_t hall_2_b_count = 0;

int32_t velocity_left_1e6 = 0;  // [m/s] * 1e6
int32_t velocity_right_1e6 = 0; // [m/s] * 1e6

bool direction_forward_left = true;
bool direction_forward_right = true;

gptimer_handle_t gptimer = nullptr;

uint32_t last_1_a = 0, last_1_b = 0, last_2_a = 0, last_2_b = 0;
uint32_t delta_1_a = 0, delta_1_b = 0, delta_2_a = 0, delta_2_b = 0;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint64_t count;
    uint32_t tick_count;
    ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &count));

    tick_count = static_cast<uint32_t>(count);
    auto gpio_num = (uint32_t)arg;

    switch (gpio_num) {
        case 36: {
            hall_1_a_count++;
            delta_1_a = tick_count - last_1_b;
            last_1_a = tick_count;
            if (delta_1_a > delta_1_b) direction_forward_left = true; else direction_forward_left = false;
            break;
        }
        case 35: {
            hall_1_b_count++;
            delta_1_b = tick_count - last_1_a;
            last_1_b = tick_count;
            if (delta_1_a > delta_1_b) direction_forward_left = true; else direction_forward_left = false;
            break;
        }
        case 25: {
            hall_2_a_count++;
            delta_2_a = tick_count - last_2_b;
            last_2_a = tick_count;
            if (delta_2_a > delta_2_b) direction_forward_right = true; else direction_forward_right = false;
            break;
        }
        case 26: {
            hall_2_b_count++;
            delta_2_b = tick_count - last_2_a;
            last_2_b = tick_count;
            if (delta_2_a > delta_2_b) direction_forward_right = true; else direction_forward_right = false;
            break;
        }
        default:
            break;
    }
}

static bool IRAM_ATTR velocity_callback(gptimer_handle_t, const gptimer_alarm_event_data_t *, void *)
{
    auto ticks_sum_left = hall_1_a_count + hall_1_b_count;
    velocity_left_1e6 = static_cast<int32_t>(ticks_sum_left * TICKS_TO_METERS_PER_SECOND_1E6);
    if (!direction_forward_left) velocity_left_1e6 = -velocity_left_1e6;

    auto ticks_sum_right = hall_2_a_count + hall_2_b_count;
    velocity_right_1e6 = static_cast<int32_t>(ticks_sum_right * TICKS_TO_METERS_PER_SECOND_1E6);
    if (!direction_forward_right) velocity_right_1e6 = -velocity_right_1e6;

    hall_1_a_count = 0;
    hall_1_b_count = 0;
    hall_2_a_count = 0;
    hall_2_b_count = 0;

    return false;
}

extern int change_kp1(int argc, char **argv);
extern int change_ki1(int argc, char **argv);
extern int change_kd1(int argc, char **argv);
extern int change_kp2(int argc, char **argv);
extern int change_kd2(int argc, char **argv);
extern int change_inertion(int argc, char **argv);
extern int print_settings(int argc, char **argv);

[[noreturn]] void vTaskMotors(void *)
{
    //interrupt of rising edge
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = static_cast<gpio_pullup_t>(1);
    gpio_config(&io_conf);

    gptimer_config_t timer_config = {
            .clk_src = GPTIMER_CLK_SRC_DEFAULT,
            .direction = GPTIMER_COUNT_UP,
            .resolution_hz = 1000000, // 1MHz, 1 tick=1us
            .flags{.intr_shared = false}
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_event_callbacks_t cbs = {
            .on_alarm = velocity_callback,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, nullptr));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    gptimer_alarm_config_t alarm_config1 = {
            .alarm_count = VELOCITY_COUNT_PERIOD * 1000, // period = 50ms
            .reload_count = 0,
            .flags{.auto_reload_on_alarm = true}
    };

    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config1));
    ESP_ERROR_CHECK(gptimer_start(gptimer));

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handlers for specific gpio pins
    gpio_isr_handler_add(GPIO_HALL_SENSOR_1_A, gpio_isr_handler, (void*) GPIO_HALL_SENSOR_1_A);
    gpio_isr_handler_add(GPIO_HALL_SENSOR_1_B, gpio_isr_handler, (void*) GPIO_HALL_SENSOR_1_B);
    gpio_isr_handler_add(GPIO_HALL_SENSOR_2_A, gpio_isr_handler, (void*) GPIO_HALL_SENSOR_2_A);
    gpio_isr_handler_add(GPIO_HALL_SENSOR_2_B, gpio_isr_handler, (void*) GPIO_HALL_SENSOR_2_B);

    static const char* TAG = "example";
#define PROMPT_STR CONFIG_IDF_TARGET

    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    repl_config.prompt = PROMPT_STR ">";
    repl_config.max_cmdline_length = 128;

    esp_console_register_help_command();

    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

    ESP_ERROR_CHECK(esp_console_start_repl(repl));


    esp_console_cmd_t change_Kp1 = {
            .command = "Kp1",
            .help = "",
            .hint = "",
            .func = change_kp1,
            .argtable = nullptr
    };
    esp_console_cmd_register(&change_Kp1);

    esp_console_cmd_t change_Ki1 = {
            .command = "Ki1",
            .help = "",
            .hint = "",
            .func = change_ki1,
            .argtable = nullptr
    };
    esp_console_cmd_register(&change_Ki1);

    esp_console_cmd_t change_Kd1 = {
            .command = "Kd1",
            .help = "",
            .hint = "",
            .func = change_kd1,
            .argtable = nullptr
    };
    esp_console_cmd_register(&change_Kd1);

    esp_console_cmd_t change_Kp2 = {
            .command = "Kp2",
            .help = "",
            .hint = "",
            .func = change_kp2,
            .argtable = nullptr
    };
    esp_console_cmd_register(&change_Kp2);

    esp_console_cmd_t change_Kd2 = {
            .command = "Kd2",
            .help = "",
            .hint = "",
            .func = change_kd2,
            .argtable = nullptr
    };
    esp_console_cmd_register(&change_Kd2);

    esp_console_cmd_t printf_settings = {
            .command = "p",
            .help = "",
            .hint = "",
            .func = print_settings,
            .argtable = nullptr
    };
    esp_console_cmd_register(&printf_settings);

    esp_console_cmd_t change_inertion_cmd = {
            .command = "iner",
            .help = "",
            .hint = "",
            .func = change_inertion,
            .argtable = nullptr
    };
    esp_console_cmd_register(&change_inertion_cmd);

    for(;;) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}