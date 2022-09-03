
#include "IMU.h"

[[noreturn]] void vMPU(void *)
{
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    spi_device_handle_t mpu_spi_handle;
    // Initialize SPI on HSPI host through SPIbus interface:
    hspi.begin(MOSI, MISO, SCLK);
    hspi.addDevice(0, CLOCK_SPEED, CS, &mpu_spi_handle);

    MPUdmp_t MPU;  // create a default MPU object
    MPU.setBus(hspi);  // set bus port, not really needed here since default is HSPI
    MPU.setAddr(mpu_spi_handle);  // set spi_device_handle, always needed!

    while (esp_err_t err = MPU.testConnection()) {
        printf("Failed to connect to the MPU, error=%#X\n", err);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("MPU connection successful");

    MPU.reset();
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(MPU.loadDMPFirmware());
    ESP_ERROR_CHECK(MPU.enableDMP());
    ESP_ERROR_CHECK(MPU.setDMPFeatures(mpud::DMP_FEATURE_LP_6X_QUAT));
    ESP_ERROR_CHECK(MPU.setDMPInterruptMode(mpud::DMP_INT_MODE_CONTINUOUS));

    // Reading sensor data
    mpud::raw_axes_t accelRaw;   // x, y, z axes as int16
    mpud::raw_axes_t gyroRaw;   // x, y, z axes as int16
    mpud::quat_q30_t quaternion;

    while (true) {
        MPU.readDMPPacket(&quaternion, &gyroRaw, &accelRaw);

        printf ("w: %12d, x: %12d, y: %12d, z: %12d\n", quaternion.w, quaternion.x, quaternion.y, quaternion.z);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}