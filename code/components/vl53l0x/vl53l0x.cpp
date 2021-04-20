#include "vl53l0x.hpp"


vl53l0x::vl53l0x(uint8_t address, gpio_num_t xshut_pin, uint8_t i2c_port)
{
   _i2c_port = i2c_port;  
   _xshut_pin = xshut_pin;
   _address = address;

   enable_device();
}

vl53l0x::~vl53l0x()
{
   gpio_set_level(_xshut_pin, 0);
}

void vl53l0x::enable()
{
   if (!enabled)
   {
      enabled = true;
      enable_device();
   }
}


void vl53l0x::disable()
{
   if (enabled)
   {
      enabled = false;
      gpio_set_level(_xshut_pin, 0);
   }
}


void vl53l0x::set_addr(uint8_t new_addr)
{
   if (enabled)
   {
      uint32_t start_time = xTaskGetTickCount();
      uint32_t time = start_time;
      esp_err_t ret = ESP_FAIL;
      while (ret != ESP_OK)
      {
         ret = write_reg(0x8A, &new_addr, 1);
         // timeout after 100 ms
         time = xTaskGetTickCount();
         if (time - start_time > 100)
         {
            printf("ERROR - set_addr failed");
            enabled = false;
            break;
         }
      }

      _address = new_addr;
   }
   else
   {
      printf("ERROR - could not set addres - device is disabled");
   }
}


void vl53l0x::wait_for_device()
{
   // wait for the device to wake up
   uint32_t start_time = xTaskGetTickCount();
   uint32_t time = start_time;
   esp_err_t ret = ESP_FAIL;
   while (ret != ESP_OK)
   {
      i2c_cmd_handle_t cmd = i2c_cmd_link_create();
      i2c_master_start(cmd);
      i2c_master_write_byte(cmd, (_address << 1) | I2C_MASTER_WRITE, 1);
      i2c_master_stop(cmd);
      ret = i2c_master_cmd_begin(_i2c_port, cmd, 100 / portTICK_RATE_MS);
      i2c_cmd_link_delete(cmd);

      // timeout after 100 ms
      time = xTaskGetTickCount();
      if (time - start_time > 100)
      {
         printf("ERROR - vl53l0x - could not connect to the device\n");
         gpio_set_level(_xshut_pin, 0);
         enabled = false;
         break;
      }
   }
}

void vl53l0x::enable_device()
{
   uint8_t temp_addr = _address;
   _address = 0x29;

   gpio_set_level(_xshut_pin, 1);

   wait_for_device();

   if (enabled)
   {
      this->set_addr(temp_addr);
      wait_for_device();
      bool ret = init_device();
      if (ret != true)
      {
         printf("ERROR - device initialization failed\n");
      }
   }
}
   

// Start continuous ranging measurements. If period_ms (optional) is 0 or not
// given, continuous back-to-back mode is used (the sensor takes measurements as
// often as possible); otherwise, continuous timed mode is used, with the given
// inter-measurement period in milliseconds determining how often the sensor
// takes a measurement.
void vl53l0x::start(uint32_t period_ms)
{
  write_reg(0x80, 0x01);
  write_reg(0xFF, 0x01);
  write_reg(0x00, 0x00);
  write_reg(0x91, stop_variable);
  write_reg(0x00, 0x01);
  write_reg(0xFF, 0x00);
  write_reg(0x80, 0x00);

  if (period_ms != 0)
  {
    uint16_t osc_calibrate_val = read_reg_16bit(OSC_CALIBRATE_VAL);

    if (osc_calibrate_val != 0)
    {
      period_ms *= osc_calibrate_val;
    }

    write_reg_32bit(SYSTEM_INTERMEASUREMENT_PERIOD, period_ms);
    write_reg(SYSRANGE_START, 0x04); // VL53L0X_REG_SYSRANGE_MODE_TIMED
  }
  else
  {
    // continuous back-to-back mode
    write_reg(SYSRANGE_START, 0x02); // VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK
  }
}


// Stop continuous measurements
void vl53l0x::stop()
{
  write_reg(SYSRANGE_START, 0x01); // VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT

  write_reg(0xFF, 0x01);
  write_reg(0x00, 0x00);
  write_reg(0x91, 0x00);
  write_reg(0x00, 0x01);
  write_reg(0xFF, 0x00);
}


// Returns a range reading in millimeters when continuous mode is active
// (readRangeSingleMillimeters() also calls this function after starting a
// single-shot range measurement)
uint16_t vl53l0x::read()
{
  startTimeout();
  while ((read_reg(RESULT_INTERRUPT_STATUS) & 0x07) == 0)
  {
    if (checkTimeoutExpired())
    {
      did_timeout = true;
      return 65535;
    }
  }

  // assumptions: Linearity Corrective Gain is 1000 (default);
  // fractional ranging is not enabled
  uint16_t range = read_reg_16bit(RESULT_RANGE_STATUS + 10);

  write_reg(SYSTEM_INTERRUPT_CLEAR, 0x01);

  return range;
}