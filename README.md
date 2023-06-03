# Advanced Mechatronics
Welcome to my repo for Advanced Mechatronics (ME 433)

See the [demo](https://youtube.com/shorts/6-6EqAZjXH0) of my final robot which uses a gyro to drive towards a specific heading.
This video shows it calibrating then adjusting the heading from -1 to 1 and back, making a figure 8.
The drive motors are controlled using a PIC32 with and IMU for feedback.
The Pico uses a camera to detect a line on the ground and eventually it will send a heading to the drive board over UART based on where the line is.

__Chips Used__
* PIC32MX170F256B (PIC32)
* AP7381 (3.3V Regualtor)
* MCP4912 (10-bit DAC over SPI)
* MCP23008 (8-bit IO Expander over I2C)
* MPU6050 (IMU over I2C)
* SSD1306 (I2C OLED Display)
* WS2812B (LED NeoPixels)
* ST7735 (LCD Screen I2C)
* OV7670 (I2C Camera)
* RP2040 (RPi Pico W)
