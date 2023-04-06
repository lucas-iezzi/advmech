#include "nu32dip.h" // constants, functions for startup and UART
#include <stdio.h>
#include <stdlib.h>  
#include <string.h>
#include <math.h>

void blink(int, int); // blink the LEDs function

int main() {
  char str[100];
  double pos; 
  double val;
  int i;
  unsigned int t;
  NU32DIP_Startup(); // cache on, interrupts on, LED/button init, UART init
  while (1) {
	if (!NU32DIP_USER){
        for (i=0; i<100; i++){
            pos = i * 6.28;
            val = sin(pos/100) + 1;
            sprintf(str,"%f\r\n",val);
            NU32DIP_WriteUART1(str);
            t = _CP0_GET_COUNT(); // should really check for overflow here
            while(_CP0_GET_COUNT() < t + 12000*10){}
        }
        t = _CP0_GET_COUNT(); // should really check for overflow here
        while(_CP0_GET_COUNT() < t + 12000*1000){}
	}
  }
}
		