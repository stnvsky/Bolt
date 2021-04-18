#include "vl53l0x.hpp"

#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/


#define ACK_VAL                            (i2c_ack_type_t)0x0              /*!< I2C ack value */
#define NACK_VAL                           (i2c_ack_type_t)0x1              /*!< I2C nack value */

enum regAddr
        {
        SYSRANGE_START                              = 0x00,

        SYSTEM_THRESH_HIGH                          = 0x0C,
        SYSTEM_THRESH_LOW                           = 0x0E,

        SYSTEM_SEQUENCE_CONFIG                      = 0x01,
        SYSTEM_RANGE_CONFIG                         = 0x09,
        SYSTEM_INTERMEASUREMENT_PERIOD              = 0x04,

        SYSTEM_INTERRUPT_CONFIG_GPIO                = 0x0A,

        GPIO_HV_MUX_ACTIVE_HIGH                     = 0x84,

        SYSTEM_INTERRUPT_CLEAR                      = 0x0B,

        RESULT_INTERRUPT_STATUS                     = 0x13,
        RESULT_RANGE_STATUS                         = 0x14,

        RESULT_CORE_AMBIENT_WINDOW_EVENTS_RTN       = 0xBC,
        RESULT_CORE_RANGING_TOTAL_EVENTS_RTN        = 0xC0,
        RESULT_CORE_AMBIENT_WINDOW_EVENTS_REF       = 0xD0,
        RESULT_CORE_RANGING_TOTAL_EVENTS_REF        = 0xD4,
        RESULT_PEAK_SIGNAL_RATE_REF                 = 0xB6,

        ALGO_PART_TO_PART_RANGE_OFFSET_MM           = 0x28,

        I2C_SLAVE_DEVICE_ADDRESS                    = 0x8A,

        MSRC_CONFIG_CONTROL                         = 0x60,

        PRE_RANGE_CONFIG_MIN_SNR                    = 0x27,
        PRE_RANGE_CONFIG_VALID_PHASE_LOW            = 0x56,
        PRE_RANGE_CONFIG_VALID_PHASE_HIGH           = 0x57,
        PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT          = 0x64,

        FINAL_RANGE_CONFIG_MIN_SNR                  = 0x67,
        FINAL_RANGE_CONFIG_VALID_PHASE_LOW          = 0x47,
        FINAL_RANGE_CONFIG_VALID_PHASE_HIGH         = 0x48,
        FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT = 0x44,

        PRE_RANGE_CONFIG_SIGMA_THRESH_HI            = 0x61,
        PRE_RANGE_CONFIG_SIGMA_THRESH_LO            = 0x62,

        PRE_RANGE_CONFIG_VCSEL_PERIOD               = 0x50,
        PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI          = 0x51,
        PRE_RANGE_CONFIG_TIMEOUT_MACROP_LO          = 0x52,

        SYSTEM_HISTOGRAM_BIN                        = 0x81,
        HISTOGRAM_CONFIG_INITIAL_PHASE_SELECT       = 0x33,
        HISTOGRAM_CONFIG_READOUT_CTRL               = 0x55,

        FINAL_RANGE_CONFIG_VCSEL_PERIOD             = 0x70,
        FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI        = 0x71,
        FINAL_RANGE_CONFIG_TIMEOUT_MACROP_LO        = 0x72,
        CROSSTALK_COMPENSATION_PEAK_RATE_MCPS       = 0x20,

        MSRC_CONFIG_TIMEOUT_MACROP                  = 0x46,

        SOFT_RESET_GO2_SOFT_RESET_N                 = 0xBF,
        IDENTIFICATION_MODEL_ID                     = 0xC0,
        IDENTIFICATION_REVISION_ID                  = 0xC2,

        OSC_CALIBRATE_VAL                           = 0xF8,

        GLOBAL_CONFIG_VCSEL_WIDTH                   = 0x32,
        GLOBAL_CONFIG_SPAD_ENABLES_REF_0            = 0xB0,
        GLOBAL_CONFIG_SPAD_ENABLES_REF_1            = 0xB1,
        GLOBAL_CONFIG_SPAD_ENABLES_REF_2            = 0xB2,
        GLOBAL_CONFIG_SPAD_ENABLES_REF_3            = 0xB3,
        GLOBAL_CONFIG_SPAD_ENABLES_REF_4            = 0xB4,
        GLOBAL_CONFIG_SPAD_ENABLES_REF_5            = 0xB5,

        GLOBAL_CONFIG_REF_EN_START_SELECT           = 0xB6,
        DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD         = 0x4E,
        DYNAMIC_SPAD_REF_EN_START_OFFSET            = 0x4F,
        POWER_MANAGEMENT_GO1_POWER_FORCE            = 0x80,

        VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV           = 0x89,

        ALGO_PHASECAL_LIM                           = 0x30,
        ALGO_PHASECAL_CONFIG_TIMEOUT                = 0x30,
        };

bool i2c_inited = false;

vl53l0x::vl53l0x(uint8_t address, uint8_t sda_pin, uint8_t scl_pin, gpio_num_t xshut_pin, uint32_t clk_speed, uint8_t i2c_port)
{
   _i2c_port = i2c_port;
   _address = address;

   if (!i2c_inited)
   {
      conf.mode = I2C_MODE_MASTER;
      conf.sda_io_num = sda_pin;
      conf.scl_io_num = scl_pin;
      conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
      conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
      conf.master.clk_speed = clk_speed;
      conf.clk_flags = 0;

      esp_err_t err1 = i2c_param_config(_i2c_port, &conf);
      esp_err_t err2 = i2c_driver_install(_i2c_port, conf.mode, 0, 0, 0);

      i2c_inited = true;
      printf("I2C inited succesfully %d %d\n", err1, err2);
   }

   gpio_set_level(xshut_pin, 1);

   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (_address << 1) | I2C_MASTER_WRITE, 1);
   i2c_master_stop(cmd);
   int8_t ret = i2c_master_cmd_begin(_i2c_port, cmd, 100 / portTICK_RATE_MS);
   i2c_cmd_link_delete(cmd);

   if (ret == ESP_OK)
   {
      printf("Device found under the address 0x%2X\n", _address);
   }


}

vl53l0x::~vl53l0x()
{

}


void vl53l0x::set_addr(uint8_t new_addr)
{
   write_reg(I2C_SLAVE_DEVICE_ADDRESS, (new_addr << 1));
}


void vl53l0x::write_reg(uint8_t reg, uint8_t data)
{
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (_address << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
   i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
   i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
   i2c_master_stop(cmd);
   int8_t ret = i2c_master_cmd_begin(_i2c_port, cmd, 100 / portTICK_RATE_MS);
   printf("write_reg ret = %d\n", ret);
   i2c_cmd_link_delete(cmd);
}


void vl53l0x::read_reg(uint8_t reg, uint8_t *data, size_t size)
{
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (_address << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
   i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (_address << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
   if (size > 1) {
        i2c_master_read(cmd, data, size - 1, ACK_VAL);
   }
   i2c_master_read_byte(cmd, data + size - 1, NACK_VAL);
   i2c_master_stop(cmd);
   esp_err_t ret = i2c_master_cmd_begin(_i2c_port, cmd, 1000 / portTICK_RATE_MS);
   i2c_cmd_link_delete(cmd);
}