

idf_component_register(SRCS "src/main.cpp" "src/IMU.cpp" "src/motors.cpp" "src/control.cpp" "src/UdpClient.cpp"
                      INCLUDE_DIRS "include/"
                      PRIV_REQUIRES SPIbus mpu9250 motors_driver console nvs_flash)
