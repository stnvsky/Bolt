## PCB layout errata

### Integrated flash pins

GPIO pins from 6 to 11 are connected to the integrated SPI flash memory and are not safe to use. PCB versions 1.x suffer from the lack of prior knowledge of that, what caused the ESP32 to not be able to boot after the motors with hall sensors have been connected to the board. To avoid building second version of the PCB (what would cost lots of time and money, especially with the current lack of the electronic parts on the market), I've decided to resign from distance sensors to gain additional pins to be used for the hall sensors. Frankly, there weren't any possibility to use distance sensors in the first place, since ESP32 doesn't have enough pins to support three not-really-necessary sensors without GPIO's expander. 

Pins IMU_CLK and IMU_CS are connected consequentially to the pins SHD (hold) and SWP (write protect). As those pins are not directly responsible for carrying information in the flash SPI bus and are connected to the IMU input pins (so they are sending information to IMU, not receiving), the impact can be possibly not disastrous. I leaved those pins as they are, since it would be very difficult to workaround the direct connection with the IMU chip.

Other pins used for the communication with the integrated flash are prepared to be connected with the hall sensors. Since the hall sensors are sending information and the same pins are connected to the MISO, MOSI, CLK ans CS pins in integrated flash, the hall sensors interfere with the flash memory and can cause the ESP32 to brick and cannot boot no more. That's why the hall sensors will be connected to alternative pins:

| Signal   | Original pin | Alternative pin |
| -------- | ------------ | --------------- |
| HALL_1_A | 21 (GPIO 7)  | 4 (GPIO 36)     |
| HALL_2_A | 22 (GPIO 8)  | 7 (GPIO 35)     |
| HALL_1_B | 20 (GPIO 6)  | 10 (GPIO 25)    |
| HALL_2_B | 19 (GPIO 11) | 11 (GPIO 26)    |

![esp32](https://github.com/stnvsky/Bolt/blob/main/pcb/v1.1/.esp32.png)



#### IMPORTANT

Flash mode needs to be set to DIO since QIO uses all four pins to send data to the integrated flash. Using QIO will result in getting resets due to watchdog timeouts.

