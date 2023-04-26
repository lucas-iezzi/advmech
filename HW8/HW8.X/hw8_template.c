#include "nu32dip.h" // constants, functions for startup and UART
#include "i2c_master_noint.h"
#include "mpu6050.h"
#include "ssd1306.h"
#include "font.h"
#include <stdio.h>

void blink(int, int); // blink the LEDs function

int main(void) {
    NU32DIP_Startup(); // cache on, interrupts on, LED/button init, UART init
    init_mpu6050();
    ssd1306_setup(); 
	
	unsigned char data[100]; // char array for the raw data
	float zXL; // floats to store the data
    char str[100]; // string for printing
    unsigned int lastBeat = _CP0_GET_COUNT();
    unsigned int loopStart = _CP0_GET_COUNT();
    unsigned int LEDon  = 0;
    float delay;
    
	ssd1306_clear();//clear screen
    
	uint8_t whoAmI = whoami();// read whoami
    sprintf(str,"%#x\r\n",whoAmI);
    NU32DIP_WriteUART1(str);// print whoami
    while (whoAmI != 0x68){	// if whoami is not 0x68, stuck in loop with LEDs on
        NU32DIP_GREEN = 1;
        NU32DIP_YELLOW = 1;
    }
    NU32DIP_GREEN = 0;
    NU32DIP_YELLOW = 0;
    
    _CP0_SET_COUNT(0);

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
        
        zXL = conv_zXL(data); // convert z-acceleration to float (g's)
        
        //print data to screen
        sprintf(str, "Z Accel = %f", zXL);
        drawString(str,0,0);
        ssd1306_update();
        
        delay = (_CP0_GET_COUNT() - loopStart);
        sprintf(str, "%f FPS", (48000000/2)/delay);
        drawString(str,0,20);
        ssd1306_update();
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