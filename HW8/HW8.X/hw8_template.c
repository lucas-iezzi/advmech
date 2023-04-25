#include "nu32dip.h" // constants, functions for startup and UART
#include "i2c_master_noint.h"
#include "mpu6050.h"
#include "ssd1306.h"
#include "font.h"
#include <stdio.h>

void blink(int, int); // blink the LEDs function

int main(void) {
    NU32DIP_Startup(); // cache on, interrupts on, LED/button init, UART init
//    init_mpu6050();
    i2c_master_setup();
    ssd1306_setup(); 
	
	unsigned char data[100]; // char array for the raw data
	float xXL, yXL, zXL, xG, yG, zG, temp; // floats to store the data
    char str[100]; // string for printing
    
	ssd1306_clear();//clear screen
//	uint8_t whoAmI = whoami();// read whoami
//    NU32DIP_WriteUART1(sprintf(str,"%#x\r\n",whoAmI));// print whoami
//    while (whoAmI != 0x68){	// if whoami is not 0x68, stuck in loop with LEDs on
//        NU32DIP_GREEN = 1;
//        NU32DIP_YELLOW = 1;
//    }
//    NU32DIP_GREEN = 0;
//    NU32DIP_YELLOW = 0;
    
//	// wait to print until you get a newline
//    NU32DIP_ReadUART1(str,100);

    while (1) {
		// use core timer for exactly 100Hz loop
        _CP0_SET_COUNT(0);
        blink(1, 5);

//        // read IMU
//        burst_read_mpu6050(data);
//		
//        // convert data
//        xXL = conv_xXL(get_xXL(data)); // convert x-acceleration to float (g's)
//        yXL = conv_yXL(get_yXL(data)); // convert y-acceleration to float (g's)
//        zXL = conv_zXL(get_zXL(data)); // convert z-acceleration to float (g's)
//        temp = conv_temp(get_temp(data)); // convert temperature to float (Celsius)
//        xG = conv_xG(get_xG(data)); // convert x-gyro rate to float (dps)
//        yG = conv_yG(get_yG(data)); // convert y-gyro rate to float (dps)
//        zG = conv_zG(get_zG(data)); // convert z-gyro rate to float (dps)
//        
//        // print out the data
//        sprintf(str, "X Acceleration=%f\r\n", xXL);
//        NU32DIP_WriteUART1(str);
//        sprintf(str, "Y Acceleration=%f\r\n", yXL);
//        NU32DIP_WriteUART1(str);
//        sprintf(str, "Z Acceleration=%f\r\n", zXL);
//        NU32DIP_WriteUART1(str);
//        sprintf(str, "X Gyroscope=%f\r\n", xG);
//        NU32DIP_WriteUART1(str);
//        sprintf(str, "Y Gyroscope=%f\r\n", yG);
//        NU32DIP_WriteUART1(str);
//        sprintf(str, "X Gyroscope=%f\r\n", zG);
//        NU32DIP_WriteUART1(str);
//        sprintf(str, "Temperature=%f\r\n", temp);
//        NU32DIP_WriteUART1(str);
        
        //print data to screen
        char m[100];
        sprintf(m,"hello");
        drawString(m,0,10);
        drawChar(m[0],0,0);
        ssd1306_update();
        
        while (_CP0_GET_COUNT() < 48000000 / 2 / 100) {//wait for loop timer to finish
        }
    }
}

//print char to screen in location
void drawChar(char c, unsigned char x, unsigned char y){
    for(int j=0; j<5; j++){
        char col = ASCII[c-0x20][j];
        for(int i=0; i<8; i++){
            ssd1306_drawPixel(x+j,y+i,(col>>i)&0b1);
        }
    }
}

void drawString(char * m, unsigned char x, unsigned char y){
    int k=0;
    while(m[k]!='\0'){
        drawChar(m[k],x+5*k,y);
        k++;
    }
}

// blink the LEDs
void blink(int iterations, int time_ms) {
    int i;
    unsigned int t;
    for (i = 0; i < iterations; i++) {
        NU32DIP_GREEN = 0; // on
        NU32DIP_YELLOW = 1; // off
        t = _CP0_GET_COUNT(); // should really check for overflow here
        // the core timer ticks at half the SYSCLK, so 24000000 times per second
        // so each millisecond is 24000 ticks
        // wait half in each delay
        while (_CP0_GET_COUNT() < t + 12000 * time_ms) {
        }

        NU32DIP_GREEN = 1; // off
        NU32DIP_YELLOW = 0; // on
        t = _CP0_GET_COUNT(); // should really check for overflow here
        while (_CP0_GET_COUNT() < t + 12000 * time_ms) {
        }
    }
}