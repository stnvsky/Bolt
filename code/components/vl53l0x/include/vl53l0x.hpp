// VL53L0X control
// Copyright © 2021 Szymon Stanik

#ifndef VL53L0X_H
#define VL53L0X_H

#include <stdint.h>
#include <stdio.h>
#include <driver/i2c.h>
#include "registers.hpp"

// I2C master will check ack from slave
#define ACK_CHECK_EN           0x1              

#define ACK_VAL               (i2c_ack_type_t)0x0              /*!< I2C ack value */
#define NACK_VAL              (i2c_ack_type_t)0x1              /*!< I2C nack value */


// Record the current time to check an upcoming timeout against
#define startTimeout() (timeout_start_ms = xTaskGetTickCount())

// Check if timeout is enabled (set to nonzero value) and has expired
#define checkTimeoutExpired() (io_timeout > 0 && ((uint16_t)(xTaskGetTickCount() - timeout_start_ms) > io_timeout))

// Decode VCSEL (vertical cavity surface emitting laser) pulse period in PCLKs
// from register value
// based on VL53L0X_decode_vcsel_period()
#define decodeVcselPeriod(reg_val)      (((reg_val) + 1) << 1)

// Encode VCSEL pulse period register value from period in PCLKs
// based on VL53L0X_encode_vcsel_period()
#define encodeVcselPeriod(period_pclks) (((period_pclks) >> 1) - 1)

// Calculate macro period in *nanoseconds* from VCSEL period in PCLKs
// based on VL53L0X_calc_macro_period_ps()
// PLL_period_ps = 1655; macro_period_vclks = 2304
#define calcMacroPeriod(vcsel_period_pclks) ((((uint32_t)2304 * (vcsel_period_pclks) * 1655) + 500) / 1000)


class vl53l0x {
    public:
        vl53l0x(uint8_t address, gpio_num_t xshut_pin, uint8_t i2c_port = I2C_NUM_0);
        ~vl53l0x();

        void enable();
        void disable();

        void start(uint32_t period_ms);
        void stop();
        uint16_t read();

    private:
        uint8_t _address;
        uint8_t _i2c_port;
        gpio_num_t _xshut_pin;

        bool enabled = true;

        uint8_t stop_variable; // read by init and used when starting measurement; is StopVariable field of VL53L0X_DevData_t structure in API
        uint32_t measurement_timing_budget_us;
        uint32_t timeout_start_ms;
        uint32_t io_timeout;
        bool did_timeout;

        enum vcselPeriodType { VcselPeriodPreRange, VcselPeriodFinalRange };

        struct SequenceStepEnables
        {
            bool tcc, msrc, dss, pre_range, final_range;
        };

        struct SequenceStepTimeouts
        {
            uint16_t pre_range_vcsel_period_pclks, final_range_vcsel_period_pclks;
            uint16_t msrc_dss_tcc_mclks, pre_range_mclks, final_range_mclks;
            uint32_t msrc_dss_tcc_us,    pre_range_us,    final_range_us;
        };

        void set_addr(uint8_t new_addr);
        void enable_device();
        bool init_device();
        void wait_for_device();  

        esp_err_t write_reg(uint8_t reg, uint8_t data);
        esp_err_t write_reg(uint8_t reg, uint8_t *data, size_t size);
        void write_reg_16bit(uint8_t reg, uint16_t data);
        void write_reg_32bit(uint8_t reg, uint32_t data);

        uint8_t read_reg(uint8_t reg);
        void read_reg(uint8_t reg, uint8_t *data, size_t size);
        uint16_t read_reg_16bit(uint8_t reg);
        uint32_t read_reg_32bit(uint8_t reg);

        bool getSpadInfo(uint8_t * count, bool * type_is_aperture);
        bool setSignalRateLimit(float limit_Mcps);
        void getSequenceStepEnables(SequenceStepEnables * enables);
        void getSequenceStepTimeouts(SequenceStepEnables const * enables, SequenceStepTimeouts * timeouts);
        bool setMeasurementTimingBudget(uint32_t budget_us);
        bool performSingleRefCalibration(uint8_t vhv_init_byte);
        uint8_t getVcselPulsePeriod(vcselPeriodType type);
        uint16_t encodeTimeout(uint32_t timeout_mclks);
        uint16_t decodeTimeout(uint16_t reg_val);
        uint32_t getMeasurementTimingBudget();
        uint32_t timeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks);
        uint32_t timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks);
};

#endif // VL53L0X_H
