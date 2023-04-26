#include "nu32dip.h" // constants, functions for startup and UART
#include "i2c_master_noint.h"
#include "mpu6050.h"
#include <stdio.h>

void blink(int, int); // blink the LEDs function

int main(void) {
    NU32DIP_Startup(); // cache on, interrupts on, LED/button init, UART init
    init_mpu6050();
	
	unsigned char data[100]; // char array for the raw data
	float xXL, yXL, zXL, xG, yG, zG, temp; // floats to store the data
    char str[100]; // string for printing
    unsigned int lastBeat = _CP0_GET_COUNT();
    unsigned int loopStart = _CP0_GET_COUNT();
    unsigned int LEDon  = 0;
	
	uint8_t whoAmI = whoami();// read whoami
    sprintf(str,"%#x \r\n",whoAmI);
    NU32DIP_WriteUART1(str);// print whoami
    while (whoAmI != 0x68){	// if whoami is not 0x68, stuck in loop with LEDs on
        NU32DIP_GREEN = 1;
        NU32DIP_YELLOW = 1;
    }
    NU32DIP_GREEN = 0;
    NU32DIP_YELLOW = 0;
    
	// wait to print until you get a newline
    NU32DIP_ReadUART1(str,100);

    while (1) {
        loopStart = _CP0_GET_COUNT();
		// use core timer for exactly 100Hz loop
        if ((_CP0_GET_COUNT() - lastBeat) > ((48000000 / 2)*0.25)){
            lastBeat = _CP0_GET_COUNT();
            if (LEDon==0){
                NU32DIP_YELLOW = 1;
                NU32DIP_GREEN = 0;
                LEDon = 1;
            }
            else{
                NU32DIP_YELLOW = 0;
                NU32DIP_GREEN = 1;
                LEDon = 0;
            }
        }

        // read IMU
        burst_read_mpu6050(data);
        
        // convert data
        xXL = conv_xXL(data); // convert x-acceleration to float (g's)
        yXL = conv_yXL(data); // convert y-acceleration to float (g's)
        zXL = conv_zXL(data); // convert z-acceleration to float (g's)
        temp = conv_temp(data); // convert temperature to float (Celsius)
        xG = conv_xG(data); // convert x-gyro rate to float (dps)
        yG = conv_yG(data); // convert y-gyro rate to float (dps)
        zG = conv_zG(data); // convert z-gyro rate to float (dps)
        
        // print out the data
        sprintf(str, "X Acceleration=%f\r\n", xXL);
        NU32DIP_WriteUART1(str);
        sprintf(str, "Y Acceleration=%f\r\n", yXL);
        NU32DIP_WriteUART1(str);
        sprintf(str, "Z Acceleration=%f\r\n", zXL);
        NU32DIP_WriteUART1(str);
        sprintf(str, "X Gyroscope=%f\r\n", xG);
        NU32DIP_WriteUART1(str);
        sprintf(str, "Y Gyroscope=%f\r\n", yG);
        NU32DIP_WriteUART1(str);
        sprintf(str, "X Gyroscope=%f\r\n", zG);
        NU32DIP_WriteUART1(str);
        sprintf(str, "Temperature=%f\r\n", temp);
        NU32DIP_WriteUART1(str);
        
        while ((_CP0_GET_COUNT() - loopStart) < 48000000 / 2 / 100) {
        }
    }
}