#include "vl53l0x.hpp"

esp_err_t vl53l0x::write_reg(uint8_t reg, uint8_t data)
{
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();

   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (_address << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
   i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);

   i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
   i2c_master_stop(cmd);

   esp_err_t ret = i2c_master_cmd_begin(_i2c_port, cmd, 100 / portTICK_PERIOD_MS);
   if (ret != ESP_OK)
   {
      printf("\n\nERROR - write_reg failed\n\n");
   }

   i2c_cmd_link_delete(cmd);

   return ret;
}

esp_err_t vl53l0x::write_reg(uint8_t reg, uint8_t *data, size_t size)
{
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();

   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (_address << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
   i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);

   i2c_master_write(cmd, data, size, ACK_CHECK_EN);
   i2c_master_stop(cmd);

   esp_err_t ret = i2c_master_cmd_begin(_i2c_port, cmd, 100 / portTICK_PERIOD_MS);
   if (ret != ESP_OK)
   {
      printf("WARNING - write_reg failed\n");
   }

   i2c_cmd_link_delete(cmd);

   return ret;
}


// Write a 16-bit register
void vl53l0x::write_reg_16bit(uint8_t reg, uint16_t data)
{
   uint8_t _data[2] = {0};
   _data[0] = (data >> 8) & 0xFF;
   _data[1] = data & 0xFF;

   i2c_cmd_handle_t cmd = i2c_cmd_link_create();

   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (_address << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
   i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
   i2c_master_write(cmd, _data, 2, ACK_CHECK_EN);
   i2c_master_stop(cmd);

   int8_t ret = i2c_master_cmd_begin(_i2c_port, cmd, 100 / portTICK_PERIOD_MS);
   if (ret != ESP_OK)
   {
      printf("WARNING - write_reg failed\n");
   }

   i2c_cmd_link_delete(cmd);
}


// Write a 32-bit register
void vl53l0x::write_reg_32bit(uint8_t reg, uint32_t data)
{
   uint8_t _data[4] = {0};
   _data[0] = (data >> 24) & 0xFF;
   _data[1] = (data >> 16) & 0xFF;
   _data[2] = (data >> 8) & 0xFF;
   _data[3] = data;

   i2c_cmd_handle_t cmd = i2c_cmd_link_create();

   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (_address << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
   i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
   i2c_master_write(cmd, _data, 4, ACK_CHECK_EN);
   i2c_master_stop(cmd);

   int8_t ret = i2c_master_cmd_begin(_i2c_port, cmd, 100 / portTICK_PERIOD_MS);
   if (ret != ESP_OK)
   {
      printf("\n\nERROR - write_reg failed\n\n");
   }

   i2c_cmd_link_delete(cmd);
}

uint8_t vl53l0x::read_reg(uint8_t reg)
{
   uint8_t data = 0;

   i2c_cmd_handle_t cmd = i2c_cmd_link_create();

   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (_address << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
   i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);

   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (_address << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
   i2c_master_read_byte(cmd, &data, NACK_VAL);
   i2c_master_stop(cmd);

   esp_err_t ret = i2c_master_cmd_begin(_i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
   if (ret != ESP_OK)
   {
      printf("\n\nERROR - read_reg failed\n\n");
   }

   i2c_cmd_link_delete(cmd);

   return data;
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

   esp_err_t ret = i2c_master_cmd_begin(_i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
   if (ret != ESP_OK)
   {
      printf("\n\nERROR - read_reg failed\n\n");
   }

   i2c_cmd_link_delete(cmd);
}


// Read a 16-bit register
uint16_t vl53l0x::read_reg_16bit(uint8_t reg)
{
   uint8_t data[2] = {0};
   uint16_t value;

   read_reg(reg, data, 2);

   value = (uint16_t)data[0] << 8 | data[1];

   return value;
}


// Read a 32-bit register
uint32_t vl53l0x::read_reg_32bit(uint8_t reg)
{
   uint8_t data[4] = {0};
   uint32_t value;

   read_reg(reg, data, 4);
   
   value = (uint32_t)data[0] << 24 | (uint32_t)data[1] << 16 | (uint32_t)data[2] << 8 | data[3];

   return value;
}
