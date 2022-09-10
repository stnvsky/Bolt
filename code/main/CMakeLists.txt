

idf_component_register(SRCS "src/main.cpp" "src/IMU.cpp" "src/motors.cpp" "src/control.cpp" "src/UdpClient.cpp" "src/UdpServer.cpp" "src/wifi.cpp"
                      INCLUDE_DIRS "include/"
                      PRIV_REQUIRES SPIbus mpu9250 motors_driver console nvs_flash esp_wifi)
