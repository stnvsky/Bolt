#include "motors.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define GPIO_INPUT_IO_0     static_cast<gpio_num_t>(25)
#define GPIO_INPUT_IO_1     static_cast<gpio_num_t>(26)
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0

uint32_t gpio_25 = 0;
uint32_t gpio_26 = 0;

static QueueHandle_t gpio_evt_queue = nullptr;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    auto gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            if (io_num == 25)
            {
                gpio_25++;
            }
            else if (io_num == 26)
            {
                gpio_26++;
            }
        }
    }
}

void vTaskMotors( void * pvParameters )
{
    MotorsDriver motors;

    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = static_cast<gpio_pulldown_t>(0);
    //disable pull-up mode
    io_conf.pull_up_en = static_cast<gpio_pullup_t>(0);
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = static_cast<gpio_pullup_t>(1);
    gpio_config(&io_conf);

    //change gpio interrupt type for one pin
    gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);

    while (1) {
        motors.set_speed(0, 10);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("(10) - gpio25: %u, gpio26: %u\n", gpio_25, gpio_26);
        gpio_25 = 0;
        gpio_26 = 0;
        motors.set_speed(0, 15);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("(15) - gpio25: %u, gpio26: %u\n", gpio_25, gpio_26);
        gpio_25 = 0;
        gpio_26 = 0;
        motors.set_speed(0, -10);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("(-10) - gpio25: %u, gpio26: %u\n", gpio_25, gpio_26);
        gpio_25 = 0;
        gpio_26 = 0;
        motors.set_speed(0, -15);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("(-15) - gpio25: %u, gpio26: %u\n", gpio_25, gpio_26);
        gpio_25 = 0;
        gpio_26 = 0;
    }
}