
#include "IMU.h"

uint32_t theta_1e6 = 0;

void vMPU( void * pvParameters )
{
    (void)pvParameters;

    vTaskDelay(3000 / portTICK_PERIOD_MS);

    spi_device_handle_t mpu_spi_handle;
    // Initialize SPI on HSPI host through SPIbus interface:
    hspi.begin(MOSI, MISO, SCLK);
    hspi.addDevice(0, CLOCK_SPEED, CS, &mpu_spi_handle);

    MPU_t MPU;  // create a default MPU object
    MPU.setBus(hspi);  // set bus port, not really needed here since default is HSPI
    MPU.setAddr(mpu_spi_handle);  // set spi_device_handle, always needed!

    while (esp_err_t err = MPU.testConnection()) {
        printf("Failed to connect to the MPU, error=%#X\n", err);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("MPU connection successful");
    ESP_ERROR_CHECK(MPU.initialize());  // initialize the chip and set initial configurations

    // Reading sensor data
    mpud::raw_axes_t accelRaw;   // x, y, z axes as int16
    mpud::float_axes_t accelG;   // accel axes in (g) gravity format

    while (true) {
        // Read
        MPU.acceleration(&accelRaw);  // fetch raw data from the registers
        accelG = mpud::accelGravity(accelRaw, mpud::ACCEL_FS_2G);
        printf ("%f\n", atan2(accelG.y, accelG.z - 0.04f));

        theta_1e6 = static_cast<uint32_t>(atan2(accelG.y, accelG.z - 0.04f) * 1000000);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}