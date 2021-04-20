/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include <driver/i2c.h>
#include "vl53l0x.hpp"
#include <cstring>

#define I2C_SDA_PIN 26
#define I2C_SCL_PIN 25

#define XSHUT_PIN_1             GPIO_NUM_15
#define XSHUT_PIN_2             GPIO_NUM_22
#define XSHUT_PIN_3             GPIO_NUM_23


void scan()
{
    printf("i2c scan: \n");
    for (uint8_t i = 1; i < 127; i++)
    {
        int ret;
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);
    
        if (ret == ESP_OK)
        {
            printf("Found device at: 0x%02x\n", i);
        }
    }
    printf("\n\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}


void init()
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_SDA_PIN;
    conf.scl_io_num = I2C_SCL_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;
    conf.clk_flags = 0;

    esp_err_t err1 = i2c_param_config(I2C_NUM_0, &conf);
    esp_err_t err2 = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);

    printf("I2C inited succesfully %d %d\n", err1, err2);

    gpio_config_t io_conf;
    io_conf.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL<<XSHUT_PIN_1) | (1ULL<<XSHUT_PIN_2) | (1ULL<<XSHUT_PIN_3);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    gpio_set_level(XSHUT_PIN_1, 0);
    gpio_set_level(XSHUT_PIN_2, 0);
    gpio_set_level(XSHUT_PIN_3, 0);

    printf("\n\n");
}


extern "C" void app_main()
{
    init();

    vl53l0x dist_1(0x01, XSHUT_PIN_1);
    vl53l0x dist_2(0x02, XSHUT_PIN_2);
    vl53l0x dist_3(0x03, XSHUT_PIN_3);

    dist_1.start(50);
    dist_2.start(50);
    dist_3.start(50);

    uint16_t d[3] = {0};

    while (1)
    {
        d[0] = dist_1.read();
        d[1] = dist_2.read();
        d[2] = dist_3.read();

        for (int i = 0; i < 3; i++)
        {
            printf("dist %d. %d mm\n", i, d[i]);
        }
        printf("\n");

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}