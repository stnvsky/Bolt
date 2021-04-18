// VL53L0X control
// Copyright © 2021 Szymon Stanik

#ifndef VL53L0X_H
#define VL53L0X_H

#include <stdint.h>
#include <stdio.h>
#include <driver/i2c.h>


class vl53l0x {
    public:
        vl53l0x(uint8_t address, uint8_t sda_pin, uint8_t scl_pin, gpio_num_t xshut_pin, \
            uint32_t clk_speed = 100000, uint8_t i2c_port = I2C_NUM_0);
        ~vl53l0x();

        void write_reg(uint8_t addr, uint8_t data);
        void read_reg(uint8_t reg, uint8_t *data, size_t size);

        void set_addr(uint8_t new_addr);

    private:
        uint8_t _address;
        uint8_t _i2c_port;
        i2c_config_t conf;
};





#endif // VL53L0X_H
