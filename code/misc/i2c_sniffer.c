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
#include "esp_log.h"
#include <driver/i2c.h>

#define I2C_SDA_PIN 26
#define I2C_SCL_PIN 25

#define TAG "i2cscanner"

#define I2C_MASTER_FREQ_HZ 100000

// setup i2c master
static esp_err_t i2c_master_init()
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_SDA_PIN;
    conf.scl_io_num = I2C_SCL_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = 0;

    i2c_param_config(I2C_NUM_1, &conf);
    return i2c_driver_install(I2C_NUM_1, conf.mode, 0, 0, 0);
}

extern "C" void app_main()
{
    // i2c init & scan
    if (i2c_master_init() != ESP_OK)
        ESP_LOGE(TAG, "i2c init failed\n");

    while (1)
    {
        printf("i2c scan: \n");
        for (uint8_t i = 1; i < 127; i++)
        {
            int ret;
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1);
            i2c_master_stop(cmd);
            ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 100 / portTICK_PERIOD_MS);
            i2c_cmd_link_delete(cmd);
        
            if (ret == ESP_OK)
            {
                printf("Found device at: 0x%2x\n", i);
            }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
}