
#include "IMU.h"

static int32_t theta_1e6 = 0;

int32_t get_theta()
{
    return theta_1e6;
}

[[noreturn]] void vtaskMPU(void *)
{
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
    mpud::float_axes_t accelGravity;   // accel axes in (g) gravity format

    mpud::raw_axes_t gyroRaw;   // x, y, z axes as int16
    mpud::float_axes_t gyroDegPerSec;   // accel axes in (g) gravity format

    const auto h = 0.01f;
    const auto b = 0.01f;

    auto x1 = 0.0f;
    auto x2 = 0.0f;

    auto last_x1 = 0.0f;
    auto last_x2 = 0.0f;

    auto p11 = 10.0f;
    auto p12 = 0.0f;
    auto p21 = 0.0f;
    auto p22 = 10.0f;

    while (true) {
        // Read
        MPU.acceleration(&accelRaw);  // fetch raw data from the registers
        MPU.rotation(&gyroRaw);
        accelGravity = mpud::accelGravity(accelRaw, mpud::ACCEL_FS_2G);
        gyroDegPerSec = mpud::gyroDegPerSec(gyroRaw, mpud::GYRO_FS_2000DPS);

        accelGravity.z -= 0.04f;
        auto th = atan2(accelGravity.y, sqrt(accelGravity.x*accelGravity.x + accelGravity.z*accelGravity.z));

        auto detK = 1.0f/(b*b + b*p11 + b*p22 + p11*p22 - p12*p21);
        auto k11 = ((p11 - h*p22)*(b + p22) - p21*(p12 - h*p22))*detK;
        auto k12 = (-p12*(p11 - h*p21) + (b + p11)*(p12 - h*p22))*detK;
        auto k21 = (p21*(b + p22) - p21*p22)*detK;
        auto k22 = (-p12*p21 + p22*(b + p11))*detK;

        auto y1 = th;
        auto y2 = gyroDegPerSec.x/1000.0;
        x1 = last_x1 - h*last_x2 + k11*(y1-last_x1) + k12*(y2-last_x2);
        x2 = last_x2 + k21*(y1-last_x1) + k22*(y2-last_x2);

        auto next_p11 = p11 - h*p21 - h*(p12-h*p22) + h*h + (h*h*h*h)/4 - (k11*p11 + k12*p21 - h*(k11*p12 + k12*p22));
        auto next_p12 = p12 - h*p22 - (h*h*h)/2 - (k11*p12 + k12*p22);
        auto next_p21 = p21 - h*p22 - (h*h*h)/2 - (k21*p11 + k22*p21 - h*(k21*p12 + k22*p22));
        auto next_p22 = p22 + h*h   - (k21*p12 + k22*p22);

        p11 = next_p11;
        p12 = next_p12;
        p21 = next_p21;
        p22 = next_p22;

        last_x1 = x1;
        last_x2 = x2;
        theta_1e6 = static_cast<int32_t>(x1 * 1000000);

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}