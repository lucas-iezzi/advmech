#include "nu32dip.h" // constants, functions for startup and UART
#include <stdio.h>

void blink(int, int); // blink the LEDs function

int main(void) {
  char blink_msg[100];
  char dur_msg[100];
  int blinks;
  int duration;
  
  NU32DIP_Startup(); // cache on, interrupts on, LED/button init, UART init
  while (1) {
    NU32DIP_WriteUART1("Type in number of blinks: ");
    NU32DIP_ReadUART1(blink_msg, 100);
    NU32DIP_WriteUART1(blink_msg);
    NU32DIP_WriteUART1("\r\n");
    NU32DIP_WriteUART1("Type in time between blink (ms): ");
    NU32DIP_ReadUART1(dur_msg, 100); // wait here until get message from computer
    NU32DIP_WriteUART1(dur_msg);
    NU32DIP_WriteUART1("\r\n"); // carriage return and newline
    blinks = atoi(blink_msg);
    duration = atoi(dur_msg);
	if (NU32DIP_USER){
		blink(blinks, duration); // 5 times, 500ms each time
	}
  }
}

// blink the LEDs
void blink(int iterations, int time_ms){
	int i;
	unsigned int t;
	for (i=0; i< iterations; i++){
		NU32DIP_GREEN = 0; // on
		NU32DIP_YELLOW = 1; // off
		t = _CP0_GET_COUNT(); // should really check for overflow here
		// the core timer ticks at half the SYSCLK, so 24000000 times per second
		// so each millisecond is 24000 ticks
		// wait half in each delay
		while(_CP0_GET_COUNT() < t + 12000*time_ms){}
		
		NU32DIP_GREEN = 1; // off
		NU32DIP_YELLOW = 0; // on
		t = _CP0_GET_COUNT(); // should really check for overflow here
		while(_CP0_GET_COUNT() < t + 12000*time_ms){}
	}
}
		