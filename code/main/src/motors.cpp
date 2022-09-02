#include "motors.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"

#define GPIO_HALL_SENSOR_1_A     static_cast<gpio_num_t>(36)
#define GPIO_HALL_SENSOR_1_B     static_cast<gpio_num_t>(35)
#define GPIO_HALL_SENSOR_2_A     static_cast<gpio_num_t>(25)
#define GPIO_HALL_SENSOR_2_B     static_cast<gpio_num_t>(26)
#define GPIO_INPUT_PIN_SEL       ((1ULL<<GPIO_HALL_SENSOR_1_A) | (1ULL<<GPIO_HALL_SENSOR_1_B) \
                                 | (1ULL<<GPIO_HALL_SENSOR_2_A) | (1ULL<<GPIO_HALL_SENSOR_2_B))
#define ESP_INTR_FLAG_DEFAULT 0

static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;;

volatile uint32_t hall_1_a_count = 0;
volatile uint32_t hall_1_b_count = 0;
volatile uint32_t hall_2_a_count = 0;
volatile uint32_t hall_2_b_count = 0;

volatile int32_t velocity_left = 0;
volatile int32_t velocity_right = 0;

volatile bool direction_forward_left = true;
volatile bool direction_forward_right = true;

static QueueHandle_t gpio_evt_queue = nullptr;
gptimer_handle_t gptimer = nullptr;

typedef struct hall_interrupt_s {
    gpio_num_t gpio_num;
    uint64_t time;
} hall_interrupt_t;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint64_t count;
    ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &count));
    hall_interrupt_t hall_data = {static_cast<gpio_num_t>(reinterpret_cast<uint32_t>(arg)), count};
    xQueueSendFromISR(gpio_evt_queue, (void *)&hall_data, NULL);
}

static bool velocity_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    velocity_left = static_cast<int32_t>(hall_1_a_count + hall_1_b_count);
    if (!direction_forward_left) velocity_left = -velocity_left;

    velocity_right = static_cast<int32_t>(hall_2_a_count + hall_2_b_count);
    if (!direction_forward_right) velocity_right = -velocity_right;

    hall_1_a_count = 0;
    hall_1_b_count = 0;
    hall_2_a_count = 0;
    hall_2_b_count = 0;

    return false;
}

[[noreturn]] static void gpio_task_example(void*)
{
    hall_interrupt_t hall_data;

    uint64_t last_1_a = 0, last_1_b = 0, last_2_a = 0, last_2_b = 0;
    uint64_t delta_1_a = 0, delta_1_b = 0, delta_2_a = 0, delta_2_b = 0;

    gptimer_config_t timer_config = {
            .clk_src = GPTIMER_CLK_SRC_DEFAULT,
            .direction = GPTIMER_COUNT_UP,
            .resolution_hz = 1000000, // 1MHz, 1 tick=1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_event_callbacks_t cbs = {
            .on_alarm = velocity_callback,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, nullptr));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    gptimer_alarm_config_t alarm_config1 = {
            .alarm_count = 50000, // period = 20ms
    };
    alarm_config1.flags.auto_reload_on_alarm = true;
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config1));
    ESP_ERROR_CHECK(gptimer_start(gptimer));

    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &hall_data, portMAX_DELAY)) {
            if (hall_data.gpio_num == GPIO_HALL_SENSOR_1_A)
            {
                delta_1_a = hall_data.time - last_1_b;
                last_1_a = hall_data.time;
                taskENTER_CRITICAL(&mux);
                hall_1_a_count++;
                if (delta_1_a > delta_1_b) direction_forward_left = true; else direction_forward_left = false;
                taskEXIT_CRITICAL(&mux);
            }
            else if (hall_data.gpio_num == GPIO_HALL_SENSOR_1_B)
            {
                delta_1_b = hall_data.time - last_1_a;
                last_1_b = hall_data.time;
                taskENTER_CRITICAL(&mux);
                hall_1_b_count++;
                if (delta_1_a > delta_1_b) direction_forward_left = true; else direction_forward_left = false;
                taskEXIT_CRITICAL(&mux);
            }
            else if (hall_data.gpio_num == GPIO_HALL_SENSOR_2_A)
            {
                delta_2_a = hall_data.time - last_2_b;
                last_2_a = hall_data.time;
                taskENTER_CRITICAL(&mux);
                hall_2_a_count++;
                if (delta_2_a > delta_2_b) direction_forward_right = true; else direction_forward_right = false;
                taskEXIT_CRITICAL(&mux);
            }
            else if (hall_data.gpio_num == GPIO_HALL_SENSOR_2_B)
            {
                delta_2_b = hall_data.time - last_2_a;
                last_2_b = hall_data.time;
                taskENTER_CRITICAL(&mux);
                hall_2_b_count++;
                if (delta_2_a > delta_2_b) direction_forward_right = true; else direction_forward_right = false;
                taskEXIT_CRITICAL(&mux);
            }
        }
    }
}

[[noreturn]] void vTaskMotors(void *)
{
    MotorsDriver motors;

    //zero-initialize the config structure.
    gpio_config_t io_conf = {};

    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = static_cast<gpio_pullup_t>(1);
    gpio_config(&io_conf);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(hall_interrupt_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, nullptr, 10, nullptr);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handlers for specific gpio pins
    gpio_isr_handler_add(GPIO_HALL_SENSOR_1_A, gpio_isr_handler, (void*) GPIO_HALL_SENSOR_1_A);
    gpio_isr_handler_add(GPIO_HALL_SENSOR_1_B, gpio_isr_handler, (void*) GPIO_HALL_SENSOR_1_B);
    gpio_isr_handler_add(GPIO_HALL_SENSOR_2_A, gpio_isr_handler, (void*) GPIO_HALL_SENSOR_2_A);
    gpio_isr_handler_add(GPIO_HALL_SENSOR_2_B, gpio_isr_handler, (void*) GPIO_HALL_SENSOR_2_B);

    motors.set_speed(0, 20);
    for(;;) {
        motors.set_speed(20, 20);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("velocity %d, %d\n", velocity_left, velocity_right);
        motors.set_speed(25, 25);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("velocity %d, %d\n", velocity_left, velocity_right);
        motors.set_speed(-20, -20);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("velocity %d, %d\n", velocity_left, velocity_right);
        motors.set_speed(-25, -25);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("velocity %d, %d\n", velocity_left, velocity_right);
    }
}